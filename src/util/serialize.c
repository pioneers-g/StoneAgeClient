/*
 * Stone Age Client - Serialization System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "serialize.h"
#include "logger.h"

/*
 * Initialize serializer for writing
 */
int serializer_init_write(Serializer* s, void* buffer, u32 size) {
    if (!s || !buffer || size == 0) return 0;

    s->buffer = (u8*)buffer;
    s->size = size;
    s->position = 0;
    s->mode = SERIALIZE_WRITE;

    return 1;
}

/*
 * Initialize serializer for reading
 */
int serializer_init_read(Serializer* s, const void* buffer, u32 size) {
    if (!s || !buffer || size == 0) return 0;

    s->buffer = (u8*)buffer;
    s->size = size;
    s->position = 0;
    s->mode = SERIALIZE_READ;

    return 1;
}

/*
 * Reset serializer position
 */
void serializer_reset(Serializer* s) {
    if (s) {
        s->position = 0;
    }
}

/*
 * Get current position
 */
u32 serializer_get_position(Serializer* s) {
    return s ? s->position : 0;
}

/*
 * Get remaining bytes
 */
u32 serializer_get_remaining(Serializer* s) {
    if (!s || s->position >= s->size) return 0;
    return s->size - s->position;
}

/*
 * Check if at end
 */
int serializer_is_end(Serializer* s) {
    return s ? (s->position >= s->size) : 1;
}

/*
 * Write u8
 */
int serializer_write_u8(Serializer* s, u8 value) {
    if (!s || s->mode != SERIALIZE_WRITE || s->position + 1 > s->size) {
        return 0;
    }

    s->buffer[s->position++] = value;
    return 1;
}

/*
 * Read u8
 */
int serializer_read_u8(Serializer* s, u8* value) {
    if (!s || s->mode != SERIALIZE_READ || s->position + 1 > s->size || !value) {
        return 0;
    }

    *value = s->buffer[s->position++];
    return 1;
}

/*
 * Write u16
 */
int serializer_write_u16(Serializer* s, u16 value) {
    if (!s || s->mode != SERIALIZE_WRITE || s->position + 2 > s->size) {
        return 0;
    }

    s->buffer[s->position++] = value & 0xFF;
    s->buffer[s->position++] = (value >> 8) & 0xFF;

    return 1;
}

/*
 * Read u16
 */
int serializer_read_u16(Serializer* s, u16* value) {
    if (!s || s->mode != SERIALIZE_READ || s->position + 2 > s->size || !value) {
        return 0;
    }

    *value = s->buffer[s->position] |
             (s->buffer[s->position + 1] << 8);
    s->position += 2;

    return 1;
}

/*
 * Write u32
 */
int serializer_write_u32(Serializer* s, u32 value) {
    if (!s || s->mode != SERIALIZE_WRITE || s->position + 4 > s->size) {
        return 0;
    }

    s->buffer[s->position++] = value & 0xFF;
    s->buffer[s->position++] = (value >> 8) & 0xFF;
    s->buffer[s->position++] = (value >> 16) & 0xFF;
    s->buffer[s->position++] = (value >> 24) & 0xFF;

    return 1;
}

/*
 * Read u32
 */
int serializer_read_u32(Serializer* s, u32* value) {
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

/*
 * Write s8
 */
int serializer_write_s8(Serializer* s, s8 value) {
    return serializer_write_u8(s, (u8)value);
}

/*
 * Read s8
 */
int serializer_read_s8(Serializer* s, s8* value) {
    return serializer_read_u8(s, (u8*)value);
}

/*
 * Write s16
 */
int serializer_write_s16(Serializer* s, s16 value) {
    return serializer_write_u16(s, (u16)value);
}

/*
 * Read s16
 */
int serializer_read_s16(Serializer* s, s16* value) {
    return serializer_read_u16(s, (u16*)value);
}

/*
 * Write s32
 */
int serializer_write_s32(Serializer* s, s32 value) {
    return serializer_write_u32(s, (u32)value);
}

/*
 * Read s32
 */
int serializer_read_s32(Serializer* s, s32* value) {
    return serializer_read_u32(s, (u32*)value);
}

/*
 * Write float
 */
int serializer_write_float(Serializer* s, float value) {
    u32 temp;
    memcpy(&temp, &value, sizeof(float));
    return serializer_write_u32(s, temp);
}

/*
 * Read float
 */
int serializer_read_float(Serializer* s, float* value) {
    u32 temp;
    if (!serializer_read_u32(s, &temp)) return 0;
    memcpy(value, &temp, sizeof(float));
    return 1;
}

/*
 * Write string (length-prefixed)
 */
int serializer_write_string(Serializer* s, const char* str, u16 max_len) {
    u16 len;
    const char* null_str = "";

    if (!s) return 0;

    if (!str) str = null_str;

    len = (u16)strlen(str);
    if (len > max_len) len = max_len;

    /* Write length */
    if (!serializer_write_u16(s, len)) return 0;

    /* Write string data */
    if (s->position + len > s->size) return 0;

    memcpy(s->buffer + s->position, str, len);
    s->position += len;

    return 1;
}

/*
 * Read string
 */
int serializer_read_string(Serializer* s, char* str, u16 max_len) {
    u16 len;

    if (!s || !str) return 0;

    /* Read length */
    if (!serializer_read_u16(s, &len)) return 0;

    if (len >= max_len) {
        len = max_len - 1;
    }

    /* Read string data */
    if (s->position + len > s->size) return 0;

    memcpy(str, s->buffer + s->position, len);
    str[len] = '\0';
    s->position += len;

    return 1;
}

/*
 * Write fixed-size string
 */
int serializer_write_fixed_string(Serializer* s, const char* str, u16 size) {
    u16 len;

    if (!s || !str) return 0;

    len = (u16)strlen(str);
    if (len > size) len = size;

    /* Write string data */
    if (s->position + size > s->size) return 0;

    memset(s->buffer + s->position, 0, size);
    memcpy(s->buffer + s->position, str, len);
    s->position += size;

    return 1;
}

/*
 * Read fixed-size string
 */
int serializer_read_fixed_string(Serializer* s, char* str, u16 size) {
    if (!s || !str) return 0;

    if (s->position + size > s->size) return 0;

    memcpy(str, s->buffer + s->position, size);
    str[size] = '\0';
    s->position += size;

    return 1;
}

/*
 * Write raw bytes
 */
int serializer_write_bytes(Serializer* s, const void* data, u32 size) {
    if (!s || !data || size == 0) return 0;

    if (s->position + size > s->size) return 0;

    memcpy(s->buffer + s->position, data, size);
    s->position += size;

    return 1;
}

/*
 * Read raw bytes
 */
int serializer_read_bytes(Serializer* s, void* data, u32 size) {
    if (!s || !data || size == 0) return 0;

    if (s->position + size > s->size) return 0;

    memcpy(data, s->buffer + s->position, size);
    s->position += size;

    return 1;
}

/*
 * Write checksum
 */
u32 serializer_calc_checksum(Serializer* s, u32 start, u32 end) {
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

/*
 * Write buffer to file
 */
int serializer_save_to_file(Serializer* s, const char* filename) {
    FILE* fp;

    if (!s || !filename) return 0;

    fp = fopen(filename, "wb");
    if (!fp) return 0;

    fwrite(s->buffer, 1, s->position, fp);
    fclose(fp);

    return 1;
}

/*
 * Load buffer from file
 */
int serializer_load_from_file(Serializer* s, const char* filename) {
    FILE* fp;
    u32 size;

    if (!s || !filename) return 0;

    fp = fopen(filename, "rb");
    if (!fp) return 0;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size > s->size) {
        fclose(fp);
        return 0;
    }

    fread(s->buffer, 1, size, fp);
    fclose(fp);

    s->position = 0;
    s->size = size;

    return 1;
}

/*
 * Peek next byte
 */
int serializer_peek_u8(Serializer* s, u8* value) {
    if (!s || s->position + 1 > s->size || !value) return 0;

    *value = s->buffer[s->position];
    return 1;
}

/*
 * Peek next u16
 */
int serializer_peek_u16(Serializer* s, u16* value) {
    if (!s || s->position + 2 > s->size || !value) return 0;

    *value = s->buffer[s->position] |
             (s->buffer[s->position + 1] << 8);
    return 1;
}

/*
 * Skip bytes
 */
int serializer_skip(Serializer* s, u32 bytes) {
    if (!s || s->position + bytes > s->size) return 0;

    s->position += bytes;
    return 1;
}

/*
 * Align to boundary
 */
int serializer_align(Serializer* s, u32 alignment) {
    u32 padding;

    if (!s || alignment == 0) return 0;

    padding = (alignment - (s->position % alignment)) % alignment;

    if (s->position + padding > s->size) return 0;

    s->position += padding;
    return 1;
}
