# Unit Test TODO List

## Test Environment Setup
- [ ] Install C compiler (gcc or MSVC) for running tests
- [ ] Configure test data paths for different environments
- [ ] Add test to main build system

## AI Settings Tests (test_pet_ai.c)

### TODO Items
1. **AISetting.dat Byte Interpretation**
   - TODO: Determine exact byte interpretation for skill arrays
   - Analysis: File shows 0x14 (20) bytes per array, but validation uses 32-bit int access
   - Question: Are they 4-bit nibbles, 8-bit bytes, 16-bit shorts, or 32-bit ints?
   - Binary reference: FUN_00401ab0 accesses arrays with int pointers

2. **Multiple Preset Handling**
   - TODO: Test reading different presets based on DAT_004d7f60 multiplier
   - Each preset is at offset: index * 0x55 + 0x10
   - Current tests only verify first preset

3. **Encrypted Header**
   - TODO: Analyze the 16-byte header at offset 0x00-0x0F
   - The header appears to contain encrypted/obfuscated data
   - Binary reference: No decryption in FUN_004017a0, just seeks past it

### FIX Items
1. **Validation Function Implementation**
   - FIX: petai_validate_settings() should match FUN_00401ab0 behavior exactly
   - Current implementation may differ in array access patterns
   - Need to verify: Does validation use byte or int access?

2. **Skill Level Default Value**
   - FIX: Confirm default value is 30 (0x1e) as shown in binary
   - FUN_00401ab0 sets invalid levels to 0x1e

## Protocol Util Tests (test_protocol_util.c)

### TODO Items
1. **DBCS Character Testing**
   - TODO: Add actual DBCS (Chinese) test strings
   - Need to confirm encoding: Shift-JIS, GB2312, or other
   - Test cases should include:
     - Chinese characters that contain byte 0x7C (|)
     - Multi-byte characters across buffer boundaries

2. **Buffer Boundary Testing**
   - TODO: Add tests for edge cases:
     - DBCS character split across buffer limit
     - Field exactly at buffer size
     - Very long fields

3. **Integration Testing**
   - TODO: Test with real protocol messages from game capture
   - Use actual XY, XYC, XYD commands from network logs

### FIX Items
1. **Return Value Consistency**
   - FIX: Verify return value semantics match binary exactly
   - FUN_00489f70 returns 0 on delimiter found, 1 on end/empty
   - FUN_00489fe0 returns 0 on delimiter found, 1 on end

## Coverage Goals

### Current Coverage Estimate
- protocol_util.c: ~70% (missing DBCS edge cases)
- pet_ai.c: ~30% (validation and save functions not tested)

### Target Coverage
- [ ] 80% line coverage for protocol_util.c
- [ ] 80% line coverage for pet_ai.c
- [ ] Add tests for ui_skin.c
- [ ] Add tests for stringutil.c

## Test Data Files

### Available Game Data
Location: D:\Games\石器时代8.0\石器时代8.0单机版\data\

Files for testing:
- AISetting.dat (1955 bytes) - AI configuration
- album_37.dat (909328 bytes) - Pet album/bestiary
- mail.dat (611840 bytes) - Mail system data
- savedata.dat (128 bytes) - Save data
- real.bin (25020180 bytes) - Sprite real data
- adrn.bin (388480 bytes) - Address/offset data
- spr.bin (103008 bytes) - Sprite index data

### Data File Analysis Needed
- [ ] Analyze album_37.dat format for pet album tests
- [ ] Analyze mail.dat format for mail system tests
- [ ] Analyze savedata.dat format for save/load tests

## Binary Analysis Questions

### FUN_00401ab0 (Validation)
1. What is DAT_004d7f68 used for?
2. What is DAT_0049e094 (set to -1 or index)?
3. What is the loop at DAT_04624042 checking?

### FUN_004017a0 (Loader)
1. What is DAT_004d7f60 (multiplier for offset)?
2. Is there error handling for partial reads?

## Test Execution

### Prerequisites
- C compiler (gcc recommended)
- Access to game data directory
- Windows environment (for file paths)

### Running Tests
```bash
cd tests
make test
```

### Expected Results
All tests should pass and show:
- Tests Run: 22+
- Tests Passed: 22+
- Tests Failed: 0
- Coverage: 80%+
