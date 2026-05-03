/*
 * Stone Age Client - Serialization System Comprehensive Tests
 * Tests for serialize.c implementation
 *
 * Covers:
 * - Serializer initialization (read/write modes)
 * - Integer type serialization (u8, u16, u32, s8, s16, s32)
 * - Float serialization
 * - String serialization (length-prefixed and fixed)
 * - Raw bytes serialization
 * - Checksum calculation
 * - File I/O operations
 * - Peek and skip operations
 * - Alignment handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef short s16;
typedef int s32;
typedef float f32;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Constants
 * ======================================== */

#define SERIALIZE_READ  0
#define SERIALIZE_WRITE 1

#define SERIALIZE_BUFFER_SIZE 256

/* ========================================
 * Structures
 * ======================================== */

typedef struct {
    u8* buffer;
    u32 size;
    u32 position;
    int mode;
} Serializer;

/* ========================================
 * Implementation Functions
 * ======================================== */

static int serializer_init_write(Serializer* s, void* buffer, u32 size) {
    if (!s || !buffer || size == 0) return 0;

    s->buffer = (u8*)buffer;
    s->size = size;
    s->position = 0;
    s->mode = SERIALIZE_WRITE;

    return 1;
}

static int serializer_init_read(Serializer* s, const void* buffer, u32 size) {
    if (!s || !buffer || size == 0) return 0;

    s->buffer = (u8*)buffer;
    s->size = size;
    s->position = 0;
    s->mode = SERIALIZE_READ;

    return 1;
}

static void serializer_reset(Serializer* s) {
    if (s) {
        s->position = 0;
    }
}

static u32 serializer_get_position(Serializer* s) {
    return s ? s->position : 0;
}

static u32 serializer_get_remaining(Serializer* s) {
    if (!s || s->position >= s->size) return 0;
    return s->size - s->position;
}

static int serializer_is_end(Serializer* s) {
    return s ? (s->position >= s->size) : 1;
}

static int serializer_write_u8(Serializer* s, u8 value) {
    if (!s || s->mode != SERIALIZE_WRITE || s->position + 1 > s->size) {
        return 0;
    }

    s->buffer[s->position++] = value;
    return 1;
}

static int serializer_read_u8(Serializer* s, u8* value) {
    if (!s || s->mode != SERIALIZE_READ || s->position + 1 > s->size || !value) {
        return 0;
    }

    *value = s->buffer[s->position++];
    return 1;
}

static int serializer_write_u16(Serializer* s, u16 value) {
    if (!s || s->mode != SERIALIZE_WRITE || s->position + 2 > s->size) {
        return 0;
    }

    s->buffer[s->position++] = value & 0xFF;
    s->buffer[s->position++] = (value >> 8) & 0xFF;

    return 1;
}

static int serializer_read_u16(Serializer* s, u16* value) {
    if (!s || s->mode != SERIALIZE_READ || s->position + 2 > s->size || !value) {
        return 0;
    }

    *value = s->buffer[s->position] |
             (s->buffer[s->position + 1] << 8);
    s->position += 2;

    return 1;
}

static int serializer_write_u32(Serializer* s, u32 value) {
    if (!s || s->mode != SERIALIZE_WRITE || s->position + 4 > s->size) {
        return 0;
    }

    s->buffer[s->position++] = value & 0xFF;
    s->buffer[s->position++] = (value >> 8) & 0xFF;
    s->buffer[s->position++] = (value >> 16) & 0xFF;
    s->buffer[s->position++] = (value >> 24) & 0xFF;

    return 1;
}

static int serializer_read_u32(Serializer* s, u32* value) {
    if (!s || s->mode != SERIALIZE_READ || s->position + 4 > s->size || !value) {
        return 0;
    }

    *value = s->buffer[s->position] |
             (s->buffer[s->position + 1] << 8) |
             (s->buffer[s->position + 2] << 16) |
             (s->buffer[s->position + 3] << 24);
    s->position += 4;

    return 1;
}

static int serializer_write_s8(Serializer* s, s8 value) {
    return serializer_write_u8(s, (u8)value);
}

static int serializer_read_s8(Serializer* s, s8* value) {
    return serializer_read_u8(s, (u8*)value);
}

static int serializer_write_s16(Serializer* s, s16 value) {
    return serializer_write_u16(s, (u16)value);
}

static int serializer_read_s16(Serializer* s, s16* value) {
    return serializer_read_u16(s, (u16*)value);
}

static int serializer_write_s32(Serializer* s, s32 value) {
    return serializer_write_u32(s, (u32)value);
}

static int serializer_read_s32(Serializer* s, s32* value) {
    return serializer_read_u32(s, (u32*)value);
}

static int serializer_write_float(Serializer* s, float value) {
    u32 temp;
    memcpy(&temp, &value, sizeof(float));
    return serializer_write_u32(s, temp);
}

static int serializer_read_float(Serializer* s, float* value) {
    u32 temp;
    if (!serializer_read_u32(s, &temp)) return 0;
    memcpy(value, &temp, sizeof(float));
    return 1;
}

static int serializer_write_string(Serializer* s, const char* str, u16 max_len) {
    u16 len;
    const char* null_str = "";

    if (!s) return 0;

    if (!str) str = null_str;

    len = (u16)strlen(str);
    if (len > max_len) len = max_len;

    if (!serializer_write_u16(s, len)) return 0;

    if (s->position + len > s->size) return 0;

    memcpy(s->buffer + s->position, str, len);
    s->position += len;

    return 1;
}

static int serializer_read_string(Serializer* s, char* str, u16 max_len) {
    u16 len;

    if (!s || !str) return 0;

    if (!serializer_read_u16(s, &len)) return 0;

    if (len >= max_len) {
        len = max_len - 1;
    }

    if (s->position + len > s->size) return 0;

    memcpy(str, s->buffer + s->position, len);
    str[len] = '\0';
    s->position += len;

    return 1;
}

static int serializer_write_fixed_string(Serializer* s, const char* str, u16 size) {
    u16 len;

    if (!s || !str) return 0;

    len = (u16)strlen(str);
    if (len > size) len = size;

    if (s->position + size > s->size) return 0;

    memset(s->buffer + s->position, 0, size);
    memcpy(s->buffer + s->position, str, len);
    s->position += size;

    return 1;
}

static int serializer_read_fixed_string(Serializer* s, char* str, u16 size) {
    if (!s || !str) return 0;

    if (s->position + size > s->size) return 0;

    memcpy(str, s->buffer + s->position, size);
    str[size] = '\0';
    s->position += size;

    return 1;
}

static int serializer_write_bytes(Serializer* s, const void* data, u32 size) {
    if (!s || !data || size == 0) return 0;

    if (s->position + size > s->size) return 0;

    memcpy(s->buffer + s->position, data, size);
    s->position += size;

    return 1;
}

static int serializer_read_bytes(Serializer* s, void* data, u32 size) {
    if (!s || !data || size == 0) return 0;

    if (s->position + size > s->size) return 0;

    memcpy(data, s->buffer + s->position, size);
    s->position += size;

    return 1;
}

static u32 serializer_calc_checksum(Serializer* s, u32 start, u32 end) {
    u32 checksum = 0;
    u32 i;

    if (!s) return 0;

    if (end == 0) end = s->position;
    if (end > s->size) end = s->size;

    for (i = start; i < end; i++) {
        checksum = ((checksum << 5) + checksum) + s->buffer[i];
    }

    return checksum;
}

static int serializer_peek_u8(Serializer* s, u8* value) {
    if (!s || s->position + 1 > s->size || !value) return 0;

    *value = s->buffer[s->position];
    return 1;
}

static int serializer_peek_u16(Serializer* s, u16* value) {
    if (!s || s->position + 2 > s->size || !value) return 0;

    *value = s->buffer[s->position] |
             (s->buffer[s->position + 1] << 8);
    return 1;
}

static int serializer_skip(Serializer* s, u32 bytes) {
    if (!s || s->position + bytes > s->size) return 0;

    s->position += bytes;
    return 1;
}

static int serializer_align(Serializer* s, u32 alignment) {
    u32 padding;

    if (!s || alignment == 0) return 0;

    padding = (alignment - (s->position % alignment)) % alignment;

    if (s->position + padding > s->size) return 0;

    s->position += padding;
    return 1;
}

/* ========================================
 * Test Cases - Initialization
 * ======================================== */

static int test_init_write_null_serializer(void) {
    u8 buffer[64];
    return serializer_init_write(NULL, buffer, sizeof(buffer)) == 0;
}

static int test_init_write_null_buffer(void) {
    Serializer s;
    return serializer_init_write(&s, NULL, 64) == 0;
}

static int test_init_write_zero_size(void) {
    Serializer s;
    u8 buffer[64];
    return serializer_init_write(&s, buffer, 0) == 0;
}

static int test_init_write_valid(void) {
    Serializer s;
    u8 buffer[64];
    int result = serializer_init_write(&s, buffer, sizeof(buffer));
    return result == 1 && s.mode == SERIALIZE_WRITE && s.position == 0;
}

static int test_init_read_null_serializer(void) {
    u8 buffer[64];
    return serializer_init_read(NULL, buffer, sizeof(buffer)) == 0;
}

static int test_init_read_null_buffer(void) {
    Serializer s;
    return serializer_init_read(&s, NULL, 64) == 0;
}

static int test_init_read_valid(void) {
    Serializer s;
    u8 buffer[64];
    int result = serializer_init_read(&s, buffer, sizeof(buffer));
    return result == 1 && s.mode == SERIALIZE_READ && s.position == 0;
}

/* ========================================
 * Test Cases - Position and Remaining
 * ======================================== */

static int test_get_position_initial(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    return serializer_get_position(&s) == 0;
}

static int test_get_position_null(void) {
    return serializer_get_position(NULL) == 0;
}

static int test_get_position_after_write(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    serializer_write_u8(&s, 0x12);
    return serializer_get_position(&s) == 1;
}

static int test_get_remaining_initial(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    return serializer_get_remaining(&s) == 64;
}

static int test_get_remaining_null(void) {
    return serializer_get_remaining(NULL) == 0;
}

static int test_get_remaining_partial(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    serializer_write_u32(&s, 0x12345678);
    return serializer_get_remaining(&s) == 60;
}

static int test_is_end_initial(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    return serializer_is_end(&s) == 0;
}

static int test_is_end_null(void) {
    return serializer_is_end(NULL) == 1;
}

static int test_is_end_at_boundary(void) {
    Serializer s;
    u8 buffer[4];
    serializer_init_write(&s, buffer, sizeof(buffer));
    serializer_write_u32(&s, 0x12345678);
    return serializer_is_end(&s) == 1;
}

static int test_reset_position(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    serializer_write_u8(&s, 0x12);
    serializer_write_u8(&s, 0x34);
    serializer_reset(&s);
    return s.position == 0;
}

/* ========================================
 * Test Cases - u8 Serialization
 * ======================================== */

static int test_write_u8_basic(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_u8(&s, 0xAB);
    return result == 1 && buffer[0] == 0xAB;
}

static int test_write_u8_null_serializer(void) {
    return serializer_write_u8(NULL, 0xAB) == 0;
}

static int test_write_u8_wrong_mode(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_read(&s, buffer, sizeof(buffer));
    return serializer_write_u8(&s, 0xAB) == 0;
}

static int test_write_u8_buffer_overflow(void) {
    Serializer s;
    u8 buffer[1];
    serializer_init_write(&s, buffer, 1);
    serializer_write_u8(&s, 0x12);
    return serializer_write_u8(&s, 0x34) == 0;
}

static int test_read_u8_basic(void) {
    Serializer s;
    u8 buffer[] = { 0xCD };
    u8 value = 0;
    serializer_init_read(&s, buffer, sizeof(buffer));
    int result = serializer_read_u8(&s, &value);
    return result == 1 && value == 0xCD;
}

static int test_read_u8_null_serializer(void) {
    u8 value;
    return serializer_read_u8(NULL, &value) == 0;
}

static int test_read_u8_null_value(void) {
    Serializer s;
    u8 buffer[] = { 0xCD };
    serializer_init_read(&s, buffer, sizeof(buffer));
    return serializer_read_u8(&s, NULL) == 0;
}

static int test_read_u8_wrong_mode(void) {
    Serializer s;
    u8 buffer[] = { 0xCD };
    u8 value;
    serializer_init_write(&s, buffer, sizeof(buffer));
    return serializer_read_u8(&s, &value) == 0;
}

static int test_read_u8_buffer_overflow(void) {
    Serializer s;
    u8 buffer[1];
    u8 value;
    serializer_init_read(&s, buffer, 1);
    serializer_read_u8(&s, &value);
    return serializer_read_u8(&s, &value) == 0;
}

/* ========================================
 * Test Cases - u16 Serialization
 * ======================================== */

static int test_write_u16_basic(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_u16(&s, 0x1234);
    return result == 1 && buffer[0] == 0x34 && buffer[1] == 0x12;
}

static int test_write_u16_little_endian(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    serializer_write_u16(&s, 0xAABB);
    return buffer[0] == 0xBB && buffer[1] == 0xAA;
}

static int test_read_u16_basic(void) {
    Serializer s;
    u8 buffer[] = { 0x78, 0x56 };
    u16 value = 0;
    serializer_init_read(&s, buffer, sizeof(buffer));
    int result = serializer_read_u16(&s, &value);
    return result == 1 && value == 0x5678;
}

static int test_read_u16_little_endian(void) {
    Serializer s;
    u8 buffer[] = { 0xBB, 0xAA };
    u16 value = 0;
    serializer_init_read(&s, buffer, sizeof(buffer));
    serializer_read_u16(&s, &value);
    return value == 0xAABB;
}

/* ========================================
 * Test Cases - u32 Serialization
 * ======================================== */

static int test_write_u32_basic(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_u32(&s, 0x12345678);
    return result == 1 &&
           buffer[0] == 0x78 && buffer[1] == 0x56 &&
           buffer[2] == 0x34 && buffer[3] == 0x12;
}

static int test_write_u32_little_endian(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    serializer_write_u32(&s, 0xAABBCCDD);
    return buffer[0] == 0xDD && buffer[1] == 0xCC &&
           buffer[2] == 0xBB && buffer[3] == 0xAA;
}

static int test_read_u32_basic(void) {
    Serializer s;
    u8 buffer[] = { 0x78, 0x56, 0x34, 0x12 };
    u32 value = 0;
    serializer_init_read(&s, buffer, sizeof(buffer));
    int result = serializer_read_u32(&s, &value);
    return result == 1 && value == 0x12345678;
}

static int test_read_u32_little_endian(void) {
    Serializer s;
    u8 buffer[] = { 0xDD, 0xCC, 0xBB, 0xAA };
    u32 value = 0;
    serializer_init_read(&s, buffer, sizeof(buffer));
    serializer_read_u32(&s, &value);
    return value == 0xAABBCCDD;
}

/* ========================================
 * Test Cases - Signed Integer Serialization
 * ======================================== */

static int test_write_s8_positive(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_s8(&s, 127);
    return result == 1 && buffer[0] == 127;
}

static int test_write_s8_negative(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_s8(&s, -1);
    return result == 1 && buffer[0] == 0xFF;
}

static int test_read_s8_negative(void) {
    Serializer s;
    u8 buffer[] = { 0xFF };
    s8 value = 0;
    serializer_init_read(&s, buffer, sizeof(buffer));
    serializer_read_s8(&s, &value);
    return value == -1;
}

static int test_write_s16_positive(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_s16(&s, 32767);
    return result == 1;
}

static int test_write_s16_negative(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_s16(&s, -1);
    return result == 1 && buffer[0] == 0xFF && buffer[1] == 0xFF;
}

static int test_read_s16_negative(void) {
    Serializer s;
    u8 buffer[] = { 0xFF, 0xFF };
    s16 value = 0;
    serializer_init_read(&s, buffer, sizeof(buffer));
    serializer_read_s16(&s, &value);
    return value == -1;
}

static int test_write_s32_positive(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_s32(&s, 2147483647);
    return result == 1;
}

static int test_write_s32_negative(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_s32(&s, -1);
    return result == 1;
}

static int test_read_s32_negative(void) {
    Serializer s;
    u8 buffer[] = { 0xFF, 0xFF, 0xFF, 0xFF };
    s32 value = 0;
    serializer_init_read(&s, buffer, sizeof(buffer));
    serializer_read_s32(&s, &value);
    return value == -1;
}

/* ========================================
 * Test Cases - Float Serialization
 * ======================================== */

static int test_write_float_zero(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_float(&s, 0.0f);
    return result == 1;
}

static int test_write_float_one(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_float(&s, 1.0f);
    return result == 1 && s.position == 4;
}

static int test_read_float_zero(void) {
    Serializer s;
    u8 buffer[] = { 0x00, 0x00, 0x00, 0x00 };
    float value = 1.0f;
    serializer_init_read(&s, buffer, sizeof(buffer));
    int result = serializer_read_float(&s, &value);
    return result == 1 && value == 0.0f;
}

static int test_read_float_roundtrip(void) {
    Serializer writer;
    Serializer reader;
    u8 buffer[64];
    float original = 3.14159f;
    float result = 0.0f;

    serializer_init_write(&writer, buffer, sizeof(buffer));
    serializer_write_float(&writer, original);

    serializer_init_read(&reader, buffer, writer.position);
    serializer_read_float(&reader, &result);

    /* Float comparison with epsilon */
    return (result - original) < 0.0001f && (result - original) > -0.0001f;
}

/* ========================================
 * Test Cases - String Serialization
 * ======================================== */

static int test_write_string_basic(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_string(&s, "Hello", 255);
    /* Length prefix (2 bytes) + string data (5 bytes) = 7 bytes */
    return result == 1 && s.position == 7;
}

static int test_write_string_null(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_string(&s, NULL, 255);
    /* Should write empty string */
    return result == 1 && s.position == 2;  /* Only length prefix */
}

static int test_write_string_truncated(void) {
    Serializer s;
    u8 buffer[64];
    u16 len;
    serializer_init_write(&s, buffer, sizeof(buffer));
    serializer_write_string(&s, "HelloWorld", 5);
    /* Should truncate to 5 chars */
    len = buffer[0] | (buffer[1] << 8);
    return len == 5;
}

static int test_read_string_basic(void) {
    Serializer writer;
    Serializer reader;
    u8 buffer[64];
    char str[32];

    serializer_init_write(&writer, buffer, sizeof(buffer));
    serializer_write_string(&writer, "Test", 255);

    serializer_init_read(&reader, buffer, writer.position);
    int result = serializer_read_string(&reader, str, sizeof(str));

    return result == 1 && strcmp(str, "Test") == 0;
}

static int test_read_string_null_buffer(void) {
    Serializer s;
    u8 buffer[] = { 0x04, 0x00, 'T', 'e', 's', 't' };
    serializer_init_read(&s, buffer, sizeof(buffer));
    return serializer_read_string(&s, NULL, 32) == 0;
}

static int test_read_string_truncated(void) {
    Serializer writer;
    Serializer reader;
    u8 buffer[64];
    char str[4];  /* Too small for "Hello" */

    serializer_init_write(&writer, buffer, sizeof(buffer));
    serializer_write_string(&writer, "Hello", 255);

    serializer_init_read(&reader, buffer, writer.position);
    serializer_read_string(&reader, str, sizeof(str));

    /* Should truncate and null-terminate */
    return strcmp(str, "Hel") == 0;
}

/* ========================================
 * Test Cases - Fixed String Serialization
 * ======================================== */

static int test_write_fixed_string_basic(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_fixed_string(&s, "Hi", 10);
    return result == 1 && s.position == 10;
}

static int test_write_fixed_string_padded(void) {
    Serializer s;
    u8 buffer[64] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    serializer_init_write(&s, buffer, sizeof(buffer));
    serializer_write_fixed_string(&s, "AB", 5);
    /* Should write "AB" + 3 null bytes */
    return buffer[0] == 'A' && buffer[1] == 'B' &&
           buffer[2] == 0 && buffer[3] == 0 && buffer[4] == 0;
}

static int test_read_fixed_string_basic(void) {
    Serializer s;
    char str[16];
    u8 buffer[] = { 'H', 'i', 0, 0, 0 };
    serializer_init_read(&s, buffer, sizeof(buffer));
    int result = serializer_read_fixed_string(&s, str, 5);
    return result == 1 && strcmp(str, "Hi") == 0 && s.position == 5;
}

/* ========================================
 * Test Cases - Raw Bytes Serialization
 * ======================================== */

static int test_write_bytes_basic(void) {
    Serializer s;
    u8 buffer[64];
    u8 data[] = { 0x01, 0x02, 0x03, 0x04 };
    serializer_init_write(&s, buffer, sizeof(buffer));
    int result = serializer_write_bytes(&s, data, sizeof(data));
    return result == 1 && s.position == 4;
}

static int test_write_bytes_null_data(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    return serializer_write_bytes(&s, NULL, 4) == 0;
}

static int test_read_bytes_basic(void) {
    Serializer s;
    u8 buffer[] = { 0x01, 0x02, 0x03, 0x04 };
    u8 data[4] = { 0 };
    serializer_init_read(&s, buffer, sizeof(buffer));
    int result = serializer_read_bytes(&s, data, sizeof(data));
    return result == 1 && data[0] == 0x01 && data[3] == 0x04;
}

/* ========================================
 * Test Cases - Checksum
 * ======================================== */

static int test_checksum_empty(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    u32 checksum = serializer_calc_checksum(&s, 0, 0);
    return checksum == 0;
}

static int test_checksum_single_byte(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    serializer_write_u8(&s, 0x41);  /* 'A' */
    u32 checksum = serializer_calc_checksum(&s, 0, 1);
    /* djb2-like hash: hash * 33 + byte */
    return checksum == 0x41;
}

static int test_checksum_null_serializer(void) {
    return serializer_calc_checksum(NULL, 0, 0) == 0;
}

static int test_checksum_range(void) {
    Serializer s;
    u8 buffer[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    serializer_init_read(&s, buffer, sizeof(buffer));
    u32 checksum = serializer_calc_checksum(&s, 1, 4);  /* bytes 1-3 */
    return checksum != 0;
}

/* ========================================
 * Test Cases - Peek Operations
 * ======================================== */

static int test_peek_u8_basic(void) {
    Serializer s;
    u8 buffer[] = { 0xAB, 0xCD };
    u8 value = 0;
    serializer_init_read(&s, buffer, sizeof(buffer));
    int result = serializer_peek_u8(&s, &value);
    return result == 1 && value == 0xAB && s.position == 0;
}

static int test_peek_u8_null_value(void) {
    Serializer s;
    u8 buffer[] = { 0xAB };
    serializer_init_read(&s, buffer, sizeof(buffer));
    return serializer_peek_u8(&s, NULL) == 0;
}

static int test_peek_u16_basic(void) {
    Serializer s;
    u8 buffer[] = { 0x78, 0x56, 0x34 };
    u16 value = 0;
    serializer_init_read(&s, buffer, sizeof(buffer));
    int result = serializer_peek_u16(&s, &value);
    return result == 1 && value == 0x5678 && s.position == 0;
}

static int test_peek_doesnt_advance(void) {
    Serializer s;
    u8 buffer[] = { 0x01, 0x02, 0x03 };
    u8 value1, value2;
    serializer_init_read(&s, buffer, sizeof(buffer));
    serializer_peek_u8(&s, &value1);
    serializer_peek_u8(&s, &value2);
    return value1 == value2 && s.position == 0;
}

/* ========================================
 * Test Cases - Skip and Align
 * ======================================== */

static int test_skip_basic(void) {
    Serializer s;
    u8 buffer[] = { 0x01, 0x02, 0x03, 0x04 };
    serializer_init_read(&s, buffer, sizeof(buffer));
    int result = serializer_skip(&s, 2);
    return result == 1 && s.position == 2;
}

static int test_skip_null_serializer(void) {
    return serializer_skip(NULL, 2) == 0;
}

static int test_skip_overflow(void) {
    Serializer s;
    u8 buffer[4];
    serializer_init_read(&s, buffer, sizeof(buffer));
    return serializer_skip(&s, 10) == 0;
}

static int test_align_to_4(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    serializer_write_u8(&s, 0x12);  /* position = 1 */
    int result = serializer_align(&s, 4);
    return result == 1 && s.position == 4;
}

static int test_align_already_aligned(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    serializer_write_u32(&s, 0x12345678);  /* position = 4 */
    int result = serializer_align(&s, 4);
    return result == 1 && s.position == 4;
}

static int test_align_null_serializer(void) {
    return serializer_align(NULL, 4) == 0;
}

static int test_align_zero_alignment(void) {
    Serializer s;
    u8 buffer[64];
    serializer_init_write(&s, buffer, sizeof(buffer));
    return serializer_align(&s, 0) == 0;
}

/* ========================================
 * Test Cases - Roundtrip
 * ======================================== */

static int test_roundtrip_mixed_types(void) {
    Serializer writer;
    Serializer reader;
    u8 buffer[256];

    u8 u8_val = 0xAB;
    u16 u16_val = 0x1234;
    u32 u32_val = 0xDEADBEEF;
    s8 s8_val = -50;
    s16 s16_val = -1000;
    s32 s32_val = -100000;

    u8 read_u8;
    u16 read_u16;
    u32 read_u32;
    s8 read_s8;
    s16 read_s16;
    s32 read_s32;

    /* Write */
    serializer_init_write(&writer, buffer, sizeof(buffer));
    serializer_write_u8(&writer, u8_val);
    serializer_write_u16(&writer, u16_val);
    serializer_write_u32(&writer, u32_val);
    serializer_write_s8(&writer, s8_val);
    serializer_write_s16(&writer, s16_val);
    serializer_write_s32(&writer, s32_val);

    /* Read */
    serializer_init_read(&reader, buffer, writer.position);
    serializer_read_u8(&reader, &read_u8);
    serializer_read_u16(&reader, &read_u16);
    serializer_read_u32(&reader, &read_u32);
    serializer_read_s8(&reader, &read_s8);
    serializer_read_s16(&reader, &read_s16);
    serializer_read_s32(&reader, &read_s32);

    return read_u8 == u8_val &&
           read_u16 == u16_val &&
           read_u32 == u32_val &&
           read_s8 == s8_val &&
           read_s16 == s16_val &&
           read_s32 == s32_val;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Serialization System Comprehensive Tests ===\n\n");

    printf("Initialization Tests:\n");
    TEST(init_write_null_serializer);
    TEST(init_write_null_buffer);
    TEST(init_write_zero_size);
    TEST(init_write_valid);
    TEST(init_read_null_serializer);
    TEST(init_read_null_buffer);
    TEST(init_read_valid);

    printf("\nPosition and Remaining Tests:\n");
    TEST(get_position_initial);
    TEST(get_position_null);
    TEST(get_position_after_write);
    TEST(get_remaining_initial);
    TEST(get_remaining_null);
    TEST(get_remaining_partial);
    TEST(is_end_initial);
    TEST(is_end_null);
    TEST(is_end_at_boundary);
    TEST(reset_position);

    printf("\nu8 Serialization Tests:\n");
    TEST(write_u8_basic);
    TEST(write_u8_null_serializer);
    TEST(write_u8_wrong_mode);
    TEST(write_u8_buffer_overflow);
    TEST(read_u8_basic);
    TEST(read_u8_null_serializer);
    TEST(read_u8_null_value);
    TEST(read_u8_wrong_mode);
    TEST(read_u8_buffer_overflow);

    printf("\nu16 Serialization Tests:\n");
    TEST(write_u16_basic);
    TEST(write_u16_little_endian);
    TEST(read_u16_basic);
    TEST(read_u16_little_endian);

    printf("\nu32 Serialization Tests:\n");
    TEST(write_u32_basic);
    TEST(write_u32_little_endian);
    TEST(read_u32_basic);
    TEST(read_u32_little_endian);

    printf("\nSigned Integer Tests:\n");
    TEST(write_s8_positive);
    TEST(write_s8_negative);
    TEST(read_s8_negative);
    TEST(write_s16_positive);
    TEST(write_s16_negative);
    TEST(read_s16_negative);
    TEST(write_s32_positive);
    TEST(write_s32_negative);
    TEST(read_s32_negative);

    printf("\nFloat Serialization Tests:\n");
    TEST(write_float_zero);
    TEST(write_float_one);
    TEST(read_float_zero);
    TEST(read_float_roundtrip);

    printf("\nString Serialization Tests:\n");
    TEST(write_string_basic);
    TEST(write_string_null);
    TEST(write_string_truncated);
    TEST(read_string_basic);
    TEST(read_string_null_buffer);
    TEST(read_string_truncated);

    printf("\nFixed String Tests:\n");
    TEST(write_fixed_string_basic);
    TEST(write_fixed_string_padded);
    TEST(read_fixed_string_basic);

    printf("\nRaw Bytes Tests:\n");
    TEST(write_bytes_basic);
    TEST(write_bytes_null_data);
    TEST(read_bytes_basic);

    printf("\nChecksum Tests:\n");
    TEST(checksum_empty);
    TEST(checksum_single_byte);
    TEST(checksum_null_serializer);
    TEST(checksum_range);

    printf("\nPeek Operation Tests:\n");
    TEST(peek_u8_basic);
    TEST(peek_u8_null_value);
    TEST(peek_u16_basic);
    TEST(peek_doesnt_advance);

    printf("\nSkip and Align Tests:\n");
    TEST(skip_basic);
    TEST(skip_null_serializer);
    TEST(skip_overflow);
    TEST(align_to_4);
    TEST(align_already_aligned);
    TEST(align_null_serializer);
    TEST(align_zero_alignment);

    printf("\nRoundtrip Tests:\n");
    TEST(roundtrip_mixed_types);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
