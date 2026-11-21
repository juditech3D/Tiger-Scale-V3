// colis_page.h — the parcel scale from a browser (/colis).
//
// A header for the reason presse_page.h gives: the Arduino prototype generator
// scans raw strings in the .ino as code. The page polls GET /api/parcel (§19)
// a few times a second and shows the weight the way the panel does: grey while
// it moves, full colour once it is stable.
//
// Copy works in any browser on its own. Pasting into another program does not:
// no web page may type into another application. That is the optional
// software connector's job (tools/connector/, a tray program on the PC). It
// serves this same page at http://127.0.0.1:47800/ and relays /api/parcel and
// /api/tare to the scale, so the page and the connector share one origin.
// The page cannot simply call the connector from the scale's own address:
// Chrome and Edge refuse a plain-http LAN page any request to loopback (Local
// Network Access is secure-context only), so served from the scale the page
// shows Copy and Tare only, and points to the connector for the rest.
#pragma once

static const char COLIS_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>TigerScale</title>
<style>
:root{--bg:#0f1115;--card:#1a1d24;--border:#2c313b;--text:#e8eaef;--muted:#7d8490;--accent:#e8821e;--ok:#3fb950;--bad:#f85149}
@media (prefers-color-scheme:light){:root{--bg:#f4f5f7;--card:#fff;--border:#d9dce2;--text:#1b1e24;--muted:#8a909b}}
*{box-sizing:border-box}[hidden]{display:none!important}body{margin:0;background:var(--bg);color:var(--text);font:16px/1.4 system-ui,sans-serif}
main{max-width:560px;margin:0 auto;padding:20px 16px}h1{font-size:20px;margin:0 0 12px}
.card{background:var(--card);border:1px solid var(--border);border-radius:14px;padding:16px;margin-bottom:12px}
#w{font-size:clamp(56px,16vw,96px);font-weight:700;text-align:center;font-variant-numeric:tabular-nums;color:var(--muted);transition:color .2s;cursor:pointer;user-select:all}
#w.st{color:var(--text)}#w.off{color:var(--bad)}
#hint{text-align:center;color:var(--muted);min-height:1.4em;font-size:14px}
.btns{display:grid;grid-template-columns:1fr 1fr;gap:8px}
button,select,input[type=text]{font:inherit;color:var(--text);background:var(--bg);border:1px solid var(--border);border-radius:10px;min-height:44px;padding:0 12px}
button{cursor:pointer;font-weight:600}button:disabled{opacity:.35;cursor:default}
.pri{background:var(--accent);border-color:var(--accent);color:#fff}.wide{grid-column:1/-1}
label{display:flex;align-items:center;justify-content:space-between;gap:10px;margin:8px 0;color:var(--text)}
label select,label input[type=text]{flex:0 1 60%;min-width:0}input[type=checkbox]{width:22px;height:22px;accent-color:var(--accent)}
.small{font-size:13px;color:var(--muted)}#conn b{font-weight:600}#conn a{color:var(--accent)}.dot{display:inline-block;width:9px;height:9px;border-radius:50%;background:var(--bad);margin-right:6px}
.dot.on{background:var(--ok)}#msg{text-align:center;min-height:1.4em;margin-top:8px;font-size:14px;color:var(--muted)}
</style></head><body><main>
<h1 id="h"></h1>
<div class="card"><div id="w">--</div><div id="hint"></div></div>
<div class="card">
<div class="btns">
<button id="copy" class="pri"></button><button id="tare"></button>
<button id="pfield" class="wide"></button><button id="papp" class="wide"></button>
</div><div id="msg"></div></div>
<div class="card">
<label><span id="l_unit"></span><select id="unit"><option value="g">g</option><option value="kg">kg</option></select></label>
<label><span id="l_sep"></span><select id="sep"><option value=",">1,234</option><option value=".">1.234</option></select></label>
<label><span id="l_suf"></span><input type="checkbox" id="suf"></label>
<label><span id="l_after"></span><select id="after"><option value="none"></option><option value="tab"></option><option value="enter"></option></select></label>
<label><span id="l_app"></span><input type="text" id="app" placeholder="notepad.exe" spellcheck="false"></label>
<label><span id="l_auto"></span><select id="auto"><option value="off"></option><option value="field"></option><option value="app"></option></select></label>
<p class="small" id="autohelp"></p>
</div>
<div class="card" id="scard" hidden><label><span id="l_scale"></span><input type="text" id="scale" placeholder="tigerscale-XXXX.local" spellcheck="false"></label>
<div class="btns"><button id="ssave"></button><button id="sscan"></button></div><div id="smsg" class="small"></div></div>
<div class="card small" id="conn"></div>
</main><script>
const fr=(navigator.language||'').toLowerCase().startsWith('fr');
// Attached to each firmware release by .github/workflows/connector.yml.
const DL='https://github.com/juditech3D/Tiger-Scale-V3/releases/latest/download/TigerScaleConnector.exe';
const L=fr?{h:'Pèse-colis',copy:'Copier',tare:'Tarer',pfield:'Coller dans le champ actif',papp:'Coller dans le logiciel choisi',
unit:'Unité',sep:'Séparateur décimal',suf:'Ajouter l’unité (g, kg)',after:'Après le collage',none:'Rien',tab:'Touche Tab',enter:'Touche Entrée',
app:'Logiciel choisi (chemin ou nom)',auto:'Collage automatique',off:'Non',afield:'Dans le champ actif',aapp:'Dans le logiciel choisi',
autohelp:'Quand le poids est stable, il est collé une seule fois par colis. Retirez le colis pour réarmer.',
open:'Ouvrez l’outil Pèse-colis sur la balance.',moving:'Poids en mouvement…',stable:'Poids stable',offline:'Capteur de poids non détecté',lost:'Balance injoignable',
copied:'Copié : ',pasted:'Collé : ',noconn:'Le connecteur logiciel n’est pas lancé sur ce PC.',noapp:'Indiquez d’abord le logiciel choisi.',fail:'Échec : ',
conn_on:'Connecteur logiciel <b>actif</b> : les boutons Coller fonctionnent.',
conn_off:'Connecteur logiciel <b>injoignable</b>. Vérifiez qu’il tourne toujours (icône près de l’horloge).',
viascale:'Pour coller le poids directement dans un autre logiciel, installez le <b>connecteur logiciel</b> sur le PC (<a href="'+DL+'">télécharger, Windows</a>) : il ouvre cette même page avec les boutons Coller.',
scale:'Adresse de la balance',ssave:'Enregistrer',sscan:'Rechercher sur le réseau',scanning:'Recherche en cours…',found:'Balance trouvée : ',notfound:'Aucune balance trouvée.',saved:'Enregistré.'}
:{h:'Parcel scale',copy:'Copy',tare:'Tare',pfield:'Paste into the active field',papp:'Paste into the chosen program',
unit:'Unit',sep:'Decimal separator',suf:'Add the unit (g, kg)',after:'After pasting',none:'Nothing',tab:'Tab key',enter:'Enter key',
app:'Chosen program (path or name)',auto:'Automatic paste',off:'Off',afield:'Into the active field',aapp:'Into the chosen program',
autohelp:'Once the weight is stable it is pasted once per parcel. Lift the parcel to re-arm.',
open:'Open the Parcel scale tool on the scale.',moving:'Weight moving…',stable:'Weight stable',offline:'Load cell not detected',lost:'Scale unreachable',
copied:'Copied: ',pasted:'Pasted: ',noconn:'The software connector is not running on this PC.',noapp:'Set the chosen program first.',fail:'Failed: ',
conn_on:'Software connector <b>running</b>: the Paste buttons work.',
conn_off:'Software connector <b>unreachable</b>. Check that it is still running (icon next to the clock).',
viascale:'To paste the weight straight into another program, install the <b>software connector</b> on the PC (<a href="'+DL+'">download, Windows</a>): it opens this same page with the Paste buttons.',
scale:'Scale address',ssave:'Save',sscan:'Search the network',scanning:'Searching…',found:'Scale found: ',notfound:'No scale found.',saved:'Saved.'};
const $=id=>document.getElementById(id);
for(const k of['h','copy','tare','pfield','papp'])$(k).textContent=L[k];
for(const k of['unit','sep','suf','after','app','auto'])$('l_'+k).textContent=L[k];
$('autohelp').textContent=L.autohelp;
const ao=$('after').options;ao[0].text=L.none;ao[1].text=L.tab;ao[2].text=L.enter;
const uo=$('auto').options;uo[0].text=L.off;uo[1].text=L.afield;uo[2].text=L.aapp;
// Served by the connector (same origin) or by the scale itself.
const viaConn=location.port==='47800'&&(location.hostname==='127.0.0.1'||location.hostname==='localhost');
const CONN='';
if(!viaConn){for(const id of['pfield','papp'])$(id).hidden=true;
for(const id of['after','app','auto'])$(id).closest('label').hidden=true;$('autohelp').hidden=true}
else{$('scard').hidden=false;$('l_scale').textContent=L.scale;$('ssave').textContent=L.ssave;$('sscan').textContent=L.sscan}
const S={unit:'g',sep:',',suf:false,after:'none',app:'',auto:'off'};
try{Object.assign(S,JSON.parse(localStorage.getItem('colis')||'{}'))}catch(e){}
for(const k in S){const el=$(k);if(!el)continue;if(el.type==='checkbox')el.checked=!!S[k];else el.value=S[k];
el.onchange=()=>{S[k]=el.type==='checkbox'?el.checked:el.value;try{localStorage.setItem('colis',JSON.stringify(S))}catch(e){}}}
let cur=null,armed=true,conn=false,msgT=0;
function say(t){$('msg').textContent=t;clearTimeout(msgT);msgT=setTimeout(()=>$('msg').textContent='',4000)}
function fmt(g){let v=S.unit==='kg'?(g/1000).toFixed(3):String(g);if(S.sep===',')v=v.replace('.',',');return S.suf?v+' '+S.unit:v}
function canUse(){return cur&&cur.active&&!cur.offline}
function draw(){const w=$('w');let hint;
if(!cur){w.textContent='--';w.className='off';hint=L.lost}
else if(cur.offline){w.textContent='--';w.className='off';hint=L.offline}
else{w.textContent=fmt(cur.grams)+(S.suf?'':' '+S.unit);w.className=cur.stable?'st':'';hint=!cur.active?L.open:cur.stable?L.stable:L.moving}
$('hint').textContent=hint;const ok=canUse();
$('copy').disabled=!ok;$('pfield').disabled=!ok||!conn;$('papp').disabled=!ok||!conn}
function copyText(t){if(navigator.clipboard&&window.isSecureContext)return navigator.clipboard.writeText(t);
const a=document.createElement('textarea');a.value=t;a.style.position='fixed';a.style.opacity='0';document.body.appendChild(a);a.select();
const ok=document.execCommand('copy');a.remove();return ok?Promise.resolve():Promise.reject(new Error('copy'))}
async function paste(toApp){if(!canUse())return;if(!conn){say(L.noconn);return}
if(toApp&&!S.app.trim()){say(L.noapp);return}const t=fmt(cur.grams);
try{const r=await fetch(CONN+'/paste',{method:'POST',headers:{'Content-Type':'text/plain'},
body:JSON.stringify({text:t,target:toApp?'app':(document.hasFocus()?'previous':'current'),app:S.app.trim(),after:S.after})});
const j=await r.json().catch(()=>({}));if(r.ok&&j.ok)say(L.pasted+t);else say(L.fail+(j.error||r.status))}
catch(e){conn=false;drawConn();say(L.noconn)}}
$('copy').onclick=()=>{if(canUse()){const t=fmt(cur.grams);copyText(t).then(()=>say(L.copied+t),()=>say(L.fail+'copy'))}};
$('w').onclick=$('copy').onclick;
$('pfield').onclick=()=>paste(false);$('papp').onclick=()=>paste(true);
$('tare').onclick=()=>fetch('/api/tare',{method:'POST'}).then(()=>{armed=true}).catch(()=>{});
function drawConn(){$('conn').innerHTML=viaConn?'<span class="dot'+(conn?' on':'')+'"></span>'+(conn?L.conn_on:L.conn_off):L.viascale;draw()}
async function cfg(body){const r=await fetch('/config',body?{method:'POST',headers:{'Content-Type':'text/plain'},body:JSON.stringify(body)}:{cache:'no-store'});return r.json()}
if(viaConn){cfg().then(j=>{$('scale').value=j.scale||''}).catch(()=>{});
$('ssave').onclick=()=>cfg({scale:$('scale').value.trim()}).then(()=>$('smsg').textContent=L.saved).catch(()=>{});
$('sscan').onclick=()=>{$('smsg').textContent=L.scanning;cfg({scan:true}).then(j=>{if(j.scale){$('scale').value=j.scale;$('smsg').textContent=L.found+j.scale}else $('smsg').textContent=L.notfound}).catch(()=>$('smsg').textContent=L.notfound)}}
async function probe(){if(!viaConn){conn=false;drawConn();return}try{const r=await fetch(CONN+'/hello',{cache:'no-store'});const j=await r.json();conn=j.app==='tigerscale-connector'}catch(e){conn=false}drawConn()}
async function poll(){try{const r=await fetch('/api/parcel',{cache:'no-store'});cur=await r.json()}catch(e){cur=null}draw();
if(cur&&cur.active&&!cur.offline){if(cur.grams<5)armed=true;
else if(armed&&cur.stable&&cur.grams>=10&&S.auto!=='off'&&conn){armed=false;paste(S.auto==='app')}}
setTimeout(poll,document.hidden?1000:300)}
poll();probe();if(viaConn)setInterval(probe,10000);
</script></body></html>)HTML";
