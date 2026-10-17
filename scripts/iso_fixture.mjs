// Synthetic NCAA-shaped ISO9660 image for browser-side tests; contains no game data.
import { createHash } from 'node:crypto';
const encoder = new TextEncoder();
export function sfo(discId) {
  const fields = [['TITLE', 'NCAA® FOOTBALL 10'], ['DISC_ID', discId]];
  const keys = fields.map(([key]) => encoder.encode(key + '\0'));
  const values = fields.map(([, value]) => encoder.encode(value + '\0'));
  const keyStart = 20 + fields.length * 16, valueStart = keyStart + keys.reduce((sum, key) => sum + key.length, 0);
  const bytes = new Uint8Array(valueStart + values.reduce((sum, value) => sum + value.length, 0));
  const view = new DataView(bytes.buffer);
  view.setUint32(0, 0x46535000, true);
  view.setUint32(8, keyStart, true); view.setUint32(12, valueStart, true); view.setUint32(16, fields.length, true);
  let keyAt = 0, valueAt = 0;
  fields.forEach((_, i) => {
    const at = 20 + i * 16;
    view.setUint16(at, keyAt, true); view.setUint16(at + 2, 0x204, true);
    view.setUint32(at + 4, values[i].length, true); view.setUint32(at + 8, values[i].length, true);
    view.setUint32(at + 12, valueAt, true);
    bytes.set(keys[i], keyStart + keyAt); bytes.set(values[i], valueStart + valueAt);
    keyAt += keys[i].length; valueAt += values[i].length;
  });
  return bytes;
}
export function record(name, sector, size, directory = false) {
  const raw = typeof name === 'number' ? new Uint8Array([name]) : encoder.encode(name);
  const bytes = new Uint8Array((33 + raw.length + 1) & ~1), view = new DataView(bytes.buffer);
  bytes[0] = bytes.length; bytes[25] = directory ? 2 : 0; bytes[32] = raw.length;
  view.setUint32(2, sector, true); view.setUint32(10, size, true); bytes.set(raw, 33);
  return bytes;
}
export const boot = encoder.encode('synthetic executable fixture, not a PSP game');
export const config = { ebootHashes: [createHash('sha256').update(boot).digest('hex')] };
export function fixture(id = 'ULUS10440') {
  const payload = Uint8Array.from({ length: 2 * 256 * 1024 + 31 }, (_, i) => i % 251);
  const bytes = new Uint8Array(25 * 2048 + payload.length), view = new DataView(bytes.buffer);
  bytes.set([1, ...encoder.encode('CD001'), 1], 16 * 2048); view.setUint16(16 * 2048 + 128, 2048, true);
  bytes.set(record(0, 20, 2048, true), 16 * 2048 + 156);
  const metadata = sfo(id);
  function directory(sector, entries) {
    let at = sector * 2048;
    for (const entry of entries) { bytes.set(entry, at); at += entry.length; }
  }
  directory(20, [record(0, 20, 2048, true), record(1, 20, 2048, true), record('PSP_GAME', 21, 2048, true)]);
  directory(21, [record('PARAM.SFO;1', 23, metadata.length), record('SYSDIR', 22, 2048, true), record('DATA.BIN;1', 25, payload.length)]);
  directory(22, [record('EBOOT.BIN;1', 24, boot.length)]);
  bytes.set(metadata, 23 * 2048); bytes.set(boot, 24 * 2048); bytes.set(payload, 25 * 2048);
  return { bytes, payload, file: new File([bytes], 'NCAA-test.iso') };
}
