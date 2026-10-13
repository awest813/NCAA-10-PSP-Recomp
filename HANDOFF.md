# NCAA Football 10 PSP Web Recomp — handoff

Snapshot: 2026-10-12. Read [ENGINEERING_LOG.md](ENGINEERING_LOG.md) for verified history and [AGENTS.md](AGENTS.md) before editing.

## Start here

For a fresh clone, work from the repository root and follow the [README build instructions](README.md#build-ncaa-from-a-fresh-checkout). The local development paths and session evidence below describe the original machine. Its active game/debug project is the inner directory:

```text
C:/Users/allen/Downloads/psp-web-recomp-main/psp-web-recomp-main
```

The outer directory is a wrapper. `PSPRecomp/profiles/web` is a junction to this project's `profile` directory. The active game/debug directory has no Git checkout; preserve existing files and do not assume a clean baseline or reset it.

GitHub publication details are in [GITHUB_READY.md](GITHUB_READY.md). The source checkout is `build/github-ready-20261012`, with `main` tracking `origin/main` at `awest813/NCAA-10-PSP-Recomp`. The initial audited snapshot was published on 2026-10-12 as [f0a57cb](https://github.com/awest813/NCAA-10-PSP-Recomp/commit/f0a57cb05c295ad56f6effe22313a64b0d31e4e9), and its remote SHA was verified. The manifest beside that folder identifies the preparation baseline; subsequent documentation updates are recorded in Git. The earlier `build/github-ready` is a preliminary audit copy. Seven framework patches now reproduce the relevant local sources from the pinned upstream base, with clean-framework and standalone NCAA codegen tests passing.

This is NCAA Football 10 PSP US (`ULUS10440`) statically recompiled to C++/WebAssembly. It uses the verified executable and NCAA assets with the inherited PSP web runtime. `NCAA2007` is its internal ELF module name. Both existing build caches select `ncaa10`; do not switch to a God of War corpus to obtain a successful build.

## Latest source audit

The latest gap pass adds save ZIP integrity/path/size checks, transactional write rollback, persistence-error reporting and startup/concurrent-import guards. The AudioWorklet now advances across every consumed chunk and caps a single oversized chunk; stereo-ramp regressions cover 22.05/44.1/48/96 kHz. PPSSPP debugger commands now await their documented unticketed completions and continue to resume after a failed capture. Six browser/debugger scripts, ten native suites, seven source-workflow cases and eight package cases pass.

Latest local package: `build/static-ncaa10-gaps-2`; header-enabled isolated preview: `http://127.0.0.1:8626/`; plain preview: `http://127.0.0.1:8627/`. On origin 8626, importing `build/ncaa10-dynasty-year-two-saves.zip`, rejecting `build/ncaa-invalid-save-crc.zip`, reloading and exporting retained all six original SHA-256 values. Evidence: `build/ncaa-gaps-save-roundtrip.json`, `build/ncaa-gaps-reloaded-saves.zip`, `build/ncaa-gaps-save-crc-rejection.jpg`. Existing origins/saves were preserved. No controller is connected; physical hardware remains untested. See [remaining gaps](docs/REMAINING_GAPS.md) for acceptance checks. Reference framebuffer capture, starvation, full-match/mode coverage and original 32 MiB divergence remain open. The rebuilt package also loaded NCAAYR2, simulated through week one, saved NCGAPW2 to isolated-origin slot 000 and reopened the same week-two schedule after browser reload. All 12 exported files match before/after reload; NCAAWEB slot 001 remains byte-identical. Original NCAAYR2 stays in its backup ZIP. Latest session on 8626 is at the week-two Air Force schedule. Evidence: `build/ncaa-gaps-dynasty-week-two-reloaded.jpg`, `build/ncaa-gaps-dynasty-progression-report.json`, `build/ncaa-gaps-dynasty-week-two-reloaded.zip`. Source fixes c6e2fea passed [GitHub source checks](https://github.com/awest813/NCAA-10-PSP-Recomp/actions/runs/38333739065).


Static-host preparation is implemented. Follow [docs/STATIC_HOSTING.md](docs/STATIC_HOSTING.md). Run `python scripts/prepare_static_ncaa.py --output build/static-ncaa10` after the web build; choose a new directory if it exists. The latest local delivery folder is `build/static-ncaa10-polished`, with companion `HOSTING.md` and `static-manifest.json`. It includes game-specific Wasm and verified boot data, but no full ISO, extracted assets or saves. Build outputs remain excluded from Git.

The follow-up static audit fixed duplicate/overlapping/unlisted preloads, stale ISO helper packaging, notices links and package-local preview commands. Worker setup now times out stalled registration and cleans up listeners/timers. ISO indexing checks the directory byte budget before reading and rejects unsupported interleaving or records crossing sectors. Ten native suites, four browser scripts, seven source-workflow cases and eight package cases pass. The refreshed header-enabled preview is `http://127.0.0.1:8625/`; the plain subdirectory preview is `http://127.0.0.1:8624/polished/`. Prior packages and gameplay sessions are retained.

The polished package cold-loaded UK away / FSU home at DAY from the matching local ISO, completed a manual kickoff and two defensive plays, and is paused at third-and-14 on the 28, first quarter, 4:25. Capture `build/static-polished-uk-fsu-replay.jpg` shows the instant replay of the completed defensive play. The browser error log was empty; the plain server received package requests only, with no disc mount or disc requests. Header-enabled startup also reached the picker. See the latest engineering entry for details and limitations.

The plain static preview at `http://127.0.0.1:8623/` loaded the matching local ISO, pressed Start, cold-loaded UK–FSU and completed a manually snapped run. It is paused at second-and-14 on the 30, first quarter, 4:38. Capture: `build/static-iso-uk-fsu-gameplay.jpg`. Its server provides no isolation headers and no disc mount; the service-worker fallback succeeded, and no `/disc` requests appeared. A separate preview at `http://127.0.0.1:8624/ncaa/` verified relative resources, worker isolation, picker readiness and invalid-file recovery under a subdirectory. Original 8621 session/saves were preserved. Public hosting and cross-browser/device validation remain untested.

Initial local-ISO validation passed all ten native host suites, four browser regression scripts, seven source-workflow tests and four static-package tests. Native/web targets rebuilt successfully. `test_local_iso.mjs` checks the actual WebFs JS bridge, including local-only reads and HTTP 200 offsets. CI includes the new synthetic tests. The normal development shell retains HTTP disc streaming; static packaging enables the picker and separate local manifest.

The fresh-checkout commands are now in the README and clone this NCAA repository. Bash NCAA generation includes identity checks and EA audio; web preparation includes verified raw UMD data. Windows setup uses a junction and LF framework checkout; the SDK pin matches Emscripten 6.0.10. `scripts/test_source_workflow.py` provides seven game-free ISO/ZIP/error regressions. `.github/workflows/source-checks.yml` checks source, browser helpers and the patched compiler without game data.

The existing Windows NCAA web target was rebuilt successfully with the updated script. All ten host suites, three browser scripts, clean framework tests and standalone NCAA codegen tests passed. Production instruction-trace settings remain empty. See the engineering log and local `build/ncaa-source-polish-audit.json` for evidence and limitations. Saves and the existing paused gameplay session were preserved.

## Current working baseline

- NCAA defaults to 64 MiB guest RAM / 56 MiB user partition in native and web builds. `?ram=32` and `PSPWEB_RAM_MB=32` retain the comparison layout.
- UK–FSU DAY gameplay works in the default browser build, including a manually snapped run. The last checked session was paused at second-and-20 on the 23, first quarter, 4:44. Browser tabs/server state may change; inspect before acting.
- Native/web builds passed, ten host tests passed and three browser regression scripts passed. Logs use the `build/ncaa-default64-` prefix.
- Detailed instruction-trace unit lists are empty in both production caches.
- Audio underruns, sustained pacing and reference accuracy remain open. The 64 MiB layout mitigates a reproduced cold DAY allocation failure; it does not explain the original 32 MiB layout divergence.

## Verify, build and serve

Run these commands from the active game/debug project. Use installed tools on another machine; the explicit paths below describe this machine.

```powershell
C:/Python314/python.exe scripts/verify_ncaa10.py games/ncaa10/root
$env:PATH = 'C:/Users/allen/.local/share/retcomm/toolchains/cmake-clang-v1/latest/bin;' + $env:PATH
./scripts/build_ncaa_web.ps1 -Iso 'NACC_Football_10_USA_PSP-PSPKiNG.iso' -Emsdk C:/Users/allen/emsdk -Python C:/Python314/python.exe -Jobs 3
```

The existing decrypted `games/ncaa10/root/EBOOT.BIN` is required. `-Generate` regenerates the executable and audio corpora; use it for compiler changes, not routine host/documentation changes. The script verifies the executable, prepares the manifest/raw UMD image and builds the page.

For the existing native configuration:

```powershell
cmake --build build/native-ncaa10 -j 3
ctest --test-dir build/native-ncaa10/profiles/web --output-on-failure
& 'C:/Users/allen/emsdk/node/24.19.0_64bit/node.exe' scripts/test_web_audio.mjs
& 'C:/Users/allen/emsdk/node/24.19.0_64bit/node.exe' scripts/test_web_osk.mjs
& 'C:/Users/allen/emsdk/node/24.19.0_64bit/node.exe' scripts/test_web_input.mjs
```

Reuse the server on 8621 if it still serves this build. Otherwise:

```powershell
C:/Python314/python.exe scripts/serve.py build/web-ncaa10/profiles/web 8621 --mount /disc=games/ncaa10/root/disc
```

Open `http://127.0.0.1:8621/` or `?profile`. The server provides cross-origin isolation and Range support. Keep `disc/__umd.iso`, extracted files and the manifest intact. Opening HTML directly is insufficient. Changing the origin/port changes the browser save-storage location.

## Enter gameplay

Enter is PSP Start. Press it at the title and the Road to Glory information screen. Existing settings may bypass the favorite-team chooser. Use Z for Cross, X for Circle, A for Square, S for Triangle; arrows are D-pad, I/J/K/L are analog, Q/W are L/R.

Choose Play Now, select teams, then confirm controller assignment with a second Cross. Confirm stadium DAY visually. In the tested FSU-home selection, Kentucky was reached by selecting the away side and moving down from Florida; inspect the team name rather than trusting a fixed input count. Use coin-toss choices and play calls as displayed. For manual kickoff, pull analog back (K), then forward (I).

Inspection delays can exhaust the in-game play clock. Pause while inspecting. If temporarily disabling the play clock for an input check, record that condition and restore On afterward. Do not change PSP timing to make a test pass.

## Evidence and saves to preserve

| Artifact | Meaning |
| --- | --- |
| `build/ncaa-default64-uk-fsu-live-gameplay.jpg` | Fresh default-mode manual handoff |
| `build/ncaa-default64-uk-fsu-browser.log` | 64 MiB startup and timing/audio observations |
| `build/ncaa-default64-initial-layout-comparison.json` | Default/explicit 64 and retained 32 initial headers |
| `build/ncaa-ram64-day-target-audit.json` | Successful 64 MiB cold DAY heap checkpoint |
| `build/ncaa-main-heap-day-failure-audit.json` | Original 32 MiB failure audit |
| `build/ncaa10-dynasty-year-two-saves.zip` | `NCAAYR2` 2010 preseason checkpoint |
| `build/ncaa10-dynasty-year-two-save-manifest.json` | Exported save hashes |
| `build/ncaa10-browser-test-saves.zip` | Original backed-up test saves |

Latest tested browser slot 000 was `NCAAYR2`; original `NCAAWEB` slot 001 was preserved. Export and hash before replacing save fixtures. Confirm current contents rather than assuming the snapshot is still current. Savedata details and earlier checkpoints are in [docs/NCAA10.md](docs/NCAA10.md).

## Next debugging work

Follow the user's priority order: gameplay/render accuracy, native-feeling behavior, pacing, audio, controls/Bluetooth, saves, browser performance, then complete Dynasty and other-mode coverage.

For accuracy, first obtain repeatable reference scenes with the same executable, teams, stadium, time, RAM mode and inputs. Official PPSSPP v1.20.4 is available under `build/reference-ppsspp/portable`; the purpose-built debugger client is `scripts/debug_ppsspp_reference.mjs`. Prior GPU screenshot commands failed, and raw VRAM guesses did not yield verified images. Do not label these as a completed visual comparison. No reference process was left running at the previous cleanup.

For the 32 MiB issue, compare initialization and allocation/free ordering before the rejected `0x39480` request. The allocator-to-reader null chain is already established; another final-crash-only trace adds little. Use `scripts/analyze_ncaa_heap.py` on complete 32/64 MiB captures and retain request IDs/last writers. Diagnostic setup and exact units are in [crash provenance](docs/NCAA10-CRASH-PROVENANCE.md). Clear trace units and the optimization override before the production regression.

For pacing/audio, use matched live scenes and a long sample. Separate guest step duration, callback gaps, GE queue delay, presentation cadence and audio starvation. Dynasty heavy processing already has reproducible long steps. Preserve real game logic while diagnosing them.
