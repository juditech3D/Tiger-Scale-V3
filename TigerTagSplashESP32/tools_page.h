// tools_page.h — the start menu's tool choice, from a browser (/tools).
//
// A header for the reason presse_page.h gives: the Arduino prototype generator
// scans raw strings in the .ino as code. The page talks to GET/POST /api/tools
// (§19): it lists every tool in menu order, with a box to show or hide it and
// arrows to move it. Tiger Scale is always shown and always first.
#pragma once

static const char TOOLS_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Tools</title>
<style>
:root{--bg:#0f1115;--card:#1a1d24;--border:#2c313b;--text:#e8eaef;--muted:#9aa1ad;--accent:#2f7fff}
@media (prefers-color-scheme:light){:root{--bg:#f4f5f7;--card:#fff;--border:#d9dce2;--text:#1b1e24;--muted:#5f6672}}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font:16px/1.4 system-ui,sans-serif}
main{max-width:560px;margin:0 auto;padding:20px 16px}h1{font-size:22px;margin:0 0 4px}p{color:var(--muted);margin:0 0 16px}
.row{display:flex;align-items:center;gap:10px;background:var(--card);border:1px solid var(--border);border-radius:12px;padding:10px 12px;margin-bottom:8px}
.row .t{flex:1;min-width:0}.row .n{font-weight:600}.row .s{color:var(--muted);font-size:13px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.row input{width:22px;height:22px;accent-color:var(--accent)}
button{font:inherit;color:var(--text);background:var(--bg);border:1px solid var(--border);border-radius:8px;min-width:40px;height:40px;cursor:pointer}
button:disabled{opacity:.3;cursor:default}#save{width:100%;margin-top:8px;background:var(--accent);border-color:var(--accent);color:#fff;font-weight:600}
#msg{text-align:center;margin-top:10px;color:var(--muted);min-height:1.4em}
</style></head><body><main>
<h1 id="h"></h1><p id="d"></p><div id="list"></div>
<button id="save"></button><div id="msg"></div>
</main><script>
const fr=(navigator.language||'').toLowerCase().startsWith('fr');
const L=fr?{h:'Outils du menu',d:'Cochez les outils à afficher sur la balance et rangez-les avec les flèches.',save:'Enregistrer',ok:'Enregistré, la balance a mis son menu à jour.',err:'Échec : ',always:'Toujours affiché, toujours en premier'}
:{h:'Menu tools',d:'Tick the tools the scale shows and order them with the arrows.',save:'Save',ok:'Saved, the scale has updated its menu.',err:'Failed: ',always:'Always shown, always first'};
document.getElementById('h').textContent=L.h;document.getElementById('d').textContent=L.d;
const save=document.getElementById('save'),msg=document.getElementById('msg'),list=document.getElementById('list');
save.textContent=L.save;let tools=[];
function draw(){list.textContent='';tools.forEach((t,k)=>{const r=document.createElement('div');r.className='row';
const c=document.createElement('input');c.type='checkbox';c.checked=t.shown;c.disabled=k===0;c.onchange=()=>{t.shown=c.checked};
const x=document.createElement('div');x.className='t';const n=document.createElement('div');n.className='n';n.textContent=t.name;
const s=document.createElement('div');s.className='s';s.textContent=k===0?L.always:t.sub;x.append(n,s);r.append(c,x);
if(k>0){const u=document.createElement('button');u.textContent='▲';u.disabled=k<2;u.onclick=()=>{[tools[k-1],tools[k]]=[tools[k],tools[k-1]];draw()};
const d=document.createElement('button');d.textContent='▼';d.disabled=k===tools.length-1;d.onclick=()=>{[tools[k+1],tools[k]]=[tools[k],tools[k+1]];draw()};r.append(u,d)}
list.append(r)})}
async function load(){const r=await fetch('/api/tools',{cache:'no-store'});tools=(await r.json()).tools;draw()}
save.onclick=async()=>{msg.textContent='';
const body=new URLSearchParams({shown:tools.filter(t=>t.shown).map(t=>t.id).join(','),order:tools.map(t=>t.id).join(',')});
try{const r=await fetch('/api/tools',{method:'POST',body});if(!r.ok)throw new Error(await r.text());msg.textContent=L.ok;await load()}catch(e){msg.textContent=L.err+e.message}};
load().catch(e=>{msg.textContent=L.err+e.message});
</script></body></html>)HTML";
