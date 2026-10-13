# PPSSPP reference evidence

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
