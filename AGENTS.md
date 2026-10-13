# Agent instructions — NCAA Football 10 PSP Web Recomp

## Scope and orientation

The user's instructions take precedence. This project targets **NCAA Football 10 PSP US, ULUS10440, for the web**. Work from the repository root containing `scripts` and `profile`. In the original local workspace this is the inner directory, beneath an outer wrapper. Read `HANDOFF.md`, `ENGINEERING_LOG.md`, and relevant sections of `docs/NCAA10.md` before substantive changes; consult `docs/NCAA10-CRASH-PROVENANCE.md` for the allocator/reader issue.

The host runtime is inherited from the God of War web ports. Preserve attribution while keeping NCAA's identity and evidence prominent. `NCAA2007` is the verified Football 10 ELF's internal module name. Do not substitute another game's executable, generated corpus, assets or screenshot.

## Identity and execution requirements

- Run `python scripts/verify_ncaa10.py games/ncaa10/root` when validating identity or preparing a build.
- NCAA builds use `PSPRECOMP_PROFILE=web`, `PSPWEB_GAME=ncaa10`, and `disc0:/PSP_GAME/SYSDIR/EBOOT.BIN`.
- Keep gameplay execution ahead-of-time compiled. Guest CPU interpreter/JIT fallbacks are outside the intended architecture. Static analysis/diagnostic instruction decoding is compatible with that requirement.
- Main and EA audio generated corpora are separate. Preserve relocated audio binding and regenerate affected output when changing the compiler/generator.
- Fix the generator or host source, rather than relying on edits to ignored generated C++ that disappear on regeneration. Record compiler fixes in `patches` as needed.

## Memory and correctness

- Keep **64 MiB guest RAM / 56 MiB user partition as the NCAA default**, with explicit 32 MiB comparison support. Other titles and the core runtime retain their own defaults unless separately authorized.
- Use the actual guest-memory size for AOT bounds, SysMem, stacks, display validation, audio scans and diagnostic captures. Emscripten linear-memory capacity is different from guest RAM.
- Preserve the original allocator and instruction semantics. Do not map zero, invent a fallback allocation, skip the faulting store, free a bank early or edit EA heap headers to hide the diagnosed failure.
- Trace the first incorrect state, request provenance and last writers. Respect delay-slot order and distinguish a fault from a completed memory write.
- Guard genuinely title-specific adaptations by the verified title and precise state/caller conditions. Generalize only with evidence and appropriate regressions.

## Build and verification

Use existing build directories and the commands in `HANDOFF.md`. Preserve the working native/web baseline. The framework/profile junction means edits through either path affect the same profile.

For code changes, run relevant host tests and affected web regression scripts. For compiler/runtime changes, run appropriate synthetic compiler tests and regenerate affected units. Rebuild the web/native targets affected by the change. Documentation-only edits need identity/link/command review, not a needless full game rebuild.

Production `PSPWEB_INSTRUCTION_TRACE_UNITS` and `PSPWEB_INSTRUCTION_TRACE_OPT_LEVEL` should be empty. Keep diagnostics opt-in; record flags, RAM mode, title hash and tested scenario in evidence.

Gameplay verification requires pressing Start and reaching actual play beyond title/menu/loading screens. Capture real output. Target NCAA's observed approximately 30 presented FPS at 100% simulation speed; do not equate 60 Hz Vblank or God of War's figures with NCAA gameplay FPS.

Do not claim full PSP/render/audio/controller/mode accuracy from a short smoke test. A passing simulated Gamepad API test does not verify a physical Bluetooth controller. Failed/black reference captures are not valid visual comparisons.

## State and documentation

- Preserve local game files, prior diagnostic captures and save exports. Export/hash saves before replacing fixtures; keep baseline and progressed checkpoints identifiable.
- Reuse a suitable running local server/session. Inspect current UI and savedata before acting on handoff snapshots. Restore temporary test settings.
- The original game/debug workspace has no Git checkout; its published source checkout is recorded in `HANDOFF.md`. GitHub clones are ordinary Git repositories. Inspect the actual root and status before Git operations, and preserve local dependencies, generated output and game state.
- Append dated engineering entries for meaningful changes, with reason, checks, artifact paths and unresolved limitations. Update the handoff to the latest reproducible state.
- Keep `README.md` focused on NCAA; label upstream history and performance evidence by title.
- Share reusable source fixes, synthetic fixtures and measured reports. Keep local disc assets, generated game translations, game binaries and RAM/save dumps out of source contributions. Preserve third-party licenses.
