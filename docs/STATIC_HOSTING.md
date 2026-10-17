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

The included `_headers` file follows the format supported by [Cloudflare Pages](https://developers.cloudflare.com/pages/configuration/headers/) and Netlify. For hosts such as GitHub Pages that do not offer these custom headers, the included pinned [coi-serviceworker](https://github.com/gzuidhof/coi-serviceworker) supplies the same-origin headers and the bootstrap reloads once before starting the engine. Service workers must be allowed. An existing unrelated service worker in the same scope should be removed or assigned a separate scope. `.nojekyll` is included for Pages uploads. The repository's own [documentation site](#documentation-site) never publishes a game package; its [launcher](#launcher) runs a package you import into your browser instead.

Preview with the normal header-enabled server:

```sh
python scripts/serve.py build/static-ncaa10 8622
```

Or test the service-worker fallback with a plain static server:

```sh
python -m http.server 8623 --bind 127.0.0.1 --directory build/static-ncaa10
```

Open `http://127.0.0.1:8623/`. Use localhost or HTTPS; `file://` does not work. No `/disc` mount, Range server or application backend is needed.

## Launcher

The GitHub Pages site includes a launcher at `play/` (for this repository, <https://awest813.github.io/NCAA-10-PSP-Recomp/play/>). It lets a static host run **your own** build with **your own** ISO without uploading either and without the host carrying any game code:

1. Build NCAA and run `scripts/prepare_static_ncaa.py` as above.
2. Open the launcher and choose that package folder (or select its files). The launcher reads `static-manifest.json`, requires disc ID `ULUS10440` and the core files (`index.html`, `index.js`, `index.wasm`, `index.data`, `local_iso.js`, `static_bootstrap.js`), checks every selected file's size and SHA-256 against the manifest, the Wasm header and the local-ISO page markers, then stores the verified files in this browser's Cache Storage. Files the manifest does not list, such as an ISO left in the folder, are never read or stored. A plain multi-file selection loses folder names, so optional `licenses/` entries are skipped.
3. Press **Launch**. `launcher-sw.js` serves the imported files under `play/game/` with COOP/COEP/CORP headers, so the threaded build is cross-origin isolated on hosts without custom headers. Missing game files return 404 instead of falling back to the network. The package's own page then asks for the ISO and runs its usual title and executable-revision checks.

The import persists until **Remove imported build** or site data is cleared, so later visits only need the ISO. Import a new package after rebuilding. A hard reload that bypasses the service worker loads the published `play/game/` stub, which returns to the launcher. Saves belong to the hosting origin, as for any static host. Import only packages you built: an imported page runs with that origin's storage. The launcher holds each file in memory once while hashing it, which needs room for the full `index.wasm`.

The launcher source is `profile/web/launcher/`. It works on any static HTTPS or localhost host that publishes those files together; it does not need the documentation site.

## Documentation site

`.github/workflows/pages.yml` publishes the project documentation and the launcher to GitHub Pages on pushes to `main` that change Markdown, `docs/`, `profile/web/launcher/`, the assembler or the workflow, and on manual dispatch. `scripts/build_pages_site.py` copies only the tracked top-level Markdown documents, `docs/*.md`, the two labeled God of War demo media files and the four launcher files into a Jekyll source under `build/`. Links between those pages become site links; links to other source files point to GitHub at the built commit. Each page is Liquid-quoted, and the home page states that the site does not host the game.

The site contains no game executable, generated translation, compiled WebAssembly, boot preload, disc data or saves. Enable it once under the repository's **Settings → Pages → Build and deployment → Source: GitHub Actions**. Check the assembly and launcher locally with:

```sh
python scripts/test_pages_site.py
python scripts/build_pages_site.py --output build/pages-site-check --strict
node scripts/test_web_launcher.mjs
node scripts/test_web_launcher_browser.mjs
```

The browser test needs Playwright with Chromium. It serves a synthetic package from a plain server below a Pages-style repository path, without isolation headers. It checks tamper rejection, the isolated cached launch (Wasm streaming, data, a worker and `SharedArrayBuffer`), the real local ISO picker with a synthetic ISO, reload persistence and removal. It does not run the real game; verify an actual build with a real ISO before relying on a deployment.

## Player flow and saves

1. Open the hosted page and choose an uncompressed NCAA Football 10 PSP US ISO (`ULUS10440`). ZIP, CSO and ZSO are unsupported in the picker.
2. The loader indexes ISO9660, validates PARAM.SFO and checks the executable revision against the build. The page identifies disc indexing, title validation and executable-revision checks as they run. An incorrect or unreadable image stays at the picker with an error and permits another selection.
3. Press Enter for PSP Start at the title and information screen. Z is Cross, X is Circle; the page lists the remaining controls.

The selected ISO stays on the device. Reads use `File.slice()` for requested file/sector ranges and feed the existing bounded disc cache; the whole ISO is not loaded into WebAssembly memory or uploaded. Choose the file again after reloading. Saves remain in browser storage for that origin. Export saves before changing domain, port, browser profile or clearing site data.

If browser storage cannot restore saves, startup continues with an on-page warning. Automatic writeback stays disabled for that session to protect unrestored browser data. Export any new in-memory saves before reloading. Failed writes also leave a visible warning; a successful later write clears it. Autosaves and imports share a serialized persistence queue. Imports wait for earlier saves before replacing files. Export requires restoration/import completion and reports read errors; a failed read is not reported as an empty save directory. An export is the backup to keep when storage is unavailable or full.

## Verification

```sh
python scripts/test_static_package.py
node scripts/test_local_iso.mjs
node scripts/test_web_saves.mjs
node scripts/test_web_persistence.mjs
```

These synthetic tests cover package integrity and preload coverage, malformed ISO/revision rejection, file and raw-sector ranges, startup gating and the actual JS disc bridge. ISO directory budgets are enforced before reading; unsupported interleaving and records crossing sector boundaries are rejected. Bootstrap tests cover stalled registration, timer/listener cleanup, blocked storage and reload loops. Real game smoke evidence is recorded in [ENGINEERING_LOG.md](../ENGINEERING_LOG.md); a short gameplay pass does not establish every mode or browser combination.
