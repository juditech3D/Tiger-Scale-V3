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
{"name":"Sublimation","color":"#A35BD1","icon":"mug","items":[
{"name":"Polyester T-shirt","icon":"tshirt","temp":200,"time":45,"pressure":"Medium","peel":"Hot","note":"Pre-press 5 s, protective paper on top"},
{"name":"Polyester sweatshirt","icon":"hoodie","temp":200,"time":55,"pressure":"Medium","peel":"Hot","note":"Cardboard inside"},
{"name":"Polyester tote bag","icon":"bag","temp":200,"time":45,"pressure":"Medium","peel":"Hot","note":"Cardboard inside"},
{"name":"Cushion cover","icon":"cushion","temp":200,"time":45,"pressure":"Medium","peel":"Hot","note":"Cardboard inside"},
{"name":"Towel / socks","icon":"tshirt","temp":190,"time":60,"pressure":"Medium","peel":"Hot","note":"Polyester only"},
{"name":"Mouse pad","icon":"mousepad","temp":195,"time":50,"pressure":"Medium","peel":"Hot","note":"Ventilate, rubber smell"},
{"name":"Aluminium / ChromaLuxe panel","icon":"plate","temp":200,"time":60,"pressure":"Medium","peel":"Hot","note":"Remove the film, image face down, absorbent paper"},
{"name":"MDF panel","icon":"plate","temp":200,"time":60,"pressure":"Medium","peel":"Hot","note":"Remove the protective film"},
{"name":"Coaster","icon":"plate","temp":190,"time":45,"pressure":"Medium","peel":"Hot","note":"Aluminium-cork or rubber"},
{"name":"Phone case","icon":"phone","temp":180,"time":80,"pressure":"Medium","peel":"Hot","note":"Remove the insert's film"},
{"name":"Cardboard puzzle","icon":"puzzle","temp":190,"time":60,"pressure":"Medium","peel":"Hot","note":"May shrink, test first"},
{"name":"Ceramic tile","icon":"plate","temp":200,"time":360,"pressure":"Medium","peel":"Hot","note":"From the back on a silicone pad, see the tile's sheet"},
{"name":"Mug","icon":"mug","temp":200,"time":180,"pressure":"Medium","peel":"Hot","note":"Mug press, gloves to take it out"},
{"name":"Stainless tumbler","icon":"bottle","temp":190,"time":90,"pressure":"Medium","peel":"Hot","note":"Tumbler press, shrink film tight"}]},
{"name":"DTF","color":"#3B8FE0","icon":"tshirt","items":[
{"name":"Cotton","icon":"tshirt","temp":160,"time":15,"pressure":"Firm","peel":"Cold","note":"Press again 10 s with paper after peeling"},
{"name":"Cotton / polyester","icon":"tshirt","temp":150,"time":12,"pressure":"Firm","peel":"Cold","note":"Press again 5 s"},
{"name":"Polyester","icon":"tshirt","temp":135,"time":10,"pressure":"Medium","peel":"Cold","note":"Watch for dye migration"},
{"name":"Nylon","icon":"bag","temp":130,"time":10,"pressure":"Medium","peel":"Cold","note":"Pre-press 3 s, test first"},
{"name":"Softshell / waterproof","icon":"hoodie","temp":130,"time":10,"pressure":"Medium","peel":"Cold","note":"Low temperature, test on an offcut"},
{"name":"Cap","icon":"cap","temp":155,"time":15,"pressure":"Firm","peel":"Cold","note":"Cap platen"}]},
{"name":"HTV","color":"#2FB36B","icon":"tshirt","items":[
{"name":"Standard PU vinyl","icon":"tshirt","temp":150,"time":12,"pressure":"Medium","peel":"Hot or cold","note":"Pre-press 2-3 s"},
{"name":"Stretch / sport vinyl","icon":"tshirt","temp":160,"time":20,"pressure":"Firm","peel":"Hot or cold","note":""},
{"name":"Eco stretch vinyl","icon":"tshirt","temp":120,"time":12,"pressure":"Medium","peel":"Hot","note":""},
{"name":"Anti-migration vinyl","icon":"tshirt","temp":130,"time":12,"pressure":"Medium","peel":"Hot or cold","note":"For sublimated polyester"},
{"name":"Glitter vinyl","icon":"tshirt","temp":160,"time":15,"pressure":"Medium","peel":"Warm","note":""},
{"name":"Holographic vinyl","icon":"tshirt","temp":160,"time":18,"pressure":"Firm","peel":"Cold","note":""},
{"name":"Metallic vinyl","icon":"tshirt","temp":150,"time":12,"pressure":"Medium","peel":"Cold","note":""},
{"name":"Reflective vinyl","icon":"tshirt","temp":150,"time":15,"pressure":"Medium","peel":"Warm","note":"Wait 15 s before peeling"},
{"name":"Thick vinyl (brick)","icon":"tshirt","temp":155,"time":20,"pressure":"Medium","peel":"Cold","note":""},
{"name":"3D puff vinyl","icon":"tshirt","temp":138,"time":9,"pressure":"Medium","peel":"Hot","note":"One press only, apply it last"}]},
{"name":"Flock","color":"#E07A2E","icon":"hoodie","items":[
{"name":"Flock on cotton","icon":"tshirt","temp":155,"time":12,"pressure":"Medium","peel":"Warm","note":"Pre-press 2-3 s"},
{"name":"Flock on polyester","icon":"tshirt","temp":150,"time":12,"pressure":"Medium","peel":"Warm","note":""}]},
{"name":"Rhinestones","color":"#D9487A","icon":"gem","items":[
{"name":"Hotfix rhinestones on cotton","icon":"gem","temp":165,"time":20,"pressure":"Firm","peel":"Cold","note":"Foam underneath, then 10 s from the back"},
{"name":"Hotfix rhinestones on polyester","icon":"gem","temp":150,"time":20,"pressure":"Firm","peel":"Cold","note":"Test first, may mark the fabric"}]},
{"name":"Transfer paper","color":"#1FA896","icon":"paper","items":[
{"name":"Light fabric paper","icon":"paper","temp":185,"time":20,"pressure":"Firm","peel":"Hot","note":"Mirrored image, printed side on the fabric"},
{"name":"Dark fabric paper","icon":"paper","temp":165,"time":20,"pressure":"Firm","peel":"Cold","note":"Peel the backing first, image up, baking paper on top"},
{"name":"Sublimation paper on fabric","icon":"paper","temp":200,"time":45,"pressure":"Medium","peel":"Hot","note":"See also Sublimation"},
{"name":"Screen-print transfer","icon":"paper","temp":160,"time":10,"pressure":"Firm","peel":"See sheet","note":"Follow the supplier's sheet"}]}
]})JSON";

static const char PRESSE_DEFAULTS_FR[] = R"JSON({"settings":{"defaults":true},"categories":[
{"name":"Sublimation","color":"#A35BD1","icon":"mug","items":[
{"name":"T-shirt polyester","icon":"tshirt","temp":200,"time":45,"pressure":"Moyenne","peel":"Chaud","note":"Pré-pressage 5 s, papier de protection dessus"},
{"name":"Sweat polyester","icon":"hoodie","temp":200,"time":55,"pressure":"Moyenne","peel":"Chaud","note":"Carton à l'intérieur"},
{"name":"Tote bag polyester","icon":"bag","temp":200,"time":45,"pressure":"Moyenne","peel":"Chaud","note":"Carton à l'intérieur"},
{"name":"Coussin / taie","icon":"cushion","temp":200,"time":45,"pressure":"Moyenne","peel":"Chaud","note":"Carton à l'intérieur"},
{"name":"Serviette / chaussettes","icon":"tshirt","temp":190,"time":60,"pressure":"Moyenne","peel":"Chaud","note":"Polyester seulement"},
{"name":"Tapis de souris","icon":"mousepad","temp":195,"time":50,"pressure":"Moyenne","peel":"Chaud","note":"Ventiler, odeur de caoutchouc"},
{"name":"Plaque alu / ChromaLuxe","icon":"plate","temp":200,"time":60,"pressure":"Moyenne","peel":"Chaud","note":"Retirer le film, image dessous, papier absorbant"},
{"name":"Panneau MDF","icon":"plate","temp":200,"time":60,"pressure":"Moyenne","peel":"Chaud","note":"Retirer le film protecteur"},
{"name":"Dessous de verre","icon":"plate","temp":190,"time":45,"pressure":"Moyenne","peel":"Chaud","note":"Alu-liège ou caoutchouc"},
{"name":"Coque de téléphone","icon":"phone","temp":180,"time":80,"pressure":"Moyenne","peel":"Chaud","note":"Retirer le film de la plaque"},
{"name":"Puzzle carton","icon":"puzzle","temp":190,"time":60,"pressure":"Moyenne","peel":"Chaud","note":"Peut se rétracter, tester"},
{"name":"Carreau céramique","icon":"plate","temp":200,"time":360,"pressure":"Moyenne","peel":"Chaud","note":"Par l'arrière sur tapis silicone, voir la notice"},
{"name":"Tasse / mug","icon":"mug","temp":200,"time":180,"pressure":"Moyenne","peel":"Chaud","note":"Presse à tasse, gants pour la sortir"},
{"name":"Gobelet inox","icon":"bottle","temp":190,"time":90,"pressure":"Moyenne","peel":"Chaud","note":"Presse à gobelet, film thermo serré"}]},
{"name":"DTF","color":"#3B8FE0","icon":"tshirt","items":[
{"name":"Coton","icon":"tshirt","temp":160,"time":15,"pressure":"Forte","peel":"Froid","note":"Repressage 10 s avec papier après pelage"},
{"name":"Coton / polyester","icon":"tshirt","temp":150,"time":12,"pressure":"Forte","peel":"Froid","note":"Repressage 5 s"},
{"name":"Polyester","icon":"tshirt","temp":135,"time":10,"pressure":"Moyenne","peel":"Froid","note":"Attention migration couleur"},
{"name":"Nylon","icon":"bag","temp":130,"time":10,"pressure":"Moyenne","peel":"Froid","note":"Pré-pressage 3 s, tester"},
{"name":"Softshell / imperméable","icon":"hoodie","temp":130,"time":10,"pressure":"Moyenne","peel":"Froid","note":"Basse température, tester sur chute"},
{"name":"Casquette","icon":"cap","temp":155,"time":15,"pressure":"Forte","peel":"Froid","note":"Gabarit casquette"}]},
{"name":"Flex","color":"#2FB36B","icon":"tshirt","items":[
{"name":"Flex PU standard","icon":"tshirt","temp":150,"time":12,"pressure":"Moyenne","peel":"Chaud ou froid","note":"Pré-pressage 2-3 s"},
{"name":"Flex stretch / sport","icon":"tshirt","temp":160,"time":20,"pressure":"Forte","peel":"Chaud ou froid","note":""},
{"name":"Flex éco-stretch","icon":"tshirt","temp":120,"time":12,"pressure":"Moyenne","peel":"Chaud","note":""},
{"name":"Flex polyester anti-migration","icon":"tshirt","temp":130,"time":12,"pressure":"Moyenne","peel":"Chaud ou froid","note":"Pour polyester sublimé"},
{"name":"Flex pailleté","icon":"tshirt","temp":160,"time":15,"pressure":"Moyenne","peel":"Tiède","note":""},
{"name":"Flex holographique","icon":"tshirt","temp":160,"time":18,"pressure":"Forte","peel":"Froid","note":""},
{"name":"Flex métallisé","icon":"tshirt","temp":150,"time":12,"pressure":"Moyenne","peel":"Froid","note":""},
{"name":"Flex réfléchissant","icon":"tshirt","temp":150,"time":15,"pressure":"Moyenne","peel":"Tiède","note":"Attendre 15 s avant de peler"},
{"name":"Flex épais (brick)","icon":"tshirt","temp":155,"time":20,"pressure":"Moyenne","peel":"Froid","note":""},
{"name":"Flex puff 3D","icon":"tshirt","temp":138,"time":9,"pressure":"Moyenne","peel":"Chaud","note":"Une seule pression, poser en dernier"}]},
{"name":"Flocage","color":"#E07A2E","icon":"hoodie","items":[
{"name":"Flock sur coton","icon":"tshirt","temp":155,"time":12,"pressure":"Moyenne","peel":"Tiède","note":"Pré-pressage 2-3 s"},
{"name":"Flock sur polyester","icon":"tshirt","temp":150,"time":12,"pressure":"Moyenne","peel":"Tiède","note":""}]},
{"name":"Strass","color":"#D9487A","icon":"gem","items":[
{"name":"Strass hotfix sur coton","icon":"gem","temp":165,"time":20,"pressure":"Forte","peel":"Froid","note":"Mousse dessous, repasser l'envers 10 s"},
{"name":"Strass hotfix sur polyester","icon":"gem","temp":150,"time":20,"pressure":"Forte","peel":"Froid","note":"Tester, risque de marque"}]},
{"name":"Papier transfert","color":"#1FA896","icon":"paper","items":[
{"name":"Papier textile clair","icon":"paper","temp":185,"time":20,"pressure":"Forte","peel":"Chaud","note":"Image retournée, face contre le tissu"},
{"name":"Papier textile foncé","icon":"paper","temp":165,"time":20,"pressure":"Forte","peel":"Froid","note":"Retirer le dos avant, image en haut, papier sulfurisé"},
{"name":"Papier sublimation textile","icon":"paper","temp":200,"time":45,"pressure":"Moyenne","peel":"Chaud","note":"Voir aussi Sublimation"},
{"name":"Transfert sérigraphie","icon":"paper","temp":160,"time":10,"pressure":"Forte","peel":"Selon notice","note":"Suivre la notice du fournisseur"}]}
]})JSON";


static const char PRESSE_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Heat press memo</title>
<style>
  :root { --bg:#0B0E14; --bg2:#121722; --card:#161B26; --card2:#1C2230; --line:#2A3142; --txt:#F3F5F9; --mute:#8A93A6;
          --acc:#F07A3E; --acc2:#FF9A4D; --cyan:#4FD1EE; --ok:#3BA55D; --ko:#E24B4A; --r:14px; }
  * { box-sizing:border-box; }
  body { margin:0; font:15px/1.45 system-ui, -apple-system, "Segoe UI", Roboto, sans-serif; color:var(--txt);
         background:radial-gradient(1200px 500px at 10% -10%, #3a1408 0%, transparent 60%), var(--bg); min-height:100vh; }
  header { position:sticky; top:0; z-index:5; display:flex; flex-wrap:wrap; gap:8px; align-items:center; padding:12px 18px;
           background:rgba(11,14,20,.86); backdrop-filter:blur(10px); border-bottom:1px solid var(--line); }
  header h1 { font-size:19px; margin:0 auto 0 0; display:flex; align-items:center; gap:10px; letter-spacing:.01em; }
  header h1::before { content:""; width:28px; height:28px; border-radius:8px; background:linear-gradient(135deg, var(--acc2), #c8462f); }
  main { max-width:1000px; margin:0 auto; padding:18px; display:grid; gap:18px; }
  button, .btn { font:inherit; border:1px solid var(--line); background:var(--card2); color:var(--txt); border-radius:10px;
                 padding:7px 13px; cursor:pointer; transition:border-color .15s, background .15s; }
  button:hover, .btn:hover { border-color:var(--mute); }
  button:focus-visible, input:focus-visible, select:focus-visible { outline:2px solid var(--acc); outline-offset:1px; }
  .primary { background:linear-gradient(90deg, #2fb36b, #27a05f); border-color:transparent; font-weight:600; }
  .danger { color:var(--ko); }
  .icon { padding:5px 9px; background:transparent; }
  input, select { font:inherit; color:var(--txt); background:var(--bg2); border:1px solid var(--line); border-radius:8px; padding:6px 8px; width:100%; }
  input[type=color] { padding:2px; width:40px; height:36px; flex:none; border-radius:10px; }
  input[type=number] { text-align:right; font-variant-numeric:tabular-nums; }
  input[type=range] { padding:0; accent-color:var(--acc); }
  .panel, .cat { background:var(--card); border:1px solid var(--line); border-radius:var(--r); padding:16px; }
  .panel h2 { font-size:16px; margin:0 0 4px; }
  .cat { border-top:4px solid var(--c, #888); }
  .cat-head { display:flex; gap:8px; align-items:center; margin-bottom:12px; }
  .cat-head input[type=text] { font-weight:600; font-size:16px; }
  .cat-head select { width:170px; flex:none; }
  .count { font-size:12px; color:var(--mute); white-space:nowrap; }
  table { width:100%; border-collapse:separate; border-spacing:0 6px; }
  th { text-align:left; font-weight:600; color:var(--mute); font-size:11px; text-transform:uppercase; letter-spacing:.05em; padding:0 4px; }
  td { padding:6px 4px; vertical-align:middle; background:var(--card2); }
  td:first-child { border-radius:10px 0 0 10px; padding-left:8px; }
  td:last-child { border-radius:0 10px 10px 0; padding-right:8px; }
  .w-temp, .w-time { width:84px; }
  .w-temp input { color:var(--acc2); font-weight:600; }
  .w-time input { color:var(--cyan); font-weight:600; }
  .w-sel { width:130px; }
  .w-icon { width:150px; }
  .w-act { width:112px; white-space:nowrap; text-align:right; }
  .banner { padding:10px 14px; border-radius:10px; background:#3a2d12; color:#f5d38a; }
  .toast { position:fixed; bottom:18px; left:50%; transform:translateX(-50%); background:#1B212C; border:1px solid var(--line);
           padding:10px 16px; border-radius:10px; opacity:0; transition:opacity .3s; pointer-events:none; }
  .toast.show { opacity:1; }
  .muted { color:var(--mute); font-size:13px; margin:0; }
  .bg-wrap { display:grid; grid-template-columns:minmax(0, 360px) minmax(0, 1fr); gap:18px; align-items:start; margin-top:12px; }
  .preview { width:100%; aspect-ratio:3/2; border-radius:10px; border:6px solid #05060a; background:#000; display:block; }
  .swatches { display:grid; grid-template-columns:repeat(auto-fill, minmax(84px, 1fr)); gap:8px; }
  .sw { padding:0; border:2px solid transparent; background:none; display:grid; gap:4px; text-align:left; font-size:12px; color:var(--mute); }
  .sw span { display:block; height:44px; border-radius:8px; background-size:cover; background-position:center; }
  .sw[aria-pressed="true"] { border-color:var(--acc); color:var(--txt); }
  .row { display:flex; flex-wrap:wrap; gap:10px; align-items:center; margin-top:12px; }
  .row label { font-size:13px; color:var(--mute); }
  @media (max-width:760px) {
    .bg-wrap { grid-template-columns:1fr; }
    thead { display:none; }
    tr { display:grid; grid-template-columns:1fr 1fr; gap:6px; padding:10px; margin-bottom:8px; background:var(--card2); border-radius:10px; }
    td, td:first-child, td:last-child { padding:0; background:none; border-radius:0; }
    td.name, td.note { grid-column:1 / -1; }
    td::before { content:attr(data-l); display:block; color:var(--mute); font-size:11px; }
    .w-temp, .w-time, .w-sel, .w-icon, .w-act { width:auto; }
    td.w-act { grid-column:1 / -1; text-align:left; }
    .cat-head { flex-wrap:wrap; }
    .cat-head select { width:auto; }
  }
  @media (prefers-reduced-motion:reduce) { * { transition:none !important; } }
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
  <section class="panel">
    <h2 data-t="bgTitle"></h2>
    <p class="muted" data-t="bgText"></p>
    <div class="bg-wrap">
      <canvas id="preview" class="preview" width="480" height="320"></canvas>
      <div>
        <div class="swatches" id="swatches"></div>
        <div class="row">
          <label class="btn"><span data-t="bgPick"></span><input id="photo" type="file" accept="image/*" hidden></label>
          <button id="dropPhoto" class="danger" data-t="bgDrop" hidden></button>
        </div>
        <div class="row">
          <label for="dim" data-t="bgDim"></label>
          <input id="dim" type="range" min="0" max="90" step="5" style="width:160px">
          <span id="dimVal" class="muted"></span>
        </div>
        <div class="row"><button id="saveBg" class="primary" data-t="bgSave"></button><span id="bgState" class="muted"></span></div>
      </div>
    </div>
  </section>
  <p class="muted" data-t="indicative"></p>
  <div id="cats" style="display:grid;gap:18px"></div>
  <div class="panel">
    <h2 data-t="factoryTitle"></h2>
    <p class="muted" data-t="factoryText"></p>
    <button id="reset" class="danger" data-t="factoryBtn" style="margin-top:10px"></button>
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
       pressures:["Light", "Medium", "Firm"], peels:["Hot", "Warm", "Cold", "Hot or cold", "See sheet", "-"], defPressure:"Medium", defPeel:"Hot",
       icon:"Thumbnail", icons:{"":"Auto (from the name)", none:"None", tshirt:"T-shirt", hoodie:"Hoodie / sweater", mug:"Mug",
       cap:"Cap", bag:"Bag", mousepad:"Mouse pad", cushion:"Cushion", plate:"Panel / plate", bottle:"Bottle", phone:"Phone case",
       puzzle:"Puzzle", gem:"Rhinestones", paper:"Transfer paper"},
       items:n => n + (n === 1 ? " product" : " products"),
       bgTitle:"Screen background", bgText:"What the memo shows behind its cards on the scale. Pick a colour, or one of your own photos: it is cropped to the screen here, then sent to the scale.",
       bgPick:"Use a photo…", bgDrop:"Remove the photo", bgDim:"Darken the photo", bgSave:"Apply on the scale",
       bgSaved:"Background applied", bgSending:"Sending the photo…", bgNoPhoto:"No photo on the scale yet",
       bgHasPhoto:"A photo is stored on the scale", bgPhoto:"My photo",
       presets:{night:"Night", ember:"Ember", ocean:"Ocean", workshop:"Workshop", lavender:"Lavender", forest:"Forest"}},
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
       pressures:["Légère", "Moyenne", "Forte"], peels:["Chaud", "Tiède", "Froid", "Chaud ou froid", "Selon notice", "-"], defPressure:"Moyenne", defPeel:"Chaud",
       icon:"Miniature", icons:{"":"Auto (selon le nom)", none:"Aucune", tshirt:"T-shirt", hoodie:"Sweat / pull", mug:"Tasse / mug",
       cap:"Casquette", bag:"Sac", mousepad:"Tapis de souris", cushion:"Coussin", plate:"Plaque", bottle:"Gourde", phone:"Coque de téléphone",
       puzzle:"Puzzle", gem:"Strass", paper:"Papier transfert"},
       items:n => n + (n === 1 ? " produit" : " produits"),
       bgTitle:"Fond d'écran", bgText:"Ce que le mémo affiche derrière ses cartes sur la balance. Choisissez une couleur, ou une de vos photos : elle est recadrée ici à la taille de l'écran, puis envoyée à la balance.",
       bgPick:"Utiliser une photo…", bgDrop:"Retirer la photo", bgDim:"Assombrir la photo", bgSave:"Appliquer sur la balance",
       bgSaved:"Fond appliqué", bgSending:"Envoi de la photo…", bgNoPhoto:"Pas encore de photo sur la balance",
       bgHasPhoto:"Une photo est enregistrée sur la balance", bgPhoto:"Ma photo",
       presets:{night:"Nuit", ember:"Braise", ocean:"Océan", workshop:"Atelier", lavender:"Lavande", forest:"Forêt"}}
};
const L = STR[(navigator.language || "en").slice(0, 2).toLowerCase()] || STR.en;
document.documentElement.lang = L === STR.fr ? "fr" : "en";
document.title = L.title;
document.querySelectorAll("[data-t]").forEach(e => { e.textContent = L[e.dataset.t]; });

const DEMO = {settings:{defaults:false}, categories:[
  {name:"Sublimation", color:"#A35BD1", items:[{name:"Polyester T-shirt", temp:200, time:45, pressure:"Medium", peel:"Hot", note:"Protective paper on top"}]},
  {name:"DTF", color:"#3B8FE0", items:[{name:"Cotton", temp:160, time:15, pressure:"Firm", peel:"Cold", note:"Press again 5 s"}]}
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
// The thumbnail shown on the scale: "" lets the scale pick one from the
// name, "none" shows none; the other values are the ids of presse_icons.h.
function iconSelect(obj) {
  const s = el("select", {title:L.icon, onchange:e => {
    if (e.target.value) obj.icon = e.target.value; else delete obj.icon;
    changed();
  }});
  for (const [id, label] of Object.entries(L.icons))
    s.append(el("option", {value:id, textContent:label, selected:(obj.icon || "") === id}));
  if (obj.icon && !(obj.icon in L.icons)) s.append(el("option", {value:obj.icon, textContent:obj.icon, selected:true}));
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
      iconSelect(cat),
      el("span", {className:"count", textContent:L.items(cat.items.length)}),
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
        el("td", {className:"w-icon", dataset:{l:L.icon}}, iconSelect(it)),
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
        el("thead", {}, el("tr", {}, ...[L.product, L.icon, L.temp, L.time, L.pressure, L.peel, L.note, ""].map(t => el("th", {textContent:t})))),
        tbody),
      el("button", {textContent:L.addItem, style:"margin-top:8px", onclick:() => {
        cat.items.push({name:L.newItem, temp:160, time:15, pressure:L.defPressure, peel:L.defPeel, note:""});
        changed(true);
      }}));
    root.append(box);
  });
  drawPreview();
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
  await loadBg();
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

// ---- The screen background (/api/presse-bg, /api/presse-bgimg) ----------
// Same presets as PRESSE_BG_PRESETS in the firmware: top and bottom colours.
const PRESETS = {night:["#0B0E14", "#0B0E14"], ember:["#3A1408", "#0E0A10"], ocean:["#062A3A", "#070C14"],
                 workshop:["#2B2F3A", "#101217"], lavender:["#2A1846", "#0D0B16"], forest:["#0F3324", "#080F0C"]};
let bg = {preset:"ember", dim:45, photo:false, hasPhoto:false};
let photoCanvas = null;      // the picked photo, already cropped to 480x320
let photoPending = false;    // picked here, not yet sent

async function loadBg() {
  if (!demo) {
    try {
      const r = await fetch("/api/presse-bg", {cache:"no-store"});
      if (r.ok) bg = Object.assign(bg, await r.json());
    } catch {}
  }
  $("#dim").value = bg.dim;
  drawSwatches();
  drawPreview();
}

function drawSwatches() {
  const root = $("#swatches");
  root.innerHTML = "";
  for (const [id, [a, b]] of Object.entries(PRESETS)) {
    const sw = el("button", {className:"sw", onclick:() => { bg.preset = id; bg.photo = false; drawSwatches(); drawPreview(); }},
      el("span", {style:`background:linear-gradient(180deg, ${a}, ${b})`}), L.presets[id]);
    sw.setAttribute("aria-pressed", String(!bg.photo && bg.preset === id));
    root.append(sw);
  }
  if (photoCanvas || bg.hasPhoto) {
    const sw = el("button", {className:"sw", onclick:() => { bg.photo = true; drawSwatches(); drawPreview(); }},
      el("span", {style:photoCanvas ? `background-image:url(${photoCanvas.toDataURL("image/jpeg", .6)})` : "background:repeating-linear-gradient(45deg,#333 0 8px,#222 8px 16px)"}), L.bgPhoto);
    sw.setAttribute("aria-pressed", String(bg.photo));
    root.append(sw);
  }
  $("#dimVal").textContent = bg.dim + " %";
  $("#dropPhoto").hidden = !bg.hasPhoto && !photoCanvas;
  $("#bgState").textContent = bg.hasPhoto ? L.bgHasPhoto : L.bgNoPhoto;
}

// A rough picture of the memo's first screen: the background, then one
// see-through card per category, three to a row from five up.
function drawPreview() {
  const c = $("#preview"), g = c.getContext("2d");
  if (bg.photo && photoCanvas) {
    g.drawImage(photoCanvas, 0, 0);
    g.fillStyle = `rgba(0,0,0,${bg.dim / 100})`;
    g.fillRect(0, 0, 480, 320);
  } else {
    const [a, b] = PRESETS[bg.preset] || PRESETS.ember;
    const gr = g.createLinearGradient(0, 0, 0, 320);
    gr.addColorStop(0, a); gr.addColorStop(1, b);
    g.fillStyle = gr; g.fillRect(0, 0, 480, 320);
  }
  g.fillStyle = "#fff"; g.font = "600 20px system-ui, sans-serif"; g.textBaseline = "middle";
  g.fillText("‹  " + L.title, 16, 24);
  g.fillStyle = "rgba(255,255,255,.14)"; g.fillRect(0, 47, 480, 1);
  const cats = data.categories.slice(0, 9), cols = cats.length >= 5 ? 3 : 2;
  const rows = Math.max(1, Math.ceil(cats.length / cols));
  const w = (456 - (cols - 1) * 10) / cols, h = Math.max(84, Math.min(120, (191 - (rows - 1) * 10) / rows));
  cats.forEach((k, i) => {
    const x = 12 + (i % cols) * (w + 10), y = 57 + Math.floor(i / cols) * (h + 10);
    if (y + h > 320) return;
    g.fillStyle = "rgba(255,255,255,.094)"; g.strokeStyle = "rgba(255,255,255,.14)";
    g.beginPath(); g.roundRect(x, y, w, h, 14); g.fill(); g.stroke();
    g.fillStyle = k.color || "#555"; g.beginPath(); g.roundRect(x + 10, y + 10, 40, 40, 10); g.fill();
    g.fillStyle = "#fff"; g.font = "600 17px system-ui, sans-serif";
    g.fillText((k.name || "").slice(0, 16), x + 10, y + h - 30);
    const t = (k.items || []).map(it => it.temp).filter(Boolean);
    if (t.length) {
      g.fillStyle = "#8A93A6"; g.font = "13px system-ui, sans-serif";
      const lo = Math.min(...t), hi = Math.max(...t);
      g.fillText(lo === hi ? `${hi} °C` : `${lo}-${hi} °C`, x + 10, y + h - 12);
    }
  });
}

// Cover-crop the picked image to 480x320, as the scale will show it.
$("#photo").onchange = async e => {
  const f = e.target.files[0];
  e.target.value = "";
  if (!f) return;
  const img = await createImageBitmap(f).catch(() => null);
  if (!img) { toast(L.badFile); return; }
  const cv = document.createElement("canvas");
  cv.width = 480; cv.height = 320;
  const s = Math.max(480 / img.width, 320 / img.height);
  const w = img.width * s, h = img.height * s;
  cv.getContext("2d").drawImage(img, (480 - w) / 2, (320 - h) / 2, w, h);
  photoCanvas = cv;
  photoPending = true;
  bg.photo = true;
  drawSwatches();
  drawPreview();
};
$("#dim").oninput = e => { bg.dim = +e.target.value; $("#dimVal").textContent = bg.dim + " %"; drawPreview(); };

// RGB565, little-endian: the byte order LVGL keeps its pixels in on the
// scale (LV_COLOR_16_SWAP 0), so the file is shown as it is read.
function toRgb565(cv) {
  const px = cv.getContext("2d").getImageData(0, 0, 480, 320).data;
  const out = new Uint16Array(480 * 320);
  for (let i = 0, j = 0; i < out.length; i++, j += 4)
    out[i] = ((px[j] & 0xF8) << 8) | ((px[j + 1] & 0xFC) << 3) | (px[j + 2] >> 3);
  return new Uint8Array(out.buffer);
}

async function sendBg(extra = {}) {
  if (demo) { toast(L.demoSend); return false; }
  const body = new URLSearchParams(Object.assign({preset:bg.preset, dim:bg.dim, photo:bg.photo ? 1 : 0}, extra));
  const r = await fetch("/api/presse-bg", {method:"POST", body});
  if (!r.ok) { toast(L.error + await r.text()); return false; }
  return true;
}

$("#saveBg").onclick = async () => {
  if (demo) { toast(L.demoSend); return; }
  if (bg.photo && photoPending) {
    toast(L.bgSending);
    const r = await fetch("/api/presse-bgimg", {method:"POST", headers:{"Content-Type":"application/octet-stream"}, body:toRgb565(photoCanvas)});
    if (!r.ok) { toast(L.error + await r.text()); return; }
    photoPending = false;
    bg.hasPhoto = true;
  }
  if (await sendBg()) { drawSwatches(); toast(L.bgSaved); }
};
$("#dropPhoto").onclick = async () => {
  photoCanvas = null; photoPending = false; bg.photo = false;
  if (bg.hasPhoto && await sendBg({drop:1})) { bg.hasPhoto = false; toast(L.bgSaved); }
  drawSwatches(); drawPreview();
};
window.addEventListener("beforeunload", e => { if (dirty) { e.preventDefault(); e.returnValue = ""; } });

load();
</script>
</body>
</html>
)HTML";
