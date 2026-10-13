/* Start the threaded build only after the page is cross-origin isolated.
 * The same-origin service worker provides headers on hosts such as Pages. */
(async function () {
  'use strict';
  const runtime = document.currentScript.dataset.runtime;
  const status = document.getElementById('status');
  const reloadKey = 'ncaa-isolation-reload:' + location.pathname;
  try {
    if (!globalThis.crossOriginIsolated) {
      if (!globalThis.isSecureContext || !navigator.serviceWorker) {
        throw new Error('Open this site over HTTPS or localhost, with service workers enabled.');
      }
      if (sessionStorage.getItem(reloadKey)) {
        throw new Error('This host did not enable browser isolation. Serve COOP: same-origin and COEP: require-corp headers.');
      }
      status.textContent = 'Preparing browser support…';
      const workerURL = new URL('coi-serviceworker.js', location.href).href;
      await navigator.serviceWorker.register(workerURL, { scope: './' });
      await Promise.race([
        new Promise(resolve => {
          const changed = () => {
            if (navigator.serviceWorker.controller && navigator.serviceWorker.controller.scriptURL === workerURL) {
              navigator.serviceWorker.removeEventListener('controllerchange', changed);
              resolve();
            }
          };
          navigator.serviceWorker.addEventListener('controllerchange', changed);
          changed();
        }),
        new Promise((_, reject) => setTimeout(() => reject(new Error('Browser support setup timed out. Reload to retry.')), 15000)),
      ]);
      sessionStorage.setItem(reloadKey, '1');
      location.reload();
      return;
    }
    // Header-enabled hosts do not need storage for the one-reload guard.
    try { sessionStorage.removeItem(reloadKey); } catch (_) {}
    const script = document.createElement('script');
    script.src = runtime;
    script.async = true;
    script.onerror = () => { status.textContent = 'Could not load the game engine. Reload to retry.'; status.className = 'status err'; };
    document.body.appendChild(script);
  } catch (error) {
    status.textContent = error.message;
    status.className = 'status err';
  }
})();
