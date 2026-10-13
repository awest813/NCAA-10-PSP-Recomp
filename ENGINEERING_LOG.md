# NCAA Football 10 PSP Web Recomp — engineering log

Last updated: 2026-10-12. This is a retrospective of verified work and the current baseline. Earlier milestones are grouped by engineering dependency; they are not assigned invented completion dates. Append future entries with the change, reason, validation, evidence and remaining limitations.

## Project identity

This project recompiles NCAA Football 10's PSP executable ahead of time into C++, then WebAssembly. The browser runs that corpus with the local PSP HLE kernel, GE/WebGL2 renderer, audio and browser services. The native executable is a validation tool for the same port.

| Identity check | Verified value |
| --- | --- |
| Disc title | `NCAA® FOOTBALL 10` |
| Disc ID | `ULUS10440` |
| PSP system version in metadata | `5.50` |
| Decrypted executable | `games/ncaa10/root/EBOOT.BIN`, 8,415,696 bytes |
| Executable SHA-256 | `2b743dc6d746d822969a9b967b13422a369ff03c6a3a3e2604e051c1642b94f6` |
| Internal ELF module name | `NCAA2007` |
| Build profile / title | `PSPRECOMP_PROFILE=web`, `PSPWEB_GAME=ncaa10` |
| Guest executable path | `disc0:/PSP_GAME/SYSDIR/EBOOT.BIN` |
| Executable corpus | `profile/generated/ncaa10` |
| Audio program corpus | `profile/generated/ncaa10_sound` |
| Browser output | `build/web-ncaa10/profiles/web/index.html` and its JS/Wasm/data companions |

The verifier was rerun successfully for this documentation update. Both current CMake caches select `ncaa10`. `profile/CMakeLists.txt` links its generated corpus and adds `PSPWEB_NCAA10` only for that title. The host loads the ELF, applies relocations and registers generated entries; the compiled gameplay path does not interpret PSP CPU instructions at runtime.

The inherited God of War runtime supplies host infrastructure. Its demo and performance figures are background, not NCAA evidence. References to other recomp projects do not mean their game code was used here.

## Milestone 1 — executable and runtime bring-up

The port required corrected UMD sector addressing, 64-bit seek argument alignment, raw UMD reads, Windows manifest handling, kernel thread/callback behavior and exported startup-thread stack sizing. Honoring NCAA's 1 KiB startup stack avoided unnecessarily consuming the guest partition.

Compiler work included VFPU `vi2s` packing and static translation of executable EA audio-bank programs. Relocated guest audio entries bind to those precompiled functions. The compiler patch is preserved in `patches/0006-ncaa10-vfpu-and-aot-audio.patch`; current working framework sources are under `PSPRecomp`.

The optional intro movie is skipped because AVC playback is unsupported. A title-specific display wait replacement is guarded by its verified caller and handler state. These adaptations are documented in [the detailed NCAA guide](docs/NCAA10.md); gameplay reaching the field does not imply every PSP service is complete.

## Milestone 2 — interactive gameplay, controls and saves

Browser runs reached kickoff, manually snapped runs and passes, defense, penalties, statistics and Instant Replay. Keyboard and standard gamepad mapping have regression coverage, including neutral rearm, focus loss and disconnect. Physical Bluetooth-controller validation remains separate from synthetic Gamepad API tests.

Spring Drills Rushing Attack completed ten repetitions. Dynasty completed a simulated season, offseason and return to the 2010 preseason. Recruiting, budgets, training, depth-chart changes, save/export/reload and subsequent live gameplay were exercised. Full mode and branch coverage remains incomplete.

The `NCAAYR2` preseason checkpoint is in `build/ncaa10-dynasty-year-two-saves.zip`; its manifest and browser/native reload evidence are beside it. Earlier checkpoints and the original `NCAAWEB` baseline were preserved. See the chronological evidence in [docs/NCAA10.md](docs/NCAA10.md), rather than assuming every listed feature passed a reference-accuracy comparison.

## Milestone 3 — cold DAY crash provenance

The failing Fresno State–San Diego State Qualcomm DAY load was traced from allocator rejection through descriptor fields, reader stack and registers to `0x08C7EE50`, instruction `0xACA40000` (`sw a0, 0(a1)`). The allocator deliberately returned zero; the reader propagated it correctly. The final store was recorded as a fault before writing memory.

The request was `0x39480` (234,624 bytes), while the largest free span was `0x391C0` (233,920 bytes): a raw shortfall of 704 bytes. Headers and alignment further constrain usable payload. Aggregate usable free memory was 397,280 bytes, with intact metadata and no adjacent free nodes awaiting simple coalescing. The failure is a contiguity/layout problem; the first divergence causing the failing layout remains unresolved.

Initial 32 MiB MAIN headers matched the PPSSPP reference. This verifies initialization only, not the complete failing load's allocation order. Optimized/unoptimized and logger-enabled/disabled runs reproduced the fault. No zero-page mapping, fabricated buffer, skipped store or EA heap-header patch was introduced. Details: [crash provenance](docs/NCAA10-CRASH-PROVENANCE.md).

## Milestone 4 — 64 MiB default and audit (2026-10-12)

NCAA native and browser builds now default to real 64 MiB guest RAM / 56 MiB user partition. The game sizes MAIN itself from SysMem: its physical interval grows from 16,991,184 to 32,759,760 bytes. The core runtime and other titles retain their defaults. Explicit `32` and `64` overrides are supported; invalid values fail visibly.

The reproduced native cold DAY checkpoint had the same 424 live allocations and 16,558,638 bytes of requested payload in both layouts. In 64 MiB mode, the formerly rejected allocation returned `0x09E7BF00`; the match reached kickoff and return. This is a tested mitigation for the original 32 MiB failure.

The audit made framebuffer RAM size explicit and tested upper-memory cached/uncached AOT byte, halfword and word access, partition capacity, exhaustion/release and the public display import. Default and explicit 64 MiB initial heap headers match byte-for-byte. Explicit 32 MiB preserves the earlier layout. Invalid native `48`/`bad` options and the browser error path were checked.

Validation recorded in `build/ncaa-default64-*`:

- Native and web production builds succeeded; detailed instruction tracing is disabled in both caches.
- All ten host suites and the browser audio, OSK and input scripts passed.
- Fresh browser startup at `?profile`, without `ram`, reported 64 MiB / 56 MiB.
- UK away / FSU home loaded at Doak Campbell Stadium DAY. A manually snapped Ace-Big / 16 Power O completed to second down.
- Local live handoff screenshot `build/ncaa-default64-uk-fsu-live-gameplay.jpg` reported 30.1 FPS / 101% speed; the browser log records 15 accumulated audio underruns. The in-game play clock was temporarily disabled for manual inspection, then restored to On.

Primary audit artifacts are `build/ncaa-default64-host-tests.log`, `ncaa-default64-native-matrix.json`, `ncaa-default64-initial-layout-comparison.json`, `ncaa-default64-default-heap-audit.json` and `ncaa-default64-uk-fsu-browser.log`.

## Current unresolved work

1. Establish repeatable PSP/PPSSPP gameplay and rendering comparisons. Reference screenshot acquisition has not yet produced a verified visual baseline.
2. Explain the original 32 MiB cold DAY allocation-order/layout difference.
3. Measure sustained pacing and eliminate audio starvation, especially during heavy Dynasty processing. Short settled 30 FPS samples do not prove a lock.
4. Verify physical controllers, Bluetooth reconnect, browser/device performance and remaining save and mode branches.

The working performance target for NCAA gameplay is approximately 30 presented FPS at 100% game speed. The PSP Vblank rate and other titles' 60 FPS measurements are different quantities.

## Documentation update (2026-10-12)

Added this engineering log, [handoff](HANDOFF.md), [agent instructions](AGENTS.md) and [other-recomp guide](OTHER_RECOMPS.md). Reframed the README around NCAA while retaining clearly labeled upstream history. This update changes documentation only; prior build/test results above remain attributed to their saved logs.

Documentation validation checked 23 local links with no missing targets, matching source/preloaded executable hashes, both title/profile caches and the WebAssembly file header. The runtime dispatch source stops on an unregistered compiled entry rather than falling back to CPU interpretation. The result is saved in `build/ncaa-documentation-audit.json`. An outer-wrapper `AGENTS.md` routes future sessions to the inner project instructions.

## GitHub preparation (2026-10-12)

The target is `awest813/NCAA-10-PSP-Recomp`. The connected API reported a public, empty repository. Preparation is local; no remote files or branches were written.

Added `patches/0007-runtime-instruction-provenance.patch` to preserve framework changes outside the ignored local checkout. All seven patches applied to a clean export of pinned base `f6e7d415c7f447b934cc3865a31eb725f353d659` and reproduced the seven relevant working source files. That clean framework built and passed CTest. The standalone NCAA codegen regression also passed after its display calls were updated to supply explicit RAM size, matching the audited host API. Evidence is under `build/github-framework-audit-20261012`.

Added publication allowlist/credential-pattern checks, a non-overwriting source packager, LF attributes, broader local-output exclusions, third-party notices and `GITHUB_READY.md`. The preliminary 126-file source snapshot passed the audit and all three browser regression scripts. The final snapshot is `build/github-ready-20261012`, with a ZIP and per-file hash manifest alongside it. It includes source, documentation, licenses and the labeled upstream demo, while excluding game files, generated game translations, SDKs, guest dumps and savedata. An independent Git index is prepared there for review; the active game/debug workspace remains intact.

A clean compiler reconstruction is verified. A fresh full game build from the upload snapshot, GitHub-hosted automation and remote publication are not claimed by this preparation.

## GitHub publication (2026-10-12)

Following the user's instruction to commit and push to main, the 126-file audited source snapshot was committed as [f0a57cb](https://github.com/awest813/NCAA-10-PSP-Recomp/commit/f0a57cb05c295ad56f6effe22313a64b0d31e4e9) and pushed normally to `origin/main`. The connected GitHub API confirmed the same SHA remotely. The tracked source audit and whitespace checks passed before publication. Game files, generated game translations, SDKs, build outputs, dumps and saves remain local. The active game/debug workspace was preserved.

Updated the publication and handoff documents to reflect the completed push. The existing ZIP and hash manifest remain dated preparation artifacts; Git is the record of these subsequent documentation changes. No additional runtime build or gameplay test was required for this documentation update.

## Source audit and polish (2026-10-12)

Audited the published source against the active NCAA workspace. Fixed the fresh-checkout README to clone this repository, distinguish local evidence from source-only clones, and show Windows and Linux NCAA web commands. Agent/handoff instructions now distinguish an ordinary Git clone from the original nested game/debug workspace. Corrected inherited text encoding and the browser subtitle's inaccurate claim that no emulation is involved; gameplay remains AOT with PSP services implemented by HLE.

The Bash NCAA generation path now verifies the exact title/executable and generates EA audio programs as well as the main corpus. The web port path also prepares `__umd.iso`. A shared Python helper unpacks ZIP input to actual ISO bytes, checks existing images by SHA-256, preserves mismatched images and removes incomplete temporary output on failure. Windows builds use this helper, resolve the SDK path and clear stale instruction-trace cache flags. Empty audio-bank input now fails explicitly. The ISO extractor now terminates with an error on truncated file data instead of looping forever.

Setup pins the full framework revision, uses LF checkouts, matches the verified Emscripten 6.0.10 SDK and creates a real profile junction under Windows Git Bash. Added a source-only GitHub Actions workflow for publication checks, synthetic source/browser tests and reconstruction/testing of the patched compiler.

Local validation passed: seven synthetic source-workflow tests; all ten existing host suites; all three browser regression scripts; the clean framework's CTest suite; the standalone NCAA codegen regression; Python compilation; Bash and PowerShell syntax; and 34 local documentation links. All seven framework patches also replayed successfully using `git am` against an isolated LF export of the pinned upstream base. The initial archive replay inherited Windows CRLF conversion; the LF replay matches setup's explicit checkout policy. Framework tests used a workspace temporary directory after the Windows sandbox rejected renames in its default temporary directory.

The updated Windows web script successfully rebuilt the existing NCAA production target with the verified 1,805,936,640-byte raw image (SHA-256 `4dedbb8436a20551bd4b3f08c208a1acecf9d5c47d4037bc4cb615bab21a3c08`). Both production trace cache values are empty. Evidence: `build/ncaa-source-polish-web-build.log` and `build/ncaa-source-polish-audit.json`. The raw image was verified and retained, not replaced. No guest instruction, allocator or 64 MiB default changed.

This audit does not add a new gameplay/reference-accuracy claim. A fresh full game build on Linux and first-time setup on another Windows machine remain unverified. The GitHub Actions result is recorded separately once available; no game data is used in that workflow.

## Static hosting and browser ISO loading (2026-10-12)

Added a browser local-ISO path to the existing AOT web runtime. `profile/web/local_iso.js` lazily indexes ISO9660, checks the NCAA US disc ID and executable revision, and supplies exact file/raw UMD ranges via `File.slice()`. The selected ISO stays local; no full-image read, upload or HTTP fallback occurs in this mode. Startup waits for a verified ISO, retries invalid selections, and writes a distinct local manifest to avoid racing the preload. The existing 256 MiB disc cache remains bounded; NCAA guest RAM remains 64 MiB. The normal development build retains HTTP streaming, with corrected HTTP 200 range offsets.

`scripts/prepare_static_ncaa.py` packages the verified web build below ignored `build/`, refusing overwrite and additional preloaded assets/state. It checks Wasm and boot-preload integrity, adds the picker configuration, relative bootstrap, headers, manifest and licenses. The package includes NCAA-specific Wasm and decrypted boot preload; it excludes the full ISO, extracted assets and saves. This is still a game-specific AOT build. No browser executable decryption or generic PSP compilation is claimed.

Static hosts with configured COOP/COEP start directly. The plain-host fallback uses unmodified MIT `coi-serviceworker`, pinned at `7b1d2a092d0d2dd2b7270b6f12f13605de26f214`, and reloads once before starting threaded Wasm. Sources and notices are retained. Added [static-hosting instructions](docs/STATIC_HOSTING.md), reusable-port guidance and source CI coverage.

The final package on the header-enabled preview `http://127.0.0.1:8622/` also reached the picker. Bootstrap regressions cover this direct path with blocked session storage, subdirectory worker registration, startup isolation gating and reload-loop rejection.

Validation: updated native/web targets built successfully (`build/ncaa-local-iso-native-build.log`, `build/ncaa-local-iso-web-build.log`). Ten native host suites, four browser scripts, seven source-workflow tests and four static-package tests passed. New synthetic tests exercise malformed/cyclic/truncated ISO handling, title/revision rejection, file/raw/tail ranges, picker gating/retry, actual EM_JS local-only bridge, HTTP 200 offsets, package hashes and rejection of extra preloads/wrong boot/invalid bounds/Wasm. Source allowlist and Python syntax audits passed.

Real browser evidence: plain `python -m http.server` on 8623, with neither isolation headers nor a disc mount, booted from the matching selected ISO. Pressed Start, selected UK away/FSU home, received the kickoff and manually snapped a run. Paused at second-and-14 on the 30, first quarter, 4:38. The page displayed approximately 30 FPS/100% speed during this short pass. Screenshot: `build/static-iso-uk-fsu-gameplay.jpg`. The server log contains only HTML, helpers, worker, JS, data and Wasm requests; no `/disc` requests. Subdirectory preview `http://127.0.0.1:8624/ncaa/` reached the picker through service-worker isolation and recovered from an invalid selected file. Original 8621 session/saves were untouched. The latest package is `build/static-ncaa10-hosting`; its manifest records companion hashes. No public deployment or cross-browser/device compatibility claim is made. Original allocator divergence and audio/pacing limitations remain as documented above.

## Static package audit and polish (2026-10-12)

Reproduced package validation failures with synthetic fixtures before fixing them: duplicate preload filenames could overwrite the verified executable; overlapping ranges and unlisted trailing bytes were accepted. The packager now validates metadata types, unique allowed filenames and complete contiguous range coverage. It also rejects a stale built ISO helper, rewrites notices to packaged license files and source links, accurately describes the included game Wasm/boot data, and provides a preview command usable from any package directory. Eight package tests now pass.

Browser isolation setup now includes service-worker registration in its 15-second timeout and cleans up the timer and controller listener on every exit. ISO indexing enforces its aggregate directory budget before reading and rejects interleaved files, extended attributes and records crossing sectors. Synthetic browser regressions cover stalled/failed registration, cleanup, malformed directory records and the pre-read budget. No guest instructions, allocator behavior or 64 MiB guest RAM / 56 MiB user partition default changed.

Validation passed: exact NCAA title/executable verification; ten native host suites; four browser regression scripts; seven source-workflow cases; eight package cases; Python compilation; source allowlist and whitespace checks. The existing web target regenerated its frontend files successfully; unchanged C++/Wasm required no relink (`build/ncaa-static-polish-web-build.log`). The new `build/static-ncaa10-polished` package has 14 verified companion hashes and valid packaged license links. Original packages and sessions remain intact.

Real browser regression used the matching local ISO on the plain subdirectory host `http://127.0.0.1:8624/polished/`, without server isolation headers or a disc mount. Pressed Start, cold-loaded UK away / FSU home at DAY, manually kicked off and completed two defensive plays. The game is paused at third-and-14 on the 28, first quarter, 4:25. `build/static-polished-uk-fsu-replay.jpg` captures the instant replay of the completed defensive play, not a live frame. The page reported approximately 30 FPS / 100% speed during this short test and its browser error log was empty. Server requests were limited to the package files, with no disc requests. Header-enabled preview `http://127.0.0.1:8625/` also reached the ISO picker. These checks do not establish sustained pacing, reference accuracy, physical-controller compatibility or a fix for the original 32 MiB allocation divergence. No public deployment was performed.

## Save integrity, resampling and reference tooling (2026-10-12)

Hardened the actual browser save parser/import path: local/central ZIP agreement, bounds, CRC32, stored/deflate size checks, bounded streaming expansion, overlapping entry rejection and safe unique destination paths. Imports validate before writing, snapshot overwritten files, roll back write failures and restore in-memory files if browser persistence reports failure. Startup and concurrent-import guards prevent premature/overlapping imports. Success now follows the sync callback. This does not certify concurrent autosaves or recovery from a filesystem that also fails rollback.

Real browser evidence on a separate origin (8626): imported the two-slot Dynasty/baseline export, exported it with all six hashes matching, rejected a deliberately bad-CRC synthetic ZIP, reloaded the rebuilt package and exported again. All six hashes still match and no corrupt fixture file exists. Reports and exports remain under ignored `build/ncaa-gaps-*`; CRC rejection capture is `build/ncaa-gaps-save-crc-rejection.jpg`. Original gameplay origins and save backups were preserved.

Reproduced non-finite AudioWorklet output when low-rate playback crossed multiple one-frame chunks. Consume every crossed chunk while preserving interpolation phase. Also bound a single oversized chunk to the existing 250 ms cap. Tests compare identical stereo ramps delivered contiguously and one frame at a time at 22.05/44.1/48/96 kHz, alongside the existing ten-second drift, reset and starvation checks. This fixes sample/chunk handling, not browser main-thread starvation or audio fidelity.

Fixed the local PPSSPP debugger client's completion protocol against the official v1.20.4 CPU/stepping sources: pause, resume, step and run-until complete through unticketed broadcasts. Already-paused/running commands query status first; a failed command in a batch no longer skips its following resume. Synthetic protocol tests pass, and actual pause/resume succeeds without the old false timeout. Both tested reference backends still fail output/current-framebuffer capture with `Could not download output`; no valid visual reference is claimed.

Validation: title/executable verification; ten native host suites; six browser/debugger regression scripts; seven source-workflow and eight package cases; source allowlist. Rebuilt frontend successfully (`build/ncaa-gaps-final-web-build.log`) and prepared `build/static-ncaa10-gaps-2` with 14 companion files. CI now runs save and reference-client regressions. No guest instruction, allocator or RAM-default change. Hardware controller testing is unavailable because the user reports no connected controller. Remaining acceptance checks are tracked in `docs/REMAINING_GAPS.md`.

The final header-enabled package reloaded and booted from the matching ISO; the plain server on 8627 reached the picker through service-worker isolation. Save hash evidence was collected before further menu interaction. At that checkpoint, loading the Dynasty save through the game was still pending; the following progressed-save entry records its subsequent validation.

## Progressed Dynasty save round trip (2026-10-12)

The rebuilt package on isolated origin 8626 loaded imported NCAAYR2 into the 2010 Dynasty preseason. Simulated through week one to week two (Air Force bye, then September 4 versus Kansas State). Saved as NCGAPW2 into the isolated copy of slot 000, retaining the original NCAAYR2 backup ZIP and all original browser origins. NCAAWEB slot 001 remains byte-identical. Exported the progressed files, reloaded the browser, selected the matching ISO, pressed Start, loaded NCGAPW2 and reopened the same week-two schedule. All 12 exported files have identical SHA-256 values before/after reload. The export also contains test profile/settings slots 011/012 created on this isolated origin.

Evidence: `build/ncaa-gaps-dynasty-week-two-saves.zip`, `build/ncaa-gaps-dynasty-week-two-reloaded.zip`, `build/ncaa-gaps-dynasty-progression-report.json`, `build/ncaa-gaps-dynasty-week-two-reloaded.jpg`, and `build/ncaa-gaps-dynasty-final-log.txt`. Audio underruns occurred during simulation/menu work; this does not close pacing or audio fidelity. No full-match or exhaustive mode claim. The PPSSPP reference process started for this pass was stopped after its capture failures, before later browser checks.

Source commit c6e2fea6bc7a32e1781ad5579493080dba28e33c was pushed to main and its remote SHA verified. GitHub source checks succeeded: https://github.com/awest813/NCAA-10-PSP-Recomp/actions/runs/38333739065. No game artifacts or save files were published.

## Browser storage audit and handoff polish (2026-10-13)

Reproduced a startup dependency leak when IDBFS restoration throws synchronously: the previous catch logged the error without releasing the registered dependency. Restoration now completes exactly once across synchronous failures, callback errors and late callbacks. The game can continue with in-memory saves and a persistent export warning. After failed restoration, writeback stays disabled for that session so unrestored browser files are not overwritten.

Autosave failures previously appeared only in the console, and autosaves/imports could overlap persistence transactions. Both now use one Promise queue with the existing 300 ms autosave debounce. Failed writes leave an on-page export warning; later successful writes clear it, and a rejected operation does not poison the queue. Save import still rolls back in-memory files after persistence failure. This does not establish isolation from guest file mutations during an asynchronous import or guarantee rollback when the underlying filesystem also fails.

New actual-shell-source regression `scripts/test_web_persistence.mjs` covers startup dependency release, asynchronous restore, late callbacks, serialized writes, quota errors, synchronous write exceptions, retry recovery, debounce and failed-restore write protection. The existing import test exercises the shared persistence implementation. CI runs the new regression. Updated static-host save guidance and consolidated HANDOFF into current evidence plus identified historical checkpoints.

Validation passed: exact NCAA identity/executable verification; ten native suites; seven browser/debugger scripts; seven source-workflow cases; eight static-package cases; source allowlist. Rebuilt the frontend (`build/ncaa-save-polish-web-build.log`) and prepared `build/static-ncaa10-save-polish` with 14 companions. No guest code, allocator semantics or 64 MiB RAM / 56 MiB user partition default changed.

Real browser check on fresh header-enabled origin 8628 imported the 12-file progressed Dynasty backup, exported it, reloaded and exported again. All 12 SHA-256 values match at each stage. Reselected the matching local ISO, pressed Start at the title/information screens, loaded NCGAPW2 and reopened the week-two Air Force schedule (September 4 versus Kansas State). No browser errors were captured. Evidence: `build/ncaa-save-polish-roundtrip.json`, `build/ncaa-save-polish-imported.zip`, `build/ncaa-save-polish-reloaded.zip`, `build/ncaa-save-polish-dynasty-restored.jpg`. The plain server on 8629 reached the picker through worker isolation; capture `build/ncaa-save-polish-static-picker.jpg`. That temporary server/tab was closed; 8628 remains at the restored schedule. Original origins and backup exports were preserved.

Storage error injection was synthetic, not a real browser quota exhaustion test. This pass did not repeat live UK–FSU gameplay or establish sustained pacing, reference accuracy, full-match/mode coverage, physical-controller behavior or the original 32 MiB allocator divergence. Remaining acceptance checks stay in `docs/REMAINING_GAPS.md`.

## Launcher, ISO and storage follow-up (2026-10-13)

Reproduced imported-file mutation while a preceding autosave transaction remained pending. Waiting for the shared persistence queue before applying the import prevents that earlier write from seeing uncommitted imported files. Imports also reject unavailable storage before mutation. This addresses ordering against preceding persistence operations, not concurrent guest file mutation during an asynchronous import.

Export now guards pending restoration/import and reports filesystem read errors instead of silently showing `No saves yet`. Engine aborts clear guest readiness and show a reload/retry message. ISO loading reports index, title and revision-check stages; a retry clears the earlier picker status while validation runs. Existing identity/hash gates and local range reads remain intact.

Actual-source regressions cover the previously failing import/autosave case, export readiness/busy/read errors, unavailable import storage, engine abort state and ISO progress. Seven browser/debugger scripts, ten native suites, seven source-workflow cases and eight static-package cases pass. NCAA executable identity verification passed. Rebuilt frontend (`build/ncaa-launcher-final-web-build.log`) and prepared `build/static-ncaa10-launcher-final` with 14 companion files. Source allowlist passes. No guest allocator/instruction/RAM-default changes.

A fresh plain-server origin 8631 rejected a non-ISO selection and accepted the matching NCAA ISO on retry through service-worker isolation. It imported the 12-file progressed Dynasty backup. Original, immediate-export and restored-export SHA-256 dictionaries all match. The browser download permission was retried after the user requested it. Evidence: `build/ncaa-launcher-audit-save-roundtrip.json`, `build/ncaa-launcher-audit-imported.zip`, `build/ncaa-launcher-audit-reloaded.zip`. The session continued into a stadium scene; `build/ncaa-launcher-audit-restored.jpg` captures its current state. Final checking-status reset was added after the initial browser preview and validated by the helper regressions/build. Original game origins and backup ZIPs remain preserved.

## Audio device clock and overrun diagnostics (2026-10-13)

Removed the explicit 44.1 kHz AudioContext request: the worklet already converts 44.1 kHz PSP PCM to the selected device graph rate. The tested default is 48 kHz. The old producer-rate diagnostic divided received frames by rendered frames; a stalled render thread therefore looked like excess guest production. Both producer/render rates now use wall time. Queue overruns and discarded source frames are visible separately from underruns. Trimming a partially consumed chunk subtracts whole source frames rather than its fractional interpolation phase; regression verifies exact discarded-frame accounting.

Synthetic tests cover actual context construction and stats delivery, reset-counter handling, whole-frame drop conservation, existing stereo continuity and 22.05/44.1/48/96 kHz resampling. On origin 8630, settled main-menu counters stayed unchanged over a 28.6-second sample. At Indiana–Purdue coin toss the counters were six underruns/133 overruns (2,909 ms dropped); those totals stayed unchanged through the final stadium/play-selection sample about 11 minutes later. This was not a full manually played match. Startup/loading had generated gaps and drops before that interval. A separate 8631 session later reported roughly 43.4 kHz input against 33.3/48 kHz rendering and further drops, so the native-rate change does not resolve all browser/device render starvation. After restart, the latest stadium capture shows seven underruns/zero overruns at roughly 44/48 kHz. Do not combine totals across sessions or claim audible/reference equivalence.

Local evidence: `build/ncaa-audio-device-baseline.txt`, `build/ncaa-audio-device-samples.json`, `build/ncaa-audio-device-stadium.jpg`. Queue limits remain 250 ms, priming remains 100 ms; no larger buffer was introduced. Render starvation, loading stalls and audible crackling/gaps remain open acceptance work.

## Supplied PPSSPP debug evidence (2026-10-13)

Inspected the user-specified Downloads debug folder without modifying its originals. ISO SHA-256 exactly matches the project. Copied only `.ppdmp`, `.sym` and `.ppmap` into ignored `build/reference-user-debug-20261013`; `inspection.json` records hashes and structural results. The version-6 dump identifies ULUS10440. Both Zstandard blocks decode to exact declared lengths; all 25,773 payload ranges and record types validate; no trailing bytes. Gzip symbol-map entries resolve exactly to the 25,554 `.sym` entries. The fault lies in the exported function at 0x08C7ECFC; the exports do not explain the original 704-byte allocator shortfall.

Obtained an actual PPSSPP Georgia–Georgia Tech gameplay window capture (`build/reference-ppsspp/ncaa-visible-window.png`) after earlier debugger framebuffer failures. Prepared a separate model-1 reference profile (`build/reference-compare-20261013`, port 8673); read-only debugger checks confirmed v1.20.4 and active NCAA2007 module from the game. Native GPU-dump replay and matching scene comparison remain unverified. Updated handoff and acceptance evidence accordingly. This documentation/evidence pass changes no runtime, allocator, RAM default or generated instructions; no game-derived payload is published. Detailed protocol/format sources and next checks are in `docs/PPSSPP_REFERENCE.md`.

## Matched SDSU rendering and live reference heap (2026-10-13)

Preserved the user's new daytime SDSU GPU dump and symbol exports under `build/reference-sdsu-day-20261013`. Version-6 ULUS10440 dump validation passes with 32,074 records and 3,146,028 pushbuffer bytes. Native PPSSPP replay shows Auburn–SDSU play selection, 1st & 25, Q1 4:57. The local experimental replay harness executes the same captured data through the existing Ge/GeGl source (5,216 primitives, 3,752 draws). It is graphics diagnostics only, with no guest CPU fallback. Native window capture succeeds; debugger framebuffer readback still fails.

Partial renders narrowed the field difference to detail passes between primitives 100 and 500. The old GL mapping collapsed PSP doubled-alpha blend factors into ordinary alpha. For fixed-zero source and destination factor 6/7 under add/reverse-subtract, the renderer now composes the exact destination modulation from two normalized terms. Synthetic actual-display-list tests failed before the fix (expected 0x604020, got 0x302010) and pass after, including alpha above 0.5 and inverse factors. The separate integer-depth fixture failed GLES equality while passing software; fragment depth now truncates to 16-bit integer precision before host comparison. That change barely alters this frame. The destination-alpha correction lowers matched window-derived RGB MAE from 13.7183 to 6.7470; field ROI from 15.2612 to 5.3173. Residual texture/edge differences and other complex blend combinations remain open. This is not proof of full rendering accuracy.

Debugger helper now selects a validated loopback port, explicitly requests encoded PNG data for `savePNGAs`, rejects raw pixels mislabeled PNG and saves raw GPU format/dimension/flip metadata. Synthetic tests reproduce the old hardcoded-port/capture problems and pass after correction. Ten native suites, seven browser/debugger scripts and eight package cases pass. Rebuilt web frontend and prepared `build/static-ncaa10-sdsu-render` (14 companions); build log `build/ncaa-sdsu-render-web-build.log`. No allocator/instruction/RAM-default changes.

At the user's invitation, inspected the live SDSU reference's memory view and used its read-only debugger for a consistent RAM snapshot. Mapping verifies 32 MiB RAM. The brief pause/capture/resume completed. MAIN physical bounds and size match the original browser pool: 0x08EC9010–0x09EFD3E0, 16,991,184 bytes. Analyzer reports 427 nodes, 420 allocations, five free blocks, 642,752 total usable free bytes, largest 601,040, valid physical/free-list links and membership, and no adjacent free pairs. No live node requests 0x39480. This post-load Auburn–SDSU state cannot establish the first divergence during the earlier failed cold Fresno State–SDSU load. Evidence stays local under `build/reference-ppsspp/sdsu-live-*`.

The new plain-server origin 8632 accepted the matching local ISO, pressed Start through the title/information screens, and reached live Utah–Utah State play. The console confirms the default 64 MiB guest RAM / 56 MiB user partition. A sampled fumble play reported 30.3 FPS / 99% speed; settled stadium samples were about 30 FPS / 100%. Evidence: `build/ncaa-sdsu-render-browser-gameplay.jpg`, `build/ncaa-sdsu-render-browser-console.json`. The native comparison is the matched SDSU frame; this separate browser match is a short smoke test. Four underruns/118 overruns accumulated during startup/menu transitions and were still present at the gameplay sample. No sustained full-match, audible-equivalence or broad-mode claim is made.
