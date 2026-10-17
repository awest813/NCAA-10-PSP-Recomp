/* Runs the translator off the page thread. The selected ISO File is read in
 * ranges here; results come back as a ZIP Blob. */
'use strict';
importScripts('local_iso.js', 'pspweb_translator.js', 'translator.js');

let compiled = null;
function createModule(options) {
  if (!compiled) compiled = WebAssembly.compileStreaming(fetch('pspweb_translator.wasm'));
  return createPspTranslator({
    ...options,
    instantiateWasm(imports, success) {
      compiled.then(module => WebAssembly.instantiate(module, imports).then(instance => success(instance, module)));
      return {};
    },
  });
}

self.onmessage = async event => {
  const started = performance.now();
  try {
    const result = await PspTranslator.translateIso(event.data.file, {
      createModule, progress: text => self.postMessage({ type: 'progress', text }),
    });
    self.postMessage({ type: 'progress', text: 'Packing the ZIP…' });
    const blob = new Blob(await PspTranslator.zip(result.files), { type: 'application/zip' });
    self.postMessage({ type: 'done', blob, provenance: result.provenance, seconds: (performance.now() - started) / 1000 });
  } catch (error) {
    self.postMessage({ type: 'error', text: error.message || String(error) });
  }
};
