"use strict";

/* ============================== 全局状态 ============================== */

const S = {
  meta: {},          // top / vfile / nxdc / board
  ports: [],         // [{name, dir, width, msb, lsb}]
  pins: [],          // [{name, dir}]
  binds: [],         // [{signal, pins}]  pins 为 MSB 在前
  suggestions: [],   // 同 binds 结构（来自后端启发式）
  dismissed: new Set(),
  pending: null,     // {signal, assigned: []}
  selected: null,    // 当前选中的信号名
  history: [], future: [],
  savedJson: "[]",
  mouse: null,
};

const $ = (sel) => document.querySelector(sel);
const PALETTE = ["#4ea1ff", "#3fd68c", "#ffc857", "#ff7ab2", "#b07aff",
                 "#5ee0e0", "#ff9c5e", "#9acd32", "#e06666", "#6699ff",
                 "#66cdaa", "#d4a5ff"];

let portByName = {}, pinByName = {};
let groups = [];          // [{id, title, canonical:[pin...]}]
let groupOfPin = {};      // pin -> group
let familyPins = {};      // 小写基名 -> {idx: pinName}

function colorOf(sig) {
  let h = 0;
  for (const c of sig) h = (h * 31 + c.charCodeAt(0)) >>> 0;
  return PALETTE[h % PALETTE.length];
}
function esc(s) {
  return s.replace(/[&<>"]/g, c => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;"}[c]));
}
function widthStr(p) { return p.width > 1 ? `[${p.msb}:${p.lsb}]` : ""; }

/* ============================== 派生查询 ============================== */

function bindOf(sig) { return S.binds.find(b => b.signal === sig) || null; }

function occupancy() {           // pin -> {signal, bit}
  const occ = {};
  for (const b of S.binds) {
    const port = portByName[b.signal];
    const hi = port ? Math.max(port.msb, port.lsb) : b.pins.length - 1;
    b.pins.forEach((pin, i) => { occ[pin] = { signal: b.signal, bit: hi - i }; });
  }
  return occ;
}

function visibleSuggestions() {
  const occ = occupancy();
  return S.suggestions.filter(s =>
    !S.dismissed.has(s.signal) &&
    !bindOf(s.signal) &&
    s.pins.every(p => !occ[p]));
}

function isLegal(pinName, port) {
  const pin = pinByName[pinName];
  return !!pin && pin.dir === port.dir;
}

/* ============================== 历史记录 ============================== */

function snapshot() {
  return JSON.stringify({ binds: S.binds, dismissed: [...S.dismissed] });
}
function pushHistory() {
  S.history.push(snapshot());
  if (S.history.length > 200) S.history.shift();
  S.future = [];
}
function restore(snap) {
  const d = JSON.parse(snap);
  S.binds = d.binds;
  S.dismissed = new Set(d.dismissed);
  S.pending = null;
  if (S.selected && !bindOf(S.selected) && !portByName[S.selected]) S.selected = null;
}
function undo() {
  if (!S.history.length) return;
  S.future.push(snapshot());
  restore(S.history.pop());
  renderAll();
}
function redo() {
  if (!S.future.length) return;
  S.history.push(snapshot());
  restore(S.future.pop());
  renderAll();
}

/* ============================== 板卡构建 ============================== */

function makePad(name, cls, label, consumed) {
  if (!pinByName[name]) return null;
  consumed.add(name);
  const d = document.createElement("div");
  d.className = "pad " + cls;
  d.dataset.pin = name;
  d.textContent = label != null ? label : name;
  return d;
}

function makeGroup(id, title, canonical) {
  const g = document.createElement("div");
  g.className = "group";
  const head = document.createElement("div");
  head.className = "group-head";
  head.textContent = title;
  head.dataset.group = id;
  g.appendChild(head);
  const info = { id, title, canonical, el: g };
  groups.push(info);
  for (const p of canonical) groupOfPin[p] = info;
  return g;
}

function buildBoard() {
  const board = $("#board");
  board.innerHTML = "";
  groups = []; groupOfPin = {}; familyPins = {};
  const consumed = new Set();

  for (const p of S.pins) {
    const m = /^(.*?)(\d+)$/.exec(p.name);
    if (m) (familyPins[m[1].toLowerCase()] ||= {})[+m[2]] = p.name;
  }
  const have = (n) => !!pinByName[n];
  const row = () => {
    const r = document.createElement("div");
    r.className = "board-row";
    board.appendChild(r);
    return r;
  };
  const padsDiv = (cls) => {
    const d = document.createElement("div");
    d.className = "pads" + (cls ? " " + cls : "");
    return d;
  };

  /* -- 第一行：VGA 屏幕 + 方向键 + PS2 / UART / RGB LED -- */
  const r1 = row();

  const vgaSingles = ["VGA_HSYNC", "VGA_VSYNC", "VGA_BLANK_N"].filter(have);
  const vgaFams = ["VGA_R", "VGA_G", "VGA_B"].filter(b => have(b + "0"));
  if (vgaSingles.length || vgaFams.length) {
    const all = [...vgaSingles];
    for (const f of vgaFams)
      for (let i = 7; i >= 0; i--) if (have(f + i)) all.push(f + i);
    const g = makeGroup("VGA", "VGA", all);
    const scr = document.createElement("div");
    scr.className = "vga-screen";
    scr.textContent = "VGA 640×480";
    g.appendChild(scr);
    const syncRow = document.createElement("div");
    syncRow.className = "pads-labelled";
    const lbl = { VGA_HSYNC: "HS", VGA_VSYNC: "VS", VGA_BLANK_N: "BLK" };
    const sp = padsDiv("");
    for (const n of vgaSingles) sp.appendChild(makePad(n, "", lbl[n], consumed));
    syncRow.appendChild(sp);
    g.appendChild(syncRow);
    for (const f of vgaFams) {
      const lr = document.createElement("div");
      lr.className = "pads-labelled";
      const rl = document.createElement("span");
      rl.className = "rowlabel";
      rl.textContent = f.slice(-1);
      rl.dataset.family = f;
      lr.appendChild(rl);
      const pd = padsDiv("");
      for (let i = 7; i >= 0; i--)
        if (have(f + i)) pd.appendChild(makePad(f + i, "", String(i), consumed));
      lr.appendChild(pd);
      g.appendChild(lr);
    }
    r1.appendChild(g);
  }

  if (have("BTNC")) {
    const order = ["BTNL", "BTNU", "BTNC", "BTND", "BTNR"].filter(have);
    const g = makeGroup("BTN", "BTN 方向键", order);
    const cross = document.createElement("div");
    cross.className = "btn-cross";
    const pos = { BTNU: "u", BTND: "d", BTNL: "l", BTNR: "r", BTNC: "c" };
    for (const n of order) {
      const p = makePad(n, "btnp " + pos[n], n.slice(3), consumed);
      cross.appendChild(p);
    }
    g.appendChild(cross);
    r1.appendChild(g);
  }

  const col1 = document.createElement("div");
  col1.style.cssText = "display:flex;flex-direction:column;gap:10px";
  if (have("PS2_CLK") || have("PS2_DAT")) {
    const order = ["PS2_CLK", "PS2_DAT"].filter(have);
    const g = makeGroup("PS2", "PS/2 键盘", order);
    const pd = padsDiv("");
    for (const n of order)
      pd.appendChild(makePad(n, "", n.replace("PS2_", ""), consumed));
    g.appendChild(pd);
    col1.appendChild(g);
  }
  if (have("UART_TX") || have("UART_RX")) {
    const order = ["UART_TX", "UART_RX"].filter(have);
    const g = makeGroup("UART", "UART 串口", order);
    const pd = padsDiv("");
    for (const n of order)
      pd.appendChild(makePad(n, "", n.replace("UART_", ""), consumed));
    g.appendChild(pd);
    col1.appendChild(g);
  }
  const rgb = ["R16", "G16", "B16", "R17", "G17", "B17"].filter(have);
  if (rgb.length) {
    const g = makeGroup("RGB", "RGB LED", rgb);
    const pd = padsDiv("");
    for (const n of rgb) pd.appendChild(makePad(n, "led", n, consumed));
    g.appendChild(pd);
    col1.appendChild(g);
  }
  if (col1.children.length) r1.appendChild(col1);

  /* -- 第二行：七段数码管 SEG7..SEG0 -- */
  const segR = row();
  for (let n = 7; n >= 0; n--) {
    if (!have(`SEG${n}A`)) continue;
    const canonical = [..."ABCDEFG"].map(c => `SEG${n}${c}`).concat([`DEC${n}P`])
      .filter(have);
    const g = makeGroup(`SEG${n}`, `SEG${n}`, canonical);
    const digit = document.createElement("div");
    digit.className = "seg-digit";
    digit.textContent = "8.";
    g.appendChild(digit);
    const pd = padsDiv("grid4");
    for (const pin of canonical)
      pd.appendChild(makePad(pin, "", pin.startsWith("DEC") ? "P" : pin.slice(-1), consumed));
    g.appendChild(pd);
    segR.appendChild(g);
  }
  if (!segR.children.length) segR.remove();

  /* -- 第三行：LED -- */
  if (familyPins["ld"]) {
    const fam = familyPins["ld"];
    const idxs = Object.keys(fam).map(Number).sort((a, b) => b - a);
    const canonical = idxs.map(i => fam[i]);
    const g = makeGroup("LED", "LED", canonical);
    const pd = padsDiv("");
    for (const i of idxs) pd.appendChild(makePad(fam[i], "led", String(i), consumed));
    g.appendChild(pd);
    row().appendChild(g);
  }

  /* -- 第四行：拨码开关 -- */
  if (familyPins["sw"]) {
    const fam = familyPins["sw"];
    const idxs = Object.keys(fam).map(Number).sort((a, b) => b - a);
    const canonical = idxs.map(i => fam[i]);
    const g = makeGroup("SW", "拨码开关", canonical);
    const pd = padsDiv("");
    for (const i of idxs) pd.appendChild(makePad(fam[i], "sw", String(i), consumed));
    g.appendChild(pd);
    row().appendChild(g);
  }

  /* -- 兜底：未归类的引脚 -- */
  const rest = S.pins.map(p => p.name).filter(n => !consumed.has(n));
  if (rest.length) {
    const g = makeGroup("MISC", "其他", rest);
    const pd = padsDiv("");
    pd.style.flexWrap = "wrap";
    for (const n of rest) pd.appendChild(makePad(n, "", n, consumed));
    g.appendChild(pd);
    row().appendChild(g);
  }
}

/* ============================== 渲染 ============================== */

function renderPorts() {
  const list = $("#port-list");
  list.innerHTML = "";
  for (const p of S.ports) {
    const b = bindOf(p.name);
    const div = document.createElement("div");
    div.className = "port-row" + (b ? " bound" : "") +
      (S.pending && S.pending.signal === p.name ? " pending" : "") +
      (S.selected === p.name ? " selected" : "");
    div.dataset.port = p.name;
    const color = b ? colorOf(p.name) : "transparent";
    div.innerHTML =
      `<span class="dot" style="background:${b ? color : "transparent"}"></span>` +
      `<span class="pname">${esc(p.name)}</span>` +
      `<span class="pwidth">${widthStr(p)}</span>` +
      `<span class="pdir">${p.dir === "input" ? "in →" : "out ←"}</span>`;
    div.title = b ? `已绑定: ${b.pins.join(", ")}` : "点击开始连线";
    list.appendChild(div);
  }
}

function renderPads() {
  const occ = occupancy();
  const pendingPort = S.pending ? portByName[S.pending.signal] : null;
  document.querySelectorAll(".pad").forEach(padEl => {
    const name = padEl.dataset.pin;
    const o = occ[name];
    padEl.classList.remove("occupied", "legal", "dimmed");
    padEl.style.background = "";
    padEl.style.borderColor = "";
    if (o) {
      const c = colorOf(o.signal);
      padEl.classList.add("occupied");
      padEl.style.background = c;
      padEl.style.borderColor = c;
      const port = portByName[o.signal];
      padEl.title = `${name} ← ${o.signal}` + (port && port.width > 1 ? `[${o.bit}]` : "");
    } else {
      padEl.title = name;
    }
    if (pendingPort) {
      if (S.pending.assigned.includes(name)) {
        const c = colorOf(S.pending.signal);
        padEl.classList.add("occupied");
        padEl.style.background = c;
        padEl.style.borderColor = c;
        padEl.title = `${name} ← ${S.pending.signal}（待提交）`;
      } else if (!o && isLegal(name, pendingPort)) {
        padEl.classList.add("legal");
      } else {
        padEl.classList.add("dimmed");
      }
    }
  });
  document.querySelectorAll(".group-head").forEach(h => {
    const g = groups.find(x => x.id === h.dataset.group);
    const ok = pendingPort && g &&
      g.canonical.some(p => !occ[p] && isLegal(p, pendingPort));
    h.classList.toggle("legal-group", !!ok);
  });
}

function renderSuggestions() {
  const list = $("#sug-list");
  const sugs = visibleSuggestions();
  $("#sug-count").textContent = sugs.length;
  $("#btn-accept-all").disabled = !sugs.length;
  list.innerHTML = sugs.length ? "" :
    `<div class="muted pad8">没有待处理的建议</div>`;
  for (const s of sugs) {
    const div = document.createElement("div");
    div.className = "sug-item";
    const pinsTxt = s.pins.length > 2
      ? `${s.pins[0]} … ${s.pins[s.pins.length - 1]}`
      : s.pins.join(", ");
    div.innerHTML =
      `<span class="sig">${esc(s.signal)}</span><span class="arrow">→</span>` +
      `<span class="pins" title="${esc(s.pins.join(", "))}">${esc(pinsTxt)}</span>` +
      `<button data-acc="${esc(s.signal)}" title="接受">✓</button>` +
      `<button data-dis="${esc(s.signal)}" title="忽略">✕</button>`;
    list.appendChild(div);
  }
}

function renderInspector() {
  const box = $("#inspector");
  if (S.pending) {
    const p = portByName[S.pending.signal];
    const next = Math.max(p.msb, p.lsb) - S.pending.assigned.length;
    box.innerHTML =
      `<div class="insp-title" style="color:${colorOf(p.name)}">${esc(p.name)} ${widthStr(p)}</div>` +
      `<div class="muted pad8">正在连线：已指定 ${S.pending.assigned.length}/${p.width} 位` +
      (p.width > 1 ? `，下一位 ${esc(p.name)}[${next}]` : "") + `</div>` +
      `<div class="insp-actions"><button data-act="cancel">取消 (Esc)</button></div>`;
    return;
  }
  const sig = S.selected;
  if (!sig) {
    box.innerHTML = `<div class="muted pad8">点击端口、连线或引脚查看详情</div>`;
    return;
  }
  const port = portByName[sig];
  const b = bindOf(sig);
  if (!port) { box.innerHTML = ""; return; }
  let html = `<div class="insp-title" style="color:${b ? colorOf(sig) : "var(--fg)"}">` +
    `${esc(sig)} ${widthStr(port)} <span class="muted">${port.dir}</span></div>`;
  if (b) {
    html += `<div class="insp-actions">` +
      (port.width > 1 ? `<button data-act="reverse">反转位序</button>` : "") +
      `<button data-act="rewire">重新连线</button>` +
      `<button data-act="unbind">删除绑定</button></div>`;
    const hi = Math.max(port.msb, port.lsb);
    html += `<table class="bit-table">` + b.pins.map((pin, i) =>
      `<tr><td>${esc(sig)}${port.width > 1 ? `[${hi - i}]` : ""}</td>` +
      `<td>←</td><td>${esc(pin)}</td></tr>`).join("") + `</table>`;
  } else {
    html += `<div class="muted pad8">尚未绑定</div>` +
      `<div class="insp-actions"><button data-act="wire">开始连线</button></div>`;
  }
  box.innerHTML = html;
}

function renderStatus() {
  const st = $("#status");
  if (S.pending) {
    const p = portByName[S.pending.signal];
    st.className = "pending";
    st.innerHTML = `正在为 <b>${esc(p.name)}${widthStr(p)}</b>（${p.dir}）连线 — ` +
      (p.width > 1
        ? `左键点击发光引脚自动连续填充；<b>右键</b>点击逐位指定；点击组标题按位号对齐；`
        : `点击一个发光的引脚完成连接；`) +
      `<kbd>Esc</kbd> 取消`;
    return;
  }
  st.className = "";
  const total = S.ports.length;
  const bound = S.ports.filter(p => bindOf(p.name)).length;
  const sugs = visibleSuggestions().length;
  st.innerHTML = `已绑定 <b>${bound}</b> / ${total} 个端口` +
    (sugs ? ` · <b>${sugs}</b> 条建议待处理` : "") +
    ` · 点击左侧端口开始连线，点击连线或彩色引脚查看绑定`;
}

/* ------------------------------ 连线绘制 ------------------------------ */

function clamp(v, lo, hi) { return Math.min(hi, Math.max(lo, v)); }

function redrawWires() {
  const svg = $("#wires");
  const mainR = $("#main").getBoundingClientRect();
  const ppR = $("#ports-pane").getBoundingClientRect();
  const bpR = $("#board-pane").getBoundingClientRect();
  const rel = (x, y) => [x - mainR.left, y - mainR.top];

  function portAnchor(sig) {
    const rowEl = document.querySelector(`.port-row[data-port="${CSS.escape(sig)}"]`);
    if (!rowEl) return null;
    const r = rowEl.getBoundingClientRect();
    return rel(r.right - 2, clamp((r.top + r.bottom) / 2, ppR.top + 8, ppR.bottom - 8));
  }
  function padsAnchor(pins) {
    let sx = 0, sy = 0, n = 0;
    for (const p of pins) {
      const padEl = document.querySelector(`.pad[data-pin="${CSS.escape(p)}"]`);
      if (!padEl) continue;
      const r = padEl.getBoundingClientRect();
      sx += (r.left + r.right) / 2; sy += (r.top + r.bottom) / 2; n++;
    }
    if (!n) return null;
    return rel(clamp(sx / n, bpR.left + 8, bpR.right - 8),
               clamp(sy / n, bpR.top + 8, bpR.bottom - 8));
  }
  function path(a, b) {
    const dx = Math.max(50, (b[0] - a[0]) * 0.4);
    return `M${a[0]},${a[1]} C${a[0] + dx},${a[1]} ${b[0] - dx},${b[1]} ${b[0]},${b[1]}`;
  }

  let html = "";
  for (const s of visibleSuggestions()) {
    const a = portAnchor(s.signal), b = padsAnchor(s.pins);
    if (!a || !b) continue;
    html += `<path d="${path(a, b)}" fill="none" stroke="#5a6478" stroke-width="1.3"
       stroke-dasharray="5 4" opacity="0.55"/>`;
  }
  for (const b of S.binds) {
    const a = portAnchor(b.signal), t = padsAnchor(b.pins);
    if (!a || !t) continue;
    const c = colorOf(b.signal);
    const w = b.pins.length > 1 ? 3.5 : 2;
    const sel = S.selected === b.signal;
    html += `<path class="wire" data-sig="${esc(b.signal)}" d="${path(a, t)}" fill="none"
       stroke="${c}" stroke-width="${sel ? w + 1.5 : w}" opacity="${sel ? 1 : 0.85}"/>` +
      `<circle cx="${a[0]}" cy="${a[1]}" r="3.2" fill="${c}"/>` +
      `<circle cx="${t[0]}" cy="${t[1]}" r="3.2" fill="${c}"/>`;
  }
  if (S.pending && S.mouse) {
    const a = portAnchor(S.pending.signal);
    if (a) {
      const m = rel(S.mouse.x, S.mouse.y);
      html += `<path d="${path(a, m)}" fill="none" stroke="var(--accent)"
         stroke-width="2" stroke-dasharray="6 5" opacity="0.9"/>`;
    }
  }
  svg.innerHTML = html;
}

let wireRAF = null;
function scheduleWires() {
  if (wireRAF) return;
  wireRAF = requestAnimationFrame(() => { wireRAF = null; redrawWires(); });
}

/* ------------------------------ 预览 ------------------------------ */

let previewTimer = null;
function schedulePreview() {
  clearTimeout(previewTimer);
  previewTimer = setTimeout(refreshPreview, 250);
}
async function refreshPreview() {
  const pre = $("#preview");
  try {
    const r = await fetch("/api/preview", {
      method: "POST", headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ binds: S.binds }),
    });
    const d = await r.json();
    pre.className = d.ok ? "" : "err";
    pre.textContent = d.ok ? d.text : "校验失败:\n" + d.errors.join("\n");
  } catch (e) {
    pre.className = "err";
    pre.textContent = "无法连接后端: " + e;
  }
  updateDirty();
}

function updateDirty() {
  $("#dirty-dot").hidden = JSON.stringify(S.binds) === S.savedJson;
}

function renderAll() {
  renderPorts();
  renderPads();
  renderSuggestions();
  renderInspector();
  renderStatus();
  $("#btn-undo").disabled = !S.history.length;
  $("#btn-redo").disabled = !S.future.length;
  $("#btn-clear").disabled = !S.binds.length;
  scheduleWires();
  schedulePreview();
}

/* ============================== 交互逻辑 ============================== */

function toast(msg, cls = "") {
  const t = $("#toast");
  t.textContent = msg;
  t.className = cls;
  t.hidden = false;
  clearTimeout(t._timer);
  t._timer = setTimeout(() => { t.hidden = true; }, 2600);
}

function startPending(sig) {
  const port = portByName[sig];
  if (!port) return;
  if (bindOf(sig)) { S.selected = sig; renderAll(); return; }
  S.pending = { signal: sig, assigned: [] };
  S.selected = sig;
  renderAll();
}
function cancelPending() {
  S.pending = null;
  renderAll();
}

function commitBind(sig, pins) {
  pushHistory();
  S.binds.push({ signal: sig, pins });
  S.pending = null;
  S.selected = sig;
  renderAll();
}

/* 从某引脚开始的"连续填充"：数字家族按位号递减，否则按组内规范顺序 */
function runFrom(pinName, count, port) {
  const occ = occupancy();
  const free = (n) => pinByName[n] && !occ[n] &&
    !S.pending.assigned.includes(n) && isLegal(n, port);
  const m = /^(.*?)(\d+)$/.exec(pinName);
  if (m) {
    const base = m[1], start = +m[2];
    const run = [];
    for (let i = 0; i < count; i++) {
      const n = base + (start - i);
      if (!free(n)) break;
      run.push(n);
    }
    if (run.length === count) return run;
  }
  const g = groupOfPin[pinName];
  if (g) {
    const idx = g.canonical.indexOf(pinName);
    const run = g.canonical.slice(idx, idx + count);
    if (run.length === count && run.every(free)) return run;
  }
  return null;
}

function assignPin(pinName, single) {
  const port = portByName[S.pending.signal];
  const occ = occupancy();
  if (occ[pinName] || S.pending.assigned.includes(pinName)) {
    toast(`引脚 ${pinName} 已被占用`, "err");
    return;
  }
  if (!isLegal(pinName, port)) {
    const pin = pinByName[pinName];
    toast(`方向不匹配：${port.dir} 端口不能接 ${pin ? pin.dir : "?"} 引脚 ${pinName}`, "err");
    return;
  }
  const remaining = port.width - S.pending.assigned.length;
  if (!single && remaining > 1) {
    const run = runFrom(pinName, remaining, port);
    if (run) { commitBind(S.pending.signal, S.pending.assigned.concat(run)); return; }
    toast("无法从这里连续填充，已改为逐位指定", "");
  }
  S.pending.assigned.push(pinName);
  if (S.pending.assigned.length === port.width) {
    commitBind(S.pending.signal, S.pending.assigned);
  } else {
    renderAll();
  }
}

/* 组标题点击：按位号对齐填充（sw[7:0] -> SW7..SW0），否则取组内规范顺序 */
function groupFill(groupId) {
  if (!S.pending) return;
  const port = portByName[S.pending.signal];
  const g = groups.find(x => x.id === groupId);
  if (!g) return;
  const occ = occupancy();
  const free = (n) => !occ[n] && !S.pending.assigned.includes(n) && isLegal(n, port);
  const remaining = port.width - S.pending.assigned.length;

  // 数字家族对齐：组内引脚若是 BASE<idx> 形式，让 idx 与位号一致
  const fam = {};
  let famOk = g.canonical.length > 0;
  for (const n of g.canonical) {
    const m = /^(.*?)(\d+)$/.exec(n);
    if (!m) { famOk = false; break; }
    fam[+m[2]] = n;
  }
  if (famOk && !S.pending.assigned.length) {
    const hi = Math.max(port.msb, port.lsb), lo = Math.min(port.msb, port.lsb);
    const aligned = [];
    for (let i = hi; i >= lo; i--) {
      if (!fam[i] || !free(fam[i])) { aligned.length = 0; break; }
      aligned.push(fam[i]);
    }
    if (aligned.length === port.width) {
      commitBind(port.name, aligned);
      return;
    }
  }
  const avail = g.canonical.filter(free).slice(0, remaining);
  if (avail.length === remaining) {
    commitBind(port.name, S.pending.assigned.concat(avail));
  } else {
    toast(`${g.title} 组内空闲且方向匹配的引脚不足 ${remaining} 个`, "err");
  }
}

/* VGA R/G/B 行标签点击：该家族按位号对齐整行填充 */
function familyAlignedFill(base) {
  const port = portByName[S.pending.signal];
  const fam = familyPins[base.toLowerCase()];
  if (!fam) return;
  if (S.pending.assigned.length) {
    toast("已有逐位指定的引脚，无法整体填充", "err");
    return;
  }
  const occ = occupancy();
  const hi = Math.max(port.msb, port.lsb), lo = Math.min(port.msb, port.lsb);
  const pins = [];
  for (let i = hi; i >= lo; i--) {
    const n = fam[i];
    if (!n || occ[n] || !isLegal(n, port)) {
      toast(`${base} 中没有足够的空闲匹配引脚来对齐填充`, "err");
      return;
    }
    pins.push(n);
  }
  commitBind(port.name, pins);
}

function unbind(sig) {
  pushHistory();
  S.binds = S.binds.filter(b => b.signal !== sig);
  renderAll();
}

function acceptSuggestion(sig) {
  const s = visibleSuggestions().find(x => x.signal === sig);
  if (!s) return;
  pushHistory();
  S.binds.push({ signal: s.signal, pins: [...s.pins] });
  renderAll();
}

async function save(quit = false) {
  try {
    const r = await fetch("/api/save", {
      method: "POST", headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ binds: S.binds, quit }),
    });
    const d = await r.json();
    if (d.ok) {
      S.savedJson = JSON.stringify(S.binds);
      updateDirty();
      if (d.shutdown) onShutdown(d.path);
      else toast(`已保存到 ${d.path}`, "ok");
    } else {
      toast("保存失败：" + d.errors[0], "err");
    }
  } catch (e) {
    toast("保存失败：" + e, "err");
  }
}

// 服务器已被"保存并退出"关闭：禁用全部操作，提示用户页面已失效
function onShutdown(path) {
  for (const id of ["btn-undo", "btn-redo", "btn-clear", "btn-save",
                    "btn-save-quit", "btn-accept-all"]) {
    const el = $("#" + id);
    if (el) el.disabled = true;
  }
  $("#status").textContent = `已保存到 ${path}，服务器已关闭，可关闭此页面`;
  toast("已保存，服务器已关闭，可关闭此页面", "ok");
}

/* ============================== 事件绑定 ============================== */

function bindEvents() {
  $("#port-list").addEventListener("click", (e) => {
    const rowEl = e.target.closest(".port-row");
    if (!rowEl) return;
    const sig = rowEl.dataset.port;
    if (S.pending) {
      if (S.pending.signal === sig) { cancelPending(); return; }
      S.pending = null;
    }
    startPending(sig);
  });

  $("#board").addEventListener("click", (e) => {
    const head = e.target.closest(".group-head");
    if (head) {
      if (S.pending) groupFill(head.dataset.group);
      return;
    }
    const rl = e.target.closest(".rowlabel");
    if (rl && rl.dataset.family) {
      if (S.pending) familyAlignedFill(rl.dataset.family);
      return;
    }
    const padEl = e.target.closest(".pad");
    if (!padEl) return;
    const pinName = padEl.dataset.pin;
    if (S.pending) {
      assignPin(pinName, e.shiftKey);  // 左键连续填充（Shift 仍可逐位）
      return;
    }
    const o = occupancy()[pinName];
    if (o) { S.selected = o.signal; renderAll(); }
    else $("#status").innerHTML =
      `引脚 <b>${esc(pinName)}</b>（${pinByName[pinName].dir}）空闲 — 先在左侧点击要连接的端口`;
  });

  // 右键 = 逐位指定（连线时屏蔽浏览器右键菜单）
  $("#board").addEventListener("contextmenu", (e) => {
    if (!S.pending) return;
    const padEl = e.target.closest(".pad");
    if (!padEl) return;
    e.preventDefault();
    assignPin(padEl.dataset.pin, true);
  });

  $("#wires").addEventListener("click", (e) => {
    const sig = e.target.dataset && e.target.dataset.sig;
    if (sig) { S.selected = sig; renderAll(); }
  });

  $("#sug-list").addEventListener("click", (e) => {
    const acc = e.target.dataset.acc, dis = e.target.dataset.dis;
    if (acc) acceptSuggestion(acc);
    else if (dis) { pushHistory(); S.dismissed.add(dis); renderAll(); }
  });

  $("#inspector").addEventListener("click", (e) => {
    const act = e.target.dataset.act;
    if (!act) return;
    if (act === "cancel") cancelPending();
    else if (act === "wire") startPending(S.selected);
    else if (act === "unbind") unbind(S.selected);
    else if (act === "rewire") {
      const sig = S.selected;
      unbind(sig);
      startPending(sig);
    } else if (act === "reverse") {
      const b = bindOf(S.selected);
      if (b) { pushHistory(); b.pins.reverse(); renderAll(); }
    }
  });

  $("#btn-accept-all").addEventListener("click", () => {
    const sugs = visibleSuggestions();
    if (!sugs.length) return;
    pushHistory();
    for (const s of sugs) S.binds.push({ signal: s.signal, pins: [...s.pins] });
    toast(`已接受 ${sugs.length} 条建议`, "ok");
    renderAll();
  });
  $("#btn-undo").addEventListener("click", undo);
  $("#btn-redo").addEventListener("click", redo);
  $("#btn-save").addEventListener("click", () => save(false));
  $("#btn-save-quit").addEventListener("click", () => save(true));
  $("#btn-clear").addEventListener("click", () => {
    if (!S.binds.length) return;
    pushHistory();
    S.binds = [];
    S.pending = null;
    renderAll();
  });

  document.addEventListener("keydown", (e) => {
    if (e.key === "Escape") {
      if (S.pending) cancelPending();
      else if (S.selected) { S.selected = null; renderAll(); }
    } else if ((e.ctrlKey || e.metaKey) && !e.shiftKey && e.key.toLowerCase() === "z") {
      e.preventDefault(); undo();
    } else if ((e.ctrlKey || e.metaKey) &&
               (e.key.toLowerCase() === "y" || (e.shiftKey && e.key.toLowerCase() === "z"))) {
      e.preventDefault(); redo();
    } else if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === "s") {
      e.preventDefault(); save();
    }
  });

  $("#main").addEventListener("mousemove", (e) => {
    if (!S.pending) return;
    S.mouse = { x: e.clientX, y: e.clientY };
    scheduleWires();
  });

  window.addEventListener("resize", scheduleWires);
  $("#board-pane").addEventListener("scroll", scheduleWires);
  $("#port-list").addEventListener("scroll", scheduleWires);

  window.addEventListener("beforeunload", (e) => {
    if (JSON.stringify(S.binds) !== S.savedJson) {
      e.preventDefault();
      e.returnValue = "";
    }
  });
}

/* ============================== 初始化 ============================== */

async function init() {
  let state;
  try {
    const r = await fetch("/api/state");
    state = await r.json();
    if (state.error) throw new Error(state.error);
  } catch (e) {
    $("#status").textContent = "加载失败：" + e.message;
    return;
  }
  S.meta = { top: state.top, vfile: state.vfile, nxdc: state.nxdc, board: state.board };
  S.ports = state.ports;
  S.pins = state.pins;
  S.binds = state.binds;
  S.suggestions = state.suggestions;
  S.savedJson = JSON.stringify(S.binds);

  portByName = Object.fromEntries(S.ports.map(p => [p.name, p]));
  pinByName = Object.fromEntries(S.pins.map(p => [p.name, p]));

  $("#proj-info").textContent =
    `top=${state.top} (${state.vfile}) · ${state.nxdc} · 板卡 ${state.board}`;
  document.title = `${state.nxdc} — NVBoard 引脚绑定器`;

  buildBoard();
  bindEvents();
  renderAll();
  for (const w of state.warnings || []) toast(w, "err");
}

init();
