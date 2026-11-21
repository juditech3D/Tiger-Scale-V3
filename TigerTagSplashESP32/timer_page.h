// timer_page.h — the timer tool's ready-made times, from a browser (/minuterie).
//
// A header for the reason presse_page.h gives: the Arduino prototype generator
// scans raw strings in the .ino as code. The page talks to GET/POST
// /api/timers and POST /api/timers/reset (§19): one row per time, a name and a
// duration, arrows to order them as the scale shows them, export and import as
// a JSON file, and the factory list back.
#pragma once

// The timer tool's limits and types (§9, runTimers). Here rather than in the
// .ino because the prototypes the converter generates take them.
#define TMR_SLOTS        3
#define TMR_MAX_PRESETS  24
#define TMR_NAME_MAX     20          // characters, not bytes
#define TMR_NAME_BYTES   (TMR_NAME_MAX * 4 + 1)
#define TMR_MAX_SEC      35999u      // 9:59:59
#define TMR_RING_MS      60000u      // a finished timer rings for one minute
#define TMR_BEEP_EVERY   2000u


struct TimerPreset { uint32_t sec; char name[TMR_NAME_BYTES]; };

enum { TMS_EMPTY, TMS_RUN, TMS_PAUSE, TMS_DONE };

struct TimerSlot {
    uint8_t  state;
    bool     chrono;
    uint32_t totalMs;     // countdown length
    uint32_t markMs;      // millis() when it last (re)started
    uint32_t accMs;       // at markMs: countdown = time left, chrono = time elapsed
    uint32_t doneAtMs, lastBeepMs;
    char     name[TMR_NAME_BYTES];
    // widgets
    lv_obj_t *card, *nameL, *ring, *timeL, *subL, *stateL, *pauseB, *pauseL, *stopB;
    lv_obj_t *addBox, *doneBox, *doneTotalL;
    uint8_t  shownState;
    bool     shownBlink;
};

struct TimerTool {
    TimerSlot   slot[TMR_SLOTS];
    TimerPreset preset[TMR_MAX_PRESETS];
    int         nPresets;
};


static const char TIMER_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Timer</title>
<style>
:root{--bg:#0f1115;--card:#1a1d24;--border:#2c313b;--text:#e8eaef;--muted:#9aa1ad;--accent:#2f7fff;--green:#3ba55d;--red:#e24b4a}
@media (prefers-color-scheme:light){:root{--bg:#f4f5f7;--card:#fff;--border:#d9dce2;--text:#1b1e24;--muted:#5f6672}}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font:16px/1.4 system-ui,sans-serif}
main{max-width:620px;margin:0 auto;padding:20px 16px}h1{font-size:22px;margin:0 0 4px}p{color:var(--muted);margin:0 0 16px}
.row{display:flex;align-items:center;gap:8px;background:var(--card);border:1px solid var(--border);border-radius:12px;padding:8px 10px;margin-bottom:8px}
input{font:inherit;color:var(--text);background:var(--bg);border:1px solid var(--border);border-radius:8px;height:40px;padding:0 10px;min-width:0}
input.n{flex:1}input.d{width:104px;text-align:center;font-variant-numeric:tabular-nums}input.bad{border-color:var(--red)}
button{font:inherit;color:var(--text);background:var(--bg);border:1px solid var(--border);border-radius:8px;min-width:40px;height:40px;cursor:pointer;padding:0 12px}
button:disabled{opacity:.3;cursor:default}button.x{color:var(--red)}
.bar{display:flex;flex-wrap:wrap;gap:8px;margin-top:12px}.bar .p{background:var(--accent);border-color:var(--accent);color:#fff;font-weight:600}
.bar .g{background:var(--green);border-color:var(--green);color:#fff;font-weight:600}
#msg{text-align:center;margin-top:12px;color:var(--muted);min-height:1.4em}
</style></head><body><main>
<h1 id="h"></h1><p id="d"></p><div id="list"></div>
<div class="bar"><button id="add" class="p"></button><button id="save" class="g"></button>
<button id="exp"></button><button id="imp"></button><button id="rst"></button></div>
<input type="file" id="file" accept=".json,application/json" hidden>
<div id="msg"></div>
</main><script>
const fr=(navigator.language||'').toLowerCase().startsWith('fr');
const L=fr?{h:'Minuterie : mes temps',d:'Les temps tout prêts de la balance. Un nom (facultatif) et une durée, par exemple 45:00 ou 1:30:00. Les flèches règlent l\'ordre sur la balance.',
add:'+ Ajouter un temps',save:'Enregistrer sur la balance',exp:'Exporter',imp:'Importer',rst:'Remettre ceux d\'usine',name:'Nom (facultatif)',ok:'Enregistré. La balance montre la nouvelle liste.',
err:'Échec : ',bad:'Durée invalide (de 0:01 à 9:59:59).',full:'La liste est pleine.',sure:'Remettre la liste d\'usine ? Tes temps seront remplacés.',badfile:'Ce fichier n\'est pas une liste de temps.'}
:{h:'Timer: my times',d:'The scale\'s ready-made times. A name (optional) and a duration, such as 45:00 or 1:30:00. The arrows set the order on the scale.',
add:'+ Add a time',save:'Save to the scale',exp:'Export',imp:'Import',rst:'Restore factory times',name:'Name (optional)',ok:'Saved. The scale shows the new list.',
err:'Failed: ',bad:'Invalid duration (0:01 to 9:59:59).',full:'The list is full.',sure:'Restore the factory list? Your times will be replaced.',badfile:'This file is not a list of times.'};
const $=id=>document.getElementById(id);
for(const k of['h','d','add','save','exp','imp','rst'])$(k).textContent=L[k];
const msg=$('msg'),list=$('list');let items=[],MAX=24,NMAX=20;
const fmt=s=>{const h=Math.floor(s/3600),m=Math.floor(s/60)%60,x=s%60,p=n=>String(n).padStart(2,'0');return h?h+':'+p(m)+':'+p(x):p(m)+':'+p(x)};
function parse(v){v=v.trim();if(!/^\d+(:\d{1,2}){0,2}$/.test(v))return 0;const a=v.split(':').map(Number);
let s=a.length===1?a[0]*60:a.length===2?a[0]*60+a[1]:a[0]*3600+a[1]*60+a[2];if(a.length>1&&a.slice(1).some(n=>n>59))return 0;return s>=1&&s<=35999?s:0}
const clean=n=>[...String(n||'').replace(/[\u0000-\u001f\u007f]/g,'').trim()].slice(0,NMAX).join('');
function draw(){list.textContent='';items.forEach((t,k)=>{const r=document.createElement('div');r.className='row';
const n=document.createElement('input');n.className='n';n.placeholder=L.name;n.value=t.name;n.maxLength=NMAX*2;n.oninput=()=>{t.name=n.value};
const d=document.createElement('input');d.className='d';d.value=t.s?fmt(t.s):'';d.inputMode='numeric';
d.oninput=()=>{t.s=parse(d.value);d.classList.toggle('bad',!t.s)};
const u=document.createElement('button');u.textContent='▲';u.disabled=k===0;u.onclick=()=>{[items[k-1],items[k]]=[items[k],items[k-1]];draw()};
const w=document.createElement('button');w.textContent='▼';w.disabled=k===items.length-1;w.onclick=()=>{[items[k+1],items[k]]=[items[k],items[k+1]];draw()};
const x=document.createElement('button');x.className='x';x.textContent='✕';x.onclick=()=>{items.splice(k,1);draw()};
r.append(n,d,u,w,x);list.append(r)});$('add').disabled=items.length>=MAX}
async function load(){const r=await fetch('/api/timers',{cache:'no-store'});const j=await r.json();MAX=j.max||24;NMAX=j.nameMax||20;items=j.presets;draw()}
$('add').onclick=()=>{if(items.length>=MAX){msg.textContent=L.full;return}items.push({name:'',s:0});draw();list.lastChild.querySelector('.d').focus()};
$('save').onclick=async()=>{msg.textContent='';if(items.some(t=>!t.s)){msg.textContent=L.bad;return}
const body=new URLSearchParams({list:items.map(t=>t.s+'\t'+clean(t.name)).join('\n')});
try{const r=await fetch('/api/timers',{method:'POST',body});if(!r.ok)throw new Error(await r.text());await load();msg.textContent=L.ok}catch(e){msg.textContent=L.err+e.message}};
$('exp').onclick=()=>{const b=new Blob([JSON.stringify({presets:items.filter(t=>t.s).map(t=>({name:clean(t.name),s:t.s}))},null,1)],{type:'application/json'});
const a=document.createElement('a');a.href=URL.createObjectURL(b);a.download='tigerscale-timers.json';a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000)};
$('imp').onclick=()=>$('file').click();
$('file').onchange=async e=>{const f=e.target.files[0];e.target.value='';if(!f)return;try{const j=JSON.parse(await f.text());
if(!Array.isArray(j.presets))throw 0;items=j.presets.filter(t=>t&&Number.isInteger(t.s)&&t.s>=1&&t.s<=35999).slice(0,MAX).map(t=>({name:clean(t.name),s:t.s}));draw();msg.textContent=''}catch(_){msg.textContent=L.badfile}};
$('rst').onclick=async()=>{if(!confirm(L.sure))return;try{const r=await fetch('/api/timers/reset',{method:'POST'});if(!r.ok)throw new Error(await r.text());await load();msg.textContent=L.ok}catch(e){msg.textContent=L.err+e.message}};
load().catch(e=>{msg.textContent=L.err+e.message});
</script></body></html>)HTML";
