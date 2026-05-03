# Test Coverage Report

## Test Suite Summary

| Module | Test File | Tests | Status |
|--------|-----------|-------|--------|
| Pet AI | test_pet_ai.c | 10 | Created |
| Protocol Util | test_protocol_util.c | 36 | Created |
| String Util | test_stringutil.c | 26 | Created |
| Game Data | test_game_data.c | 9 | Created |
| Map Entry | test_map_entry.c | 9 | Created |
| Battle Unit | test_battle_unit.c | 9 | Created |
| Character Protocol | test_character_protocol.c | 12 | Created |
| Skill Protocol | test_skill_protocol.c | 12 | Created |
| Pet Data | test_pet_data.c | 12 | Created |
| Isometric Renderer | test_isometric.c | 15 | Created |
| Battle Action Executor | test_battle_action_exec.c | 15 | Created |
| Pathfinding System | test_pathfind.c | 30 | Created |
| Battle Calculations | test_battle_calc.c | 30 | Created |
| Sprite Rendering | test_sprite.c | 30 | Created |
| Sound System | test_sound.c | 30 | Created |
| Input System | test_input.c | 30 | Created |
| Network Core | test_network_core.c | 30 | Created |
| UI Window System | test_ui_window.c | 30 | Created |
| Font Rendering | test_font_render.c | 30 | Created |
| Sprite Resource Loading | test_sprite_loader.c | 30 | Created |
| Pet AI Comprehensive | test_pet_ai_comprehensive.c | 30 | Created |
| **Total** | | **465** | |

## Test Categories

### 1. Pet AI System Tests (test_pet_ai.c)
Tests for FUN_004017a0 (AI settings loader) and FUN_00401ab0 (validator):

- File existence and size validation
- AI mode parsing and validation
- Primary skill data structure
- Skill level validation
- Auto battle flag validation
- Multiple preset handling
- Known value verification

**TODO Items Found:**
1. Need to determine exact byte interpretation for skill arrays
2. Need to test multiple presets
3. Need to analyze encrypted header

### 2. Protocol Utility Tests (test_protocol_util.c)
Tests for FUN_00489f70 and FUN_00489fe0 (DBCS-aware parsing):

- First/middle/last field parsing
- Non-existent field handling
- Empty string handling
- Single field (no delimiter)
- Buffer size limits
- Different delimiters
- DBCS character handling (placeholder)
- Empty field (consecutive delimiters)
- Field counting
- Real protocol format tests

**TODO Items Found:**
1. Need actual DBCS (Chinese) test strings
2. Need buffer boundary edge case tests
3. Need integration tests with real protocol data

### 3. String Utility Tests (test_stringutil.c)
Tests for FUN_0044a940 and related functions:

- String truncation with ellipsis
- DBCS-aware string copy
- DBCS string length
- Name validation
- Case conversion
- String trimming
- Number formatting with commas
- DBCS string comparison

## Coverage Analysis

### Files Tested
| Source File | Functions | Estimated Coverage |
|-------------|-----------|-------------------|
| protocol_util.c | 9 | ~90% |
| stringutil.c | 9 | ~80% |
| pet_ai.c | 6 | ~40% (needs more tests) |
| pathfind.c | 12 | ~60% |
| battle_calc.c | 10 | ~70% |
| sprite.c | 15 | ~65% |
| sound.c | 15 | ~60% |
| input.c | 18 | ~55% |
| network.c | 20 | ~50% |
| ui_window.c | 22 | ~55% | |

### New Functions Tested (2026-05-02)
- `string_to_int()` - Integer parsing from string (FUN_00492973)
- `parse_field_int()` - Parse integer field (FUN_0048a050)
- `parse_base62()` - Base-62 decoding (FUN_0048a0a0)
- `parse_field_base62()` - Parse Base-62 field (FUN_0048a120)
- `string_unescape()` - Unescape string (FUN_0048a170)

### Files Needing Tests
- ui_skin.c
- ui_window.c
- render.c (complex, needs mocking)
- network.c (needs mocking)
- battle.c (complex)

### New Tests Added (2026-05-02)

**7. Character Protocol Tests (test_character_protocol.c)**
Tests for FUN_0045ffb0 case 0x50 (character stats) and FUN_00464670 (character list):

- Character list parsing
- Detailed stats parsing with bitmask=1 (all fields)
- Selective field updates with bitmask
- Title and guild name string fields
- HP/MP field parsing
- Level/experience field parsing
- Empty packet handling
- Large bitmask handling

**Key Findings from Binary Analysis:**
- Field 1 (bitmask): Determines which fields are present
- Bitmask == 1: All 31 fields present sequentially
- Bitmask != 1: Use bitmask bits to determine fields
- Field 30 (0x1e): Title string (max 17 bytes) at DAT_0462bef8
- Field 31 (0x1f): Guild name (max 33 bytes) at DAT_0462bf09

**Bitmask Values for Fields:**
```
0x00000002: field_02 (HP)
0x00000004: field_03 (Max HP)
0x00000008: field_04 (MP)
0x00000010: field_05 (Max MP)
...
0x02000000: title string
0x04000000: guild name string
```

### 12. Pathfinding System Tests (test_pathfind.c)
Tests for FUN_00443e80, FUN_00447150, FUN_00443fe0, FUN_00441ff0:

- Direction 0-7 delta conversion (8 directions)
- Invalid direction handling
- Angle calculation (cardinal and diagonal)
- Angle normalization
- Same position path handling
- Path initialization/clear
- Movement path active check
- Get next step functionality
- Direction constants verification
- Max path steps constant (100)
- Pathfind distance calculation
- Pathfind estimate time
- Path status values
- Context initialization/shutdown
- Stop/cancel pathfinding
- Waypoint management

**Key Findings from Binary Analysis:**
- Direction 0: dx=-1, dy=1 (North)
- Direction 1: dx=-1, dy=0 (Northwest)
- Direction 2: dx=-1, dy=-1 (West)
- Direction 3: dx=0, dy=-1 (Southwest)
- Direction 4: dx=1, dy=-1 (South)
- Direction 5: dx=1, dy=0 (Southeast)
- Direction 6: dx=1, dy=1 (East)
- Direction 7: dx=0, dy=1 (Northeast)
- Max path steps: 100 (from DAT_045829e4 limit)
- Move delay: 200ms between steps
- Angle uses atan2 approximation with 65-entry lookup table (DAT_004bc2b0)

### 13. Battle Calculation Tests (test_battle_calc.c)
Tests for damage calculation, element system, buffs, capture rate:

- Basic damage calculation with variance
- Skill damage calculation
- Damage with attack/defense buffs
- Minimum damage guarantee
- Null parameter handling
- Hit chance calculation (90% base)
- Hit stunned targets (always hit)
- Critical hit calculation (5% base)
- Critical against debuffed targets (15%)
- Capture rate at different HP levels
- Capture rate with items
- Capture rate level penalty
- Escape chance calculation
- Escape with speed difference
- Buff application and removal
- Multiple buffs handling
- Invalid buff ID handling
- Skill usage with MP check
- Element constants verification

**Key Findings from Binary Analysis:**
- Element advantage: Fire > Wind > Earth > Water > Fire
- Light/Dark mutual weakness (1.5x each way)
- Base hit rate: 90%
- Base critical rate: 5%
- Critical bonus vs debuffed: +10%
- Attack buff: +20%, Defense buff: +20%
- Base escape chance: 50%
- Capture formula: (100 - HP%) / 2

### 14. Sprite Rendering Tests (test_sprite.c)
Tests for sprite rendering, blending, RLE decompression:

- RGB to 565/555 color conversion
- Color split functions (565/555)
- Basic alpha blending (565/555)
- Transparent pixel handling in blend
- Additive blending with clamping
- RLE 8-bit literal decompression
- RLE 8-bit run-length decompression
- RLE 8-bit mixed literal/run
- RLE 16-bit literal decompression
- Sprite ID range validation (normal vs high-res)
- Render mode constants verification
- Sprite type constants (UI, Icon, Button, Effect)
- Pixel format constants (555/565)
- Palette group constants
- Sprite system init/shutdown
- Animation initialization
- Animation play/stop
- Animation frame update
- Animation looping behavior
- Null animation handling
- Render flags verification

**Key Findings from Binary Analysis:**
- Normal sprites: ID 0-499999
- High-res sprites: ID 500000-549999
- Max queue size: 4096 (0x1000)
- Transparent color 565: 0xF81F (magic pink)
- Transparent color 555: 0x7C1F
- Sprite types: 0x68=UI, 0x69=Icon, 0x6d=Button, 0x6e=Effect
- Render modes: Normal=0, Additive=1, Subtractive=2, Custom=3
- RLE format: count|0x80 for run, count for literal
- Blend formula: dest + (src - dest) * alpha / 32

### 15. Sound System Tests (test_sound.c)
Tests for DirectSound audio, BGM/SE playback, WAV loading:

- BGM ID constants (BGM_ID_S0 through BGM_ID_T12)
- SE series 1 constants (sa1_xx, IDs 0-22)
- SE series 2/3 constants (sa2_xx, sa3_xx)
- Ambient sound constants (saam_xx)
- Special sound constants (sas_xx)
- Extended sound constants (sax_xx)
- Sound flags (loop, 3D, stream)
- Sound buffer limit (64 buffers, 260 char paths)
- Sound context initialization
- Sound structure size validation
- Streaming BGM structure with events
- WAV format constants (WAVE, fmt, data chunk IDs)
- WAVEFORMATEX structure for PCM audio
- WAV header parsing
- BGM path format verification
- SE path format verification
- Sound path building
- Volume range (-10000 to 0 millibels)
- Pan range (-10000 to 10000)
- Master volume control
- Sound playback without init
- Sound stop/is_playing tests
- Stop all sounds
- BGM init/get_current/is_playing/stop
- Streaming BGM status

**Key Findings from Binary Analysis:**
- BGM paths: data\bgm\sabgm_s0.wav through sabgm_t12.wav (8 files)
- SE paths: data\se\sa1_xx.wav, sa2_xx.wav, sa3_xx.wav, sas_xx.wav, sax_xx.wav
- WAV chunk IDs: WAVE=0x45564157, fmt=0x20746d66, data=0x61746164
- PCM format: 44100Hz, 16-bit, stereo (block align 4, 176400 bytes/sec)
- Volume range: -10000 to 0 (DirectSound millibels)
- Pan range: -10000 (left) to 10000 (right)
- Sound structure: ~280-400 bytes
- StreamingBGM: 3 events for double-buffer notification
- Max sound buffers: 64
- Max path length: 260

### 16. Input System Tests (test_input.c)
Tests for DirectInput keyboard/mouse, key repeat, IME support:

- Key code constants (Escape, Enter, Space, Arrow keys)
- Key state flags (Pressed, Held, Repeat, Release)
- Key repeat thresholds (30 frames delay, 23 frames rate)
- Mouse button constants and state flags
- Key repeat initial delay verification
- Key repeat trigger at 30 frames
- Key repeat rate (23 frame interval)
- Key release counter reset
- Key press/release/held detection
- Modifier key detection (Shift, Ctrl, Alt)
- Mouse position tracking
- Mouse button press/double-click/drag
- Mouse wheel delta
- Input context initialization
- IME composition string and cursor
- IME candidate list
- Key buffer overflow handling
- Key name lookup
- DIK to VK key mapping
- Device enumeration

**Key Findings from Binary Analysis:**
- Key repeat delay: 30 frames (0x1e) before first repeat
- Key repeat rate: 23 frames (0x17) between repeats
- Key state arrays at DAT_04ebe488, DAT_04ebe490
- Repeat counter array at DAT_04ebe420 (2 bytes per key)
- State flags: 0x1000=Pressed, 0x2000=Held, 0x8000=Repeat, 0x4000=Release
- Modifier flags: 0x10=Shift, 0x20=Ctrl, 0x40=Alt, 0x80=Extended
- IME functions at FUN_00491780, FUN_00491c40

## Issues Found During Testing

### Binary Analysis Issues
1. **AISetting.dat Format**
   - Header (0x00-0x0F) appears encrypted/obfuscated
   - Skill array byte interpretation unclear (byte vs int access)

2. **DBCS Handling**
   - Need to confirm encoding (Shift-JIS vs GB2312)
   - Need test strings with Chinese characters

3. **Validation Logic**
   - FUN_00401ab0 has complex validation with multiple conditions
   - Some fields reference other data structures

### Implementation Issues
1. **FIX Required:** pet_ai.c validation should match binary exactly
2. **TODO:** Add error handling for file read failures
3. **TODO:** Add bounds checking for array access

### Pathfinding System Issues
1. **TODO:** Test calculate_angle() with actual game coordinate data
2. **TODO:** Verify angle table values match DAT_004bc2b0 exactly
3. **TODO:** Test calculate_movement_path() with real map collision data
4. **TODO:** Add integration tests with actual map loading
5. **TODO:** Test pathfinding with obstacles using game data from `D:\Games\石器时代8.0\石器时代8.0单机版\data\`
6. **TODO:** Verify diagonal movement cost (14) vs orthogonal (10) matches binary

### Battle Calculation System Issues
1. **TODO:** Verify element table values from binary data region
2. **TODO:** Test element multiplier effects on damage
3. **TODO:** Add tests for skill database loading
4. **TODO:** Verify damage variance formula (90-110%) from binary
5. **TODO:** Test buff/debuff interaction with damage calculation
6. **TODO:** Verify capture rate formula matches binary logic
7. **TODO:** Test item effects in battle with real item data

### Sprite Rendering System Issues
1. **TODO:** Verify sprite dimension lookup table (DAT_00e8f234, DAT_0081c7f4)
2. **TODO:** Test sprite data loading with real sprite files
3. **TODO:** Verify RLE decompression matches FUN_0048a550 exactly
4. **TODO:** Test alpha blend formulas with precision verification
5. **TODO:** Add integration tests with DirectX surface rendering
6. **TODO:** Test render queue overflow handling (max 4096)
7. **TODO:** Verify high-res sprite handling (ID >= 500000)

### Sound System Issues
1. **TODO:** Test with real sound files from `D:\Games\石器时代8.0\石器时代8.0单机版\data\`
2. **TODO:** Verify WAV loading with actual BGM/SE files
3. **TODO:** Test streaming BGM with notification events
4. **TODO:** Verify DirectSound buffer creation matches binary
5. **TODO:** Test 3D sound positioning (SOUND_FLAG_3D)
6. **TODO:** Verify volume conversion formula matches binary
7. **TODO:** Test sound fallback behavior when DirectSound unavailable

### Input System Issues
1. **TODO:** Test with actual DirectInput device enumeration
2. **TODO:** Verify key repeat timing with frame timing
3. **TODO:** Test IME composition with Chinese characters
4. **TODO:** Verify mouse coordinate transformation
5. **TODO:** Test key buffer flushing behavior
6. **TODO:** Verify modifier key combinations
7. **TODO:** Test joystick/gamepad if supported

### 17. Network Core Tests (test_network_core.c)
Tests for network core loop, buffer management, protocol dispatcher:

- Network buffer size constants (0x4000, 0x1FFF)
- Connection state constants (Disconnected=0, Connecting=1, Connected=3)
- Heartbeat timeout (30000ms)
- WSA error codes (WSAEWOULDBLOCK=0x2733)
- Buffer append/overflow handling
- Buffer shift operations (recv/send)
- Packet extraction with newline detection
- Text field parsing (parse_text_field)
- Integer field parsing (parse_field_int)
- Base-62 field decoding (parse_base62)
- String unescape function (string_unescape)
- Protocol command constants (ClientLogin, CharList, etc.)
- Protocol dispatcher lookup
- Protocol field counting
- Connection initialization
- Connection state transitions
- Heartbeat tracking
- WSA error handling (disconnect vs retry)
- Null buffer handling
- Disconnected operation protection
- Send queue add/overflow
- Memory protection flags (PAGE_READWRITE=4)
- Large packet handling (200-4391 bytes)
- Large packet delay (2000ms)

**Key Findings from Binary Analysis:**
- Buffer size: 0x4000 (16384) bytes
- Recv buffer: DAT_045f1bf8
- Send buffer: gBuffer
- Connection state: DAT_04ebffd8
- Heartbeat: 30 second interval
- Large packet: 200-4391 bytes triggers 2s delay + split recv
- VirtualProtect: Used to toggle memory protection
- select() for non-blocking I/O
- Protocol commands: ClientLogin, CreateNewChar, CharDelete, CharLogin, CharList, CharLogout, ProcGet, PlayerNumGet

### 18. UI Window System Tests (test_ui_window.c)
Tests for window creation, 9-sprite grid, render queue:

- Window type constants (Normal, Modal, Transparent, Button)
- Window sprite offsets (9-sprite grid positions)
- Window state constants (Init, Opening, Render, Closing)
- Sprite type constants (UI=0x68, Icon=0x69, Button=0x6d, Effect=0x6e)
- Render queue size (max 4096)
- Window structure size and creation
- Window creation with different types
- Window center and animation calculations
- 9-sprite grid calculation and sprite IDs
- Render queue add/overflow handling
- Render queue blend modes (alpha 144/192/256)
- Sprite ID validation (>=100 valid, <-1 special)
- Window render state machine
- Window opening animation (10 frames)
- Window hit testing and close button
- Window z-order and decorations
- Sprite dimension/offset lookup
- High-res sprite range (500000-549999)
- Normal sprite range (0-499999)

**Key Findings from Binary Analysis:**
- Window struct size: 0x40 bytes header
- 9-sprite grid: 3x3 cells for corners, edges, center
- Animation: 10 frames for open transition
- Sprite types: 0x68=UI, 0x69=Icon, 0x6d=Button, 0x6e=Effect
- Render queue: max 0x1000 (4096) entries
- Blend modes: 0xa0000000=alpha256, 0x90000000=alpha144, 0xc0000000=alpha192
- Window creation: FUN_00448610
- Window render: FUN_00448270
- Sprite queue add: FUN_0047e210

### 19. Font Rendering Tests (test_font_render.c)
Tests for text queue, font creation, text rendering:

- Text queue size (max 0x400 = 1024 entries)
- Text entry structure size (0x110 = 272 bytes)
- Font creation parameters (weight=400, charset=0x86 GB2312)
- Text color constants (black, white, red, green, blue)
- Text background mode (transparent=1, opaque=2)
- Text queue initialization
- Text queue add entry
- Text queue overflow handling
- Text entry position storage
- Text entry string storage
- Text entry color and font size
- Text entry type flags (normal, shadow, outline, special)
- Font creation
- Font size validation
- Text rendering shadow offset (+1 pixel)
- Text rendering outline (8 offset positions)
- Text two-pass rendering (shadow pass 0, main pass 1)
- DBCS character detection (GB2312 lead/trail bytes)
- DBCS string length calculation
- Text width calculation
- Chinese character width (2x ASCII)
- Text type 3 special rendering (offsets 0x16/0x17)
- Text layer priority
- Text queue clear
- Text entry offsets verification
- Text buffer size (255 bytes)
- Text queue memory regions
- Color array location
- Font handle storage
- Text rendering completion

**Key Findings from Binary Analysis:**
- Max text queue: 0x400 (1024) entries from FUN_0041d7c0
- Entry size: 0x110 (272) bytes
- Font creation: FUN_00415a70 uses CreateFontA
- Font charset: 0x86 (GB2312_CHARSET) for Chinese
- Font weight: 400 (normal)
- Text render: FUN_00414820 with two-pass rendering
- Shadow offset: +1 pixel in both X and Y
- Outline offsets: 8 positions around text (-1,-1 to +1,+1)
- Type 3 special: Uses offsets 0x16 (22) and 0x17 (23)
- Text string buffer: 255 bytes max
- Memory regions: DAT_005676f8 (entries), DAT_005ab6f8 (count)
- Color array: PTR_DAT_004a2720 with 10 entries

### 20. Sprite Resource Loading Tests (test_sprite_loader.c)
Tests for sprite data loading, RLE decompression, dimension lookup:

- Sprite ID range constants (standard 0-499999, extended 500000-549999)
- Sprite info entry size (0x14 bytes)
- Sprite dimension entry size (0x50 bytes)
- Extended sprite info size (0x28 bytes)
- RLE magic header verification ("RD")
- RLE control byte literal run parsing
- RLE control byte RLE run parsing
- RLE count encoding 4-bit
- RLE count encoding 12-bit
- RLE count encoding 20-bit
- RLE decompression simple
- RLE decompression literal
- Standard sprite dimension lookup
- Extended sprite dimension lookup
- Invalid sprite dimension lookup
- Extended sprite index calculation
- Standard sprite offset lookup
- Extended sprite offset lookup
- Invalid sprite offset lookup
- Sprite data loading bounds check
- Sprite info structure offsets
- Sprite data loading return values
- Extended sprite bounds check
- Extended sprite info offsets
- Extended sprite dimension offsets
- Sprite dimension table location
- Sprite offset table location
- Extended sprite info location
- Palette location
- Sprite file handle

**Key Findings from Binary Analysis:**
- Sprite ranges: 0-499999 (standard), 500000-549999 (extended), >=550000 invalid
- RLE format: "RD" magic, control byte with bit-encoded count
- Control byte: bit7=mode, bit6=source, bits5-4=count size, bits3-0=count
- Count encoding: 4-bit (bits3-0), 12-bit (nibble<<8|byte), 20-bit (nibble<<16|bytes)
- Sprite info table: DAT_00e8f228, 0x14 bytes per entry
- Sprite dimension table: DAT_00e8f234, 0x50 bytes per entry
- Sprite offset table: DAT_00a04c64, 4 bytes per entry
- Extended info table: DAT_0081c7e4, 0x28 bytes per entry
- Extended dimension table: DAT_0081c7f4, 40 bytes per entry
- Extended index formula: (sprite_id * 5 - 2500000) * 8

### 21. Pet AI Comprehensive Tests (test_pet_ai_comprehensive.c)
Tests for AI settings loading, validation, skill types using real game data:

- File existence and size (1955 bytes)
- Preset count calculation (23 presets)
- Header encryption verification (first 16 bytes obfuscated)
- First preset AI mode (offset 0x10 = 0)
- First preset primary skill (skill ID 9)
- Preset separator pattern
- Full preset structure reading (69 bytes)
- AI mode validation (0 or 3)
- Skill level validation (0-100, %5, default 30)
- Skill ID validation (0-9)
- Default skill level value (0x1e = 30)
- Auto battle flag validation (0 or 1)
- Skill type action mapping (47 valid types)
- Skill type validation range
- Pet skill structure size (0x418 = 1048 bytes)
- Pet skill count limit (20 skills)
- Skill field parsing (| delimiter)
- All preset AI modes verification
- Preset data consistency across presets
- Skill level array values
- Memory region addresses
- Preset index calculation (preset * 0x55 + 0x10)
- Validation entry point steps
- Skill level type detection (types 1,2,7-11)
- Auto battle flag storage
- Tertiary skill level verification
- Secondary skill levels verification
- File read function behavior
- Complete preset validation

**Key Findings from Binary Analysis:**
- File: AISetting.dat = 1955 bytes with 23 presets
- Header: 16 bytes encrypted/obfuscated at file start
- Preset: 69 bytes data + 16 bytes separator = 85 bytes (0x55)
- Structure: AI mode(4) + skills(20) + levels(20) + secondary(20) + tertiary(4) + flag(1)
- AI mode: 0=off, 3=on (non-zero becomes 3)
- Skill levels: 0-100, divisible by 5, default 30 (0x1e)
- Primary skills: IDs 0-9, -1 for unused slots
- Memory: DAT_004d9050 (AI mode), DAT_004d7ea4 (skills), DAT_004d7f30 (levels)
- Offset: preset_index * 0x55 + 0x10

## Test Execution

### Prerequisites
- GCC or MSVC compiler
- Windows environment
- Access to game data at `D:\Games\石器时代8.0\石器时代8.0单机版\`

### Running Tests
```bash
cd D:\code\StoneAgeClient\tests
make test
```

### Expected Output
```
========================================
Stone Age Client - AI Settings Tests
========================================

[pet_ai]
  Running: AI file exists... PASSED
  Running: AI file size... PASSED
  ...
========================================
Tests Run:    10
Tests Passed: 10
Tests Failed: 0
Coverage:     100.0%
========================================
```

## Next Steps

1. **Setup Compiler Environment**
   - Install MinGW or Visual Studio
   - Configure build system

2. **Complete Test Coverage**
   - Add tests for remaining pet_ai.c functions
   - Add tests for ui_skin.c
   - Add tests for sprite loading

3. **Integration Tests**
   - Test with actual game data files
   - Test with captured network packets

4. **Coverage Tools**
   - Use gcov for line coverage
   - Target 80%+ for all core modules
