# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

This is a C project reverse-engineered from `sa_9061.exe` (Stone Age MMORPG client). It uses CMake + Ninja via Visual Studio 2022, targeting Windows with DirectX 7 APIs. Build output goes to `out/build/`.

```batch
# First time: configure (generates build files in out/build/x64-debug)
configure.bat

# After changes: rebuild
rebuild.bat
```

`configure.bat` runs `cmake --preset x64-debug` (requires VS 2022 at `D:\Program Files\Microsoft Visual Studio\2022\Community` and vcpkg at `D:\code\vcpkg`). `rebuild.bat` runs `cmake --build out/build/x64-debug --config Debug` with Ninja.

## Running Tests

Tests are individual executables, one per `.c` file in `tests/`. Configure and build first, then:

```bash
# Run all tests via CTest
cd out/build/x64-debug && ctest --output-on-failure

# Run a single test by name
ctest -R test_battle_state_machine

# Or run the test exe directly (console app, outputs pass/fail counts to stdout)
out/build/x64-debug/tests/test_battle_state_machine_comprehensive.exe
```

Test files are standalone C with an assert-based framework (no external test library). Some tests link against source files from `src/` (mapped in `tests/CMakeLists.txt` lines 137-153).

## Architecture Overview

**Reverse-engineering from sa_9061.exe** — the codebase aims to match the original binary's behavior. Functions may carry dual naming: a semantic C name (`battle_start_encounter`) and a Ghidra address (`FUN_0040a1a0`). Global variables with `s_` or `g_` prefixes correspond to original data segments.

**Source organization** — `src/` contains ~30 module directories, each with related functionality split across multiple files. The pattern is `src/<module>/<module>_<aspect>.c` (e.g., `battle/battle_calc.c`, `battle/battle_render.c`). Headers live in `include/`.

**Auto-discovery with exclusions** — CMake uses `file(GLOB_RECURSE)` to collect all `.c` files, then removes obsolete duplicates via `EXCLUDE_FILES`. When renaming or splitting a file, add the old path to `EXCLUDE_FILES` — don't delete it until you're sure nothing references it.

**`src/stubs.c`** — placeholder implementations for symbols that don't exist yet elsewhere. This is the glue that lets the project compile while modules are being built out. Functions in stubs.c are no-ops or return-zero. When implementing a real module, move its stubs out.

**Key subsystems:**
- **Graphics**: DirectDraw 7 with 16-bit color (565/555), paletted surfaces, alpha blending in `src/sprite/sprite_blend.c`
- **Network**: Custom binary protocol with Base-62 encoded bitmasks, text protocol (`\n` delimited), heartbeat at 30s intervals. Packet format: `[size:2][opcode:2][checksum:2][data]`. Core dispatch in `src/network/network_protocol.c` and `src/protocol/protocol_text.c`
- **Game loop**: `src/game/gameloop.c` — timing obfuscation (string-based), frame limiting, state dispatch via `src/game/gamestate.c` state machine
- **Battle**: Turn-based combat with 9-state machine, action queue, element system (7 elements), damage formulas in `src/battle/battle_calc.c`
- **Login**: DES encryption with custom S-boxes (key `"f;encor1c"`) in `src/login/login_crypto.c`
- **UI**: 9-sprite grid windows, skin animation (40 frames), widget system in `src/ui/uiwidget.c`
- **Rendering**: Two-pass text rendering, RLE sprite decompression, isometric diamond iteration in `src/map/map_iso.c`

**Memory model** — The original binary used fixed addresses (e.g., `DAT_0455xxxx`). Global variables and function pointers correspond to these regions, now represented as C globals.

## Key Constraints

- **Win32 only** — DirectDraw, DirectSound, DirectInput APIs. Cannot be built for Linux/macOS without rewriting these layers.
- **Compiles but not fully runnable** — many subsystems are complete per PROJECT_STATUS.md, but not all protocol handlers are wired up, and some functions remain as stubs.
- **x86 target** — the original `sa_9061.exe` is a 32-bit binary. The CMake config defaults to `Win32` (x86). x64 presets exist but the project is designed against 32-bit layout assumptions.
- Tests are compiled individually (not as a unified test runner) — each test `.c` file produces its own `.exe`.
