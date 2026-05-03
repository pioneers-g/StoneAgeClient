/*
 * Stone Age Client - Pet System Header
 * Reverse engineered from sa_9061.exe (FUN_00463c20, FUN_00463d20, FUN_0045fb80, FUN_0045fdc0,
 * FUN_00425dc0 - capture handler, FUN_0042af40 - escape handler)
 *
 * Main pet header - includes sub-modules:
 * - pet_protocol.h: Protocol parsing functions
 * - pet_battle.h: Battle handlers
 */

#ifndef PET_H
#define PET_H

#include "types.h"

/* Forward declarations for protocol structures (defined in pet_protocol.h) */
typedef struct PetInfo PetInfo;
typedef struct EscapePetInfo EscapePetInfo;

/* Include sub-module headers after forward declarations */
#include "pet_protocol.h"
#include "pet_battle.h"

/* Constants */
#define MAX_PET_SLOTS           5
#define MAX_PET_SKILLS_PER_PET  7
#define MAX_PET_LEVEL           100
#define MAX_SKILL_LEVEL         10
#define MAX_PET_NAME_LEN        29    /* 0x1d from FUN_00425dc0 */
#define MAX_ENEMY_PET_SLOTS     104   /* 0x68 from DAT_0455b370 */
#define MAX_ESCAPE_PET_SLOTS    80    /* 0x50 from DAT_0455b370 in FUN_0042af40 */

/* Pet entry from text protocol - FUN_0045ffb0 case 0x57 */
#define MAX_PETS_PER_CHAR       7     /* From case 0x57 loop counter */
#define PET_ENTRY_SIZE_57       0x6a  /* 106 bytes per pet entry */
#define MAX_PET_NAME_57         25    /* 0x19 from case 0x57 */
#define MAX_PET_MEMO_57         73    /* 0x49 from case 0x57 */

/* Pet state */
typedef enum {
    PET_STATE_IDLE = 0,
    PET_STATE_BATTLE = 1,
    PET_STATE_DEAD = 2,
    PET_STATE_RESTING = 3
} PetState;

/* Pet AI mode */
typedef enum {
    PET_AI_MANUAL = 0,
    PET_AI_AUTO = 1,
    PET_AI_DEFENSIVE = 2,
    PET_AI_AGGRESSIVE = 3
} PetAIMode;

/* Pet skill structure */
typedef struct {
    u16 skill_id;
    u16 id;               /* Skill ID (alias for skill_id) */
    u8  level;
    u8  mp_cost;
    u16 power;            /* Skill power */
    u16 type;             /* Skill type */
    u16 element;          /* Skill element */
    u16 cooldown;         /* Current cooldown */
    u32 current_exp;
    u32 max_exp;
    u32 use_count;
} PetSkill;

/*
 * Pet info structure - 0x88 (136) bytes per entry
 * From FUN_00425dc0 analysis (capture handler)
 * Array at DAT_0455b5e0
 */
#pragma pack(push, 1)
typedef struct PetInfo {
    char name[MAX_PET_NAME_LEN];      /* 0x00: Pet name (0x1d = 29 bytes) */
    u8  padding1[3];                  /* 0x1d: Padding to align */

    /* Stats from delimited parsing - FUN_00425230 */
    u16 field_0x20[45];               /* 0x20: 3 fields x 15 entries each (0x2d/0x27/0x27 sizes) */

    u32 data_0x10;                    /* From offset -0x10 in loop */
    u32 data_0x0c;                    /* From offset -0x0c in loop */
    u32 data_0x08;                    /* From offset -0x08 in loop */
    u16 data_0x04;                    /* From offset -0x04 in loop */
    u16 data_0x02;                    /* From offset -0x02 in loop */
    u32 data_0x74;                    /* From offset 0x74 in loop */
    u8  more_data[22];                /* Remaining data to reach 0x88 bytes */
} PetInfo;
#pragma pack(pop)

/*
 * Escape pet structure - 0x70 (112) bytes per entry
 * From FUN_0042af40 analysis (escape handler)
 * Array at DAT_0454bba4
 */
#pragma pack(push, 1)
typedef struct EscapePetInfo {
    char name[25];                    /* 0x00: Pet name (0x19 = 25 bytes) */
    u8  padding1[3];                  /* 0x19: Padding */

    u16 field_0x1c[36];               /* 0x1c: 3 fields x 12 entries each (0x2d/0x27/0x27 sizes) */

    u32 data_0x0c;                    /* From offset -0x0c in loop */
    u32 data_0x08;                    /* From offset -0x08 in loop */
    u8  more_data[16];                /* Remaining data to reach 0x70 bytes */
} EscapePetInfo;
#pragma pack(pop)

/*
 * Pet data structure - consolidated from Pet and PetData definitions
 * Matches binary protocol expectations from FUN_00425dc0
 */
typedef struct {
    u32 id;
    u32 type_id;
    char name[24];

    /* Level and experience */
    u16 level;
    u16 padding1;
    u32 experience;         /* Current experience points */

    /* HP/MP - current and max values */
    u16 hp;
    u16 max_hp;
    u16 mp;
    u16 max_mp;

    /* Stats */
    u16 attack;
    u16 defense;
    u16 agility;
    u16 vitality;
    u16 speed;
    u16 magic;

    /* Base stats */
    u16 base_hp;
    u16 base_mp;
    u16 base_attack;
    u16 base_defense;
    u16 base_speed;
    u16 base_magic;

    /* Growth rates */
    u16 growth_rate_hp;
    u16 growth_rate_mp;
    u16 growth_rate_atk;
    u16 growth_rate_def;
    u16 growth_rate_spd;
    u16 growth_rate_mag;

    /* Display */
    u16 sprite_id;
    u16 portrait_id;

    /* Skills */
    PetSkill skills[MAX_PET_SKILLS_PER_PET];
    int skill_count;                    /* Number of learned skills */

    /* State */
    PetState state;
    PetAIMode ai_mode;
    u8  loyalty;
    u8  hunger;
    u8  can_mount;
    u8  flags;
    u8  slot;
    u8  element;
    u8  growth_type;

    /* Evolution */
    u16 evolve_level;
    u16 evolve_type;
    u16 evolve_sprite;

    /* Battle */
    u16 battle_x;
    u16 battle_y;
    u8  battle_side;
    u8  battle_pos;

    /* Battle stats */
    u16 kills;
    u16 deaths;
    u32 total_damage;
    u32 total_healing;

} PetData;

/*
 * Pet entry from text protocol - FUN_0045ffb0 case 0x57
 * Entry size: 0x6A (106) bytes
 * Base address: DAT_0462b008 + char_index * 0x2e6
 * 5 fields per pet, 7 pets per character
 */
#pragma pack(push, 1)
typedef struct {
    u16 field_1;                    /* Offset -6: Field 1 (u16) */
    u16 field_2;                    /* Offset -4: Field 2 (u16) */
    u16 field_3;                    /* Offset -2: Field 3 (u16) */
    u8  active;                     /* Offset 0: Active flag */
    u8  padding1;                   /* Offset 1: Padding */
    char name[MAX_PET_NAME_57];     /* Offset 2: Pet name (0x19 = 25 bytes) */
    char memo[MAX_PET_MEMO_57];     /* Offset 0x1B: Memo (0x49 = 73 bytes) */
    u8  padding2[2];                /* Pad to 0x6A bytes */
} PetDataEntry57;
#pragma pack(pop)

/* Global pet data from server - case 0x57 */
extern PetDataEntry57 g_pet_data_57[MAX_PETS_PER_CHAR];
extern int g_pet_data_57_count;

/* Text protocol parser - FUN_0045ffb0 case 0x57 */
void pet_parse_data_57(const char* packet_data, int char_index);

/* Pet context */
typedef struct {
    PetData slots[MAX_PET_SLOTS];
    int count;

    /* Selection */
    int selected_index;
    int battle_pet_index;
    int mounted_slot;

    /* Packet results */
    int list_received;
    int status_received;
    int name_result;
    int skills_received;
    int release_result;
    int capture_result;

    /* Capture data */
    u16 capture_slot;
    u16 capture_level;
    u16 capture_hp;
    u16 capture_max_hp;

    /* Skill loading thread */
    HANDLE skill_thread;
    DWORD thread_id;
    int skills_loaded;

    /* Skill database */
    void* skills;
    void* type_data;

    /* Battle summon state - from FUN_004327b0 */
    int summon_pending;             /* DAT_04558d3c */
    int selected_skill;             /* DAT_0454f5b0 */
    int active_slot;                /* Active pet slot index */
    int skill_active;               /* DAT_0455efc8 */
    u32 current_action;             /* DAT_004b83ec */
    void* skill_context;            /* DAT_0455ef98 */

    /* Attack state - from FUN_0042e870 */
    u32 attack_target;              /* DAT_0454de9c */
    u32 attack_type;                /* DAT_004b848c */

    /* Escape state - from FUN_0042af40 */
    int escape_flag;                /* DAT_0455b0f0 */
    int escape_result;              /* DAT_0454f400 */
    char escape_pet_name[26];       /* DAT_0454b9ac */
    int escape_pet_count;           /* From DAT_0455b370 */

    /* Battle skills - from FUN_0042f130 */
    struct {
        u16 id;
        u16 level;
        u16 mp_cost;
        u16 power;
        u16 element;
        u16 type;
        u16 target_type;
        char name[512];
        char description[512];
    } battle_skills[MAX_PET_SKILLS_PER_PET];
    int battle_skill_count;

    /* Capture state */
    u32 capture_flags;              /* Capture mode flags */
    u8  capture_active;             /* Is capture in progress */

} PetContext;

/* Global pet context */
extern PetContext g_pet;

/* Initialization */
int pet_init(void);
void pet_shutdown(void);

/* Pet access */
PetData* pet_get_slot(int slot);
PetData* pet_get_selected(void);
PetData* pet_get_battle_pet(void);
PetData* pet_get_by_id(u32 pet_id);
int pet_get_slot_index(u32 pet_id);
int pet_select(int slot);
int pet_set_battle(int slot);

/* Pet management */
int pet_add(PetData* pet);
int pet_remove(int slot);
int pet_rename(int slot, const char* name);
int pet_release(int slot);
int pet_feed(int slot, u32 item_id);

/* Skills */
int pet_set_skill(int slot, int skill_slot, u16 skill_id, u8 level);
int pet_use_skill(int slot, int skill_slot, u32 target_id);
void pet_skill_level_up(int slot, int skill_slot);

/* Battle */
void pet_battle_update(int slot);
int pet_take_damage(int slot, int damage);
void pet_heal(int slot, int hp, int mp);
int pet_capture(u32 target_id);
int pet_calculate_capture_rate(int enemy_hp_percent, int enemy_level, int capture_item_id);
void pet_handle_capture_result(u32 flags, u16 result);

/* Experience and leveling */
void pet_gain_exp(int slot, u32 exp);
void pet_level_up(int slot);

/* Stats */
void pet_calculate_stats(PetData* pet);

/* AI */
void pet_set_ai_mode(int slot, PetAIMode mode);

/* Items */
int pet_use_item(int slot, u32 item_id);

/* Mount */
int pet_toggle_mount(int slot);

/* Evolution and fusion */
int pet_evolve(int slot);
int pet_fuse(int slot1, int slot2);

/* Packet handlers */
void pet_handle_list_packet(void* data, u32 size);
void pet_handle_status_packet(void* data, u32 size);
void pet_handle_name_result(void* data, u32 size);
void pet_handle_skill_list(void* data, u32 size);
void pet_handle_release_result(void* data, u32 size);

/* Thread */
DWORD WINAPI pet_skill_load_thread(LPVOID param);

/* Utility */
int pet_get_count(void);
int pet_slot_empty(int slot);

#endif /* PET_H */
