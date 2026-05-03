/*
 * Stone Age Client - Protocol Handler Header
 * Reverse engineered from sa_9061.exe (FUN_0045ffb0)
 *
 * Main protocol dispatcher handling binary packet types 0x42-0x58
 */

#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include "types.h"
#include "character.h"

/* ========================================
 * Constants
 * ======================================== */

#define MAX_PLAYER_SKILLS       26    /* From loop limit 0xEB / 9 fields */
#define MAX_PETS_PER_CHAR       7     /* From case 0x57 loop counter */
#define MAX_PET_NAME_57         25    /* 0x19 from case 0x57 */
#define MAX_PET_MEMO_57         73    /* 0x49 from case 0x57 */
#define MAX_BATTLE_UNITS        20    /* From case 0x4E array size */
#define MAX_UNIT_NAME           17    /* 0x11 from case 0x4E */

/* ========================================
 * Data Structures
 * ======================================== */

/*
 * Skill list entry - 0x60 (96) bytes per entry
 * From FUN_0045ffb0 case 0x53 analysis
 * Memory: DAT_04624042, stride 0x60
 */
#pragma pack(push, 1)
typedef struct {
    u16 skill_id;           /* Offset 0x00: Field 1 - Skill database ID */
    u16 level;              /* Offset 0x02: Field 2 - Current skill level */
    u16 type;               /* Offset 0x04: Field 3 - Skill type */
    u16 target_type;        /* Offset 0x06: Field 4 - Target type */
    u32 field_5;            /* Offset 0x08: Field 5 - Additional data */
    u32 field_6;            /* Offset 0x0C: Field 6 - Additional data */
    u32 field_7;            /* Offset 0x10: Field 7 - Additional data */
    u8  padding1[12];       /* Offset 0x14: Padding */
    char name[17];          /* Offset 0x20: Field 8 - Name (max 0x11) */
    u8  padding2[24];       /* Offset 0x31: Padding */
    char description[85];   /* Offset 0x49: Field 9 - Description (max 0x55) */
} SkillListEntry;
#pragma pack(pop)

/*
 * Pet data entry from case 0x57 - 0x6A (106) bytes
 * From FUN_0045ffb0 case 0x57 analysis
 * Memory: DAT_0462b008 + char_index * 0x2e6
 */
#pragma pack(push, 1)
typedef struct {
    u16 field_1;            /* Offset -6: Field 1 (u16) */
    u16 field_2;            /* Offset -4: Field 2 (u16) */
    u16 field_3;            /* Offset -2: Field 3 (u16) */
    u8  active;             /* Offset 0: Active flag (1 = has pet) */
    u8  padding1;           /* Offset 1: Padding */
    char name[MAX_PET_NAME_57];    /* Offset 2: Pet name (0x19 = 25 bytes) */
    char memo[MAX_PET_MEMO_57];    /* Offset 0x1B: Memo (0x49 = 73 bytes) */
    u8  padding2[2];        /* Pad to 0x6A bytes */
} PetDataEntry57;
#pragma pack(pop)

/*
 * Battle unit from case 0x4E - DAT_046308f0 region
 * Stride 0x30 (48) bytes per unit
 */
#pragma pack(push, 1)
typedef struct {
    u8  active;             /* Offset 0x00: Active flag */
    u8  padding1[3];        /* Padding */
    u32 entity_id;          /* Offset 0x04: Entity ID (DAT_046308f4) */
    u16 hp;                 /* Offset 0x08: Current HP (DAT_046308f8) */
    u16 max_hp;             /* Offset 0x0A: Max HP (DAT_046308fc) */
    u16 mp;                 /* Offset 0x0C: Current MP (DAT_04630900) */
    u16 max_mp;             /* Offset 0x0E: Max MP (DAT_04630904) */
    char name[MAX_UNIT_NAME]; /* Offset 0x10: Unit name (DAT_04630908) */
    u8  padding2[7];        /* Pad to 0x30 bytes */
    void* linked_entity;    /* Offset 0x28: Linked entity pointer (DAT_0463091c) */
} BattleUnit;
#pragma pack(pop)

/*
 * Game state sync from case 0x44
 * DAT_005ab738 region
 */
#pragma pack(push, 1)
typedef struct {
    u32 last_sync_time;     /* DAT_005ab750: Server timestamp */
    u32 last_sync_tick;     /* DAT_005ab744: Local tick time */
    int  sync_param;        /* DAT_005ab74c: Sync parameter */
    u32 padding[4];         /* Additional state data */
} GameStateSync;
#pragma pack(pop)

/* ========================================
 * Global Data
 * ======================================== */

/* Detailed character stats from case 0x50 */
extern DetailedCharacterStats g_detailed_stats;

/* Skill list from case 0x53 */
extern SkillListEntry g_skill_list[MAX_PLAYER_SKILLS];
extern int g_skill_list_count;

/* Pet data from case 0x57 */
extern PetDataEntry57 g_pet_data_57[MAX_PETS_PER_CHAR];
extern int g_pet_data_57_count;

/* Battle units from case 0x4E */
extern BattleUnit g_battle_units[MAX_BATTLE_UNITS];
extern int g_battle_unit_count;

/* Game state sync from case 0x44 */
extern GameStateSync g_game_state;

/* ========================================
 * Protocol Case Handlers
 * ======================================== */

/*
 * Case 0x42 - Inventory/Item data handler
 * Format: 0x42<char_index>|fields...
 */
void protocol_handle_case_42(const char* packet_data);

/*
 * Case 0x43 - Map enter handler
 * Format: 0x43|map_id|x|y|...
 */
void protocol_handle_case_43(const char* packet_data);

/*
 * Case 0x44 - Time sync handler
 * Format: 0x44|timestamp|param
 */
void protocol_handle_case_44(const char* packet_data);

/*
 * Case 0x45 - Player ID setup handler
 * Format: 0x45|player_id|field2
 */
void protocol_handle_case_45(const char* packet_data);

/*
 * Case 0x46 - Title/Guild data handler
 * Format: 0x46|title|fields...
 */
void protocol_handle_case_46(const char* packet_data);

/*
 * Case 0x49 - Character list handler
 * Format: 0x49|fields... (14 fields per character)
 */
void protocol_handle_case_49(const char* packet_data);

/*
 * Case 0x4A - Pet status handler
 * Format: 0x4A<char_index>|active|hp|max_hp|level|name|memo
 */
void protocol_handle_case_4a(const char* packet_data);

/*
 * Case 0x4B - Pet detailed stats handler (bitmask)
 * Format: 0x4B<index>|bitmask|fields...
 */
void protocol_handle_case_4b(const char* packet_data);

/*
 * Case 0x4D - HP/MP update handler
 * Format: 0x4D|hp|mp|stamina
 */
void protocol_handle_case_4d(const char* packet_data);

/*
 * Case 0x4E - Battle pet status handler (bitmask)
 * Format: 0x4E<index>|bitmask|fields...
 */
void protocol_handle_case_4e(const char* packet_data);

/*
 * Case 0x50 - Detailed character stats handler
 * Format: 0x50|bitmask|fields...
 * Uses bitmask to determine which fields are present
 */
void protocol_handle_case_50(const char* packet_data);

/*
 * Case 0x53 - Skill list handler
 * Format: 0x53|skill_id|level|type|target|...|name|description|
 * 9 fields per skill, max ~26 skills
 */
void protocol_handle_case_53(const char* packet_data);

/*
 * Case 0x57 - Pet data handler
 * Format: 0x57<char_index><byte>|field1|field2|field3|name|memo|
 * 5 fields per pet, 7 pets per character
 */
void protocol_handle_case_57(const char* packet_data);

/*
 * Case 0x58 - Field update handler
 * Format: 0x58|field1|field2
 */
void protocol_handle_case_58(const char* packet_data);

/* ========================================
 * Main Dispatcher
 * ======================================== */

/*
 * Main protocol dispatcher - FUN_0045ffb0
 * Dispatches based on first byte of packet
 */
void protocol_handler_dispatch(const char* packet_data);

/* ========================================
 * Protocol Case Constants
 * ======================================== */

#define PROTO_CASE_INVENTORY       0x42
#define PROTO_CASE_MAP_ENTER       0x43
#define PROTO_CASE_TIME_SYNC       0x44
#define PROTO_CASE_PLAYER_ID       0x45
#define PROTO_CASE_TITLE_GUILD     0x46
#define PROTO_CASE_CHAR_LIST       0x49
#define PROTO_CASE_PET_STATUS      0x4A
#define PROTO_CASE_PET_DETAILED    0x4B
#define PROTO_CASE_HPMP_UPDATE     0x4D
#define PROTO_CASE_BATTLE_PET      0x4E
#define PROTO_CASE_DETAILED_STATS  0x50
#define PROTO_CASE_SKILL_LIST      0x53
#define PROTO_CASE_PET_DATA        0x57
#define PROTO_CASE_FIELD_UPDATE    0x58

/* ========================================
 * Helper Functions for Case Handlers
 * ======================================== */

/* Time sync helper - FUN_00477ca0 */
void game_time_sync(u32 timestamp);

/* Game state helpers - FUN_0041e6d0, FUN_0041e760 */
void game_state_update(GameStateSync* state);
u32 game_state_calculate(GameStateSync* state);

/* Timer update - FUN_00412710 */
void timer_update(u32 value, int param);

/* Battle transition - FUN_0047a3f0 */
void battle_transition_cleanup(void);

/* Field state functions - FUN_00477b90, FUN_004403e0 */
void field_clear_state(void);
void field_init_state(void);

/* Character position setup - FUN_00440dd0, FUN_00440530 */
void character_setup_map_position(u32 map_id, u32 y, u32 direction);
void character_set_map_position(u32 map_id, u32 x, u32 y);

/* UI update */
void ui_update(void);

#endif /* PROTOCOL_HANDLER_H */
