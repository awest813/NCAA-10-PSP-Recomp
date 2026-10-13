# Static hosting with a local NCAA ISO

Build the NCAA web target using the [fresh-checkout instructions](../README.md#build-ncaa-from-a-fresh-checkout), then run from the repository root:

```sh
python scripts/prepare_static_ncaa.py --output build/static-ncaa10
```

The packager verifies the NCAA executable, checks the Emscripten preload, and copies only the required web files and notices. Duplicate filenames, overlapping ranges, unlisted bytes and stale local-ISO helpers are rejected. Choose a new output directory for each package; it refuses to overwrite an existing one. Optional `--build` and `--game-root` select different local build/input directories. The output must stay below `build/`.

Upload the **contents** of that directory to an HTTPS static host. Keep `index.html`, `index.js`, `index.wasm`, `index.data`, `local_iso.js`, `static_bootstrap.js`, `coi-serviceworker.js`, notices and licenses together. Root and subdirectory deployments use relative paths. `static-manifest.json` records file sizes and hashes; `HOSTING.md` travels with the package.

This game-specific AOT package contains compiled NCAA code and the verified decrypted boot preload. It contains no full ISO, extracted disc assets or saves. Game-derived build files stay outside the source repository. The current implementation still requires that verified boot preload; selecting an ISO does not compile or decrypt an arbitrary PSP title in the browser.

## Host requirements

Use HTTPS, serve `.wasm` as `application/wasm`, and avoid rewriting missing files to `index.html`. Threaded WebAssembly needs [cross-origin isolation](https://developer.mozilla.org/en-US/docs/Web/API/Window/crossOriginIsolated). Configure these response headers when available:

```text
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
Cross-Origin-Resource-Policy: same-origin
```

The included `_headers` file follows the format supported by [Cloudflare Pages](https://developers.cloudflare.com/pages/configuration/headers/) and Netlify. For hosts such as GitHub Pages that do not offer these custom headers, the included pinned [coi-serviceworker](https://github.com/gzuidhof/coi-serviceworker) supplies the same-origin headers and the bootstrap reloads once before starting the engine. Service workers must be allowed. An existing unrelated service worker in the same scope should be removed or assigned a separate scope. `.nojekyll` is included for Pages uploads; this project does not automatically publish a site.

Preview with the normal header-enabled server:

```sh
python scripts/serve.py build/static-ncaa10 8622
```

Or test the service-worker fallback with a plain static server:

```sh
python -m http.server 8623 --bind 127.0.0.1 --directory build/static-ncaa10
```

Open `http://127.0.0.1:8623/`. Use localhost or HTTPS; `file://` does not work. No `/disc` mount, Range server or application backend is needed.

## Player flow and saves

1. Open the hosted page and choose an uncompressed NCAA Football 10 PSP US ISO (`ULUS10440`). ZIP, CSO and ZSO are unsupported in the picker.
2. The loader indexes ISO9660, validates PARAM.SFO and checks the executable revision against the build. An incorrect or unreadable image stays at the picker with an error.
3. Press Enter for PSP Start at the title and information screen. Z is Cross, X is Circle; the page lists the remaining controls.

The selected ISO stays on the device. Reads use `File.slice()` for requested file/sector ranges and feed the existing bounded disc cache; the whole ISO is not loaded into WebAssembly memory or uploaded. Choose the file again after reloading. Saves remain in browser storage for that origin. Export saves before changing domain, port, browser profile or clearing site data.

## Verification

```sh
python scripts/test_static_package.py
node scripts/test_local_iso.mjs
```

These synthetic tests cover package integrity and preload coverage, malformed ISO/revision rejection, file and raw-sector ranges, startup gating and the actual JS disc bridge. ISO directory budgets are enforced before reading; unsupported interleaving and records crossing sector boundaries are rejected. Bootstrap tests cover stalled registration, timer/listener cleanup, blocked storage and reload loops. Real game smoke evidence is recorded in [ENGINEERING_LOG.md](../ENGINEERING_LOG.md); a short gameplay pass does not establish every mode or browser combination.
