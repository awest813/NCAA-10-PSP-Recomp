# PPSSPP reference evidence

## User debug folder inspected on 2026-10-13

Local source: `C:/Users/allen/Downloads/NACC_Football_10_USA_PSP-PSPKiNG`.
Preserved debug copies and machine-readable report: `build/reference-user-debug-20261013/inspection.json`.
Original files were read without modification. No disc or debug payload is published with source.

| File | Verified result | Use and limit |
| --- | --- | --- |
| `NACC_Football_10_USA_PSP-PSPKiNG.iso` | 1,805,936,640 bytes; SHA-256 `4dedbb8436a20551bd4b3f08c208a1acecf9d5c47d4037bc4cb615bab21a3c08`, exactly matching the project ISO | Confirms the same supplied disc image. Continue verifying the decrypted executable with `scripts/verify_ncaa10.py`. |
| `ULUS10440_0001.ppdmp` | 340,270 bytes; `PPSSPPGE` version 6; game ID `ULUS10440`; both Zstandard blocks decode to their declared lengths | Graphics command/data capture for a reproducible renderer comparison. Native replay and matchup remain unverified. |
| `symbols.sym` | 25,554 symbols, including 25,230 `z_un_` names and 324 named library functions/imports | Function boundaries and some library identities; most names are automatically generated, not recovered game source names. |
| `map.ppmap` | Gzip decompresses to 922,272 bytes; module `NCAA2007`, base `0x08804000`, declared extent `0x006C4F00` | Its 25,554 entries exactly match `.sym` after module-relative addresses are resolved. It is a symbol map, not a heap-allocation snapshot. |

The frame dump contains 25,773 records and a 2,913,100-byte decoded pushbuffer. All record payload ranges lie within it; all types are recognized by the version-6 format; there are no trailing bytes. There are 4,705 register batches, 4,168 vertex records, 537 CLUT records, 356 CLUT-address records, 821 texture-0 records and 15,183 framebuffer-texture records across levels 0–3. The register batches contain 35,199 words and 4,168 PRIM commands. The initial and final display records both select `0x04000000`, stride 512, pixel format 0. Structural validation does not establish visual correctness or confirm a specific game scene.

The symbols put the known fault `0x08C7EE50` inside `z_un_08c7ecfc`, beginning at `0x08C7ECFC` and spanning `0x4A8` bytes. Named allocator-library entries include `_malloc_r` at `0x08CB837C` and `_free_r` at `0x08CB89B0`. These labels do not establish that those routines caused the separate EA animation-pool failure. The GPU dump contains no guest CPU instruction history, request provenance, allocation history or total/largest-free heap measurements. The 704-byte shortfall therefore remains unexplained by these files.

## Capture and replay state

An actual PPSSPP 1.20.4 gameplay window capture is available locally at `build/reference-ppsspp/ncaa-visible-window.png`. It shows Georgia versus Georgia Tech and includes window chrome; it is not a matched UK–FSU rendering comparison. The original reference configuration selects PSP model 0 (32 MiB). Do not compare it against the 64 MiB web default without recording that difference.

A separate reference profile was prepared under `build/reference-compare-20261013`, with PSP model 1, internal resolution 1, no frame skipping, sound disabled and loopback debugger port 8673. The executable matches the original official portable binary. Debugger observation confirmed PPSSPP 1.20.4 running the `NCAA2007` module from the game; it did not verify playback of the `.ppdmp`. Preserve active reference games and browser saves. Repeatable dump replay and a matched scene remain the next rendering milestone.

PPSSPP's debugger `type: "base64"` returns raw pixel bytes with width/format/flip metadata. `type: "uri"` returns an encoded PNG data URI. Saving raw base64 bytes with a `.png` suffix does not create an image. Output/current-framebuffer readback has returned `Could not download output` in the tested sessions; those failed captures supply no visual evidence.

## Next comparison

1. Open the preserved `.ppdmp` copy in an isolated PPSSPP session and verify the displayed scene. Record PPSSPP version, backend, resolution and capture hash.
2. Obtain the same scene and camera from the AOT web runtime, or implement a diagnostic GPU-record replay harness that exercises its renderer with the same captured commands. Keep this separate from guest CPU execution.
3. Compare the actual outputs, identify the first differing draw/state transition, fix the renderer source and repeat. A frame replay can validate rendering; it cannot validate CPU progression, audio or allocator behavior.
4. For the 32 MiB divergence, collect a separate cold-load CPU/heap trace at the allocator checkpoints in `NCAA10-CRASH-PROVENANCE.md`.

Format and behavior references: official PPSSPP 1.20.4 [record format](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Debugger/RecordFormat.h), [recorder](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Debugger/Record.cpp), [playback](https://github.com/hrydgard/ppsspp/blob/v1.20.4/GPU/Debugger/Playback.cpp), and [framebuffer protocol](https://github.com/hrydgard/ppsspp/blob/v1.20.4/Core/Debugger/WebSocket/GPUBufferSubscriber.cpp).
