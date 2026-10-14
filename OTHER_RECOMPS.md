# Ways this NCAA port can help other recomp projects

This guide describes reusable engineering work from the NCAA Football 10 PSP web port. A working NCAA scene is evidence for that scene; another title still needs its own executable identity, HLE coverage and gameplay/reference tests.

## Reusable contributions

| Area | Starting point here | Useful contribution to another project |
| --- | --- | --- |
| Static compiler semantics | `PSPRecomp`, `patches/0006-ncaa10-vfpu-and-aot-audio.patch`, `scripts/test_ncaa_codegen.py` | A small synthetic instruction fixture with correct register, memory and delay-slot results |
| Embedded executable programs | `scripts/generate_ncaa_sound.py`, `profile/host/ncaa_sound.cpp` | A documented AOT extraction/binding method for relocated code stored outside the main ELF |
| Disc and host ABI | `profile/host/io.cpp`, `webfs.cpp`, `scripts/manifest.py`, `serve.py` | Sector/byte boundary, 64-bit argument alignment, Range-read and Windows newline regressions |
| Kernel behavior | `profile/host/kernel.cpp`, scheduler and utility tests | Minimal callback, thread lifetime, startup stack, semaphore or timing reproductions |
| Memory diagnostics | `ncaa_heap_trace.hpp`, `ncaa_pointer_trace.hpp`, `scripts/analyze_ncaa_heap.py` | Request-linked allocation/free history, last-writer provenance and allocator integrity checks |
| Rendering | `profile/host/ge.cpp`, `ge_gl.cpp`, `profile/tests/ge.cpp` | Tiny display lists and expected pixels for lighting, texture coordinates, fog and state changes |
| Browser audio | `profile/web/shell.html`, `scripts/test_web_audio.mjs` | Sample-rate, stereo/chunk, drift, reset and underrun recovery tests |
| Static hosting and local disc reads | `profile/web/local_iso.js`, `static_bootstrap.js`, `scripts/prepare_static_ncaa.py` | Lazy ISO indexing, revision checks, raw-sector ranges and startup gating; adapt title identity and AOT boot requirements for each port |
| Input and dialogs | `scripts/test_web_input.mjs`, `test_web_osk.mjs`, host controller/utility tests | Focus-loss release, disconnect, neutral rearm and dialog input suppression fixtures |
| Save validation | `profile/host/savedata.cpp`, NCAA save manifests | Export/hash/reload procedures that compare observable state across fresh processes |
| Pacing diagnosis | GE worker and presentation tests, `?profile` timing | Matched scenes separating game CPU, graphics queue, callback gaps and presented frames |

Paths are relative to the repository root containing `scripts` and `profile`. Existing synthetic fixtures can be shared with their source/license context. Actual game-derived corpora, disc assets and captured guest RAM are local diagnostic material, not portable test fixtures.

## A practical bring-up sequence

1. Identify the new title, region and decrypted executable hash. Give it a separate generated directory, build output and save origin. Never link its assets against NCAA's corpus.
2. Generate AOT code and first run a native validation build. Record the earliest missing import, unsupported instruction or incorrect state instead of treating the final crash as the cause.
3. Implement required PSP behavior with a synthetic regression. Preserve alignment, aliases, calling convention, delay slots and yields. Use primary hardware/runtime reference material for disputed semantics.
4. Validate boot, Start, menus, asset loading, actual gameplay and save/reload separately. Include a representative cold load; a warm cache can hide ordering problems.
5. Compare a repeatable scene with a PSP/reference implementation. Match title hash, inputs, settings and memory layout. Record unverified differences rather than calling a screenshot accurate by appearance alone.
6. Build for the browser with isolated-worker headers and disc streaming, then verify browser-specific input, storage, graphics and audio behavior.
7. Profile correctness-preserving work before optimizing. Record a sustained sample and transitions, not just the best settled FPS counter.

## Lessons to transfer carefully

**Trace pointer provenance across threads.** An asynchronous reader can be where a bad pointer is consumed. Tie allocator return, descriptor construction, submission, stack load and final access together with a request ID. Record last writers and pre-fault state. Keep instrumentation observational and compare diagnostic/production behavior.

**Measure usable contiguous space.** NCAA had enough aggregate free bytes but no suitably aligned block for one request. Compare pool boundaries, headers, alignment, live allocations and coalescing before growing memory. This port's 64 MiB default is a title-specific mitigation; it is not evidence that all PSP titles need 64 MiB or that the original 32 MiB discrepancy is fixed.

**Separate guest RAM from WebAssembly capacity.** Increasing Emscripten linear memory alone does not expand a game's SysMem partition. A coherent larger layout requires backed guest memory, dynamic allocation limits, valid aliases, stack placement, framebuffer checks and upper-memory regression tests.

**Look for code outside the main executable.** NCAA's EA sound banks contain executable programs. Statically compiling their known instruction sequences and binding relocated addresses preserved the AOT architecture. Other games need their own identification and relocation checks; do not scan and execute arbitrary data as a shortcut.

**Distinguish timing quantities.** Vblank cadence, simulation speed, display submission rate, presented FPS and audio sample production are separate. Do not force a game's timing to match a different title's FPS. Measure hot instrumentation itself so diagnostics do not become the performance problem.

**Replay the same graphics input.** A verified PPSSPP GE dump can isolate renderer behavior from CPU progression. Keep original captures local, record title/hash/scene/settings, validate the capture structure, then compare matching outputs. A native window screenshot can supply a reference when debugger readback fails, but retain scaling/crop limitations. Protocol raw pixel base64 needs format/stride/flip metadata; an encoded PNG URI is a different representation.

**Check blend algebra at saturation.** Mapping PSP `2*alpha` to ordinary host source alpha darkens destination modulation. For a zero source contribution, `2*a*D = a*D + a*D` and `(1-2*a)*D = (1-a)*D - a*D` fit normal host blend operands without prematurely clamping the doubled factor. Test alpha above 0.5, inverse factors and state reset. This correction covers that specific algebraic case; it does not establish all PSP blend modes accurate. Depth storage precision also matters: fractional drawing depths that share one integer 16-bit value must compare accordingly.

## Preparing a useful patch or report

**Test dithering across pipeline modes.** Ordered dither changes RGB before saturation and framebuffer conversion; clear mode and 8888 can also expose it. Check all signed nibble values, matrix-only changes, surface origins and internal resolution scaling. Blended output needs dithering after blending; a source-color offset does not implement that order. Preserve saturation-boundary fixtures so an early clamp cannot hide the difference.

**Treat a mip chain as one cache resource.** Include each level's layout and content in cache identity/revalidation, count all decoded texels toward capacity, and keep pending draws from observing mutated data. Test a higher-level-only RAM update and a same-frame VRAM write while the base hash remains unchanged. Use distinct level colors and per-level indexed palettes to reveal incorrect selection and CLUT sharing. Explicit texel fetch still needs an appropriate sampler and backed mip chain on the tested GLES path.

**Keep LOD in guest pixel units.** Host resolution scaling must not silently shift the selected mip. Check automatic derivatives, signed constant bias, slope with varying clip w, nearest-level rounding and interpolation fractions separately. Compare the same captured frame after the change; unchanged draw counts alone do not establish unchanged GPU cost or full browser FPS.

**Keep minification and magnification separate.** GE `TEXFILTER` carries independent settings. Test a striped texture at both shrinking and enlarging scales, change only one filter and revisit the original state. Include wrap/clamp combinations to exercise sampler-cache selection. This validates filtering within one mip level; mip selection and interpolation between levels require their own fixtures.

**Separate reference methods.** A coherent, repeatable software-rendered VRAM capture avoids window scaling, but is still a software reference rather than hardware proof. Report its format, stride, hashes and capture consistency, and retain hardware-window results separately. The same change can improve one metric and worsen the other.

**Validate GPU sampling without readback.** Packing a 16-bit target only when the CPU downloads it leaves GPU texture sampling at the wrong precision. Draw into each target format, sample it into a separate 8888 target and assert that no download occurred. Unblended output can be packed in the shader; blended output needs conversion after the blend equation. A targeted fixture can prove a semantics fix even when a window-derived frame metric gets worse; report both results and retain the unresolved frame differences.

**Verify the reference backend.** Command-line switches differ by platform and version. Windows PPSSPP v1.20.4 selects software with `--graphics=software`; `-software` is parsed as its pause switch. Requested INI settings and filenames do not prove the actual renderer. Reject all-zero or failed readbacks before calculating visual accuracy.

Provide the concrete trigger and observed before/after behavior, the smallest source change, a synthetic regression when feasible, tested platforms/settings and relevant license attribution. State whether the change is general PSP behavior or a title-specific adaptation. Include a repro sequence and compact diagnostic values; share only data suitable for the destination.

A useful bug report might say: “An aligned allocation fails despite sufficient aggregate space; here are the pool bounds, physical/free-list integrity, requested size, alignment and relevant allocation/free order.” It should not conclude “the STORE emitter is broken” solely from a null destination.

Run the target project's tests as well as the fixture that exposed the issue. If a fix depends on a particular compiler/emitter version, record that dependency and regenerate output. Make no compatibility claim for titles not tested.

## References and boundaries

The local [internals guide](docs/internals.md), [engineering log](ENGINEERING_LOG.md) and [crash provenance](docs/NCAA10-CRASH-PROVENANCE.md) are the starting evidence. `patches` preserves framework changes; `profile/host` contains runtime changes; `profile/tests` and `scripts/test_web_*.mjs` contain reusable synthetic checks.

The user also supplied [lcs-recomp](https://github.com/elmasas/lcs-recomp) as a possible reference. It is not a dependency in the current NCAA build path. Inspect its actual code, architecture and license before borrowing any technique; do not assume its game addresses or engine behavior apply to NCAA.

This guide prepares local contributions and reports. It does not itself publish changes or send messages to another project.
