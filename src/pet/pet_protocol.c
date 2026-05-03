/*
 * Stone Age Client - Pet Protocol Parsing
 * Reverse engineered from sa_9061.exe
 * Split from pet.c for code organization
 *
 * Core protocol parsing functions:
 * - FUN_00489f70: Field parsing
 * - FUN_0048a050: Integer parsing
 * - FUN_0048a170: String unescaping
 * - FUN_00425230: Subfield parsing
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "pet.h"
#include "logger.h"

/* External pet context */
extern PetContext g_pet;

/* Pet info arrays - matching DAT_0455b5e0 and DAT_0454bba4 */
static PetInfo s_pet_info[MAX_ENEMY_PET_SLOTS];
static EscapePetInfo s_escape_info[MAX_ESCAPE_PET_SLOTS];
static int s_pet_info_count = 0;
static int s_escape_info_count = 0;

/*
 * Parse delimited string field - FUN_00489f70 pattern
 * Extracts the Nth field (1-indexed) from a delimiter-separated string
 */
int pet_parse_field(const char* str, char delimiter, int field_index, char* output, int max_len) {
    const char* ptr = str;
    int current_field = 1;
    int out_pos = 0;
    unsigned char c;

    if (!str || !output || max_len <= 0) {
        return -1;
    }

    output[0] = '\0';

    /* Skip to the desired field */
    while (current_field < field_index && *ptr) {
        c = (unsigned char)*ptr;

        /* Handle multi-byte characters */
        if (c >= 0x80) {
            /* DBCS lead byte */
            if (ptr[1] == '\0') break;
            if (*ptr == (unsigned char)delimiter) {
                current_field++;
                ptr++;
            } else {
                ptr += 2;
            }
        } else {
            if (c == (unsigned char)delimiter) {
                current_field++;
            }
            ptr++;
        }
    }

    if (!*ptr) {
        return -1;  /* Field not found */
    }

    /* Extract the field */
    while (*ptr && out_pos < max_len - 1) {
        c = (unsigned char)*ptr;

        if (c >= 0x80) {
            /* DBCS character */
            if (ptr[1] == '\0') break;
            if (out_pos + 1 >= max_len - 1) break;

            output[out_pos++] = *ptr++;
            output[out_pos++] = *ptr++;
        } else {
            if (c == (unsigned char)delimiter) {
                break;
            }
            output[out_pos++] = *ptr++;
        }
    }

    output[out_pos] = '\0';
    return 0;
}

/*
 * Parse integer from delimited field - FUN_0048a050 pattern
 */
int pet_parse_field_int(const char* str, char delimiter, int field_index) {
    char buffer[128];

    if (pet_parse_field(str, delimiter, field_index, buffer, sizeof(buffer)) != 0) {
        return -1;
    }

    return atoi(buffer);
}

/*
 * Base-62 decoding - FUN_0048a0a0 pattern
 * Decodes Base-62 encoded string to integer
 * Character mapping: 0-9 -> 0-9, a-z -> 10-35, A-Z -> 36-61
 */
int pet_decode_base62(const char* str) {
    int result = 0;
    int sign = 1;
    const char* ptr = str;
    char c;

    if (!str || !*str) {
        return 0;
    }

    /* Handle negative sign */
    if (*ptr == '-') {
        sign = -1;
        ptr++;
    }

    while ((c = *ptr) != '\0') {
        result = result * 62;  /* 0x3e = 62 */

        if (c >= '0' && c <= '9') {
            result += c - '0';          /* 0-9 map to 0-9 */
        } else if (c >= 'a' && c <= 'z') {
            result += c - 'a' + 10;     /* a-z map to 10-35 */
        } else if (c >= 'A' && c <= 'Z') {
            result += c - 'A' + 36;     /* A-Z map to 36-61 */
        } else {
            return 0;  /* Invalid character */
        }

        ptr++;
    }

    return result * sign;
}

/*
 * Parse Base-62 encoded field - FUN_0048a120 pattern
 * Used for bitmask fields in protocol packets (cases 0x4B, 0x4E, 0x50)
 */
int pet_parse_field_base62(const char* str, char delimiter, int field_index) {
    char buffer[128];

    if (pet_parse_field(str, delimiter, field_index, buffer, sizeof(buffer)) != 0) {
        return -1;
    }

    if (buffer[0] == '\0') {
        return -1;
    }

    return pet_decode_base62(buffer);
}

/*
 * Unescape string - FUN_0048a170 pattern
 * Handles escape sequences: \S -> ' ', \n -> newline, \r -> CR, \\ -> '\'
 * Also handles game-specific: \c -> ',', \z -> '|', \y -> '\'
 */
void pet_unescape_string(char* str) {
    char* read = str;
    char* write = str;
    char c;

    if (!str) return;

    while (*read) {
        /* Check for DBCS lead byte */
        if (IsDBCSLeadByte(*read)) {
            *write++ = *read++;
            if (*read) {
                *write++ = *read++;
            }
            continue;
        }

        c = *read++;

        if (c == '\\' && *read) {
            /* Escape sequence */
            char esc = *read++;

            switch (esc) {
                case 'S':
                case 's':
                    *write++ = ' ';
                    break;
                case 'n':
                    *write++ = '\n';
                    break;
                case 'r':
                    *write++ = '\r';
                    break;
                case '\\':
                    *write++ = '\\';
                    break;
                case 'c':
                    *write++ = ',';
                    break;
                case 'z':
                    *write++ = '|';
                    break;
                case 'y':
                    *write++ = '\\';
                    break;
                default:
                    *write++ = esc;
                    break;
            }
        } else {
            *write++ = c;
        }
    }

    *write = '\0';
}

/*
 * Escape string - FUN_0048a200 pattern
 * Reverse of unescape: converts special characters to escape sequences
 * Used when sending data to server
 *
 * Escape table from DAT_004d5828/DAT_004d5829:
 *   ' ' -> \S, '\n' -> \n, '\r' -> \r, '\\' -> \\,
 *   ',' -> \c, '|' -> \z
 */
int pet_escape_string(const char* src, char* dest, int max_len) {
    const char* read = src;
    char* write = dest;
    int written = 1;  /* Count for null terminator */
    char c;

    if (!src || !dest || max_len < 1) {
        return 0;
    }

    while (*read && written < max_len) {
        /* Check for DBCS lead byte */
        if (IsDBCSLeadByte(*read)) {
            if (written + 2 >= max_len) break;
            *write++ = *read++;
            if (*read) {
                *write++ = *read++;
                written += 2;
            }
            continue;
        }

        c = *read++;

        /* Check if character needs escaping */
        switch (c) {
            case ' ':
                if (written + 2 >= max_len) goto done;
                *write++ = '\\';
                *write++ = 'S';
                written += 2;
                break;
            case '\n':
                if (written + 2 >= max_len) goto done;
                *write++ = '\\';
                *write++ = 'n';
                written += 2;
                break;
            case '\r':
                if (written + 2 >= max_len) goto done;
                *write++ = '\\';
                *write++ = 'r';
                written += 2;
                break;
            case '\\':
                if (written + 2 >= max_len) goto done;
                *write++ = '\\';
                *write++ = '\\';
                written += 2;
                break;
            case ',':
                if (written + 2 >= max_len) goto done;
                *write++ = '\\';
                *write++ = 'c';
                written += 2;
                break;
            case '|':
                if (written + 2 >= max_len) goto done;
                *write++ = '\\';
                *write++ = 'z';
                written += 2;
                break;
            default:
                *write++ = c;
                written++;
                break;
        }
    }

done:
    *write = '\0';
    return (int)(write - dest);
}

/*
 * Parse subfields into array - FUN_00425230 pattern
 * Parses a string with nested delimiters (10 for subfields, comma for elements)
 */
void pet_parse_subfields(char* dest, const char* src, int field_size, int field_count, int max_elem) {
    char buffer[256];
    int field_idx;
    char* write_ptr = dest;
    int len;

    if (!dest || !src || field_size <= 0 || field_count <= 0) return;

    /* Limit max elements */
    if (max_elem > field_size - 1) {
        max_elem = field_size - 1;
    }

    memset(dest, 0, field_size * field_count);

    for (field_idx = 1; field_idx <= field_count; field_idx++) {
        /* Get subfield from the source */
        if (pet_parse_field(src, 10, field_idx, buffer, sizeof(buffer)) != 0) {
            /* No more subfields, clear remaining */
            memset(write_ptr, 0, field_size * (field_count - field_idx + 1));
            break;
        }

        /* Copy to destination */
        len = strlen(buffer);
        if (len >= max_elem) {
            len = max_elem - 1;
        }

        memcpy(write_ptr, buffer, len);
        write_ptr[field_size - 1] = '\0';  /* Ensure null termination */

        write_ptr += field_size;
    }
}

/*
 * Parse capture pet list - FUN_00425dc0 pattern
 * Parses packet data with format: |field1|field2|...|fieldN|
 */
void pet_parse_capture_list(const char* packet_data) {
    const char* ptr = packet_data;
    char buffer[256];
    char name_buffer[64];
    int mode;
    int i;
    int field_offset;
    PetInfo* pet;
    int valid_count = 0;

    if (!packet_data) return;

    /* First field determines mode - DAT_04552f24 = FUN_0048a050(..., 1) */
    mode = pet_parse_field_int(packet_data, '|', 1);

    if (mode == 0) {
        /* Battle end - friendly encounter */
        /* DAT_0455a0fc = 10 */
        g_pet.capture_flags = 10;

        /* Parse encounter data */
        pet_parse_field(packet_data, '|', 3, name_buffer, 0x1a);
        pet_unescape_string(name_buffer);

        /* Store encounter name - DAT_045529ec */
        strncpy(s_pet_info[0].name, name_buffer, MAX_PET_NAME_LEN - 1);

        LOG_DEBUG("Battle end: %s", name_buffer);
    } else {
        /* Pet capture mode - DAT_0455a0fc = 100 */
        g_pet.capture_flags = 100;

        /* Parse capture data */
        pet_parse_field(packet_data, '|', 2, name_buffer, 0x1a);
        pet_unescape_string(name_buffer);

        LOG_DEBUG("Pet capture: %s", name_buffer);
    }

    /* Parse pet entries - DAT_0455b370 = 0x68 (104) entries max */
    s_pet_info_count = 0;
    for (i = 0; i < MAX_ENEMY_PET_SLOTS; i++) {
        /* Each pet has 7 fields, starting at field 8 for mode 0 */
        field_offset = (mode == 0) ? (8 + i * 7) : (7 + i * 7);

        /* Parse pet name - field_size = 0x3f (63) */
        if (pet_parse_field(packet_data, '|', field_offset - 1, name_buffer, 0x3f) != 0) {
            break;
        }

        pet_unescape_string(name_buffer);

        if (name_buffer[0] != '\0') {
            pet = &s_pet_info[valid_count];
            strncpy(pet->name, name_buffer, MAX_PET_NAME_LEN - 1);

            /* Parse additional fields */
            pet->data_0x10 = pet_parse_field_int(packet_data, '|', field_offset);
            pet->data_0x0c = pet_parse_field_int(packet_data, '|', field_offset + 1);
            pet->data_0x08 = pet_parse_field_int(packet_data, '|', field_offset + 2);

            /* Parse skill data */
            pet_parse_field(packet_data, '|', field_offset + 3, buffer, 0xff);

            valid_count++;
        }
    }

    /* Calculate final count: (count + 7) / 8 - DAT_0455b370 = (valid_count + 7) / 8 */
    s_pet_info_count = (valid_count + 7) / 8;
}

/*
 * Parse escape pet list - FUN_0042af40 pattern
 * Handles pets that run away during battle
 */
void pet_parse_escape_list(const char* packet_data) {
    char buffer[256];
    char name_buffer[64];
    int count;
    int i;
    int field_offset;
    EscapePetInfo* pet;
    int valid_count = 0;

    if (!packet_data) return;

    /* Clear flags - DAT_0455b0f0, DAT_0454f400 */
    g_pet.escape_flag = 0;
    g_pet.escape_result = 0;

    /* Check field 1 for count */
    count = pet_parse_field_int(packet_data, '|', 1);
    if (count == 0) return;

    /* Parse player name (field 2) */
    pet_parse_field(packet_data, '|', 2, name_buffer, 0x1a);
    pet_unescape_string(name_buffer);

    /* Store at DAT_0454b9ac */
    strncpy(g_pet.escape_pet_name, name_buffer, sizeof(g_pet.escape_pet_name) - 1);

    /* Parse target name (field 3) */
    pet_parse_field(packet_data, '|', 3, name_buffer, 0x1a);
    pet_unescape_string(name_buffer);

    /* Parse subfields - FUN_00425230 */
    /* Store in DAT_0454f604 region */

    /* Parse each escape pet - DAT_0455b370 = 0x50 (80) entries max */
    s_escape_info_count = 0;
    for (i = 0; i < MAX_ESCAPE_PET_SLOTS; i++) {
        /* Each pet has 4 fields, starting at field 5 */
        field_offset = 5 + i * 4;

        /* Parse pet name */
        if (pet_parse_field(packet_data, '|', field_offset - 1, name_buffer, 0x3f) != 0) {
            break;
        }

        pet_unescape_string(name_buffer);

        if (name_buffer[0] != '\0') {
            pet = &s_escape_info[valid_count];
            strncpy(pet->name, name_buffer, sizeof(pet->name) - 1);

            /* Parse fields */
            pet->data_0x0c = pet_parse_field_int(packet_data, '|', field_offset);
            pet->data_0x08 = pet_parse_field_int(packet_data, '|', field_offset + 2);

            valid_count++;
        }
    }

    /* Calculate final count */
    s_escape_info_count = (valid_count + 7) / 8;
    g_pet.escape_pet_count = s_escape_info_count;
}

/*
 * Get pet info by index
 */
PetInfo* pet_get_info(int index) {
    if (index < 0 || index >= s_pet_info_count) {
        return NULL;
    }
    return &s_pet_info[index];
}

/*
 * Get escape pet info by index
 */
EscapePetInfo* pet_get_escape_info(int index) {
    if (index < 0 || index >= s_escape_info_count) {
        return NULL;
    }
    return &s_escape_info[index];
}

/*
 * Get pet info count
 */
int pet_get_info_count(void) {
    return s_pet_info_count;
}

/*
 * Pet skill data entry - 0x418 bytes per entry
 * From FUN_0042f130 analysis
 */
typedef struct {
    u32 skill_id;              /* Offset -8 from base: skill ID */
    u32 field_04;              /* Offset -4: additional field */
    char name[512];            /* Offset 0: skill name (0x200 bytes) */
    char description[512];     /* Offset 0x200: skill description (0x200 bytes) */
    u32 field_400;             /* Offset 0x400: field 5 */
    u32 field_404;             /* Offset 0x404: field 4 */
    u32 field_408;             /* Offset 0x408: field 7 */
    u32 field_40c;             /* Offset 0x40c: field 8 */
} PetSkillDataEntry;

/* Pet skill data array - DAT_0454ffe0 region */
static PetSkillDataEntry s_pet_skill_data[7];  /* Max 7 pets per character */
static int s_pet_skill_count = 0;

/* Skill context state - DAT_0455efc8 region */
static int s_skill_context_active = 0;     /* DAT_0455efc8 */
static u32 s_skill_action_state = 0;       /* DAT_004b83ec */
static void* s_skill_window = NULL;        /* DAT_0455ef98 */

/* Counts from packet - DAT_0455efd0, DAT_0455efcc, DAT_0455efd4 */
static int s_skill_count_1 = 0;
static int s_skill_count_2 = 0;
static int s_skill_count_3 = 0;

/*
 * Parse pet skill data - FUN_0042f130
 * Called when receiving pet skill information from server
 *
 * Packet format: |field1|field2|field3|count|...
 * Each skill entry has subfields separated by spaces
 */
void pet_parse_skill_data(const char* packet_data) {
    const char* ptr = packet_data;
    char field_buffer[256];
    char subfield_buffer[512];
    char name_buffer[512];
    char desc_buffer[512];
    int i;
    int int_val;
    PetSkillDataEntry* entry;

    if (!packet_data) return;

    /* Mark skill context as active - DAT_0455efc8 = 1 */
    s_skill_context_active = 1;

    /* Check action state - DAT_004b83ec != 0xf */
    if (s_skill_action_state != 0xf) {
        s_skill_action_state = 0xf;

        /* Close skill window if open - FUN_004011c0(DAT_0455ef98) */
        if (s_skill_window != NULL) {
            /* Mark window for close */
            *(u32*)((char*)s_skill_window + 0x24) = 1;
            s_skill_window = NULL;
        }
    }

    /* Clear pet skill data array - DAT_0454ffe0 region */
    /* Entry stride is 0x418 bytes, loops while ptr < 0x45528d0 */
    memset(s_pet_skill_data, 0, sizeof(s_pet_skill_data));
    s_pet_skill_count = 0;

    /* Parse field 3: count_1 - DAT_0455efd0 */
    pet_parse_field(ptr, '|', 3, field_buffer, sizeof(field_buffer));
    s_skill_count_1 = atoi(field_buffer);

    /* Parse field 4: count_2 - DAT_0455efcc */
    pet_parse_field(ptr, '|', 4, field_buffer, sizeof(field_buffer));
    s_skill_count_2 = atoi(field_buffer);

    /* Parse field 5: count_3 (loop count) - DAT_0455efd4 */
    pet_parse_field(ptr, '|', 5, field_buffer, sizeof(field_buffer));
    s_skill_count_3 = atoi(field_buffer);

    LOG_DEBUG("Pet skill data: counts=%d, %d, %d", s_skill_count_1, s_skill_count_2, s_skill_count_3);

    /* Parse each skill entry */
    /* Loop: iVar3 = 0; while (iVar3 < DAT_0455efd4) */
    for (i = 0; i < s_skill_count_3 && i < 7; i++) {
        entry = &s_pet_skill_data[i];

        /* Get skill entry string (field 6+i, max 0x7f = 127 bytes) */
        pet_parse_field(ptr, '|', 6 + i, subfield_buffer, sizeof(subfield_buffer));

        /* Parse subfields within skill entry (space delimiter = 0x20) */

        /* Subfield 1: Skill ID (max 0xf = 15 bytes) */
        pet_parse_field(subfield_buffer, ' ', 1, field_buffer, sizeof(field_buffer));
        entry->skill_id = (u32)atoi(field_buffer);

        /* Subfield 2: Name (max 0x1ff = 511 bytes) */
        pet_parse_field(subfield_buffer, ' ', 2, name_buffer, sizeof(name_buffer));
        pet_unescape_string(name_buffer);
        strncpy(entry->name, name_buffer, sizeof(entry->name) - 1);

        /* Subfield 3: Description (max 0x1ff = 511 bytes) */
        pet_parse_field(subfield_buffer, ' ', 3, desc_buffer, sizeof(desc_buffer));
        pet_unescape_string(desc_buffer);
        strncpy(entry->description, desc_buffer, sizeof(entry->description) - 1);

        /* Subfield 5: field_400 (max 0xf = 15 bytes) */
        pet_parse_field(subfield_buffer, ' ', 5, field_buffer, sizeof(field_buffer));
        entry->field_400 = (u32)atoi(field_buffer);

        /* Subfield 4: field_404 (max 0xf = 15 bytes) */
        pet_parse_field(subfield_buffer, ' ', 4, field_buffer, sizeof(field_buffer));
        entry->field_404 = (u32)atoi(field_buffer);

        /* Subfield 6: field_04 (max 0xf = 15 bytes) */
        pet_parse_field(subfield_buffer, ' ', 6, field_buffer, sizeof(field_buffer));
        entry->field_04 = (u32)atoi(field_buffer);

        /* Subfield 7: field_408 (max 0xf = 15 bytes) */
        pet_parse_field(subfield_buffer, ' ', 7, field_buffer, sizeof(field_buffer));
        entry->field_408 = (u32)atoi(field_buffer);

        /* Subfield 8: field_40c (max 0xf = 15 bytes) */
        pet_parse_field(subfield_buffer, ' ', 8, field_buffer, sizeof(field_buffer));
        entry->field_40c = (u32)atoi(field_buffer);

        LOG_DEBUG("Pet skill %d: id=%u, name=%s", i, entry->skill_id, entry->name);

        s_pet_skill_count++;
    }

    /* Update global pet context battle skills */
    for (i = 0; i < s_pet_skill_count && i < MAX_PET_SKILLS_PER_PET; i++) {
        entry = &s_pet_skill_data[i];

        g_pet.battle_skills[i].id = (u16)entry->skill_id;
        g_pet.battle_skills[i].level = (u16)entry->field_400;
        g_pet.battle_skills[i].mp_cost = (u16)entry->field_404;
        g_pet.battle_skills[i].power = (u16)entry->field_04;
        g_pet.battle_skills[i].element = (u16)entry->field_408;
        g_pet.battle_skills[i].type = (u16)entry->field_40c;

        strncpy(g_pet.battle_skills[i].name, entry->name, sizeof(g_pet.battle_skills[i].name) - 1);
        strncpy(g_pet.battle_skills[i].description, entry->description, sizeof(g_pet.battle_skills[i].description) - 1);
    }
    g_pet.battle_skill_count = s_pet_skill_count;
    g_pet.skill_active = s_skill_context_active;
    g_pet.current_action = s_skill_action_state;
}

/*
 * Get pet skill data by index
 */
PetSkillDataEntry* pet_get_skill_data(int index) {
    if (index < 0 || index >= s_pet_skill_count) {
        return NULL;
    }
    return &s_pet_skill_data[index];
}

/*
 * Get pet skill count
 */
int pet_get_skill_count(void) {
    return s_pet_skill_count;
}

/*
 * Clear pet skill data
 */
void pet_clear_skill_data(void) {
    memset(s_pet_skill_data, 0, sizeof(s_pet_skill_data));
    s_pet_skill_count = 0;
    s_skill_context_active = 0;
    s_skill_action_state = 0;
}
