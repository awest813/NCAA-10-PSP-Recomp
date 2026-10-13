// Verify the actual reference CLI against the documented asynchronous protocol.
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {resolve,sep} from 'node:path';
import vm from 'node:vm';
const source = readFileSync(new URL('./debug_ppsspp_reference.mjs',import.meta.url),'utf8').replace(/^import .*;\r?\n/gm,'');
let stepping = false, output = [], commands = [], captures = [];
class Socket {
  handlers = {};
  constructor() { queueMicrotask(() => this.emit('open',{})); }
  addEventListener(name,fn) { (this.handlers[name] ||= []).push(fn); }
  emit(name,value) { for (const fn of this.handlers[name] || []) fn(value); }
  send(json) {
    const req = JSON.parse(json); commands.push(req.event);
    let reply = {event:req.event,ticket:req.ticket};
    if (req.event === 'version') reply.version='synthetic';
    if (req.event === 'cpu.status') reply.stepping=stepping;
    if (req.event === 'cpu.stepping' || req.event === 'cpu.resume' || req.event === 'cpu.stepInto') {
      stepping = req.event !== 'cpu.resume';
      reply = {event:stepping ? 'cpu.stepping' : 'cpu.resume'}; // No ticket, per PPSSPP.
    }
    if (req.event === 'gpu.buffer.renderColor') reply = {event:'error',ticket:req.ticket,message:'Could not download output'};
    if (req.event === 'memory.read') reply.base64 = 'AQID';
    queueMicrotask(() => this.emit('message',{data:JSON.stringify(reply)}));
  }
  close() { this.closed = true; }
}
const script = new vm.Script('(async function(){'+source+'})()');
const context = vm.createContext({WebSocket:Socket,resolve,sep,Buffer,setTimeout,clearTimeout,
  process:{argv:['node','script','synthetic.jsonl'],stdin:{}},
  console:{log(line) { output.push(JSON.parse(line)); }},
  async appendFile() {}, async writeFile(path,bytes) { captures.push([path,[...bytes]]); },
  createInterface() { return (async function*() {
    yield JSON.stringify([{event:'cpu.stepping'},{event:'cpu.stepping'},{event:'cpu.stepInto'},
      {event:'gpu.buffer.renderColor'},{event:'cpu.resume'},{event:'cpu.resume'},
      {event:'memory.read',address:0,size:3,saveBase64As:'synthetic.bin'}]);
  })(); },
});
await script.runInContext(context);
assert.deepEqual(commands, ['version','cpu.status','cpu.stepping','cpu.status','cpu.stepInto',
  'gpu.buffer.renderColor','cpu.status','cpu.resume','cpu.status','memory.read']);
assert(output.some(item => item.error === 'Could not download output'));
assert(!output.some(item => item.error?.includes('timeout')));
assert.equal(stepping,false,'failed capture must still execute the following resume');
assert.deepEqual(captures[0][1],[1,2,3]);
console.log('PASS: unticketed pause/step/resume events, already-paused/running guards, failed-capture resume and binary capture');
