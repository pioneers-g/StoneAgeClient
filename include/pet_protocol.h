/*
 * Stone Age Client - Pet Protocol Parsing Header
 * Split from pet.h for code organization
 */

#ifndef PET_PROTOCOL_H
#define PET_PROTOCOL_H

#include "types.h"

/* PetInfo and EscapePetInfo are defined in pet.h */
typedef struct PetInfo PetInfo;
typedef struct EscapePetInfo EscapePetInfo;

/* Field parsing - FUN_00489f70 pattern */
int pet_parse_field(const char* str, char delimiter, int field_index, char* output, int max_len);

/* Integer parsing - FUN_0048a050 pattern */
int pet_parse_field_int(const char* str, char delimiter, int field_index);

/* Base-62 integer parsing - FUN_0048a120 pattern
 * Used for bitmask fields in protocol packets (0x4B, 0x4E, 0x50)
 * Base-62 encoding uses: 0-9 (0-9), a-z (10-35), A-Z (36-61)
 */
int pet_parse_field_base62(const char* str, char delimiter, int field_index);

/* Base-62 decoding - FUN_0048a0a0 pattern */
int pet_decode_base62(const char* str);

/* String unescaping - FUN_0048a170 pattern */
void pet_unescape_string(char* str);

/* String escaping - FUN_0048a200 pattern */
int pet_escape_string(const char* src, char* dest, int max_len);

/* Subfield parsing - FUN_00425230 pattern */
void pet_parse_subfields(char* dest, const char* src, int field_size, int field_count, int max_elem);

/* Capture list parsing - FUN_00425dc0 pattern */
void pet_parse_capture_list(const char* packet_data);

/* Escape list parsing - FUN_0042af40 pattern */
void pet_parse_escape_list(const char* packet_data);

/* Pet info access */
PetInfo* pet_get_info(int index);
EscapePetInfo* pet_get_escape_info(int index);
int pet_get_info_count(void);

/* Pet skill data parsing - FUN_0042f130 */
void pet_parse_skill_data(const char* packet_data);
int pet_get_skill_count(void);
void pet_clear_skill_data(void);

#endif /* PET_PROTOCOL_H */
