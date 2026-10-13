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
