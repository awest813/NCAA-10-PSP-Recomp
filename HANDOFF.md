# NCAA Football 10 PSP Web Recomp — handoff

Snapshot: 2026-10-13. Read [ENGINEERING_LOG.md](ENGINEERING_LOG.md) for verified history and [AGENTS.md](AGENTS.md) before editing.

## Start here

For a fresh clone, work from the repository root and follow the [README build instructions](README.md#build-ncaa-from-a-fresh-checkout). The local development paths and session evidence below describe the original machine. Its active game/debug project is the inner directory:

```text
C:/Users/allen/Downloads/psp-web-recomp-main/psp-web-recomp-main
```

The outer directory is a wrapper. `PSPRecomp/profiles/web` is a junction to this project's `profile` directory. The active game/debug directory has no Git checkout; preserve existing files and do not assume a clean baseline or reset it.

GitHub publication details are in [GITHUB_READY.md](GITHUB_READY.md). The source checkout is `build/github-ready-20261012`, with `main` tracking `origin/main` at `awest813/NCAA-10-PSP-Recomp`. The initial audited snapshot was published on 2026-10-12 as [f0a57cb](https://github.com/awest813/NCAA-10-PSP-Recomp/commit/f0a57cb05c295ad56f6effe22313a64b0d31e4e9), and its remote SHA was verified. The manifest beside that folder identifies the preparation baseline; subsequent documentation updates are recorded in Git. The earlier `build/github-ready` is a preliminary audit copy. Seven framework patches now reproduce the relevant local sources from the pinned upstream base, with clean-framework and standalone NCAA codegen tests passing.

This is NCAA Football 10 PSP US (`ULUS10440`) statically recompiled to C++/WebAssembly. It uses the verified executable and NCAA assets with the inherited PSP web runtime. `NCAA2007` is its internal ELF module name. Both existing build caches select `ncaa10`; do not switch to a God of War corpus to obtain a successful build.

## Latest source audit

The 2026-10-13 storage audit fixes startup dependency cleanup when browser storage throws or reports restore failure. Startup continues with a persistent on-page warning; automatic writeback is blocked after failed restoration to protect unrestored data. Autosaves and imports now share a serialized persistence queue. Failed writes leave a visible export warning; successful retries clear it. Synthetic tests cover dependency release, delayed callbacks, quota errors, retry recovery, debounce and failed-restore protection. This does not establish isolation from guest file mutations during import or actual browser quota behavior.

Latest local package: `build/static-ncaa10-save-polish` (14 companion files). Header-enabled isolated preview: `http://127.0.0.1:8628/`. The verified local ISO boots with 64 MiB guest RAM. The fresh origin imported `build/ncaa-gaps-dynasty-week-two-saves.zip`; all 12 exported file hashes match both immediately and after browser reload. Evidence: `build/ncaa-save-polish-roundtrip.json`, `build/ncaa-save-polish-imported.zip`, `build/ncaa-save-polish-reloaded.zip`. After reselecting the ISO and pressing Start, loading NCGAPW2 reopened the week-two Air Force schedule versus Kansas State on September 4. Capture: `build/ncaa-save-polish-dynasty-restored.jpg`. The plain-server fallback on 8629 reached the ISO picker; that temporary server was stopped after the check. Original origins and backups remain intact.

Validation: seven browser/debugger regression scripts, ten native suites, seven source-workflow cases and eight package cases pass. The frontend rebuilt successfully (`build/ncaa-save-polish-web-build.log`). CI includes the new persistence regression. No guest instruction, allocator or memory-default change. Reference framebuffer capture, starvation, full-match/mode coverage and original 32 MiB divergence remain open. No controller is connected. See [remaining gaps](docs/REMAINING_GAPS.md).

For static-host preparation, follow [docs/STATIC_HOSTING.md](docs/STATIC_HOSTING.md). Run `python scripts/prepare_static_ncaa.py --output build/static-ncaa10` after the web build, using a new directory. The package includes title-specific Wasm and verified boot data, with no full ISO, extracted assets or saves. Build outputs remain excluded from Git.

## Prior verified checkpoints

These are historical sessions and evidence; inspect the current UI before interacting. Detailed checks and limitations are in [ENGINEERING_LOG.md](ENGINEERING_LOG.md).

- `build/static-ncaa10-gaps-2`, header-enabled origin 8626: imported NCAAYR2, simulated week one, saved NCGAPW2 to isolated slot 000 and reopened the week-two Air Force schedule after reload. All 12 progressed export hashes match; NCAAWEB slot 001 remains byte-identical. Original NCAAYR2 remains in its backup ZIP. Evidence: `build/ncaa-gaps-dynasty-week-two-reloaded.jpg`, `build/ncaa-gaps-dynasty-progression-report.json`. The plain-server worker check on 8627 passed; that temporary server was subsequently stopped.
- `build/static-ncaa10-polished`, plain subdirectory origin 8624/polished: UK away / FSU home at DAY, manual kickoff and two defensive plays, paused at third-and-14 on the 28, Q1 4:25. `build/static-polished-uk-fsu-replay.jpg` shows the completed play's instant replay. Header-enabled origin 8625 reached the picker.
- `build/static-ncaa10-hosting`, plain origin 8623: selected the matching local ISO, pressed Start, cold-loaded UK–FSU and manually snapped a run. Paused at second-and-14 on the 30, Q1 4:38. Evidence: `build/static-iso-uk-fsu-gameplay.jpg`. Worker isolation and package-only requests passed without a disc mount. Subdirectory origin 8624/ncaa verified picker readiness and invalid-file recovery.
- Source fixes c6e2fea and documentation e7adb71 passed GitHub source checks; latest prior run: https://github.com/awest813/NCAA-10-PSP-Recomp/actions/runs/38334738369. Save ZIP integrity/rollback, AudioWorklet chunk boundaries and unticketed PPSSPP completion handling have synthetic coverage. PPSSPP framebuffer capture still fails with `Could not download output`; no valid reference comparison is claimed.

## Current working baseline

- NCAA defaults to 64 MiB guest RAM / 56 MiB user partition in native and web builds. `?ram=32` and `PSPWEB_RAM_MB=32` retain the comparison layout.
- UK–FSU DAY gameplay works in short manual checks. Preserved gameplay and Dynasty checkpoints are listed above; browser tabs/server state may change, so inspect before acting.
- Latest checks: rebuilt web frontend; ten native suites, seven browser/debugger scripts, seven source-workflow cases and eight package cases passed. Earlier native/web baseline logs use the `build/ncaa-default64-` prefix.
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
