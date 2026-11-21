// resine_page.h — the resin mixing tool's presets: their editor (/resine), the
// factory list, and the in-memory shape the tool works from.
//
// A header rather than raw string literals inside the .ino for the reason
// screen_page.h gives: the Arduino prototype generator scans raw strings in the
// .ino as code. ResinPreset lives here too: the generator emits prototypes for
// the .ino's functions at the top of the file, so a type they take must exist
// before any .ino line does.
//
// The page talks to GET/POST /api/resine and POST /api/resine/reset (§19).
// Same editor pattern and look as the heat press memo's (/presse): edit the
// list, export and import it as a file, put the factory list back, and a demo
// mode when opened without a scale behind it.
#pragma once

#include <stdint.h>

// One preset as the tool uses it. a : b is the ratio by weight exactly as the
// owner typed it ("2 : 1", "100 : 45"); pot is the working time in minutes,
// 0 for none.
struct ResinPreset {
    char     name[48];
    uint32_t color;
    float    a, b;
    uint16_t pot;
};
#define RESIN_MAX_PRESETS 40

// Written by resineLoad() when no list exists yet, in English or French by the
// language set at that moment. Values are indicative and the scale says so.
static const char RESINE_DEFAULTS_EN[] = R"JSON({"settings":{"defaults":true},"presets":[
{"name":"Epoxy 1:1","color":"#2FB36B","a":1,"b":1,"pot":30,"note":"Coating, gluing"},
{"name":"Epoxy casting 2:1","color":"#3B8FE0","a":2,"b":1,"pot":40,"note":"Layers up to 2 cm"},
{"name":"Epoxy 3:1","color":"#A35BD1","a":3,"b":1,"pot":30,"note":""},
{"name":"River table","color":"#1FA896","a":100,"b":45,"pot":120,"note":"Deep pour, slow cure"},
{"name":"Polyurethane 1:1","color":"#E07A2E","a":1,"b":1,"pot":5,"note":"Fast casting resin, work quickly"},
{"name":"Tin silicone","color":"#D9487A","a":100,"b":5,"pot":30,"note":"Moulds, 5 % catalyst"},
{"name":"Platinum silicone 1:1","color":"#F2B705","a":1,"b":1,"pot":20,"note":"Moulds, weigh precisely"},
{"name":"Polyester + catalyst","color":"#E24B4A","a":100,"b":2,"pot":15,"note":"1 to 3 % MEKP, ventilate"}
]})JSON";

static const char RESINE_DEFAULTS_FR[] = R"JSON({"settings":{"defaults":true},"presets":[
{"name":"Époxy 1:1","color":"#2FB36B","a":1,"b":1,"pot":30,"note":"Vernis, collage"},
{"name":"Époxy coulée 2:1","color":"#3B8FE0","a":2,"b":1,"pot":40,"note":"Couches jusqu'à 2 cm"},
{"name":"Époxy 3:1","color":"#A35BD1","a":3,"b":1,"pot":30,"note":""},
{"name":"Table rivière","color":"#1FA896","a":100,"b":45,"pot":120,"note":"Coulée épaisse, prise lente"},
{"name":"Polyuréthane 1:1","color":"#E07A2E","a":1,"b":1,"pot":5,"note":"Résine de coulée rapide, travailler vite"},
{"name":"Silicone étain","color":"#D9487A","a":100,"b":5,"pot":30,"note":"Moules, catalyseur 5 %"},
{"name":"Silicone platine 1:1","color":"#F2B705","a":1,"b":1,"pot":20,"note":"Moules, peser précisément"},
{"name":"Polyester + catalyseur","color":"#E24B4A","a":100,"b":2,"pot":15,"note":"1 à 3 % de PMEC, ventiler"}
]})JSON";


static const char RESINE_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Resin presets</title>
<style>
  :root { --bg:#0B0E14; --bg2:#121722; --card:#161B26; --card2:#1C2230; --line:#2A3142; --txt:#F3F5F9; --mute:#8A93A6;
          --acc:#1ABC9C; --acc2:#3FD3B0; --cyan:#4FD1EE; --ok:#3BA55D; --ko:#E24B4A; --r:14px; }
  * { box-sizing:border-box; }
  body { margin:0; font:15px/1.45 system-ui, -apple-system, "Segoe UI", Roboto, sans-serif; color:var(--txt);
         background:radial-gradient(1200px 500px at 10% -10%, #08352c 0%, transparent 60%), var(--bg); min-height:100vh; }
  header { position:sticky; top:0; z-index:5; display:flex; flex-wrap:wrap; gap:8px; align-items:center; padding:12px 18px;
           background:rgba(11,14,20,.86); backdrop-filter:blur(10px); border-bottom:1px solid var(--line); }
  header h1 { font-size:19px; margin:0 auto 0 0; display:flex; align-items:center; gap:10px; letter-spacing:.01em; }
  header h1::before { content:""; width:28px; height:28px; border-radius:8px; background:linear-gradient(135deg, var(--acc2), #12806b); }
  main { max-width:1000px; margin:0 auto; padding:18px; display:grid; gap:18px; }
  button, .btn { font:inherit; border:1px solid var(--line); background:var(--card2); color:var(--txt); border-radius:10px;
                 padding:7px 13px; cursor:pointer; transition:border-color .15s, background .15s; }
  button:hover, .btn:hover { border-color:var(--mute); }
  button:focus-visible, input:focus-visible { outline:2px solid var(--acc); outline-offset:1px; }
  .primary { background:linear-gradient(90deg, #2fb36b, #27a05f); border-color:transparent; font-weight:600; }
  .danger { color:var(--ko); }
  .icon { padding:5px 9px; background:transparent; }
  input { font:inherit; color:var(--txt); background:var(--bg2); border:1px solid var(--line); border-radius:8px; padding:6px 8px; width:100%; }
  input[type=color] { padding:2px; width:40px; height:36px; border-radius:10px; }
  input[type=number] { text-align:right; font-variant-numeric:tabular-nums; }
  .panel { background:var(--card); border:1px solid var(--line); border-radius:var(--r); padding:16px; }
  .panel h2 { font-size:16px; margin:0 0 4px; }
  table { width:100%; border-collapse:separate; border-spacing:0 6px; }
  th { text-align:left; font-weight:600; color:var(--mute); font-size:11px; text-transform:uppercase; letter-spacing:.05em; padding:0 4px; }
  td { padding:6px 4px; vertical-align:middle; background:var(--card2); }
  td:first-child { border-radius:10px 0 0 10px; padding-left:8px; border-left:4px solid var(--c, #888); }
  td:last-child { border-radius:0 10px 10px 0; padding-right:8px; }
  .w-col { width:52px; }
  .w-ratio { width:190px; }
  .ratio { display:flex; gap:6px; align-items:center; }
  .ratio input { color:var(--cyan); font-weight:600; }
  .per { font-size:12px; color:var(--mute); margin-top:3px; }
  .w-pot { width:90px; }
  .w-pot input { color:var(--acc2); font-weight:600; }
  .w-act { width:112px; white-space:nowrap; text-align:right; }
  .banner { padding:10px 14px; border-radius:10px; background:#3a2d12; color:#f5d38a; }
  .toast { position:fixed; bottom:18px; left:50%; transform:translateX(-50%); background:#1B212C; border:1px solid var(--line);
           padding:10px 16px; border-radius:10px; opacity:0; transition:opacity .3s; pointer-events:none; }
  .toast.show { opacity:1; }
  .muted { color:var(--mute); font-size:13px; margin:0; }
  @media (max-width:760px) {
    thead { display:none; }
    tr { display:grid; grid-template-columns:1fr 1fr; gap:6px; padding:10px; margin-bottom:8px; background:var(--card2);
         border-radius:10px; border-left:4px solid var(--c, #888); }
    td, td:first-child, td:last-child { padding:0; background:none; border-radius:0; border:0; }
    td.name, td.note, td.w-ratio { grid-column:1 / -1; }
    td::before { content:attr(data-l); display:block; color:var(--mute); font-size:11px; }
    .w-col, .w-ratio, .w-pot, .w-act { width:auto; }
    td.w-act { grid-column:1 / -1; text-align:left; }
  }
  @media (prefers-reduced-motion:reduce) { * { transition:none !important; } }
</style>
</head>
<body>
<header>
  <h1 data-t="title"></h1>
  <button id="add" data-t="add"></button>
  <button id="export" data-t="export"></button>
  <label class="btn"><span data-t="import"></span><input id="import" type="file" accept=".json" hidden></label>
  <button id="save" class="primary" data-t="save"></button>
</header>
<main>
  <div id="demo" class="banner" hidden data-t="demo"></div>
  <div class="banner" data-t="warn"></div>
  <section class="panel">
    <h2 data-t="listTitle"></h2>
    <p class="muted" data-t="listText"></p>
    <table>
      <thead><tr id="heads"></tr></thead>
      <tbody id="rows"></tbody>
    </table>
  </section>
  <div class="panel">
    <h2 data-t="factoryTitle"></h2>
    <p class="muted" data-t="factoryText"></p>
    <button id="reset" class="danger" data-t="factoryBtn" style="margin-top:10px"></button>
  </div>
</main>
<div id="toast" class="toast"></div>

<script>
// Two languages, picked from the browser, as on the heat press memo's page.
const STR = {
  en: {title:"Resin presets", add:"+ Preset", export:"Export", import:"Import", save:"Save to the scale",
       demo:"Demo mode: the scale cannot be reached, changes will not be saved.",
       warn:"Factory presets are common values. Always check the ratio on your resin's data sheet: it must be the ratio by weight, not by volume.",
       listTitle:"Presets", listText:"In the order the scale shows them. The working time starts the timer once the dose of B is reached (0 = no timer).",
       factoryTitle:"Factory presets", factoryText:"Puts back the original list. Export first to keep yours.",
       factoryBtn:"Restore factory presets", colour:"Colour", name:"Name", ratio:"Ratio A : B (by weight)", pot:"Working time, min",
       note:"Note", notePh:"Note (optional)", namePh:"Resin name", up:"Up", down:"Down", del:"Delete",
       per:(b) => `100 g of A takes ${b} g of B`, perBad:"Enter two numbers above 0",
       newName:"New preset", saved:"Saved to the scale", demoSend:"Demo mode: nothing is sent", error:"Error: ",
       imported:"Imported, remember to save", badFile:"Invalid file", badRatio:"A preset has an empty ratio",
       resetQ:"Replace the whole list with the factory presets?", resetDone:"Factory presets restored"},
  fr: {title:"Pré-réglages résine", add:"+ Pré-réglage", export:"Exporter", import:"Importer", save:"Enregistrer sur la balance",
       demo:"Mode démo : balance non joignable, les modifications ne seront pas enregistrées.",
       warn:"Les pré-réglages d'usine sont des valeurs courantes. Vérifiez toujours le rapport sur la fiche de votre résine : il doit être donné en poids, pas en volume.",
       listTitle:"Pré-réglages", listText:"Dans l'ordre où la balance les affiche. Le temps de travail lance le minuteur une fois la dose de B atteinte (0 = pas de minuteur).",
       factoryTitle:"Pré-réglages d'usine", factoryText:"Remet la liste d'origine. Exportez d'abord pour garder la vôtre.",
       factoryBtn:"Restaurer les pré-réglages d'usine", colour:"Couleur", name:"Nom", ratio:"Rapport A : B (en poids)", pot:"Temps de travail, min",
       note:"Note", notePh:"Note (optionnel)", namePh:"Nom de la résine", up:"Monter", down:"Descendre", del:"Supprimer",
       per:(b) => `100 g de A demandent ${b} g de B`, perBad:"Indiquez deux nombres supérieurs à 0",
       newName:"Nouveau pré-réglage", saved:"Enregistré sur la balance", demoSend:"Mode démo : rien n'est envoyé", error:"Erreur : ",
       imported:"Importé, pensez à enregistrer", badFile:"Fichier invalide", badRatio:"Un pré-réglage a un rapport vide",
       resetQ:"Remplacer toute la liste par les pré-réglages d'usine ?", resetDone:"Pré-réglages d'usine restaurés"}
};
const L = STR[(navigator.language || "en").slice(0, 2).toLowerCase()] || STR.en;
document.documentElement.lang = L === STR.fr ? "fr" : "en";
document.title = L.title;
document.querySelectorAll("[data-t]").forEach(e => { e.textContent = L[e.dataset.t]; });

const DEMO = {settings:{defaults:false}, presets:[
  {name:"Epoxy 1:1", color:"#2FB36B", a:1, b:1, pot:30, note:"Coating, gluing"},
  {name:"Epoxy casting 2:1", color:"#3B8FE0", a:2, b:1, pot:40, note:"Layers up to 2 cm"},
  {name:"Tin silicone", color:"#D9487A", a:100, b:5, pot:30, note:"Moulds, 5 % catalyst"}
]};

let data = {settings:{}, presets:[]};
let demo = false;
let dirty = false;

const $ = s => document.querySelector(s);
function toast(msg) {
  const t = $("#toast");
  t.textContent = msg;
  t.classList.add("show");
  clearTimeout(t._h);
  t._h = setTimeout(() => t.classList.remove("show"), 2200);
}
function el(tag, props = {}, ...kids) {
  const e = document.createElement(tag);
  for (const [k, v] of Object.entries(props)) {
    if (k.startsWith("on")) e.addEventListener(k.slice(2), v);
    else if (k === "dataset") Object.assign(e.dataset, v);
    else e[k] = v;
  }
  e.append(...kids);
  return e;
}
function move(i, d) {
  const j = i + d;
  if (j < 0 || j >= data.presets.length) return;
  [data.presets[i], data.presets[j]] = [data.presets[j], data.presets[i]];
  changed(true);
}
function changed(rerender) {
  dirty = true;
  if (rerender) render();
}
function perText(p) {
  if (!(p.a > 0) || !(p.b > 0)) return L.perBad;
  return L.per(Math.round(p.b * 1000 / p.a) / 10);
}

function render() {
  $("#heads").replaceChildren(...[L.colour, L.name, L.ratio, L.pot, L.note, ""].map(t => el("th", {textContent:t})));
  const rows = $("#rows");
  rows.innerHTML = "";
  data.presets.forEach((p, i) => {
    const tr = el("tr");
    tr.style.setProperty("--c", p.color);
    const per = el("div", {className:"per", textContent:perText(p)});
    const num = (key, step) => el("input", {type:"number", min:0, step, value:p[key] ?? "",
      oninput:e => { p[key] = parseFloat(e.target.value) || 0; per.textContent = perText(p); changed(); }});
    tr.append(
      el("td", {className:"w-col", dataset:{l:L.colour}}, el("input", {type:"color", value:p.color || "#607D8B", title:L.colour,
        oninput:e => { p.color = e.target.value; tr.style.setProperty("--c", p.color); changed(); }})),
      el("td", {className:"name", dataset:{l:L.name}}, el("input", {value:p.name || "", placeholder:L.namePh, maxLength:40,
        oninput:e => { p.name = e.target.value; changed(); }})),
      el("td", {className:"w-ratio", dataset:{l:L.ratio}}, el("div", {className:"ratio"}, num("a", "any"), ":", num("b", "any")), per),
      el("td", {className:"w-pot", dataset:{l:L.pot}}, el("input", {type:"number", min:0, max:999, value:p.pot ?? 0,
        oninput:e => { p.pot = Math.max(0, Math.min(999, parseInt(e.target.value) || 0)); changed(); }})),
      el("td", {className:"note", dataset:{l:L.note}}, el("input", {value:p.note || "", placeholder:L.notePh,
        oninput:e => { p.note = e.target.value; changed(); }})),
      el("td", {className:"w-act"},
        el("button", {className:"icon", title:L.up, textContent:"↑", onclick:() => move(i, -1)}),
        el("button", {className:"icon", title:L.down, textContent:"↓", onclick:() => move(i, 1)}),
        el("button", {className:"icon danger", title:L.del, textContent:"✕", onclick:() => { data.presets.splice(i, 1); changed(true); }}))
    );
    rows.append(tr);
  });
}

async function load() {
  try {
    const r = await fetch("/api/resine", {cache:"no-store"});
    if (!r.ok) throw 0;
    data = await r.json();
  } catch {
    demo = true;
    data = structuredClone(DEMO);
    $("#demo").hidden = false;
  }
  data.settings ??= {};
  data.presets ??= [];
  render();
}

async function post(url, body) {
  if (demo) { toast(L.demoSend); return false; }
  const r = await fetch(url, {method:"POST", headers:{"Content-Type":"application/json"}, body});
  if (!r.ok) { toast(L.error + await r.text()); return false; }
  return true;
}

$("#add").onclick = () => {
  data.presets.push({name:L.newName, color:"#607D8B", a:100, b:50, pot:30, note:""});
  changed(true);
  window.scrollTo({top:document.body.scrollHeight, behavior:"smooth"});
};
$("#save").onclick = async () => {
  if (data.presets.some(p => !(p.a > 0) || !(p.b > 0))) { toast(L.badRatio); return; }
  data.settings = Object.assign({}, data.settings, {defaults:false});
  if (await post("/api/resine", JSON.stringify(data))) { dirty = false; toast(L.saved); }
};
$("#export").onclick = () => {
  const a = el("a", {href:URL.createObjectURL(new Blob([JSON.stringify(data, null, 2)], {type:"application/json"})), download:"resin-presets.json"});
  a.click();
  URL.revokeObjectURL(a.href);
};
$("#import").onchange = async e => {
  const f = e.target.files[0];
  if (!f) return;
  try {
    const d = JSON.parse(await f.text());
    if (!Array.isArray(d.presets)) throw 0;
    data = d;
    data.settings ??= {};
    changed(true);
    toast(L.imported);
  } catch { toast(L.badFile); }
  e.target.value = "";
};
$("#reset").onclick = async () => {
  if (!confirm(L.resetQ)) return;
  if (await post("/api/resine/reset", "")) { dirty = false; await load(); toast(L.resetDone); }
};
window.addEventListener("beforeunload", e => { if (dirty) { e.preventDefault(); e.returnValue = ""; } });

load();
</script>
</body>
</html>
)HTML";
