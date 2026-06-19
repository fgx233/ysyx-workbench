#!/usr/bin/env python3
"""
NVBoard pin binder 本地服务（仅标准库）。

用法:
    python3 pin_binder/server.py            # 在项目根目录运行
    python3 pin_binder/server.py --port 8642 --no-browser

启动后在浏览器中打开 http://127.0.0.1:<port>/ 即可图形化编辑引脚绑定，
保存时写回项目的 .nxdc 约束文件。
"""

import argparse
import json
import os
import re
import sys
import threading
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import core

HERE = os.path.dirname(os.path.abspath(__file__))
WEB_DIR = os.path.join(HERE, "web")

CONTENT_TYPES = {
    ".html": "text/html; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".js": "application/javascript; charset=utf-8",
    ".svg": "image/svg+xml",
    ".png": "image/png",
}


class Project:
    """定位并读取项目的各项输入；每次请求时重新加载，
    这样改了 top.v 后刷新页面即可看到变化。"""

    def __init__(self, root, board_path, top=None, nxdc=None, vsrc="vsrc"):
        self.root = os.path.abspath(root)
        self.board_path = board_path
        self.cli_top = top
        self.cli_nxdc = nxdc
        self.vsrc_rel = vsrc

    def _read_makefile_var(self, text, var, default):
        m = re.search(r'^\s*%s\s*[:?]?=\s*(\S+)' % re.escape(var), text, re.M)
        return m.group(1) if m else default

    def load(self):
        makefile = os.path.join(self.root, "Makefile")
        mtext = open(makefile).read() if os.path.exists(makefile) else ""
        top = self.cli_top or self._read_makefile_var(mtext, "TOPNAME", "top")
        nxdc_rel = self.cli_nxdc or self._read_makefile_var(
            mtext, "NXDC_FILES", "constr/top.nxdc")
        nxdc_path = os.path.join(self.root, nxdc_rel)

        pins = core.parse_board(open(self.board_path).read())

        # 在 vsrc 下找包含顶层模块的文件
        vfile, ports, warnings = None, [], []
        vsrc = os.path.join(self.root, self.vsrc_rel)
        candidates = []
        for dirpath, _, files in os.walk(vsrc):
            for f in sorted(files):
                if f.endswith((".v", ".sv")):
                    candidates.append(os.path.join(dirpath, f))
        for path in candidates:
            text = open(path).read()
            if re.search(r'\bmodule\s+%s\b' % re.escape(top), text):
                try:
                    ports, pw = core.parse_ports(text, top)
                    warnings.extend(pw)
                    vfile = path
                    break
                except ValueError:
                    continue
        if vfile is None:
            raise RuntimeError(
                f"在 {vsrc} 下找不到顶层模块 {top} 的 ANSI 端口声明")

        binds, bind_warnings = [], []
        if os.path.exists(nxdc_path):
            _, raw_binds = core.parse_nxdc(open(nxdc_path).read())
            # 逐条校验，丢弃和当前端口/板卡不匹配的旧绑定
            for b in raw_binds:
                errs = core.validate(binds + [b], ports, pins)
                if errs:
                    bind_warnings.append(
                        f"已忽略 nxdc 中的绑定 {b['signal']}: {errs[-1]}")
                else:
                    binds.append(b)

        return {
            "top": top,
            "vfile": os.path.relpath(vfile, self.root),
            "nxdc": os.path.relpath(nxdc_path, self.root),
            "nxdc_path": nxdc_path,
            "board": os.path.basename(self.board_path),
            "ports": ports,
            "pins": pins,
            "binds": binds,
            "suggestions": core.suggest(ports, pins),
            "warnings": warnings + bind_warnings,
        }


class Handler(BaseHTTPRequestHandler):
    project = None  # 由 main() 注入

    def log_message(self, fmt, *args):  # 安静一点
        pass

    def _send(self, code, body, ctype="application/json; charset=utf-8"):
        data = body if isinstance(body, bytes) else json.dumps(
            body, ensure_ascii=False).encode()
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(data)

    def _read_body(self):
        length = int(self.headers.get("Content-Length", 0))
        return json.loads(self.rfile.read(length) or b"{}")

    # ------------------------------------------------------------- GET

    def do_GET(self):
        path = self.path.split("?", 1)[0]
        if path == "/api/state":
            try:
                state = self.project.load()
                state.pop("nxdc_path")
                self._send(200, state)
            except Exception as e:
                self._send(500, {"error": str(e)})
            return
        # 静态文件
        if path == "/":
            path = "/index.html"
        fpath = os.path.normpath(os.path.join(WEB_DIR, path.lstrip("/")))
        if fpath.startswith(WEB_DIR) and os.path.isfile(fpath):
            ext = os.path.splitext(fpath)[1]
            self._send(200, open(fpath, "rb").read(),
                       CONTENT_TYPES.get(ext, "application/octet-stream"))
        else:
            self._send(404, {"error": "not found"})

    # ------------------------------------------------------------- POST

    def do_POST(self):
        try:
            if self.path == "/api/preview":
                self._handle_emit(write=False)
            elif self.path == "/api/save":
                self._handle_emit(write=True)
            else:
                self._send(404, {"error": "not found"})
        except Exception as e:
            self._send(500, {"ok": False, "errors": [str(e)]})

    def _handle_emit(self, write):
        body = self._read_body()
        binds = body.get("binds", [])
        state = self.project.load()
        errors = core.validate(binds, state["ports"], state["pins"])
        if errors:
            self._send(200, {"ok": False, "errors": errors})
            return
        text = core.emit_nxdc(state["top"], binds, state["ports"])
        if write:
            path = state["nxdc_path"]
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w") as f:
                f.write(text)
        self._send(200, {"ok": True, "text": text,
                         "path": state["nxdc"] if write else None})


def main():
    ap = argparse.ArgumentParser(description="NVBoard 图形化引脚绑定器")
    ap.add_argument("--project", default=os.path.dirname(HERE),
                    help="项目根目录（默认: pin_binder 的上级目录）")
    ap.add_argument("--board", default=None,
                    help="板卡描述文件（默认: $NVBOARD_HOME/board/N4）")
    ap.add_argument("--top", default=None, help="顶层模块名（默认读 Makefile）")
    ap.add_argument("--nxdc", default=None,
                    help="nxdc 文件相对路径（默认读 Makefile）")
    ap.add_argument("--vsrc", default="vsrc",
                    help="Verilog 源码目录，相对项目根或绝对路径（默认: vsrc）")
    ap.add_argument("--port", type=int, default=8642)
    ap.add_argument("--no-browser", action="store_true",
                    help="启动后不自动打开浏览器")
    args = ap.parse_args()

    board = args.board
    if board is None:
        nvb = os.environ.get("NVBOARD_HOME")
        if not nvb:
            sys.exit("错误: 未设置 $NVBOARD_HOME，请用 --board 指定板卡描述文件")
        board = os.path.join(nvb, "board", "N4")
    if not os.path.isfile(board):
        sys.exit(f"错误: 板卡描述文件不存在: {board}")

    Handler.project = Project(args.project, board, args.top, args.nxdc,
                              args.vsrc)
    Handler.project.load()  # 启动前先做一次完整加载，尽早暴露配置错误

    url = f"http://127.0.0.1:{args.port}/"
    server = ThreadingHTTPServer(("127.0.0.1", args.port), Handler)
    print(f"NVBoard 引脚绑定器已启动: {url}")
    print(f"  项目:   {Handler.project.root}")
    print(f"  板卡:   {board}")
    print("按 Ctrl+C 退出")
    if not args.no_browser:
        threading.Timer(0.3, lambda: webbrowser.open(url)).start()
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n已退出")


if __name__ == "__main__":
    main()
