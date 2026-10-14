# PPSSPP reference evidence

## Current GPU surface filtering and source-feedback evidence (2026-10-14)

GPU-rendered texture windows now use the same byte-channel/four-bit interpolation as decoded textures. Each tap wraps/clamps within the window, then receives the scaled surface origin. Logical coordinates truncate toward zero to eight fractional bits. Increased internal resolution preserves physical source samples; it is an enhancement rather than native PSP hardware evidence. Packed 565/5551/4444 source alpha is expanded before weighting, consistent with decoding guest texels in the [pinned sampler](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/Sampler.cpp). No reference source was copied.

Actual GE fixtures reproduce old blue 75 versus expected 74, filtered alpha rejecting a masked test after a partial resolve, and GPU level-zero LOD bias being ignored (red 4 instead of 6). Partial source-alpha resolves now require an unfiltered RGBA8 path as well as the existing alpha-preservation conditions. LOD mode/bias/slope is shared by GPU and decoded sources. Coverage includes all four source formats, nonzero window origins with neighboring sentinels, negative wrap/clamp, full/masked alpha after stencil writes, 1x/2x, and two distinct 2x samples within one logical pixel. Reverting the alpha-resolve condition reproduces the rejection; `old-partial-alpha-test.log` records it. The initial fixture's unaligned texture address was corrected before accepting any result.

An overlapping self-texture fixture exposed stale source data with blending disabled: two draws shared one snapshot. Feedback ordering now depends on overlapping guest surface spans, including separate GL objects with format aliases. Disjoint sources need no extra split. Two filtered/modulated draws produce `(20,27,37)` then `(10,13,18)` with two snapshots, at 1x/2x. In the RGB565-output/RGBA8888-source alias case, the first packed write `0x20C2` becomes source `(194,32,194)`; the second write must be `0x608C`. Pointer-only detection fails that fixture (`pointer-only-feedback-test.log`). Tests prohibit rendering CPU downloads; synchronization/snapshots remain GPU operations. Existing shared-edge, blend/stencil/depth and source-only cases pass.

All eleven measured direct RGB565 and pre-conversion RGBA outputs remain byte-for-byte equal to `c5d9e13`. Presented RGBA8 equals direct RGB565 in every case. Full RGB MAE remains SDSU **0.401468** (field **0.056140**) and GT **0.880504** (pause ROI **0**). This checkpoint fixes confirmed synthetic gaps without improving these two complete frames. Per-scene `surface-filter-accepted-comparison.json/png` and `build/reference-surface-filter-20261014/surface-filter-accepted-summary.json` retain panels/hashes; `surface-filter-provenance.json` records scope, failures, sources and limits. Modified prefixes remain diagnostics, not gameplay screenshots. Original captures are intact.

Accepted O3/DNDEBUG ABBA native medians versus `c5d9e13`: SDSU **169.8425 → 169.0515 ms (−0.47%)**, GT **141.3045 → 141.200 ms (−0.07%)**. Maximums worsen **288.919 → 302.441 ms** / **149.246 → 157.950 ms**. Each run repeats 16 frames, first two excluded, with FrameProfile enabled in both variants. Timing includes parser/CPU GE submission/presentation/GPU completion and excludes final reads. Repeated raw hashes match the single-frame results. Counts remain 1,982 / 919 / 618,114 draws/copies/pixels for SDSU and 1,930 / 840 / 833,528 for GT. These frames have zero feedback and alpha/stencil resolve passes in both variants. See `surface-filter-accepted-performance.json`; prior filtering-only timings are exploratory, and none establish browser FPS.

Latest package `build/static-ncaa10-surface-filter-final` has 14 companions, verified boot data and no ISO. Accepted native/web builds, ten native suites, seven source-workflow cases, eight package cases and exact NCAA identity pass. Logs: `build/ncaa-surface-filter-accepted-native-build.log` / `build/ncaa-surface-filter-accepted-web-build.log`; final Wasm shader inspected. No browser/reference process was opened. Transform/coverage, residual scene differences, broader windows/frames/hardware alpha/stencil/masks and software diagnostic filtering remain open. The earlier center/subpixel trial stays excluded. Fresh browser gameplay/audio/FPS and broad controller/save/mode/device coverage remain required. Default 64 MiB / 56 MiB and AOT execution are unchanged.

## Prior decoded-texture filtering evidence (2026-10-14)

Decoded level-zero textures now share the existing mip sampler's byte-channel/four-bit weighted interpolation. The [pinned reference sampler](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/Sampler.cpp) truncates coordinates to eight fractional bits toward zero before choosing texels/four-bit weights, including negative coordinates. An independently implemented actual GE regression reproduces old blue 75 versus expected 74. Exact RGB/alpha, nearest/linear, negative wrap/clamp, fractional boundaries, level-zero with/without an uploaded mip and 1x/2x fixtures pass. Rendering adds no CPU download. Existing min/mag fixtures now require exact truncated 127/63 instead of allowing rounded 128/64. GPU surface windows still use scaled GL sampling; the software diagnostic backend's filtering is also incomplete.

| Corrected direct software comparison | Before RGB MAE | After RGB MAE | Fixed ROI before → after |
| --- | --- | --- | --- |
| SDSU | 0.402295 | **0.401468** | Field: 0.060150 → **0.056140** |
| Georgia Tech–Georgia | 0.883931 | **0.880504** | Pause-menu region: **0 → 0** |

GT prefix 4092 improves 3.443479 → 3.434646; 4094/4100/4101 improve 2.696403 → 2.689456 with exact menu ROI preserved. SDSU prefixes 39/40/45/100/323 are unchanged. All eleven presented RGBA8 outputs equal direct RGB565. Reports/panels are `decoded-filter-comparison.json/png`; aggregate/hashes are in `build/reference-filter-20261014/decoded-filter-summary.json`. Original capture/reference data remains intact. Modified prefix images are diagnostics, not gameplay screenshots.

Matched O3 ABBA native medians versus `70fe877` are SDSU 168.9175 → 168.8645 ms (−0.03%) and GT 140.582 → 141.408 ms (+0.59%). Maximums are 263.654 → 293.827 and 153.080 → 149.143 ms. Each run uses 16 frames, first two excluded; parser/CPU submission/presentation/GPU completion are timed, final reads excluded. Repeated raw output hashes match the single-frame results. Counts remain 1,982 / 919 / 618,114 draws/copies/pixels for SDSU and 1,930 / 840 / 833,528 for GT. See `decoded-filter-performance.json`; these are not browser/gameplay FPS. An initial benchmark lacked its optional copy-counter define; it was corrected before the final matched comparison.

Separately, the first remaining SDSU prefix has 2,915 differing pre-conversion RGB pixels, concentrated on full rows. At `(0,107)`, reference source/destination RGB imply alpha 21; a local alpha probe gives 18. Studied [pixel centers](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/Rasterizer.cpp) use 7/16, with [screen subpixel quantization](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/TransformUnit.cpp). A trial combining those reduces differing pixels to 994 but worsens both complete frames (0.552336 / 1.074831), so it is excluded from production. Quantization alone worsens that prefix; quad-shared slope alone does not change it. The trial is evidence for further transform/coverage work, not an accepted accuracy improvement or hardware proof. `sampling-boundary-analysis.json` retains provenance and limitations.

Latest package `build/static-ncaa10-decoded-filter` contains 14 companions, verified boot data and no ISO. Native/web builds, ten native suites, seven source-workflow cases, eight package cases and exact NCAA verification pass. Fresh browser gameplay/pacing/audio remains pending app access. GPU surface filtering, geometry/coverage, hardware alpha/stencil/masks and broader mode/controller/save/device coverage remain open. Default 64 MiB / 56 MiB and AOT execution are unchanged.

## Prior interpolated-color evidence (2026-10-14)

The first affected menu-text draw is ordinal 4100, but a prefix before it already contains the one-unit error. Background draw 4093 interpolates an opaque gray gradient from 70 to 27 over 272 pixels. At `(24,145)`, the channel is 46.998: the prior renderer stores 46; the reference stores 47. Repeated raw reads agree. The font draws use nearest filtering/no mips, so no sampling change was made for this fix.

Primary RGBA and secondary RGB now round to nearest-even bytes before texture functions, alpha tests and blend products. Later fixed-point divisions retain truncation. This matches the x64 [rasterizer interpolation](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/Rasterizer.cpp); its ARM64/generic paths differ, so hardware/cross-backend equivalence remains unproven. The optimized [pixel blend path](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/DrawPixelX86.cpp) was also checked and retains separate half-step products. Its reference-only local file hash is recorded in `color-rounding-provenance.json`; no reference code was copied into the port.

Synthetic display lists reproduce 46 instead of 47 in both prior backends and now pass RGB/alpha gradients, texture/no-texture, blend/no-blend, exact ties and alpha-test boundaries. A direct backend fixture covers secondary-color rounding. Zero secondary colors skip unused shader work; vertex-only nonzero/zero/nonzero transitions preserve output despite unchanged draw registers. No extra GPU snapshot or CPU readback was added.

| Corrected direct software comparison | Before RGB MAE | After RGB MAE | Fixed ROI before → after |
| --- | --- | --- | --- |
| SDSU | 0.432580 | **0.402295** | Field: 0.060150 → 0.060150 |
| Georgia Tech–Georgia | 1.023075 | **0.883931** | Pause-menu region: 0.487218 → **0** |

The pause ROI is exactly equal in RGB565 and pre-conversion RGBA for GT prefixes 4094, 4100 and 4101. Earlier prefix 4092 worsens slightly from 3.440824 to 3.443479 overall; its fixed ROI is unchanged. Prefix 4094/4100/4101 full-frame error remains 2.696403 because earlier scene differences remain. SDSU prefixes 39/40/45/100/323 are unchanged. All eleven reference-compared RGBA8 presentations equal direct RGB565 readback. `color-rounding-comparison.json/png` retains panels and hashes. Aggregate/provenance live under `build/reference-gt-prefix-20261014`; original data is preserved, modified prefixes are diagnostic only, and the isolated reference was closed.

Final matched O3 ABBA native medians versus `0300ab3` are SDSU 168.064 → 168.3595 ms (+0.18%) and GT 140.333 → 143.5235 ms (+2.27%). Maximums worsen 261.170 → 311.266 ms and 148.060 → 152.189 ms. Each run repeats 16 frames with the first two excluded; timings include the experimental parser/CPU submission/presentation/`glFinish`, excluding final reads. These are not browser FPS. Final repeated raw hashes match single-frame output after the zero-secondary optimization. Copies/pixels/draws are unchanged: SDSU 919 / 618,114 / 1,982; GT 840 / 833,528 / 1,930. See `color-rounding-final-performance.json`; preliminary timing is kept separately rather than mixed with final results.

Latest package: `build/static-ncaa10-color-rounding-final`, with 14 companions, verified boot data and no ISO. Final native/web builds, ten native suites, eight package cases, source-workflow checks and exact NCAA verification pass. Fresh browser gameplay/pacing/audio remains pending app access. Earlier rendering, hardware/alpha/stencil/mask precision, sustained performance and broader mode/controller/save coverage remain open. Default 64 MiB / 56 MiB user partition and AOT execution are unchanged.

## Prior ordered shader-blend evidence (2026-10-14)

The independent draw-39 fixture reproduced GLES `(114,147,67)` instead of reference `(114,146,67)`. The renderer now truncates each half-step fixed-point contribution separately, combines them, then dithers before saturation and target RGB packing. This follows the studied [pixel pipeline](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/DrawPixel.cpp), independently implemented. Software and GLES cover six blend equations, factor classes, fixed-color aliases and doubled-alpha boundaries. Reserved equations match the software reference's fallback, without claiming hardware equivalence.

WebGL2 destination reads use a separate GPU texture. A convex separation test groups consecutive triangles whose interiors do not overlap; shared edges can share one copy, overlap starts another ordered group, and searches are capped at 256 triangles. Each copy is limited to the group's scissored region. Stencil alpha is resolved before subsequent destination-alpha reads. Source-only equations need no destination copy and retain batching, while self-texturing still gets ordered source snapshots. Tests verify repeated overlaps, disjoint/shared-edge batching, origins, stencil/depth/alpha-test ordering, self-feedback, four framebuffer formats and scaled output. They assert no render-path CPU download. Blend copy counts/area are exposed in native profiling.

| Corrected direct software comparison | Before RGB MAE | After RGB MAE | Fixed ROI before → after |
| --- | --- | --- | --- |
| SDSU | 0.585381 | **0.432580** | Field: 0.319599 → **0.060150** |
| Georgia Tech–Georgia | 1.044534 | **1.023075** | Pause-menu text: 0.062055 → **0.487218** |

The second ROI uses the same coordinates as the SDSU field ROI, but contains pause-menu text. Its worsening is preserved as the next sampling/coverage precision target. Prefix 39 remains exact; prefix 40 improves 0.355211 → 0.010098 and its field ROI becomes exact. Prefix 45/100 errors are 0.033816 / 0.033923; prefix 323 remains 2.069738. Reports and reference / prior / current panels are `ordered-blend-comparison.json/png`; `ordered-blend-summary.json` aggregates results. Direct RGB565 and presented RGBA8 match in all seven cases. Original dumps and captures are preserved; prefixes are modified diagnostics.

Matched native O3 ABBA runs have 16 repeated frames each, discarding the first two. Combined medians are SDSU 168.332 → 172.2645 ms (+2.34%) and GT 137.0855 → 140.667 ms (+2.61%). SDSU maximum worsens 204.168 → 301.349 ms; GT maximum is 156.285 → 152.020 ms. This is parser/CPU-GE submission/presentation/`glFinish` wall time on ANGLE/Intel Arc A750, not browser gameplay FPS. Final reads are outside timing. Repeated outputs match single-frame hashes. Counts: SDSU 1,982 draws, 919 copies / 618,114 copied pixels; GT 1,930 draws, 840 copies / 833,528 pixels. Source-only-copy elimination and scissor rejection avoid unnecessary work, but sustained pacing and device coverage remain unverified. `ordered-blend-performance.json` retains all samples and limitations. Earlier O2-vs-O3 exploratory runs are not the accepted timing comparison.

Latest package `build/static-ncaa10-exact-blend` has 14 companions, verified boot data and no ISO. Native/web rebuilds, ten native suites, eight package cases and exact NCAA verification pass. Fresh browser testing still awaits Chrome app access; no user game/save state or original reference session was modified. Remaining render precision includes menu text, geometry/coverage, alpha/stencil/masks and broader hardware/frame coverage. AOT execution and the 64 MiB / 56 MiB default are unchanged.

## Prior first-divergence and texture-function evidence (2026-10-14)

Original draw ordinal 39 is the first differing post-field draw: corrected diagnostic prefixes 38 and 39 match, while prefix 40 differs. It uses modulated/doubled indexed texture color, source-alpha blending and stencil. A cloned diagnostic payload inserts blend-disable immediately before that draw. Its pre-conversion source RGB differs only by one green-channel unit across row 218. Production rendering is untouched by that diagnostic modification.

A local shader probe writes pre-blend source alpha into RGB because framebuffer alpha belongs to stencil. Combining that alpha with independently captured reference source/destination RGB and separate half-step fixed-point blend products predicts 77,245 / 80,160 pixels exactly in `(0,105)-(480,272)` (96.36%, maximum channel error two). At `(0,105)`, source `(40,63,16)`, destination `(121,154,72)` and source alpha 21 predict reference `(114,146,67)`; GLES returns `(114,147,67)`. This supports blend precision as the next target, rather than proving that all remaining error has one cause. The alpha probe uses recomp sampling, so it is not an independent hardware-alpha measurement. See the pinned [blend pipeline](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/DrawPixel.cpp).

Independent synthetic lists reproduce incorrect texture-function arithmetic: software modulation gives 156 instead of 157, and old GLES rejects an alpha-tested draw, leaving magenta. Software/GLES now implement byte-channel modulation, decal, environment blend, replace and add, with RGB doubling before division and alpha unaffected by doubling. Tests cover all five functions, RGB/RGBA, doubling and exact alpha-test outcomes. A perspective fixture checks 256 interior pixels with constant byte colors; removing the small floating-point guard causes a one-unit loss. Color varyings carry byte units. The studied [texture-function semantics](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/Sampler.cpp) were independently implemented; no reference code was copied. Production adds no framebuffer copies/downloads.

Full-frame RGB error does **not** improve: SDSU 0.573736 → **0.585381**, field 0.316391 → 0.319599; Georgia Tech–Georgia 1.022723 → **1.044534**, field unchanged at 0.062055. Prefix 39 remains exact, while prefix 40 changes 0.282447 → 0.355211. Stage correctness is not proof of lower overall error with approximate downstream blending. Preserve these results when implementing exact GPU blend rounding, dithering and packing; keep overlapping primitives ordered and measure their performance cost.

Evidence stays local: `build/reference-sdsu-prefix-20261014/first-divergent-blend-analysis.json`, `texture-function-summary.json`, prefix 38/39/40/45 and `source40`; each measured folder has `texture-function-comparison.json/png`, raw buffers and a replay log. Every presented RGBA8 image matches direct RGB565 output. Original dumps/captures are preserved; modified prefixes/probes are not gameplay screenshots. Isolated reference process closed. Latest package `build/static-ncaa10-texture-functions`; native game rebuild, ten native suites, web build, eight package cases and exact NCAA verification pass. Fresh browser/gameplay/pacing/audio is unverified in this checkpoint.

## Capture precision correction (2026-10-14; pre-texture-function baseline)

The earlier local GLES replay harness accepted EGL's default color configuration, which was RGBA4444 on this machine. Its `glReadPixels` images therefore lost color precision even though rendering used RGBA8 attachments. Earlier GLES-candidate MAE values and frame-wide color/noise conclusions below are historical and invalid for color-fidelity analysis. Original PPSSPP reference images, project software direct-framebuffer output and synthetic tests reading explicit FBO/guest-memory pixels are unaffected. Old artifacts are preserved and their JSON reports now identify the correction.

The harness now explicitly requests and checks RGBA8. Its primary output is direct guest-format RGB565 framebuffer readback; RGBA8 presented output must agree byte for byte before a comparison is accepted. Native test contexts also enforce at least eight bits per channel. Both full supplied captures pass this agreement check.

| Corrected current renderer | Full RGB MAE | Field ROI MAE |
| --- | --- | --- |
| SDSU against direct PPSSPP software RGB565 | 0.5737 | 0.3164 |
| Georgia Tech–Georgia against direct PPSSPP software RGB565 | 1.0227 | 0.0621 |
| SDSU against the earlier cropped hardware window | 6.3190 | 2.9133 |

Reports are `corrected-capture-comparison.json`; images are `corrected-capture-comparison.png`, `recomp-direct565.png` and `recomp-rgba8.png` in each original capture folder. The side-by-side image is PPSSPP software / current renderer. Raw candidate framebuffer SHA-256 and surface bits are retained. Exporting raw display memory intentionally adds one diagnostic GPU download after rendering. This is not a runtime-performance regression or a browser FPS measurement. Source `e0091fd` replayed with the corrected capture method gives the same display bytes as the RGB565 blend fix in these two scenes, so the lower error is a capture correction, not a demonstrated effect of that fix.

The actual source correction is independently proven: RGB565 blend destination alpha must be zero, while sampling RGB565 textures must remain opaque. Tests previously expected zero but received green `0x07E0`; all destination-alpha factors and source/destination contributions now pass in software/GLES, with a separate opaque-texture alpha-test check. This follows the pinned [reference pixel handling](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/DrawPixel.cpp). Another regression exposed presentation inheriting a game's sampler: expected red became `(191,64,0)` with linear filtering and black with mip filtering. Presentation now explicitly clears that sampler binding before applying its own nearest filtering. Both cases pass through an explicit RGBA8 output FBO.

## Diagnostic draw-prefix comparisons

Local `build/reference-sdsu-prefix-20261014/make_prefix.py` clones affected register payloads before replacing later PRIM commands with NOP, avoiding mutation of deduplicated payloads still used by earlier records. Each copy retains the original final display-conversion draw 5215 and all data/state records. Originals stay intact. These are modified diagnostic captures, not original gameplay. The isolated official v1.20.4 reference used `--graphics=software`; repeated RGB565 reads match within each prefix. It was closed after capture. Inputs, manifests, reference captures and the local comparison scripts stay under ignored `build/`.

With corrected capture precision, prefix 13/14 RGB MAE is 0.0791, prefix 30 is 0.0784, prefix 37 is exactly zero, prefix 100 is 0.4968 and prefix 323 is 2.5179. Prefix 37 ends after the base-field pass: pre-conversion RGB and final RGB565 display both agree. The remaining divergence is in later detail/geometry passes, rather than a demonstrated base-field color error. `corrected-summary.json` and each prefix's `corrected-capture-comparison.json/png` preserve results. Small sky coverage differences precede the field; later overlays can cover earlier differences. Do not infer full renderer accuracy from one exact prefix or two close full frames.

Ten native suites, eight package cases, web build and NCAA identity verification pass. Latest package: `build/static-ncaa10-presentation-alpha`. Browser gameplay/FPS is still pending Chrome app access. GPU post-blend dithering/packing, alpha/stencil edge cases, hardware LOD/precision, full scene/mode coverage and performance remain unverified. Default guest RAM is 64 MiB.

## Historical checkpoints below

Their GLES image metrics are superseded by the capture correction above. Structural validation, reference hashes, independent pixel regressions and stated non-rendering evidence retain their original scope.

## User debug folder inspected on 2026-10-13

Local source: `C:/Users/allen/Downloads/NACC_Football_10_USA_PSP-PSPKiNG`.
Preserved debug copies and machine-readable report: `build/reference-user-debug-20261013/inspection.json`.
Original files were read without modification. No disc or debug payload is published with source.

| File | Verified result | Use and limit |
| --- | --- | --- |
| `NACC_Football_10_USA_PSP-PSPKiNG.iso` | 1,805,936,640 bytes; SHA-256 `4dedbb8436a20551bd4b3f08c208a1acecf9d5c47d4037bc4cb615bab21a3c08`, exactly matching the project ISO | Confirms the same supplied disc image. Continue verifying the decrypted executable with `scripts/verify_ncaa10.py`. |
| `ULUS10440_0001.ppdmp` | 340,270 bytes; `PPSSPPGE` version 6; game ID `ULUS10440`; both Zstandard blocks decode to their declared lengths | Graphics command/data capture for a reproducible renderer comparison. Native replay now verifies a Georgia Tech–Georgia pause frame; see the separate SDSU capture below. |
| `symbols.sym` | 25,554 symbols, including 25,230 `z_un_` names and 324 named library functions/imports | Function boundaries and some library identities; most names are automatically generated, not recovered game source names. |
| `map.ppmap` | Gzip decompresses to 922,272 bytes; module `NCAA2007`, base `0x08804000`, declared extent `0x006C4F00` | Its 25,554 entries exactly match `.sym` after module-relative addresses are resolved. It is a symbol map, not a heap-allocation snapshot. |

The frame dump contains 25,773 records and a 2,913,100-byte decoded pushbuffer. All record payload ranges lie within it; all types are recognized by the version-6 format; there are no trailing bytes. There are 4,705 register batches, 4,168 vertex records, 537 CLUT records, 356 CLUT-address records, 821 texture-0 records and 15,183 framebuffer-texture records across levels 0–3. The register batches contain 35,199 words and 4,168 PRIM commands. The initial and final display records both select `0x04000000`, stride 512, pixel format 0. Structural validation does not establish visual correctness or confirm a specific game scene.

The symbols put the known fault `0x08C7EE50` inside `z_un_08c7ecfc`, beginning at `0x08C7ECFC` and spanning `0x4A8` bytes. Named allocator-library entries include `_malloc_r` at `0x08CB837C` and `_free_r` at `0x08CB89B0`. These labels do not establish that those routines caused the separate EA animation-pool failure. The GPU dump contains no guest CPU instruction history, request provenance, allocation history or total/largest-free heap measurements. The 704-byte shortfall therefore remains unexplained by these files.

## Capture and replay state

An actual PPSSPP 1.20.4 gameplay window capture is available locally at `build/reference-ppsspp/ncaa-visible-window.png`. It shows Georgia versus Georgia Tech and includes window chrome; it is not a matched UK–FSU rendering comparison. The original reference configuration selects PSP model 0 (32 MiB). Do not compare it against the 64 MiB web default without recording that difference.

A separate reference profile was prepared under `build/reference-compare-20261013`, with PSP model 1, internal resolution 1, no frame skipping, sound disabled and loopback debugger port 8673. The executable matches the original official portable binary. The isolated window subsequently verified replay of the first dump (Georgia Tech–Georgia pause menu), then the new SDSU daytime dump described below. Preserve active reference games and browser saves. The profile settings are requested settings; do not infer the actual PPSSPP graphics backend solely from its INI.

PPSSPP's debugger `type: "base64"` returns raw pixel bytes with width/format/flip metadata. `type: "uri"` returns an encoded PNG data URI. Saving raw base64 bytes with a `.png` suffix does not create an image. Output/current-framebuffer readback has returned `Could not download output` in the tested sessions; those failed captures supply no visual evidence.

## Matched SDSU daytime frame (2026-10-13)

The subsequently supplied `ULUS10440_0002.ppdmp` is 406,786 bytes, SHA-256 `d16a234319efe74f2e3816a60f102ab6dcf3fcd99e4577419091d6b59022d21d`. It identifies ULUS10440/version 6, has 32,074 records, expands to a 3,146,028-byte pushbuffer, and passes decompression/payload bounds checks with no trailing bytes. `sdsu.sym` and `sdsu.ppmap` are byte-identical to the first exports. Copies and `inspection.json` are under `build/reference-sdsu-day-20261013`.

Native PPSSPP replay visibly shows Auburn away at SDSU, daytime, ACE-BIG play selection, 1st & 25, Q1 4:57. `ppsspp-replay-window.png` preserves the replay window. A local diagnostic harness (`build/ge-replay-diagnostic/replay.cpp`, executable `build/native-ncaa10/profiles/web/ncaa_ge_replay.exe`) feeds the decoded capture records to the existing GE/GLES renderer with 64 MiB guest memory, without running guest CPU instructions. It handles the record types in these captures; it is an experimental local tool, not a complete published dump player. Decoded inputs are `commands.bin` and `pushbuffer.bin`. The replay executes 5,216 primitives, 11,859 retained GL triangles and 3,752 GL draws on ANGLE/Intel Arc A750. There are zero GPU downloads and two uploads in this frame.

Partial frame renders locate the field-detail change between the first 100 and 500 primitives (`partial/progress.png`). Draws include destination-only modulation: fixed source contribution zero, destination factor twice source alpha. The old GL mapping treated factors 6/7 as ordinary alpha/inverse alpha and darkened the field. The source fix represents `2*a*D` as `a*D + a*D`, and `(1-2*a)*D` as `(1-a)*D - a*D`. This preserves factors above one without clamping the doubled factor into a normalized source value. It applies to fixed-zero source, destination factor 6/7 and add/reverse-subtract equations. Other complex blend combinations are not established accurate by this fix.

The native renderer also now quantizes fragment depth to integer 16-bit values before host depth comparison. A separate synthetic fixture reproduces the previous fractional-depth equality failure. This depth change barely affects the SDSU frame and is not the explanation for its field artifact.

For this matched frame, window-derived full-image RGB mean absolute difference drops from 13.7183 to 6.7470 on the 0–255 scale after the blend correction. The field ROI `(20,145)-(115,180)` drops from 15.2612 to 5.3173. The reference viewport was cropped at `(1,51)-(961,595)` and BOX-resized from 960×544 to 480×272; this includes display-scaling effects and is not exact framebuffer equivalence. Residual field-texture/edge differences remain. Evidence: `recomp-replay.png`, `recomp-depth16.png`, `recomp-blend-fix.png`, `comparison.json`, `render-comparison.png` and `primitive-dump.log` in the capture folder.

## Render-target precision follow-up

GPU-to-GPU sampling previously retained RGBA8 channel values when the PSP target format was 565/5551/4444. Packing only during CPU readback left the GPU texture path incorrect. Unblended writes and clears now pack and expand RGB to the selected target precision in the fragment shader. A synthetic display-list test fails before the fix (`0x3165FF` expected, `0x3264FA` observed) and passes after in both software and GLES. It covers three formats, four colors and clear/draw paths, asserts GPU target recognition and prohibits an intervening CPU download. This follows the format conversion behavior in PPSSPP v1.20.4 [SetPixelColor](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/DrawPixel.cpp); no reference implementation was copied.

This change deliberately excludes blended writes, which need quantization after the host blend equation. Alpha/stencil precision, dithering and full raster precision remain open. The matched window-derived SDSU full RGB MAE increases from 6.7470 to 7.6379, with field ROI increasing from 5.3173 to 5.3813. Evidence: `recomp-quantized.png` and `comparison.json`. These results do not establish improved overall frame agreement. A nearest-filter diagnostic also failed to resolve the residual field difference. Display filtering changes window-comparison metrics; keep those experiments separate from exact framebuffer comparisons.

The attempted software reference in `build/reference-software-20261013` is excluded. Windows v1.20.4 parses `-software` as `-s`, leaving a paused hardware replay; its actual software selection is `--graphics=software`, as shown in the pinned [Windows command-line parser](https://github.com/hrydgard/ppsspp/blob/v1.20.4/Windows/main.cpp). INI intent alone did not verify the backend. `ppsspp-software-window.png` is therefore a hardware replay despite its filename. `ppsspp-software-native.png` was produced from all-zero VRAM after a pause timeout, and the corresponding `software-comparison.json` metrics are invalid. Their local metadata now records this explicitly. The test-only instance was closed; original sessions and captures were preserved. The subsequent verified software references are recorded below.

The final web build and eight package cases pass; package `build/static-ncaa10-target-precision` contains 14 companions without the ISO. Ten native suites, debugger regressions and title/executable verification pass. No fresh browser gameplay test was performed for this final precision patch; the existing origin 8632 runs the preceding build. The 64 MiB guest RAM / 56 MiB user partition default is unchanged.

## Verified direct software references and filter correction

The subsequent isolated reference used the actual Windows v1.20.4 `--graphics=software` argument, confirmed from its process command line and the pinned parser. Direct VRAM reads now contain coherent RGB565 output. Three separate SDSU reads and two Georgia Tech–Georgia reads match byte for byte within each scene. Each raw capture is 278,528 bytes, stride 512; decoded visible output is 480×272 with no window crop or scaling. CPU pause still does not complete during GE replay, so these are repeated consistent full-frame reads of a static replay, not a successful paused debugger screenshot.

SDSU raw SHA-256: `bf83a6d2fd732354de777686017ca493e3f9ec7ad789a534780d17a3831bcf44`. Georgia Tech–Georgia raw SHA-256: `e07445dfe185ba02a8a5772d79cca3576520891eec08edeb945954ef09246ef9`. Captures are `build/reference-ppsspp/sdsu-software-correct-vram565*` and `gt-software-correct-vram565*`; profile intent, exclusions and repeatability are recorded in `build/reference-software-20261013/profile-manifest.json`. The test-only process was closed after capture. Failed/mislabeled earlier images remain excluded.

| Direct software-reference comparison | Full RGB MAE, 0–255 | Field ROI MAE |
| --- | --- | --- |
| SDSU before target precision correction | 5.4133 | 5.4374 |
| SDSU after target precision correction | 4.7815 | 5.3183 |
| SDSU after independent filter correction | 4.7815 | 5.3183 |
| Project software GE against SDSU reference | 2.5339 | 2.5006 |
| Georgia Tech–Georgia pause, before/after filter correction | 5.1176 | Not measured |

Reports and comparison images are `verified-software-comparison.json/png` in the respective capture folders. Both frames retain field/player differences. These software-reference metrics use a different reference and method from the earlier hardware-window metrics; do not combine them or claim hardware accuracy. The project software backend is also imperfect.

The filter investigation found an independent source bug: host minification and magnification both followed the GE mag bit. They now have separate state and 16 cached sampler combinations covering nearest/linear and U/V wrap/clamp. An actual-display-list shrinking fixture previously expected gray `0x808080` but received white `0xFFFFFF`; it now passes alongside enlargement, wrap/clamp and state-reset checks. PSP min/mag parameters are independent in the [PSPSDK implementation](https://github.com/pspdev/pspsdk/blob/master/src/gu/sceGuTexFilter.c). This change samples level zero as before; mip selection and mip blending remain unimplemented. Captured SDSU primitive 323 has `TEXMODE=0x020001`, `TEXFILTER=0x000107`, `TEXLEVEL=0x000002`, `TEXSLOPE=0x3DB439`; record those states before investigating the remaining detail draw, rather than assuming the missing mip path is the established cause.

The latest web package is `build/static-ncaa10-texture-filter`; web build, ten native suites, eight package cases and ULUS10440 identity verification pass. Browser inventory exposed no browser and IAB creation failed, so this source change has no fresh live browser gameplay verification. The temporary server was stopped. Default guest memory remains 64 MiB / 56 MiB user partition.

## Regular mip-chain implementation

A subsequent state inspection found a separate concrete omission: final SDSU primitive 5215 sets dither enable `0x20=1`, blend enable `0x21=0` and RGB565 target `D2=0`. The captured signed matrix is `[-4,0,-3,1; 2,-2,3,-1; -3,1,-4,0; 3,-1,2,-2]`. `final-draw-dither-state.json` preserves its relevant registers. Neither current backend implements ordered dithering. This is the next synthetic-pixel regression target; its contribution to the scene difference has not yet been measured.

The subsequent source change addresses a confirmed missing path: only base texture data reached GLES even when a display list requested other mip levels. A synthetic red/green/blue chain selected green mip 1 but returned red level 0 before the change. Regular chains now decode from each level's address/stride/size, share one cache entry and GL texture, and revalidate higher-level content. Cache capacity includes all mip texels; GL uploads and state hashes include the chain. Swizzled byte coverage includes complete 16-byte by 8-row tiles. Indexed chains honor shared versus per-level palettes.

The shader handles automatic, constant (including alias mode 3) and slope selection with signed sixteenth-level bias, exponent/mantissa LOD approximation and four-bit filter fractions. Slope uses screen-linear clip w; automatic derivatives are converted to native PSP pixel units at higher render scales. Within-level and between-level interpolation operate on decoded byte channels. Mip-capable samplers are required on the tested ANGLE/GLES path even for explicit texel fetches; uploaded higher levels otherwise sampled black. Non-mipmapped textures retain the preceding sampler path. These independently implemented semantics were checked against the pinned [GPU state definitions](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/GPUState.h) and [software sampling parameter calculation](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/Rasterizer.cpp).

Regressions cover all eight levels, nearest and mixed mip selection, signed bias/clamping, all LOD modes, disabling mipmaps, min/mag choice near zero detail, within-level filtering, shared/per-level CLUTs, swizzle, higher-level RAM and same-frame VRAM changes, perspective slope and 2× rendering. The existing lighting/depth/blend/target-precision fixtures still pass.

| Same direct software reference | Before mip support | After mip support |
| --- | --- | --- |
| SDSU full RGB MAE | 4.7815 | 4.6312 |
| SDSU field ROI MAE | 5.3183 | 5.2904 |
| Georgia Tech–Georgia full RGB MAE | 5.1176 | 5.0452 |

The earlier hardware-window method slightly worsens for SDSU: full RGB MAE 7.6379 to 7.6518, field ROI 5.3813 to 5.5334. This is retained in `comparison.json`. The software and window metrics disagree; neither proves PSP hardware fidelity. Slope/LOD behavior follows the studied software reference and still needs independent hardware validation.

Final source replays preserve primitive/draw counts, zero GPU downloads and two uploads in each capture. The coarse field noise is reduced, but field/player shading differences remain. Evidence: `recomp-mipmaps.png`, `mipmaps-replay.log`, `mipmap-comparison.png` and updated `verified-software-comparison.json` in each capture folder. The three-column image is software reference / preceding renderer / mip-enabled renderer. It is a native replay comparison, not fresh browser gameplay or PSP hardware proof.

Current scope: regular, backed mip chains; malformed/irregular chains stop at their valid prefix. That fallback, complete hardware LOD/raster precision and all filter edge cases are unverified. GPU-rendered mip chains can require coherent CPU readback; the supplied frames do not trigger it. The software diagnostic backend still samples level zero. Broader blend, alpha/stencil/dithering and scene fidelity remain open. Web build/package and ten native suites pass, but no browser was available to automation for fresh gameplay or sustained FPS measurement. Package: `build/static-ncaa10-mipmaps`; default guest memory stays 64 MiB.

## Ordered dithering checkpoint (2026-10-14)

The missing dither stage is now present in software rasterization and GLES unblended draws/clears. It applies the signed 4x4 matrix to RGB before saturation and packing, with logical PSP pixel phase preserved across surface offsets and 2x host rendering. The initial fixture failed with RGB565 word `0x7B4C` instead of `0x732B`. It now passes across 565/5551/4444/8888 RGB, two matrices including signed extrema, enable/disable transitions, clear/ordinary drawing, target offsets and scaled rendering. Alpha/stencil precision is outside this fixture. Software add/subtract blend cases also verify dither before saturation rather than after clamping.

The behavior follows the independently studied pinned [pixel pipeline](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/DrawPixel.cpp) and [dither state setup](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Software/FuncId.cpp), including clear mode and 8888. No reference source was copied. GLES blended draws still need dithering after the host blend equation; applying it to source color would be incorrect, so that path remains open.

| Same reference method | Before dither | After dither |
| --- | --- | --- |
| SDSU direct software, full RGB MAE | 4.6312 | 4.3781 |
| SDSU direct software, field ROI | 5.2904 | 4.8830 |
| Georgia Tech–Georgia direct software, full RGB MAE | 5.0452 | 4.7421 |
| SDSU hardware window, full RGB MAE | 7.6518 | 7.7588 |
| SDSU hardware window, field ROI | 5.5334 | 5.4629 |

The software comparisons improve; hardware-window results are mixed. Residual field/player shading and general rendering accuracy remain open. Draw/download/upload counts are unchanged. `recomp-dither.png`, `dither-comparison.png`, `dither-replay.log` and updated comparison JSON files preserve results in both capture folders. The three columns are software reference / prior mip renderer / dither renderer. These are native capture replays, not live browser output.

Web build, ten native suites, eight static-package cases and exact title/executable verification pass. Latest package: `build/static-ncaa10-dither`; log: `build/ncaa-dither-web-build.log`. With the browser connector inventory empty, native Chrome access was attempted through Computer Use. Its app approval timed out before navigation; no live gameplay or sustained FPS result is claimed. The temporary loopback server was stopped. Chrome access approval was requested for the pending live check. Default guest RAM remains 64 MiB / 56 MiB user partition.

## Authorized live SDSU heap inspection

The user opened the original reference's memory viewer and authorized inspection. Read-only debugger mapping confirms 32 MiB RAM / 24 MiB user memory and the active NCAA2007 module. A brief debugger pause captured `build/reference-ppsspp/sdsu-live-32mib-20261013.ram`; the following resume completed. `sdsu-live-main-heap.json` was produced with `scripts/analyze_ncaa_heap.py`.

MAIN is at `0x08EC9020`, with physical bounds `0x08EC9010` to `0x09EFD3E0`: 16,991,184 bytes, matching the browser's original 32 MiB pool size. The loaded-match snapshot contains 427 physical nodes, 420 allocations and five free blocks, with 642,752 usable free bytes in total and 601,040 in the largest block. Physical/free-list links and membership pass the analyzer, and no adjacent free nodes remain uncoalesced. There is no currently allocated node with requested size `0x39480` in this snapshot. This post-load Auburn–SDSU state is not the failed cold Fresno State–SDSU allocation checkpoint; it cannot explain the original 704-byte capacity shortfall or prove its root cause fixed.

## Debugger capture commands

`scripts/debug_ppsspp_reference.mjs` defaults to loopback port 8672. Set `PSPWEB_REFERENCE_PORT=8673` for the isolated replay. Use `savePNGAs` with GPU commands to request an encoded PNG URI and verify its signature. Raw GPU `saveBase64As` captures now retain dimensions/format/flip metadata in a `.json` sidecar, and reject raw pixels mislabeled with a `.png` suffix. Synthetic tests cover both forms and failed-capture resume. Actual debugger readback still fails even during GE replay; native window capture supplies the reference image.

## Next comparison

1. Preserve the verified replay scenes and capture hashes. Confirm actual backend/settings before using them for additional comparisons.
2. Harden and publish the diagnostic GPU-record replay harness with synthetic fixtures and explicit format coverage. Keep replay separate from guest CPU execution.
3. Compare the actual outputs, identify the first differing draw/state transition, fix the renderer source and repeat. A frame replay can validate rendering; it cannot validate CPU progression, audio or allocator behavior.
4. For the 32 MiB divergence, collect a separate cold-load CPU/heap trace at the allocator checkpoints in `NCAA10-CRASH-PROVENANCE.md`.

Format and behavior references: official PPSSPP 1.20.4 [record format](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Debugger/RecordFormat.h), [recorder](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Debugger/Record.cpp), [playback](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Debugger/Playback.cpp), and [framebuffer protocol](https://github.com/hrydgard/ppsspp/blob/v1.20.4/Core/Debugger/WebSocket/GPUBufferSubscriber.cpp).
