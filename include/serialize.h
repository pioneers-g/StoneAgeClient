/*
 * Stone Age Client - Serialization System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "types.h"

/* Serialization mode */
typedef enum {
    SERIALIZE_READ = 0,
    SERIALIZE_WRITE = 1
} SerializeMode;

/* Serializer structure */
typedef struct {
    u8* buffer;
    u32 size;
    u32 position;
    SerializeMode mode;

} Serializer;

/* Initialization */
int serializer_init_write(Serializer* s, void* buffer, u32 size);
int serializer_init_read(Serializer* s, const void* buffer, u32 size);
void serializer_reset(Serializer* s);

/* Position */
u32 serializer_get_position(Serializer* s);
u32 serializer_get_remaining(Serializer* s);
int serializer_is_end(Serializer* s);

/* Write functions */
int serializer_write_u8(Serializer* s, u8 value);
int serializer_write_u16(Serializer* s, u16 value);
int serializer_write_u32(Serializer* s, u32 value);
int serializer_write_s8(Serializer* s, s8 value);
int serializer_write_s16(Serializer* s, s16 value);
int serializer_write_s32(Serializer* s, s32 value);
int serializer_write_float(Serializer* s, float value);
int serializer_write_string(Serializer* s, const char* str, u16 max_len);
int serializer_write_fixed_string(Serializer* s, const char* str, u16 size);
int serializer_write_bytes(Serializer* s, const void* data, u32 size);

/* Read functions */
int serializer_read_u8(Serializer* s, u8* value);
int serializer_read_u16(Serializer* s, u16* value);
int serializer_read_u32(Serializer* s, u32* value);
int serializer_read_s8(Serializer* s, s8* value);
int serializer_read_s16(Serializer* s, s16* value);
int serializer_read_s32(Serializer* s, s32* value);
int serializer_read_float(Serializer* s, float* value);
int serializer_read_string(Serializer* s, char* str, u16 max_len);
int serializer_read_fixed_string(Serializer* s, char* str, u16 size);
int serializer_read_bytes(Serializer* s, void* data, u32 size);

/* Peek functions */
int serializer_peek_u8(Serializer* s, u8* value);
int serializer_peek_u16(Serializer* s, u16* value);

/* Utility */
int serializer_skip(Serializer* s, u32 bytes);
int serializer_align(Serializer* s, u32 alignment);
u32 serializer_calc_checksum(Serializer* s, u32 start, u32 end);

/* File operations */
int serializer_save_to_file(Serializer* s, const char* filename);
int serializer_load_from_file(Serializer* s, const char* filename);

#endif /* SERIALIZE_H */
