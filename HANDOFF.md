# NCAA Football 10 PSP Web Recomp — handoff

Snapshot: 2026-10-14. Read [ENGINEERING_LOG.md](ENGINEERING_LOG.md) for verified history and [AGENTS.md](AGENTS.md) before editing.

## Start here

For a fresh clone, work from the repository root and follow the [README build instructions](README.md#build-ncaa-from-a-fresh-checkout). The local development paths and session evidence below describe the original machine. Its active game/debug project is the inner directory:

```text
C:/Users/allen/Downloads/psp-web-recomp-main/psp-web-recomp-main
```

The outer directory is a wrapper. `PSPRecomp/profiles/web` is a junction to this project's `profile` directory. The active game/debug directory has no Git checkout; preserve existing files and do not assume a clean baseline or reset it.

GitHub publication details are in [GITHUB_READY.md](GITHUB_READY.md). The source checkout is `build/github-ready-20261012`, with `main` tracking `origin/main` at `awest813/NCAA-10-PSP-Recomp`. The initial audited snapshot was published on 2026-10-12 as [f0a57cb](https://github.com/awest813/NCAA-10-PSP-Recomp/commit/f0a57cb05c295ad56f6effe22313a64b0d31e4e9), and its remote SHA was verified. The manifest beside that folder identifies the preparation baseline; subsequent documentation updates are recorded in Git. The earlier `build/github-ready` is a preliminary audit copy. Seven framework patches now reproduce the relevant local sources from the pinned upstream base, with clean-framework and standalone NCAA codegen tests passing.

This is NCAA Football 10 PSP US (`ULUS10440`) statically recompiled to C++/WebAssembly. It uses the verified executable and NCAA assets with the inherited PSP web runtime. `NCAA2007` is its internal ELF module name. Both existing build caches select `ncaa10`; do not switch to a God of War corpus to obtain a successful build.

## Latest rendering and reference evidence

### Current interpolated-color checkpoint (2026-10-14)

The GT pause-menu discrepancy begins in background draw **4093**, before text draw **4100**. The opaque gradient is 70 → 27 over 272 pixels: at `(24,145)`, 46.998 was truncated to 46 while the x64 software reference produces 47. Font draws use nearest filtering with no mip levels. Primary RGBA and secondary RGB now round to nearest-even bytes before texture functions, alpha tests and blending; those later fixed-point divisions still truncate. The pinned x64 rasterizer and optimized pixel path were checked independently. ARM64/generic reference interpolation paths differ, so this is a verified x64-reference match, not complete PSP hardware proof.

Synthetic lists reproduce 46 instead of 47 in both prior backends. New coverage includes RGB/alpha gradients, textured/untextured and blended/unblended paths, exact halfway ties, alpha-test boundaries and secondary colors. A zero-secondary fast path skips unused shader work; vertex-only nonzero → zero → nonzero transitions are tested with unchanged draw state. GPU snapshot/primitive ordering and 64 MiB defaults remain intact.

Full-frame RGB MAE improves SDSU **0.432580 → 0.402295** and GT **1.023075 → 0.883931**. SDSU field ROI remains 0.060150; the GT pause-menu ROI improves **0.487218 → 0**, matching the reference exactly. GT diagnostic prefixes 4094/4100/4101 match that ROI in both pre-conversion RGBA and final RGB565. Earlier GT prefix 4092 slightly worsens **3.440824 → 3.443479** overall; earlier scene accuracy remains open. SDSU prefixes 39/40/45/100/323 are unchanged. Reports/panels: `color-rounding-comparison.json/png`; aggregate, provenance and raw-reference hashes: `build/reference-gt-prefix-20261014/color-rounding-summary.json` and `color-rounding-provenance.json`. Modified prefixes remain diagnostics, not gameplay screenshots; isolated reference closed.

Final O3 ABBA native replay medians versus `0300ab3` are SDSU **168.064 → 168.3595 ms (+0.18%)**, GT **140.333 → 143.5235 ms (+2.27%)**. Maximums are 261.170 → 311.266 ms and 148.060 → 152.189 ms. This includes parser/CPU submission/presentation/GPU completion and excludes final reads; it is not browser FPS. Repeated final images match the single-frame comparisons byte for byte. Draws/copies/area remain 1,982 / 919 / 618,114 pixels and 1,930 / 840 / 833,528 pixels. `color-rounding-final-performance.json` retains samples and limitations; exploratory pre-optimization timing is separate.

Latest package: `build/static-ncaa10-color-rounding-final` (14 companions, verified boot data, no ISO). Final native/web rebuilds, ten native suites, eight package cases, seven source-workflow cases and exact NCAA verification pass. Logs: `build/ncaa-color-rounding-final-native-build.log` and `build/ncaa-color-rounding-final-web-build.log`. Final Wasm contains both byte quantization and the zero-secondary branch. Fresh browser/gameplay/FPS/audio still awaits Chrome app access. Next rendering work: earlier GT/SDSU scene differences, geometry/coverage and fractional sampling, with hardware/alpha/stencil/mask coverage still open. The broader pacing/audio, physical controls, saves/modes/device and original 32 MiB work remains required. AOT execution and 64 MiB / 56 MiB user partition are unchanged.

### Prior ordered shader-blend checkpoint (2026-10-14)

The draw-39 fixture now produces `(114,146,67)` in software/GLES instead of GLES `(114,147,67)`. Both backends truncate separate half-step source/destination products before combining them. GLES uses GPU destination snapshots, then performs dithering, saturation and target RGB packing in the shader. Coverage includes six equations, every factor class and fixed-color aliases, doubled-alpha boundaries, all four target formats and 1x/2x rendering. Reserved equations follow the studied software-reference behavior; this is not hardware proof.

Consecutive triangles with disjoint interiors share a snapshot; overlapping groups execute in order. Shared-edge quads remain one draw. Source-only equations avoid destination copies/splitting unless a GPU texture source requires ordering. Tests cover repeated overlap, disjoint batching, relocated targets, stencil increments affecting subsequent destination alpha, depth rejection/z-fail stencil, alpha-test discard and self-texture feedback. They prohibit CPU downloads during these render paths. Native profiling now reports blend copies/pixels.

Final direct software-reference RGB MAE improves SDSU **0.5854 → 0.4326**, field ROI **0.3196 → 0.0602**; Georgia Tech–Georgia **1.0445 → 1.0231**. Its fixed ROI is pause-menu text, not field: that region worsens **0.0621 → 0.4872** and remains an explicit precision target. Prefix 39 stays exact; prefix 40 improves **0.3552 → 0.0101**, with its field ROI matching exactly. Reports/images: `ordered-blend-comparison.json/png` in each measured capture folder; aggregate `ordered-blend-summary.json`. All seven presented RGBA8 outputs match direct RGB565 readback. These are graphics diagnostics, not fresh gameplay screenshots.

Matched O3 native ABBA replays (16 repeats/run, first two discarded) have median wall times SDSU 168.332 → 172.2645 ms (+2.34%), GT 137.0855 → 140.667 ms (+2.61%). SDSU maximum rises 204.168 → 301.349 ms; GT maximum is 156.285 → 152.020 ms. This includes the diagnostic parser/CPU submission/presentation/GPU completion and excludes final readbacks; it cannot establish browser FPS. Final repeated images match single-frame output. Per-frame destination copies/pixels: 919 / 618,114 and 840 / 833,528. Draw counts fall to 1,982 / 1,930 because batches outside the scissor are not submitted. Timings, raw hashes and limitations: `build/reference-sdsu-prefix-20261014/ordered-blend-performance.json`.

Latest package: `build/static-ncaa10-exact-blend` (14 companions, no ISO). Native/web rebuilds, ten native suites, eight package cases and exact NCAA verification pass; logs `build/ncaa-exact-blend-native-build.log` and `build/ncaa-exact-blend-web-build.log`. Fresh browser gameplay/FPS/audio is still pending Chrome app access. Next rendering work: pause-menu text sampling/coverage and residual scene differences, with broader alpha/stencil/mask precision still open. Sustained pacing/audio, controllers, saves/modes/device coverage and original 32 MiB divergence remain open. Default 64 MiB / 56 MiB user partition and AOT execution are unchanged.

### Prior texture-function and first-divergent-blend checkpoint (2026-10-14)

New diagnostic prefixes narrow the first mismatch to original draw ordinal **39**: prefixes 38/39 (draws through ordinal 37/38) match exactly, then prefix 40 differs. Disabling blending only before draw 39 in a cloned payload reduces pre-conversion source RGB error to one green-channel unit on row 218. A local shader probe exposes source alpha as RGB independently of framebuffer stencil alpha. Separate fixed-point blend products predict 77,245 of 80,160 lower-field reference pixels exactly (96.36%); maximum prediction channel error is two. This supports blend precision as the next target, with remaining sampling/geometry differences still possible. Evidence: `build/reference-sdsu-prefix-20261014/first-divergent-blend-analysis.json`, prefix 38/39/40/45 and `source40`. These are modified diagnostics, not gameplay screenshots. The isolated reference was closed.

Texture modulation, decal, environment blend, replace and add now use byte-channel arithmetic in software/GLES, including RGB doubling before division and undoubled alpha. Tests reproduce software 156 versus expected 157 and an old GLES alpha-test rejection that leaves magenta. A perspective fixture checks constant non-power-of-two byte channels across 256 interior pixels; without the numerical guard it loses one unit. Shader color varyings carry byte units. No framebuffer copies or readbacks were added to production rendering.

**Full-frame comparison worsens slightly:** SDSU corrected RGB MAE changes 0.5737 → **0.5854** (field 0.3164 → 0.3196); Georgia Tech–Georgia changes 1.0227 → **1.0445** (field unchanged at 0.0621). These are targeted texture-stage corrections, not an overall accuracy improvement. Prefix 39 remains exact; prefix 40 changes 0.2824 → 0.3552. Reports/images: `texture-function-comparison.json/png` in the existing capture folders and `texture-function-summary.json` in the prefix folder. RGBA8 presentation equals direct RGB565 output in every measured case. General GPU blend rounding/post-blend dithering/packing remain open; preserve primitive order and measure performance when implementing them.

Latest package: `build/static-ncaa10-texture-functions` (14 companions, no ISO). Native game rebuild, ten native suites, final web build, eight package cases and exact title verification pass. Log: `build/ncaa-texture-function-web-build.log` (final up-to-date check); the final Wasm contains the changed shader and numerical guard. Fresh browser gameplay/FPS/audio remains pending Chrome app access. Default 64 MiB / 56 MiB user partition and AOT execution are unchanged.

### Prior capture correction and blend/presentation checkpoint (2026-10-14)

**Correction:** the earlier native GLES replay harness selected EGL RGBA4444. Its captured images lost color precision, so previous GLES image-error values and related frame-wide color/noise conclusions are superseded. The PPSSPP reference VRAM images, project software direct-framebuffer output and synthetic FBO/guest-memory pixel regressions are unaffected. The local harness now requests/verifies RGBA8 and exports raw guest-format display memory; presented output must match that readback exactly.

Corrected full-frame direct software-reference RGB MAE is **0.5737 for SDSU** (field ROI 0.3164) and **1.0227 for Georgia Tech–Georgia** (field ROI 0.0621). RGBA8 presentation and direct RGB565 readback match byte for byte in both scenes. The SDSU hardware-window comparison is 6.3190 / field 2.9133 and remains a different method. This is a measurement correction, not an improvement caused by today's source fixes. Replaying prior source `e0091fd` through the corrected harness gives identical raw display bytes before/after the RGB565 blend fix. Reports/images: each capture folder's `corrected-capture-comparison.json/png`; older reports carry explicit correction metadata.

Two source bugs are fixed. RGB565 destination-alpha factors now read zero in software/GLES while RGB565 textures remain opaque; the fixture previously stored green `0x07E0` instead of zero. Presentation now unbinds a game sampler before applying nearest, non-mipmapped filtering. Its regression previously returned `(191,64,0)` with an inherited linear sampler and black with an inherited mipmapped sampler instead of red. Native test contexts explicitly request/verify RGBA8.

Diagnostic prefix copies preserve early draws plus original final draw 5215; originals remain intact. Corrected prefix 37, ending after the base field, matches the reference RGB565 display exactly. Prefix 100 has RGB MAE 0.4968 and prefix 323 has 2.5179; later detail/geometry coverage is the next comparison target. Tools, manifests and captures remain local under `build/reference-sdsu-prefix-20261014`; the local replay harness is still experimental. Prefix images are modified diagnostics, not gameplay screenshots. The isolated reference process was closed.

Latest package: `build/static-ncaa10-presentation-alpha`; log: `build/ncaa-presentation-alpha-web-build.log`. Ten native suites, eight package cases, web build and exact NCAA verification pass. Fresh browser/gameplay/FPS remains pending Chrome app access; no new approval was assumed or bypassed. The earlier access request remains pending. GPU post-blend dithering/packing, full hardware accuracy, sustained pacing/audio and broad mode/controller/device coverage remain open. Default 64 MiB / 56 MiB user partition and AOT execution are unchanged.

### Prior ordered-dither checkpoint (GLES capture metrics superseded)

The captured final draw's missing ordered dither is now implemented for GLES unblended draws/clears and software rasterization. Signed matrix values affect RGB before saturation and target packing, including 8888 and clear mode. Matrix phase follows logical PSP pixels, including surface origins and 2x rendering. The first fixture expected RGB565 word 0x732B but received 0x7B4C; it now passes in both backends. Coverage includes four target formats, matrix changes, negative/positive limits, enable/disable, clears, relocated targets and scaling. Software blend tests verify dithering before saturation. GLES blended dithering still requires a post-blend path and remains open; it is deliberately not applied to the source color.

Direct software-reference RGB MAE improves from 4.6312 to 4.3781 for SDSU and 5.0452 to 4.7421 for Georgia Tech-Georgia. SDSU field ROI improves from 5.2904 to 4.8830. The hardware-window metric worsens overall from 7.6518 to 7.7588 while its field ROI improves from 5.5334 to 5.4629. Both methods remain documented separately. Local reports and dither-comparison.png are in the existing capture folders; neither is fresh browser gameplay or hardware proof.

Latest package: build/static-ncaa10-dither (14 companions, no ISO); log: build/ncaa-dither-web-build.log. Ten native suites, eight package cases, web build and NCAA identity verification pass. Native Chrome fallback was attempted after the browser connector exposed no browser, but Computer Use app approval timed out before navigation. Live gameplay/FPS remains pending; the temporary server on 8633 was stopped. A request to approve Chrome app access is pending. Keep original game/save state intact. Default 64 MiB guest RAM / 56 MiB user partition and AOT execution are unchanged.

### Prior mipmap checkpoint

Next concrete regression target: SDSU primitive 5215 enables ordered dithering (`0x20=1`) for an unblended RGB565 final draw (`0x21=0`, `D2=0`). Signed rows are `[-4,0,-3,1]`, `[2,-2,3,-1]`, `[-3,1,-4,0]`, `[3,-1,2,-2]`. The renderer currently has no dither stage. Full state is local `build/reference-sdsu-day-20261013/final-draw-dither-state.json`; establish exact pixel results before attributing the remaining scene error or adding a workaround.

Keep reference methods separate: the SDSU hardware-window RGB metric slightly worsens from 7.6379 to 7.6518 (field ROI 5.3813 to 5.5334), while the direct software metric below improves. Hardware slope/LOD behavior remains unverified; this implements the studied software-reference rules rather than establishing hardware equivalence.

The GLES/WebGL renderer now decodes, caches and uploads regular mip chains, selects automatic/constant/slope LOD with signed bias, and applies four-bit within-level and between-level filtering. The first regression selected green mip 1 but received red level 0 before the fix. Coverage now includes all eight levels, shared/per-level indexed palettes, swizzle, higher-level RAM/VRAM changes, filter transitions, perspective clip-w slope and native-pixel LOD at 2× rendering. A chain remains one cache entry, with all its texels counted toward eviction. GPU targets with requested mips use coherent VRAM decoding; the ordinary GPU-target path remains for level-zero sampling.

Matched direct software-reference RGB MAE improves from 4.7815 to 4.6312 for SDSU, and from 5.1176 to 5.0452 for Georgia Tech–Georgia. SDSU field ROI improves from 5.3183 to 5.2904. Coarse field noise is reduced, but field/player shading still differs. Replays retain 3,752 / 3,087 draws respectively, zero downloads and two uploads. These counts do not prove sustained browser performance. Local reports and `mipmap-comparison.png` are in the existing capture folders.

Latest package: `build/static-ncaa10-mipmaps`; log: `build/ncaa-mipmap-web-build.log`. Web build, ten native suites, eight package cases and exact NCAA identity verification pass. Browser inventory still exposes no browser, so fresh browser gameplay/FPS/audio verification remains pending. Software diagnostic rasterization still samples level zero. Irregular/unbacked mip chains stop at the valid regular prefix; that fallback is not verified as PSP hardware behavior. Hardware LOD/precision accuracy, general blending, stencil/dithering, residual scene differences and performance remain open. Default memory remains 64 MiB / 56 MiB user partition; guest execution remains AOT.

### Earlier reference and filter checkpoints

The next rendering pass separates GE minification and magnification filters. Previously both host filters followed only the mag bit of `TEXFILTER`; a shrinking striped-texture fixture expected `0x808080` and received `0xFFFFFF`. The renderer now caches all 16 min/mag/wrap combinations. Tests cover independent nearest/linear choices, shrinking/enlarging, wrap/clamp and state reset. This does not implement mipmap level selection; the supplied frames' within-level filter bits already agree, so their output is unchanged by this fix.

Verified software references now supersede the failed attempt described below. Actual Windows command lines used `--graphics=software` with official v1.20.4. Three SDSU and two Georgia Tech–Georgia full VRAM reads match within each scene and contain coherent nonzero RGB565 frames. Direct 480×272 comparisons use stride 512 with no window scaling. SDSU RGB MAE is 5.4133 before the earlier target-packing fix and 4.7815 after; the latest filter fix retains 4.7815. Georgia Tech–Georgia pause MAE is 5.1176 before/after this filter fix. These are software-reference comparisons, not hardware equivalence. The project software renderer itself differs from PPSSPP, so it is not a ground-truth oracle. Captures and reports: `build/reference-ppsspp/*-software-correct-*` and each capture folder's `verified-software-comparison.json`.

Latest package: `build/static-ncaa10-texture-filter`, 14 companions, no ISO. Build log: `build/ncaa-texture-filter-web-build.log`. Ten native suites, eight package cases and NCAA identity verification pass. Fresh browser gameplay remains unverified: browser inventory returned no available browser and opening IAB failed. The test-only plain server on 8633 and isolated PPSSPP replay were stopped after this check. User game files and save origins were not modified. Residual field/player differences remain; inspect captured mip-level state and first differing draws next, without assuming mipmapping is already proven to be the cause.

The final precision pass packs RGB for unblended writes and clears into PSP 565/5551/4444 precision before GPU texture sampling. Its synthetic fixture previously expected `0x3165FF` but received `0x3264FA`; software and GLES now pass across all three formats, multiple colors and clears, with assertions that GPU sampling performs no CPU download. Blended 16-bit writes still need packing after blending; alpha/stencil precision and dithering remain open. The matched SDSU window metric increases from 6.7470 to 7.6379 after this change, so this is a targeted precision correction, not evidence of improved overall frame agreement.

Final validation passed: ten native suites, debugger regressions, exact ULUS10440 executable verification and eight static-package cases. Web build log: `build/ncaa-target-precision-web-build.log`; latest package: `build/static-ncaa10-target-precision` (14 companions, no ISO). This package has not received a fresh browser gameplay smoke test. Origin 8632 still serves the previous package described below; preserve that match. The prior source commit `451eeb3` passed [GitHub source checks](https://github.com/awest813/NCAA-10-PSP-Recomp/actions/runs/38479165824).

The isolated software-reference attempt is invalid: Windows PPSSPP v1.20.4 interprets `-software` as its `-s` pause switch. Use `--graphics=software` and verify the actual backend on a future attempt. The mislabeled `ppsspp-software-window.png` is a hardware replay; `ppsspp-software-native.png` came from all-zero VRAM and its comparison metrics must be ignored. Local metadata records these exclusions. That test instance was closed; original native reference sessions remain intact. See [reference evidence](docs/PPSSPP_REFERENCE.md).

The 2026-10-13 SDSU daytime dump now replays in isolated PPSSPP and the existing GE/GLES renderer through a local diagnostic harness. The matched Auburn–SDSU frame exposed incorrect doubled destination-alpha blending. The fixed-zero-source case now represents the exact factor algebra using host blending; tests cover factors 6/7 across alpha zero, midrange and saturation. A separate fractional-depth equality regression led to 16-bit fragment-depth quantization. These changes preserve guest instructions, allocator and 64 MiB defaults. Residual field-texture differences remain; the matched window-derived RGB MAE improves from 13.7183 to 6.7470, not to zero.

Evidence and limitations are in [PPSSPP reference evidence](docs/PPSSPP_REFERENCE.md). The new dump/captures/replay inputs are under `build/reference-sdsu-day-20261013`; the original exports stay under `build/reference-user-debug-20261013`. The experimental harness source is local `build/ge-replay-diagnostic/replay.cpp`; it handles the supplied record types, not every possible GE dump. Debugger capture now supports `PSPWEB_REFERENCE_PORT` and explicit encoded PNG output; actual framebuffer readback still fails, so reference pixels come from the native window.

Authorized read-only inspection of the live reference SDSU match captured `build/reference-ppsspp/sdsu-live-32mib-20261013.ram` and `sdsu-live-main-heap.json`. The reference has 32 MiB RAM and MAIN remains 16,991,184 bytes, matching the browser's original pool. Its loaded-match physical/free-list integrity passes, with five free blocks and a largest usable block of 601,040 bytes. This is not the original cold-load failure checkpoint. The reference was resumed after capture; preserve its memory viewer and game state.

Ten native suites, seven browser/debugger scripts and eight static-package cases pass. Frontend build log: `build/ncaa-sdsu-render-web-build.log`. Latest package: `build/static-ncaa10-sdsu-render` (14 companions), preview `http://127.0.0.1:8632/?profile`. The current source checkout remains `build/github-ready-20261012`; source-only publication excludes all dumps, generated output and game binaries.

The updated plain-server browser preview reached live Utah–Utah State gameplay after local ISO selection, Start and menu/controller choices. Its log confirms 64 MiB RAM / 56 MiB user partition; sampled live output was around 30 FPS / 99–100% speed. `build/ncaa-sdsu-render-browser-gameplay.jpg` and `build/ncaa-sdsu-render-browser-console.json` preserve this short smoke test. It is not the matched SDSU frame, a full match or sustained audio/performance validation. Startup had accumulated four underruns/118 overruns before the settled gameplay sample; audio remains open. Origin 8632 and its current match were left for continued work.

## Latest launcher, ISO, storage and audio audit

The 2026-10-13 follow-up fixes an import/autosave race: imported files now wait for the preceding persistence transaction before entering the filesystem. Exports require restoration and imports to finish, and filesystem read failures are reported instead of being mislabeled as an empty save directory. Unavailable storage blocks imports before mutation. The launcher reports engine aborts and clears guest readiness; local ISO checks show index, title and executable-revision stages, with status reset on retry.

Latest package: `build/static-ncaa10-launcher-final` (14 companions). Plain-server preview: `http://127.0.0.1:8631/?profile`. This origin rejected an invalid selected file, accepted the matching ISO on retry, imported the 12-file progressed Dynasty backup and retained all original hashes across export/reload/export. Evidence: `build/ncaa-launcher-audit-save-roundtrip.json`, `build/ncaa-launcher-audit-imported.zip`, `build/ncaa-launcher-audit-reloaded.zip`, `build/ncaa-launcher-audit-restored.jpg`. The current session has continued into a stadium scene; preserve it and its saves. The final package includes a small checking-status reset added after the first preview; regressions cover the final helper. Frontend log: `build/ncaa-launcher-final-web-build.log`.

Audio now uses the device-selected graph rate, with the existing PSP-to-device resampler. Diagnostics count overruns and discarded source frames, and use wall time for producer/render rates. The old rate calculation confused render starvation with producer overproduction. Fractional-position trimming now counts whole source frames exactly. A 48 kHz test on origin 8630 retained counters at six underruns/133 overruns from coin toss through roughly 11 minutes of stadium/play-selection observation; startup/loading had already accumulated those events. Another 8631 sample still showed render throughput around 33.3/48 kHz and further drops. The latest restored stadium capture shows seven underruns/zero overruns in that restarted session. These are separate sessions, not a continuous full-match test. Device-rate selection does not establish that crackling/gaps are fixed. Evidence: `build/ncaa-audio-device-baseline.txt`, `build/ncaa-audio-device-samples.json`, `build/ncaa-audio-device-stadium.jpg`.

Seven browser/debugger scripts, ten native suites, seven source-workflow cases and eight package cases pass. New regressions reproduce the import mutation race, export/read failures, abort handling, ISO progress, native-rate context selection and exact audio drop accounting. No guest instruction, allocator or 64 MiB RAM / 56 MiB user partition default changed. See [remaining gaps](docs/REMAINING_GAPS.md).

## Prior storage audit

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
