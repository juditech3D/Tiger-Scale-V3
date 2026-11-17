// presse_page.h — the heat press memo's editor (/presse) and its factory list.
//
// A header rather than raw string literals inside the .ino for the reason
// screen_page.h gives: the Arduino prototype generator scans raw strings in the
// .ino as code, and a page script full of `function name()` would become a
// stream of bogus C++ prototypes. Headers are not scanned.
//
// The page talks to GET/POST /api/presse and POST /api/presse/reset (§19). It
// edits categories and products, exports and imports the list as a file, and
// runs in a demo mode when opened without a scale behind it. Adapted from the
// standalone Memo Presse module's configuration page, minus its Wi-Fi and
// press-lever settings: the scale has its own network and no lever switch.
#pragma once

#include <ArduinoJson.h>
#include <esp_heap_caps.h>
#include <stdlib.h>

// The memo's JSON document lives in PSRAM, with a DRAM fallback (never ps_malloc).
struct PresseSpiRamAllocator {
    void *allocate(size_t n) {
        void *p = heap_caps_malloc(n, MALLOC_CAP_SPIRAM);
        return p ? p : malloc(n);
    }
    void deallocate(void *p) { free(p); }
    void *reallocate(void *p, size_t n) {
        void *q = heap_caps_realloc(p, n, MALLOC_CAP_SPIRAM);
        return q ? q : realloc(p, n);
    }
};
typedef BasicJsonDocument<PresseSpiRamAllocator> PresseJsonDocument;

// Written by presseLoad() when no list exists yet, in English or French by the language set at
// that moment (the list is the owner's data from then on, edited in any
// language). Values are indicative and labelled as such on the screen.
static const char PRESSE_DEFAULTS_EN[] = R"JSON({"settings":{"defaults":true},"categories":[
{"name":"Sublimation","color":"#8E44AD","items":[
{"name":"Polyester T-shirt","temp":195,"time":50,"pressure":"Medium","peel":"Hot","note":"Protective paper on top"},
{"name":"Mouse pad","temp":200,"time":45,"pressure":"Light","peel":"Hot","note":"Fabric side to the paper"},
{"name":"Aluminium / MDF panel","temp":200,"time":60,"pressure":"Medium","peel":"Hot","note":"Remove the protective film"},
{"name":"Cushion cover","temp":200,"time":50,"pressure":"Medium","peel":"Hot","note":"Cardboard inside"}]},
{"name":"DTF","color":"#2980B9","items":[
{"name":"Cotton","temp":160,"time":15,"pressure":"Firm","peel":"Cold","note":"Press again 5 s after peeling"},
{"name":"Polyester","temp":135,"time":12,"pressure":"Medium","peel":"Cold","note":"Watch for dye migration"},
{"name":"Cotton / polyester","temp":150,"time":12,"pressure":"Medium","peel":"Cold","note":"Press again 5 s"}]},
{"name":"HTV","color":"#27AE60","items":[
{"name":"PU vinyl on cotton","temp":150,"time":15,"pressure":"Medium","peel":"Warm","note":""},
{"name":"Glitter vinyl","temp":160,"time":15,"pressure":"Firm","peel":"Warm","note":""},
{"name":"Vinyl on polyester","temp":130,"time":10,"pressure":"Medium","peel":"Hot","note":"Low temperature"}]},
{"name":"Flock","color":"#D35400","items":[
{"name":"Flock on cotton","temp":160,"time":15,"pressure":"Firm","peel":"Hot","note":""}]}
]})JSON";

static const char PRESSE_DEFAULTS_FR[] = R"JSON({"settings":{"defaults":true},"categories":[
{"name":"Sublimation","color":"#8E44AD","items":[
{"name":"T-shirt polyester","temp":195,"time":50,"pressure":"Moyenne","peel":"Chaud","note":"Papier de protection dessus"},
{"name":"Tapis de souris","temp":200,"time":45,"pressure":"Légère","peel":"Chaud","note":"Tissu côté papier"},
{"name":"Plaque alu / MDF","temp":200,"time":60,"pressure":"Moyenne","peel":"Chaud","note":"Retirer le film protecteur"},
{"name":"Coussin / taie","temp":200,"time":50,"pressure":"Moyenne","peel":"Chaud","note":"Carton à l'intérieur"}]},
{"name":"DTF","color":"#2980B9","items":[
{"name":"Coton","temp":160,"time":15,"pressure":"Forte","peel":"Froid","note":"Repressage 5 s après pelage"},
{"name":"Polyester","temp":135,"time":12,"pressure":"Moyenne","peel":"Froid","note":"Attention migration couleur"},
{"name":"Coton / polyester","temp":150,"time":12,"pressure":"Moyenne","peel":"Froid","note":"Repressage 5 s"}]},
{"name":"Flex","color":"#27AE60","items":[
{"name":"Flex PU coton","temp":150,"time":15,"pressure":"Moyenne","peel":"Tiède","note":""},
{"name":"Flex pailleté","temp":160,"time":15,"pressure":"Forte","peel":"Tiède","note":""},
{"name":"Flex polyester","temp":130,"time":10,"pressure":"Moyenne","peel":"Chaud","note":"Basse température"}]},
{"name":"Flocage","color":"#D35400","items":[
{"name":"Flock coton","temp":160,"time":15,"pressure":"Forte","peel":"Chaud","note":""}]}
]})JSON";


static const char PRESSE_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Heat press memo</title>
<style>
  :root { --bg:#0B0E14; --card:#141821; --line:#2E3646; --txt:#FFFFFF; --mute:#8A93A6; --acc:#2F7FFF; --ok:#3BA55D; --ko:#E24B4A; }
  * { box-sizing:border-box; }
  body { margin:0; font:15px/1.4 system-ui, -apple-system, Segoe UI, Roboto, sans-serif; background:var(--bg); color:var(--txt); }
  header { position:sticky; top:0; z-index:5; display:flex; flex-wrap:wrap; gap:8px; align-items:center; padding:12px 16px; background:#080A0F; border-bottom:1px solid var(--line); }
  header h1 { font-size:18px; margin:0 auto 0 0; }
  main { max-width:960px; margin:0 auto; padding:16px; }
  button, .btn { font:inherit; border:1px solid var(--line); background:var(--card); color:var(--txt); border-radius:8px; padding:7px 12px; cursor:pointer; }
  button:hover { border-color:var(--mute); }
  .primary { background:var(--acc); border-color:var(--acc); font-weight:600; }
  .danger { color:var(--ko); }
  .icon { padding:5px 9px; }
  input, select { font:inherit; color:var(--txt); background:#0E1118; border:1px solid var(--line); border-radius:6px; padding:6px 8px; width:100%; }
  input[type=color] { padding:2px; width:44px; height:34px; }
  input[type=number] { text-align:right; }
  .cat { background:var(--card); border:1px solid var(--line); border-left:6px solid var(--c, #888); border-radius:10px; margin:0 0 16px; padding:12px; }
  .cat-head { display:flex; gap:8px; align-items:center; margin-bottom:10px; }
  .cat-head input[type=text] { font-weight:600; font-size:16px; }
  table { width:100%; border-collapse:collapse; }
  th { text-align:left; font-weight:500; color:var(--mute); font-size:12px; padding:4px; }
  td { padding:3px 4px; vertical-align:middle; }
  .w-temp, .w-time { width:84px; }
  .w-sel { width:120px; }
  .w-act { width:112px; white-space:nowrap; text-align:right; }
  .banner { padding:10px 14px; border-radius:8px; margin-bottom:16px; background:#3a2d12; color:#f5d38a; }
  .panel { background:var(--card); border:1px solid var(--line); border-radius:10px; padding:12px; margin-bottom:16px; }
  .panel h2 { font-size:15px; margin:0 0 10px; }
  .toast { position:fixed; bottom:16px; left:50%; transform:translateX(-50%); background:#1B212C; border:1px solid var(--line); padding:10px 16px; border-radius:8px; opacity:0; transition:opacity .3s; pointer-events:none; }
  .toast.show { opacity:1; }
  .muted { color:var(--mute); font-size:13px; }
  @media (max-width:700px) {
    thead { display:none; }
    tr { display:grid; grid-template-columns:1fr 1fr; gap:6px; padding:8px 0; border-bottom:1px solid var(--line); }
    td { padding:0; }
    td.name, td.note { grid-column:1 / -1; }
    td::before { content:attr(data-l); display:block; color:var(--mute); font-size:11px; }
    .w-temp, .w-time, .w-sel, .w-act { width:auto; }
    td.w-act { grid-column:1 / -1; text-align:left; }
  }
</style>
</head>
<body>
<header>
  <h1 data-t="title"></h1>
  <button id="addCat" data-t="addCat"></button>
  <button id="export" data-t="export"></button>
  <label class="btn"><span data-t="import"></span><input id="import" type="file" accept=".json" hidden></label>
  <button id="save" class="primary" data-t="save"></button>
</header>
<main>
  <div id="demo" class="banner" hidden data-t="demo"></div>
  <p class="muted" data-t="indicative"></p>
  <div id="cats"></div>
  <div class="panel">
    <h2 data-t="factoryTitle"></h2>
    <p class="muted" data-t="factoryText"></p>
    <button id="reset" class="danger" data-t="factoryBtn"></button>
  </div>
</main>
<div id="toast" class="toast"></div>

<script>
// Two languages, picked from the browser: the scale's own web UI carries the
// eleven, this editor is a single page and keeps to the two its list ships in.
const STR = {
  en: {title:"Heat press memo", addCat:"+ Category", export:"Export", import:"Import", save:"Save to the scale",
       demo:"Demo mode: the scale cannot be reached, changes will not be saved.",
       indicative:"Factory values are indicative: check them against your suppliers' data sheets.",
       factoryTitle:"Factory values", factoryText:"Puts back the original list (indicative values). Export first to keep yours.",
       factoryBtn:"Restore factory values", product:"Product", temp:"Temp. °C", time:"Time s", pressure:"Pressure",
       peel:"Peel", note:"Note", notePh:"Note (optional)", productPh:"Product / material", catPh:"Category name",
       colour:"Colour on the screen", up:"Up", down:"Down", del:"Delete", delCat:"Delete the category",
       delCatQ:(n, k) => `Delete the category "${n}" and its ${k} products?`, addItem:"+ Product",
       newItem:"New product", newCat:"New category", saved:"Saved to the scale", demoSend:"Demo mode: nothing is sent",
       error:"Error: ", imported:"Imported, remember to save", badFile:"Invalid file",
       resetQ:"Replace the whole list with the factory values?", resetDone:"Factory values restored",
       pressures:["Light", "Medium", "Firm"], peels:["Hot", "Warm", "Cold", "-"], defPressure:"Medium", defPeel:"Hot"},
  fr: {title:"Mémo presse", addCat:"+ Catégorie", export:"Exporter", import:"Importer", save:"Enregistrer sur la balance",
       demo:"Mode démo : balance non joignable, les modifications ne seront pas enregistrées.",
       indicative:"Les valeurs d'usine sont indicatives : vérifiez-les avec les fiches techniques de vos fournisseurs.",
       factoryTitle:"Valeurs d'usine", factoryText:"Remet la liste d'origine (valeurs indicatives). Exportez d'abord pour garder la vôtre.",
       factoryBtn:"Restaurer les valeurs d'usine", product:"Produit", temp:"Temp. °C", time:"Temps s", pressure:"Pression",
       peel:"Pelage", note:"Note", notePh:"Note (optionnel)", productPh:"Produit / matière", catPh:"Nom de la catégorie",
       colour:"Couleur sur l'écran", up:"Monter", down:"Descendre", del:"Supprimer", delCat:"Supprimer la catégorie",
       delCatQ:(n, k) => `Supprimer la catégorie « ${n} » et ses ${k} produits ?`, addItem:"+ Produit",
       newItem:"Nouveau produit", newCat:"Nouvelle catégorie", saved:"Enregistré sur la balance", demoSend:"Mode démo : rien n'est envoyé",
       error:"Erreur : ", imported:"Importé, pensez à enregistrer", badFile:"Fichier invalide",
       resetQ:"Remplacer toute la liste par les valeurs d'usine ?", resetDone:"Valeurs d'usine restaurées",
       pressures:["Légère", "Moyenne", "Forte"], peels:["Chaud", "Tiède", "Froid", "-"], defPressure:"Moyenne", defPeel:"Chaud"}
};
const L = STR[(navigator.language || "en").slice(0, 2).toLowerCase()] || STR.en;
document.documentElement.lang = L === STR.fr ? "fr" : "en";
document.title = L.title;
document.querySelectorAll("[data-t]").forEach(e => { e.textContent = L[e.dataset.t]; });

const DEMO = {settings:{defaults:false}, categories:[
  {name:"Sublimation", color:"#8E44AD", items:[{name:"Polyester T-shirt", temp:195, time:50, pressure:"Medium", peel:"Hot", note:"Protective paper on top"}]},
  {name:"DTF", color:"#2980B9", items:[{name:"Cotton", temp:160, time:15, pressure:"Firm", peel:"Cold", note:"Press again 5 s"}]}
]};

let data = {settings:{}, categories:[]};
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
function select(options, value, onchange) {
  const s = el("select", {onchange});
  for (const o of options) s.append(el("option", {value:o, textContent:o, selected:o === value}));
  if (value && !options.includes(value)) s.append(el("option", {value, textContent:value, selected:true}));
  return s;
}
function move(arr, i, d) {
  const j = i + d;
  if (j < 0 || j >= arr.length) return;
  [arr[i], arr[j]] = [arr[j], arr[i]];
  changed(true);
}
function changed(rerender) {
  dirty = true;
  if (rerender) render();
}

function render() {
  const root = $("#cats");
  root.innerHTML = "";
  data.categories.forEach((cat, ci) => {
    cat.items ??= [];
    const box = el("section", {className:"cat"});
    box.style.setProperty("--c", cat.color);

    const head = el("div", {className:"cat-head"},
      el("input", {type:"color", value:cat.color, title:L.colour,
        oninput:e => { cat.color = e.target.value; box.style.setProperty("--c", cat.color); changed(); }}),
      el("input", {type:"text", value:cat.name, placeholder:L.catPh,
        oninput:e => { cat.name = e.target.value; changed(); }}),
      el("button", {className:"icon", title:L.up, textContent:"↑", onclick:() => move(data.categories, ci, -1)}),
      el("button", {className:"icon", title:L.down, textContent:"↓", onclick:() => move(data.categories, ci, 1)}),
      el("button", {className:"icon danger", title:L.delCat, textContent:"✕", onclick:() => {
        if (confirm(L.delCatQ(cat.name, cat.items.length))) { data.categories.splice(ci, 1); changed(true); }
      }})
    );

    const tbody = el("tbody");
    cat.items.forEach((it, ii) => {
      const num = (key, min, max) => el("input", {type:"number", min, max, value:it[key],
        oninput:e => { it[key] = Math.max(min, Math.min(max, parseInt(e.target.value) || 0)); changed(); }});
      tbody.append(el("tr", {},
        el("td", {className:"name", dataset:{l:L.product}}, el("input", {value:it.name, placeholder:L.productPh, oninput:e => { it.name = e.target.value; changed(); }})),
        el("td", {className:"w-temp", dataset:{l:L.temp}}, num("temp", 0, 250)),
        el("td", {className:"w-time", dataset:{l:L.time}}, num("time", 0, 999)),
        el("td", {className:"w-sel", dataset:{l:L.pressure}}, select(L.pressures, it.pressure, e => { it.pressure = e.target.value; changed(); })),
        el("td", {className:"w-sel", dataset:{l:L.peel}}, select(L.peels, it.peel, e => { it.peel = e.target.value; changed(); })),
        el("td", {className:"note", dataset:{l:L.note}}, el("input", {value:it.note || "", placeholder:L.notePh, oninput:e => { it.note = e.target.value; changed(); }})),
        el("td", {className:"w-act"},
          el("button", {className:"icon", title:L.up, textContent:"↑", onclick:() => move(cat.items, ii, -1)}),
          el("button", {className:"icon", title:L.down, textContent:"↓", onclick:() => move(cat.items, ii, 1)}),
          el("button", {className:"icon danger", title:L.del, textContent:"✕", onclick:() => { cat.items.splice(ii, 1); changed(true); }}))
      ));
    });

    box.append(head,
      el("table", {},
        el("thead", {}, el("tr", {}, ...[L.product, L.temp, L.time, L.pressure, L.peel, L.note, ""].map(t => el("th", {textContent:t})))),
        tbody),
      el("button", {textContent:L.addItem, style:"margin-top:8px", onclick:() => {
        cat.items.push({name:L.newItem, temp:160, time:15, pressure:L.defPressure, peel:L.defPeel, note:""});
        changed(true);
      }}));
    root.append(box);
  });
}

async function load() {
  try {
    const r = await fetch("/api/presse", {cache:"no-store"});
    if (!r.ok) throw 0;
    data = await r.json();
  } catch {
    demo = true;
    data = structuredClone(DEMO);
    $("#demo").hidden = false;
  }
  data.settings ??= {};
  data.categories ??= [];
  render();
}

async function post(url, body) {
  if (demo) { toast(L.demoSend); return false; }
  const r = await fetch(url, {method:"POST", headers:{"Content-Type":"application/json"}, body});
  if (!r.ok) { toast(L.error + await r.text()); return false; }
  return true;
}

$("#addCat").onclick = () => {
  data.categories.push({name:L.newCat, color:"#607D8B", items:[]});
  changed(true);
  window.scrollTo({top:document.body.scrollHeight, behavior:"smooth"});
};
$("#save").onclick = async () => {
  // A saved list is the owner's own: the scale stops calling it indicative.
  data.settings = Object.assign({}, data.settings, {defaults:false});
  if (await post("/api/presse", JSON.stringify(data))) { dirty = false; toast(L.saved); }
};
$("#export").onclick = () => {
  const a = el("a", {href:URL.createObjectURL(new Blob([JSON.stringify(data, null, 2)], {type:"application/json"})), download:"heat-press-memo.json"});
  a.click();
  URL.revokeObjectURL(a.href);
};
$("#import").onchange = async e => {
  const f = e.target.files[0];
  if (!f) return;
  try {
    const d = JSON.parse(await f.text());
    if (!Array.isArray(d.categories)) throw 0;
    data = d;
    data.settings ??= {};
    changed(true);
    toast(L.imported);
  } catch { toast(L.badFile); }
  e.target.value = "";
};
$("#reset").onclick = async () => {
  if (!confirm(L.resetQ)) return;
  if (await post("/api/presse/reset", "")) { dirty = false; await load(); toast(L.resetDone); }
};
window.addEventListener("beforeunload", e => { if (dirty) { e.preventDefault(); e.returnValue = ""; } });

load();
</script>
</body>
</html>
)HTML";
