#!/usr/bin/env python3
"""
NVBoard pin binder 核心逻辑（无任何第三方依赖）：
  - 解析板卡引脚描述文件（$NVBOARD_HOME/board/N4）
  - 解析 Verilog 顶层模块端口（ANSI 风格端口声明）
  - 解析 / 生成 .nxdc 约束文件
  - 引脚绑定的校验与按名称的自动建议

该模块不依赖 GUI，可单独测试。
"""

import re

# 这些端口由仿真主循环（main.cpp）驱动，不通过 nxdc 绑定
EXCLUDED_PORTS = ("clk", "clock", "rst", "rstn", "rst_n", "reset", "resetn")


# ---------------------------------------------------------------- 板卡描述

def parse_board(text):
    """解析板卡描述文件，返回 [{"name", "dir"}]，顺序与文件一致。

    dir 的语义与 DUT 端口一致：input 引脚（开关/按钮）驱动 DUT 的
    input 端口，output 引脚（LED 等）接收 DUT 的 output 端口。
    """
    pins = []
    seen = set()
    for lineno, raw in enumerate(text.splitlines(), 1):
        line = raw.split('#', 1)[0].strip()
        if not line:
            continue
        seg = line.split()
        if len(seg) != 2 or seg[0] not in ("input", "output"):
            raise ValueError(f"板卡描述第 {lineno} 行无法解析: {raw!r}")
        if seg[1] in seen:
            raise ValueError(f"板卡描述第 {lineno} 行重复定义引脚 {seg[1]}")
        seen.add(seg[1])
        pins.append({"name": seg[1], "dir": seg[0]})
    return pins


# ------------------------------------------------------------ Verilog 端口

def _strip_comments(text):
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    text = re.sub(r'//[^\n]*', ' ', text)
    return text


def _extract_port_header(text, top):
    """返回 module <top> 的端口列表括号内的原文（不含括号）。"""
    m = re.search(r'\bmodule\s+%s\b' % re.escape(top), text)
    if not m:
        return None
    i = m.end()
    n = len(text)
    while i < n and text[i].isspace():
        i += 1
    # 跳过参数表 #( ... )
    if i < n and text[i] == '#':
        i += 1
        while i < n and text[i].isspace():
            i += 1
        if i < n and text[i] == '(':
            depth = 0
            while i < n:
                if text[i] == '(':
                    depth += 1
                elif text[i] == ')':
                    depth -= 1
                    if depth == 0:
                        i += 1
                        break
                i += 1
        while i < n and text[i].isspace():
            i += 1
    if i >= n or text[i] != '(':
        return None
    depth = 0
    start = i + 1
    while i < n:
        if text[i] == '(':
            depth += 1
        elif text[i] == ')':
            depth -= 1
            if depth == 0:
                return text[start:i]
        i += 1
    return None


_PORT_ITEM = re.compile(
    r'^(?:(input|output|inout)\s+)?'
    r'(?:(?:wire|reg|logic)\s+)?'
    r'(?:signed\s+)?'
    r'(?:\[\s*([^\]]+?)\s*:\s*([^\]]+?)\s*\]\s*)?'
    r'([A-Za-z_]\w*)$'
)


def parse_ports(verilog_text, top):
    """解析顶层模块端口，返回 (ports, warnings)。

    ports: [{"name", "dir", "width", "msb", "lsb"}]，顺序与声明一致，
    已剔除 clk/rst 等由仿真环境驱动的端口。
    """
    text = _strip_comments(verilog_text)
    header = _extract_port_header(text, top)
    if header is None:
        raise ValueError(f"在 Verilog 源码中找不到模块 {top} 的 ANSI 端口声明")

    ports, warnings = [], []
    prev_dir, prev_msb, prev_lsb = None, 0, 0
    for item in header.split(','):
        item = ' '.join(item.split())
        if not item:
            continue
        m = _PORT_ITEM.match(item)
        if not m:
            warnings.append(f"无法解析端口声明 {item!r}，已跳过")
            continue
        direction, msb_s, lsb_s, name = m.groups()
        if direction is None:
            if prev_dir is None:
                warnings.append(f"端口 {name} 缺少方向，已跳过")
                continue
            direction = prev_dir
            if msb_s is None:
                msb, lsb = prev_msb, prev_lsb
            else:
                try:
                    msb, lsb = int(msb_s), int(lsb_s)
                except ValueError:
                    warnings.append(f"端口 {name} 的位宽 [{msb_s}:{lsb_s}] 不是常量，已跳过")
                    continue
        else:
            if msb_s is None:
                msb, lsb = 0, 0
            else:
                try:
                    msb, lsb = int(msb_s), int(lsb_s)
                except ValueError:
                    warnings.append(f"端口 {name} 的位宽 [{msb_s}:{lsb_s}] 不是常量，已跳过")
                    prev_dir = direction
                    continue
        prev_dir, prev_msb, prev_lsb = direction, msb, lsb
        if direction == "inout":
            warnings.append(f"端口 {name} 是 inout，NVBoard 不支持，已跳过")
            continue
        if name.lower() in EXCLUDED_PORTS:
            continue
        ports.append({
            "name": name, "dir": direction,
            "width": abs(msb - lsb) + 1, "msb": msb, "lsb": lsb,
        })
    return ports, warnings


# ----------------------------------------------------------------- .nxdc

def parse_nxdc(text):
    """解析 .nxdc 文件，返回 (top, binds)。binds: [{"signal", "pins"}]，
    pins 顺序为 MSB 在前（与 nxdc 语义一致）。"""
    top = None
    binds = []
    lines = text.splitlines()
    if not lines:
        return None, []
    m = re.match(r'\s*top\s*=\s*(\w+)\s*$', lines[0].split('#', 1)[0])
    if m:
        top = m.group(1)
    for raw in lines[1:]:
        line = raw.split('#', 1)[0].strip()
        if not line:
            continue
        if '(' in line:
            mv = re.match(r'(\w+)\s*\(([^)]*)\)\s*$', line)
            if mv:
                pins = [p.strip() for p in mv.group(2).split(',') if p.strip()]
                binds.append({"signal": mv.group(1), "pins": pins})
        else:
            seg = line.split()
            if len(seg) == 2:
                binds.append({"signal": seg[0], "pins": [seg[1]]})
    return top, binds


def emit_nxdc(top, binds, ports):
    """生成 .nxdc 文本。绑定按端口声明顺序排列；第一行必须是 top=<name>
    （NVBoard 的解析脚本只认第一行）。"""
    order = {p["name"]: i for i, p in enumerate(ports)}
    binds = sorted(binds, key=lambda b: order.get(b["signal"], 10 ** 6))
    out = [f"top={top}", "", "# 由 pin_binder 生成（引脚顺序为 MSB 在前）"]
    for b in binds:
        if len(b["pins"]) == 1:
            out.append(f'{b["signal"]} {b["pins"][0]}')
        else:
            out.append(f'{b["signal"]} ({", ".join(b["pins"])})')
    return "\n".join(out) + "\n"


# ----------------------------------------------------------------- 校验

def validate(binds, ports, pins):
    """校验绑定列表，返回错误信息列表（空列表表示通过）。"""
    errors = []
    port_by_name = {p["name"]: p for p in ports}
    pin_by_name = {p["name"]: p for p in pins}
    seen_signal = set()
    pin_user = {}
    for b in binds:
        sig, plist = b["signal"], b["pins"]
        port = port_by_name.get(sig)
        if port is None:
            errors.append(f"信号 {sig} 不是顶层模块的端口")
            continue
        if sig in seen_signal:
            errors.append(f"信号 {sig} 被绑定了多次")
            continue
        seen_signal.add(sig)
        if len(plist) != port["width"]:
            errors.append(
                f"信号 {sig} 位宽为 {port['width']}，但绑定了 {len(plist)} 个引脚")
        for pn in plist:
            pin = pin_by_name.get(pn)
            if pin is None:
                errors.append(f"引脚 {pn} 不存在于板卡描述中（信号 {sig}）")
                continue
            if pin["dir"] != port["dir"]:
                errors.append(
                    f"方向不匹配：{port['dir']} 端口 {sig} 不能绑定 "
                    f"{pin['dir']} 引脚 {pn}")
            if pn in pin_user:
                errors.append(f"引脚 {pn} 被 {pin_user[pn]} 和 {sig} 重复使用")
            else:
                pin_user[pn] = sig
    return errors


# ------------------------------------------------------------- 自动建议

def _norm(s):
    return s.replace('_', '').lower()


# 无法靠通用规则命中的少量别名（NVBoard 习惯命名）
_SCALAR_ALIAS = {"ps2data": "PS2_DAT"}
_FAMILY_ALIAS = {"led": "ld", "ledr": "ld"}


def suggest(ports, pins):
    """根据名称启发式生成绑定建议，返回与 binds 同构的列表。
    只生成方向合法、引脚互不冲突的建议。"""
    pin_by_name = {p["name"]: p for p in pins}
    norm_to_pin = {}
    for p in pins:
        norm_to_pin.setdefault(_norm(p["name"]), p["name"])

    # 按"基名+数字"归类的引脚家族，如 SW -> {0: "SW0", ...}
    families = {}
    for p in pins:
        m = re.fullmatch(r'(.*?)(\d+)', p["name"])
        if m:
            families.setdefault(_norm(m.group(1)), {})[int(m.group(2))] = p["name"]

    def dir_ok(port, pin_names):
        return all(
            pn in pin_by_name and pin_by_name[pn]["dir"] == port["dir"]
            for pn in pin_names)

    suggestions = []
    used = set()

    def take(port, pin_names):
        if dir_ok(port, pin_names) and not (set(pin_names) & used):
            suggestions.append({"signal": port["name"], "pins": list(pin_names)})
            used.update(pin_names)
            return True
        return False

    for port in ports:
        name, w = port["name"], port["width"]
        if w == 1:
            cand = norm_to_pin.get(_norm(name)) or _SCALAR_ALIAS.get(_norm(name))
            if cand:
                take(port, [cand])
            continue

        # 七段数码管：segN[7:0] -> SEGNA..SEGNG, DECNP
        m = re.fullmatch(r'seg(\d+)', name, re.I)
        if m and w == 8:
            n = m.group(1)
            cand = [f"SEG{n}{c}" for c in "ABCDEFG"] + [f"DEC{n}P"]
            if all(c in pin_by_name for c in cand) and take(port, cand):
                continue

        # 方向键：btn[4:0] -> 左上中下右（NVBoard 的惯例顺序）
        if _norm(name) == "btn" and w == 5:
            cand = ["BTNL", "BTNU", "BTNC", "BTND", "BTNR"]
            if all(c in pin_by_name for c in cand) and take(port, cand):
                continue

        # 通用规则：端口名匹配引脚家族，位号一一对应，MSB 在前
        base = _norm(name)
        fam = families.get(base) or families.get(_FAMILY_ALIAS.get(base, ""))
        if fam:
            lo, hi = min(port["msb"], port["lsb"]), max(port["msb"], port["lsb"])
            if all(i in fam for i in range(lo, hi + 1)):
                take(port, [fam[i] for i in range(hi, lo - 1, -1)])

    return suggestions
