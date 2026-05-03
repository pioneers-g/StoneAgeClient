/*
 * Stone Age Client - Unit Tests for Battle Unit Status
 * Test file: test_battle_unit.c
 *
 * Tests case 0x4e handler from FUN_0045ffb0 (binary packet dispatcher)
 * Tests battle unit creation, update, and removal
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/battle.h"

/* External functions from battle_protocol.c */
extern void battle_unit_status_update(int slot_index, u32 bitmask, const char* data);
extern void battle_unit_remove(int slot_index);
extern int battle_find_unit_slot(u32 char_id);
extern void battle_slots_init(void);
extern void battle_set_player_char_id(u32 char_id);

/* External battle context */
extern BattleContext g_battle;

/* ========================================
 * Test Cases for Battle Unit Status
 * ======================================== */

/*
 * Test 1: Initialize battle slots
 */
static void test_battle_slots_init(void) {
    TEST_BEGIN("Battle slots init");

    battle_slots_init();

    /* Find non-existent char should return -1 */
    int slot = battle_find_unit_slot(12345);
    TEST_ASSERT(slot == -1, "Should not find char in empty slots");

    TEST_END();
}

/*
 * Test 2: Create battle unit with all fields
 */
static void test_battle_unit_create(void) {
    TEST_BEGIN("Battle unit create");

    battle_slots_init();

    /* Create unit with bitmask = 1 (all fields) */
    /* Format: bitmask|char_id|field_08|level|hp|status|name */
    const char* data = "1|1000|5000|50|100|0|TestUnit";
    battle_unit_status_update(0, 1, data);

    /* Verify unit was created */
    int slot = battle_find_unit_slot(1000);
    TEST_ASSERT(slot == 0, "Should find unit at slot 0");

    TEST_END();
}

/*
 * Test 3: Update specific fields with bitmask
 */
static void test_battle_unit_update_fields(void) {
    TEST_BEGIN("Battle unit update fields");

    battle_slots_init();

    /* Create unit first */
    const char* create_data = "1|2000|5000|30|200|0|TestUnit2";
    battle_unit_status_update(1, 1, create_data);

    /* Update just HP (bitmask bit 4 = 0x10) */
    const char* update_data = "16|150";  /* Just field 5 (HP) */
    battle_unit_status_update(1, 0x10, update_data);

    /* Verify unit still exists */
    int slot = battle_find_unit_slot(2000);
    TEST_ASSERT(slot == 1, "Unit should still be at slot 1");

    TEST_END();
}

/*
 * Test 4: Remove battle unit
 */
static void test_battle_unit_remove(void) {
    TEST_BEGIN("Battle unit remove");

    battle_slots_init();

    /* Create unit */
    const char* data = "1|3000|5000|25|50|0|RemoveMe";
    battle_unit_status_update(2, 1, data);

    /* Verify created */
    int slot = battle_find_unit_slot(3000);
    TEST_ASSERT(slot == 2, "Unit should be at slot 2");

    /* Remove with bitmask = 0 */
    battle_unit_status_update(2, 0, "");

    /* Verify removed */
    slot = battle_find_unit_slot(3000);
    TEST_ASSERT(slot == -1, "Unit should be removed");

    TEST_END();
}

/*
 * Test 5: Multiple units
 */
static void test_battle_multiple_units(void) {
    TEST_BEGIN("Battle multiple units");

    battle_slots_init();

    /* Create multiple units */
    battle_unit_status_update(0, 1, "1|100|1000|10|100|0|Unit1");
    battle_unit_status_update(1, 1, "1|101|1000|10|100|0|Unit2");
    battle_unit_status_update(2, 1, "1|102|1000|10|100|0|Unit3");

    /* Verify all created */
    TEST_ASSERT(battle_find_unit_slot(100) == 0, "Unit 100 at slot 0");
    TEST_ASSERT(battle_find_unit_slot(101) == 1, "Unit 101 at slot 1");
    TEST_ASSERT(battle_find_unit_slot(102) == 2, "Unit 102 at slot 2");

    TEST_END();
}

/*
 * Test 6: Player character reference
 */
static void test_battle_player_char(void) {
    TEST_BEGIN("Battle player char");

    battle_slots_init();
    battle_set_player_char_id(9999);

    /* Create player unit */
    battle_unit_status_update(0, 1, "1|9999|1000|50|500|0|Player");

    /* Verify player unit */
    int slot = battle_find_unit_slot(9999);
    TEST_ASSERT(slot == 0, "Player should be at slot 0");

    TEST_END();
}

/*
 * Test 7: Battle context update
 */
static void test_battle_context_update(void) {
    TEST_BEGIN("Battle context update");

    battle_slots_init();

    /* Create unit */
    battle_unit_status_update(0, 1, "1|5000|2000|40|300|0|ContextTest");

    /* Check g_battle was updated */
    TEST_ASSERT(g_battle.units[0].char_id == 5000, "g_battle unit char_id");
    TEST_ASSERT(g_battle.units[0].level == 40, "g_battle unit level");
    TEST_ASSERT(g_battle.units[0].hp == 300, "g_battle unit hp");

    TEST_END();
}

/*
 * Test 8: Invalid slot index
 */
static void test_battle_invalid_slot(void) {
    TEST_BEGIN("Battle invalid slot");

    battle_slots_init();

    /* Try invalid slot index */
    battle_unit_status_update(-1, 1, "1|100|1000|10|100|0|Invalid");
    battle_unit_status_update(100, 1, "1|100|1000|10|100|0|Invalid");

    /* Should not crash */
    TEST_ASSERT(1, "Should handle invalid slots gracefully");

    TEST_END();
}

/*
 * Test 9: Empty name handling
 */
static void test_battle_empty_name(void) {
    TEST_BEGIN("Battle empty name");

    battle_slots_init();

    /* Create unit with empty name field */
    battle_unit_status_update(0, 1, "1|6000|3000|35|150|0|");

    /* Should still create unit */
    int slot = battle_find_unit_slot(6000);
    TEST_ASSERT(slot == 0, "Unit should be created with empty name");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(battle_unit) {
    test_battle_slots_init();
    test_battle_unit_create();
    test_battle_unit_update_fields();
    test_battle_unit_remove();
    test_battle_multiple_units();
    test_battle_player_char();
    test_battle_context_update();
    test_battle_invalid_slot();
    test_battle_empty_name();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Battle Unit Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(battle_unit);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
