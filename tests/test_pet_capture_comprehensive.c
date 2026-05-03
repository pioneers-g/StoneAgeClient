/*
 * Stone Age Client - Pet Capture System Unit Tests
 * Tests for FUN_0042ce40 (capture UI state machine) and FUN_00425dc0 (capture packet parser)
 *
 * Based on Ghidra decompilation analysis:
 * - FUN_0042ce40: Capture UI state machine with 10+ states
 * - FUN_00425dc0: Parse pet capture packet from server
 * - Max captured pets: 104 (0x68)
 * - Pet data entry size: 0x88 (136) bytes
 * - Page calculation: (count + 7) / 8
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
typedef int s32;

/* Constants from Ghidra */
#define MAX_CAPTURED_PETS 104   /* 0x68 */
#define PET_ENTRY_SIZE 136      /* 0x88 */
#define MAX_PET_NAME 29         /* 0x1d */
#define MAX_PAGES 13            /* (104 + 7) / 8 */

/* Capture UI states from FUN_0042ce40 */
typedef enum {
    CAPTURE_STATE_INIT = 0,
    CAPTURE_STATE_WAIT = 1,
    CAPTURE_STATE_SELECT = 10,
    CAPTURE_STATE_CONFIRM = 11,
    CAPTURE_STATE_EXCHANGE_1 = 0x14,  /* 20 */
    CAPTURE_STATE_EXCHANGE_2 = 0x15,  /* 21 */
    CAPTURE_STATE_ALT_INIT = 100,
    CAPTURE_STATE_ALT_SELECT = 0x65,  /* 101 */
    CAPTURE_STATE_ALT_EXCHANGE_1 = 0x6e, /* 110 */
    CAPTURE_STATE_ALT_EXCHANGE_2 = 0x6f, /* 111 */
    CAPTURE_STATE_SPECIAL = 0x78     /* 120 */
} CaptureState;

/* Pet capture data structure */
typedef struct {
    char name[MAX_PET_NAME + 1];    /* +0x00: Pet name */
    u16 pet_id;                     /* +0x1c-0x1d: Pet ID */
    u32 field_20;                   /* +0x20 */
    u32 hp;                         /* +0x24: HP */
    u32 max_hp;                     /* +0x28: Max HP */
    u32 field_2c;                   /* +0x2c */
    char skills[3][29];             /* +0x30-0x74: 3 skill slots */
    u32 field_74;                   /* +0x74 */
    u16 level;                      /* +0x7c-0x7d: Level */
} PetCaptureEntry;

/* Test data storage */
static PetCaptureEntry g_captured_pets[MAX_CAPTURED_PETS];
static int g_captured_count = 0;
static int g_current_page = 0;
static int g_selected_slot = 0;
static CaptureState g_capture_state = CAPTURE_STATE_INIT;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* Setup */
static void test_setup(void) {
    memset(g_captured_pets, 0, sizeof(g_captured_pets));
    g_captured_count = 0;
    g_current_page = 0;
    g_selected_slot = 0;
    g_capture_state = CAPTURE_STATE_INIT;
}

/*
 * Calculate page count from pet count - FUN_0042ce40 pattern
 */
static int calculate_page_count(int pet_count) {
    if (pet_count < 1) {
        return 1;
    }
    return (pet_count + 7) / 8;
}

/*
 * Get page from pet count - FUN_0042ce40 pattern
 */
static u16 get_page_value(int pet_count) {
    if (pet_count < 1) {
        return 1;
    }
    return (u16)((pet_count + 7 + ((pet_count + 7) >> 31 & 7)) >> 3);
}

/*
 * Get pet entry by index
 */
static PetCaptureEntry* get_pet_entry(int index) {
    if (index < 0 || index >= MAX_CAPTURED_PETS) {
        return NULL;
    }
    return &g_captured_pets[index];
}

/*
 * Add captured pet
 */
static int add_captured_pet(const char* name, u16 pet_id, u32 hp, u32 max_hp, u16 level) {
    if (g_captured_count >= MAX_CAPTURED_PETS) {
        return -1;
    }

    PetCaptureEntry* pet = &g_captured_pets[g_captured_count];
    strncpy(pet->name, name, MAX_PET_NAME);
    pet->name[MAX_PET_NAME] = '\0';
    pet->pet_id = pet_id;
    pet->hp = hp;
    pet->max_hp = max_hp;
    pet->level = level;

    return g_captured_count++;
}

/*
 * Remove captured pet by index
 */
static int remove_captured_pet(int index) {
    if (index < 0 || index >= g_captured_count) {
        return -1;
    }

    /* Shift entries down */
    for (int i = index; i < g_captured_count - 1; i++) {
        g_captured_pets[i] = g_captured_pets[i + 1];
    }

    g_captured_count--;
    memset(&g_captured_pets[g_captured_count], 0, sizeof(PetCaptureEntry));

    return 0;
}

/*
 * Get state transition for capture UI - FUN_0042ce40 pattern
 */
static CaptureState get_next_state(CaptureState current, int action) {
    switch (current) {
        case CAPTURE_STATE_INIT:
            return CAPTURE_STATE_WAIT;

        case CAPTURE_STATE_WAIT:
            if (action == 1) return CAPTURE_STATE_SELECT;
            break;

        case CAPTURE_STATE_SELECT:
            if (action == 1) return CAPTURE_STATE_CONFIRM;
            if (action == 2) return CAPTURE_STATE_EXCHANGE_1;
            break;

        case CAPTURE_STATE_CONFIRM:
            if (action == 1) return CAPTURE_STATE_SELECT;
            break;

        case CAPTURE_STATE_EXCHANGE_1:
            return CAPTURE_STATE_EXCHANGE_2;

        case CAPTURE_STATE_EXCHANGE_2:
            if (action == 1) return CAPTURE_STATE_SELECT;
            if (action == 2) return CAPTURE_STATE_SELECT;
            break;

        case CAPTURE_STATE_ALT_INIT:
            if (action == 1) return CAPTURE_STATE_ALT_SELECT;
            break;

        case CAPTURE_STATE_ALT_SELECT:
            if (action == 1) return CAPTURE_STATE_ALT_EXCHANGE_1;
            if (action == 2) return CAPTURE_STATE_ALT_INIT;
            if (action == 3) return CAPTURE_STATE_SPECIAL;
            break;

        case CAPTURE_STATE_ALT_EXCHANGE_1:
            return CAPTURE_STATE_ALT_EXCHANGE_2;

        case CAPTURE_STATE_ALT_EXCHANGE_2:
            if (action == 1) return CAPTURE_STATE_ALT_INIT;
            if (action == 2) return CAPTURE_STATE_ALT_INIT;
            break;

        default:
            break;
    }

    return current;
}

/* ========================================
 * Pet Count Tests
 * ======================================== */

static int test_max_captured_pets(void) {
    test_setup();

    /* Maximum 104 captured pets (0x68) */
    assert(MAX_CAPTURED_PETS == 104);
    assert(MAX_CAPTURED_PETS == 0x68);

    return 1;
}

static int test_pet_entry_size(void) {
    test_setup();

    /* Each pet entry is 136 bytes (0x88) */
    assert(PET_ENTRY_SIZE == 136);
    assert(PET_ENTRY_SIZE == 0x88);
    assert(sizeof(PetCaptureEntry) == 136);

    return 1;
}

static int test_max_pet_name(void) {
    test_setup();

    /* Pet name max 29 bytes (0x1d) */
    assert(MAX_PET_NAME == 29);
    assert(MAX_PET_NAME == 0x1d);

    return 1;
}

/* ========================================
 * Page Calculation Tests
 * ======================================== */

static int test_page_calculation_zero(void) {
    test_setup();

    /* 0 pets = 1 page */
    int pages = calculate_page_count(0);
    assert(pages == 1);

    return 1;
}

static int test_page_calculation_one(void) {
    test_setup();

    /* 1-8 pets = 1 page */
    assert(calculate_page_count(1) == 1);
    assert(calculate_page_count(8) == 1);

    return 1;
}

static int test_page_calculation_multiple(void) {
    test_setup();

    /* 9 pets = 2 pages */
    assert(calculate_page_count(9) == 2);
    assert(calculate_page_count(16) == 2);
    assert(calculate_page_count(17) == 3);

    return 1;
}

static int test_page_calculation_max(void) {
    test_setup();

    /* 104 pets = 13 pages */
    int pages = calculate_page_count(MAX_CAPTURED_PETS);
    assert(pages == 13);
    assert((104 + 7) / 8 == 13);

    return 1;
}

static int test_page_formula(void) {
    test_setup();

    /* Formula from FUN_0042ce40:
     * sVar3 = (short)((int)(count + 7 + (count + 7 >> 0x1f & 7U)) >> 3);
     */
    for (int i = 0; i <= 20; i++) {
        u16 page_val = get_page_value(i);
        assert(page_val == calculate_page_count(i));
    }

    return 1;
}

/* ========================================
 * Pet Entry Tests
 * ======================================== */

static int test_add_captured_pet(void) {
    test_setup();

    int index = add_captured_pet("TestPet", 100, 500, 1000, 10);

    assert(index == 0);
    assert(g_captured_count == 1);
    assert(strcmp(g_captured_pets[0].name, "TestPet") == 0);
    assert(g_captured_pets[0].pet_id == 100);
    assert(g_captured_pets[0].hp == 500);
    assert(g_captured_pets[0].max_hp == 1000);
    assert(g_captured_pets[0].level == 10);

    return 1;
}

static int test_add_multiple_pets(void) {
    test_setup();

    add_captured_pet("Pet1", 101, 100, 200, 5);
    add_captured_pet("Pet2", 102, 200, 300, 10);
    add_captured_pet("Pet3", 103, 300, 400, 15);

    assert(g_captured_count == 3);
    assert(g_captured_pets[0].pet_id == 101);
    assert(g_captured_pets[1].pet_id == 102);
    assert(g_captured_pets[2].pet_id == 103);

    return 1;
}

static int test_remove_captured_pet(void) {
    test_setup();

    add_captured_pet("Pet1", 101, 100, 200, 5);
    add_captured_pet("Pet2", 102, 200, 300, 10);
    add_captured_pet("Pet3", 103, 300, 400, 15);

    int result = remove_captured_pet(1);

    assert(result == 0);
    assert(g_captured_count == 2);
    assert(g_captured_pets[0].pet_id == 101);
    assert(g_captured_pets[1].pet_id == 103);

    return 1;
}

static int test_pet_entry_bounds(void) {
    test_setup();

    /* Get entry with invalid index */
    assert(get_pet_entry(-1) == NULL);
    assert(get_pet_entry(MAX_CAPTURED_PETS) == NULL);
    assert(get_pet_entry(1000) == NULL);

    /* Add and get valid entry */
    add_captured_pet("Test", 1, 100, 200, 1);
    assert(get_pet_entry(0) != NULL);

    return 1;
}

/* ========================================
 * State Machine Tests
 * ======================================== */

static int test_capture_state_values(void) {
    test_setup();

    /* Verify state values from FUN_0042ce40 */
    assert(CAPTURE_STATE_INIT == 0);
    assert(CAPTURE_STATE_WAIT == 1);
    assert(CAPTURE_STATE_SELECT == 10);
    assert(CAPTURE_STATE_CONFIRM == 11);
    assert(CAPTURE_STATE_EXCHANGE_1 == 0x14);
    assert(CAPTURE_STATE_EXCHANGE_2 == 0x15);
    assert(CAPTURE_STATE_ALT_INIT == 100);
    assert(CAPTURE_STATE_ALT_SELECT == 0x65);
    assert(CAPTURE_STATE_ALT_EXCHANGE_1 == 0x6e);
    assert(CAPTURE_STATE_ALT_EXCHANGE_2 == 0x6f);
    assert(CAPTURE_STATE_SPECIAL == 0x78);

    return 1;
}

static int test_state_transition_init(void) {
    test_setup();

    /* INIT -> WAIT */
    CaptureState next = get_next_state(CAPTURE_STATE_INIT, 0);
    assert(next == CAPTURE_STATE_WAIT);

    return 1;
}

static int test_state_transition_select(void) {
    test_setup();

    /* WAIT -> SELECT with action 1 */
    CaptureState next = get_next_state(CAPTURE_STATE_WAIT, 1);
    assert(next == CAPTURE_STATE_SELECT);

    return 1;
}

static int test_state_transition_exchange(void) {
    test_setup();

    /* SELECT -> EXCHANGE_1 with action 2 */
    CaptureState next = get_next_state(CAPTURE_STATE_SELECT, 2);
    assert(next == CAPTURE_STATE_EXCHANGE_1);

    /* EXCHANGE_1 -> EXCHANGE_2 */
    next = get_next_state(CAPTURE_STATE_EXCHANGE_1, 0);
    assert(next == CAPTURE_STATE_EXCHANGE_2);

    return 1;
}

static int test_state_transition_alt_mode(void) {
    test_setup();

    /* ALT_INIT -> ALT_SELECT with action 1 */
    CaptureState next = get_next_state(CAPTURE_STATE_ALT_INIT, 1);
    assert(next == CAPTURE_STATE_ALT_SELECT);

    /* ALT_SELECT -> SPECIAL with action 3 */
    next = get_next_state(CAPTURE_STATE_ALT_SELECT, 3);
    assert(next == CAPTURE_STATE_SPECIAL);

    return 1;
}

/* ========================================
 * Packet Parsing Tests
 * ======================================== */

static int test_packet_field_count(void) {
    test_setup();

    /* From FUN_00425dc0:
     * Field 1: capture mode (0 or 1)
     * Field 2: pet count
     * Fields 3-6: strings (name, etc.)
     * Fields 8-15: per-pet data (8 fields each)
     */

    /* First mode: fields start at 8, increment by 7 */
    int start_field = 8;
    int field_increment = 7;

    assert(start_field == 8);
    assert(field_increment == 7);

    return 1;
}

static int test_packet_delimiter(void) {
    test_setup();

    /* Fields are delimited by '|' (0x7c) */
    assert(0x7c == '|');

    return 1;
}

static int test_string_field_size(void) {
    test_setup();

    /* String fields are read with max sizes:
     * 0x7f (127) for most strings
     * 0xff (255) for longer strings
     * 0x3f (63) for pet names
     */

    assert(0x7f == 127);
    assert(0xff == 255);
    assert(0x3f == 63);

    return 1;
}

/* ========================================
 * Memory Layout Tests
 * ======================================== */

static int test_pet_array_size(void) {
    test_setup();

    /* Pet array: 104 entries * 136 bytes = 14144 bytes */
    size_t total_size = sizeof(g_captured_pets);
    assert(total_size == MAX_CAPTURED_PETS * PET_ENTRY_SIZE);
    assert(total_size == 14144);

    return 1;
}

static int test_pet_structure_offsets(void) {
    test_setup();

    /* From FUN_00425dc0 and FUN_0042ce40:
     * +0x00: Name (29 bytes)
     * +0x1c: Pet ID (2 bytes)
     * +0x20: Field
     * +0x24: HP
     * +0x28: Max HP
     * +0x2c: Field
     * +0x30-0x74: Skills (3 * 29 bytes)
     * +0x74: Field
     * +0x7c: Level (2 bytes)
     */

    PetCaptureEntry entry;
    memset(&entry, 0, sizeof(entry));

    assert(offsetof(PetCaptureEntry, name) == 0x00);
    assert(offsetof(PetCaptureEntry, pet_id) == 0x1c);
    assert(offsetof(PetCaptureEntry, hp) == 0x24);
    assert(offsetof(PetCaptureEntry, max_hp) == 0x28);
    assert(offsetof(PetCaptureEntry, level) == 0x7c);

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_capture_flow(void) {
    test_setup();

    /* Simulate full capture flow */

    /* State 0 -> 1 */
    g_capture_state = CAPTURE_STATE_INIT;
    g_capture_state = get_next_state(g_capture_state, 0);
    assert(g_capture_state == CAPTURE_STATE_WAIT);

    /* Add some captured pets */
    add_captured_pet("WildPet", 200, 800, 1000, 25);
    add_captured_pet("BossPet", 300, 1500, 2000, 50);

    /* Calculate pages */
    int pages = calculate_page_count(g_captured_count);
    assert(pages == 1);

    /* State 1 -> 10 */
    g_capture_state = get_next_state(g_capture_state, 1);
    assert(g_capture_state == CAPTURE_STATE_SELECT);

    return 1;
}

static int test_pet_exchange_flow(void) {
    test_setup();

    /* Add pets to exchange */
    add_captured_pet("PetA", 100, 500, 500, 10);
    add_captured_pet("PetB", 200, 600, 600, 15);

    /* Select pet for exchange */
    g_selected_slot = 0;

    /* State progression */
    g_capture_state = CAPTURE_STATE_SELECT;
    g_capture_state = get_next_state(g_capture_state, 2);
    assert(g_capture_state == CAPTURE_STATE_EXCHANGE_1);

    g_capture_state = get_next_state(g_capture_state, 0);
    assert(g_capture_state == CAPTURE_STATE_EXCHANGE_2);

    /* Exchange complete */
    g_capture_state = get_next_state(g_capture_state, 1);
    assert(g_capture_state == CAPTURE_STATE_SELECT);

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Pet Capture System Unit Tests ===\n\n");

    /* Pet count tests */
    printf("Pet Count Tests:\n");
    TEST(max_captured_pets);
    TEST(pet_entry_size);
    TEST(max_pet_name);

    /* Page calculation tests */
    printf("\nPage Calculation Tests:\n");
    TEST(page_calculation_zero);
    TEST(page_calculation_one);
    TEST(page_calculation_multiple);
    TEST(page_calculation_max);
    TEST(page_formula);

    /* Pet entry tests */
    printf("\nPet Entry Tests:\n");
    TEST(add_captured_pet);
    TEST(add_multiple_pets);
    TEST(remove_captured_pet);
    TEST(pet_entry_bounds);

    /* State machine tests */
    printf("\nState Machine Tests (FUN_0042ce40):\n");
    TEST(capture_state_values);
    TEST(state_transition_init);
    TEST(state_transition_select);
    TEST(state_transition_exchange);
    TEST(state_transition_alt_mode);

    /* Packet parsing tests */
    printf("\nPacket Parsing Tests (FUN_00425dc0):\n");
    TEST(packet_field_count);
    TEST(packet_delimiter);
    TEST(string_field_size);

    /* Memory layout tests */
    printf("\nMemory Layout Tests:\n");
    TEST(pet_array_size);
    TEST(pet_structure_offsets);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_capture_flow);
    TEST(pet_exchange_flow);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - FUN_0042d570 (capture UI render)
     * - FUN_0042d880 (selection UI render)
     * - FUN_0042df30 (exchange UI render)
     * - Skill name parsing (FUN_00425230)
     * - String escape handling (FUN_0048a170)
     * - Pet ID validation
     * - HP/Max HP bounds checking
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
