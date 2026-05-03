/*
 * Debug test for attack death detection
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;

#define MAX_BATTLE_UNITS     20
#define MAX_UNIT_NAME        24

typedef struct {
    u32 id;
    char name[MAX_UNIT_NAME];
    u16 hp;
    u16 max_hp;
    u16 attack;
    u16 defense;
    u8  is_alive;
    u8  is_player;
    u8  side;
    u8  level;
    s16 x, y;
    u32 sprite_id;
    u8  element;
    u8  reserved[7];
} BattleUnit;

typedef struct {
    u32 action_type;
    u32 param1;
    u32 param2;
    u32 param3;
    u32 param4;
    const char* data;
} ActionContext;

typedef struct {
    u8  death_flag;
    u8  is_combo;
    u8  action_complete;
    u8  reserved;
    u32 effect_id;
    u32 sound_id;
} BattleActionState;

static BattleUnit s_units[MAX_BATTLE_UNITS];
static int s_unit_count = 0;
static BattleActionState s_action_state = {0};

void battle_init(void) {
    memset(s_units, 0, sizeof(s_units));
    memset(&s_action_state, 0, sizeof(BattleActionState));
    s_unit_count = 0;
}

BattleUnit* battle_get_unit(u32 id) {
    int i;
    for (i = 0; i < MAX_BATTLE_UNITS; i++) {
        if (s_units[i].id == id && s_units[i].is_alive) {
            return &s_units[i];
        }
    }
    return NULL;
}

BattleUnit* battle_add_unit(u32 id, const char* name, u16 hp, u16 attack, u16 defense, u8 side) {
    int i;
    for (i = 0; i < MAX_BATTLE_UNITS; i++) {
        if (s_units[i].id == 0) {
            s_units[i].id = id;
            strncpy(s_units[i].name, name, MAX_UNIT_NAME - 1);
            s_units[i].hp = hp;
            s_units[i].max_hp = hp;
            s_units[i].attack = attack;
            s_units[i].defense = defense;
            s_units[i].is_alive = 1;
            s_units[i].side = side;
            s_units[i].level = 1;
            s_unit_count++;
            return &s_units[i];
        }
    }
    return NULL;
}

u16 battle_calc_damage(BattleUnit* attacker, BattleUnit* target, void* skill) {
    u16 base_damage;
    s16 damage;

    if (!attacker || !target) return 0;

    base_damage = attacker->attack;
    damage = base_damage - (target->defense / 2);

    if (damage < 1) damage = 1;

    return (u16)damage;
}

void action_show_effect(u32 effect_id, u32 x, u32 y) {
    s_action_state.effect_id = effect_id;
}

void action_play_sound(u32 sound_id) {
    s_action_state.sound_id = sound_id;
}

void battle_action_attack(const ActionContext* ctx) {
    u32 attacker_id = ctx->param1;
    u32 target_id = ctx->param2;
    u16 damage = 0;
    BattleUnit* attacker;
    BattleUnit* target;

    attacker = battle_get_unit(attacker_id);
    target = battle_get_unit(target_id);

    if (!attacker || !target) return;

    if (target->is_alive) {
        damage = battle_calc_damage(attacker, target, NULL);

        /* FIX: Check death BEFORE subtracting to avoid unsigned wrap-around */
        if (damage >= target->hp) {
            target->hp = 0;
            target->is_alive = 0;
            s_action_state.death_flag = 1;
        } else {
            target->hp -= damage;
        }

        action_show_effect(1, target->x, target->y);
        action_play_sound(1);
    }
}

int main(void) {
    BattleUnit* enemy;
    ActionContext ctx = {0, 1, 2, 0, 0, NULL};
    int result;

    printf("=== Debug Attack Death Test ===\n\n");

    /* Initialize */
    battle_init();

    /* Add units */
    battle_add_unit(1, "Hero", 100, 100, 0, 0);
    battle_add_unit(2, "Enemy", 50, 0, 0, 1);

    printf("Before attack:\n");
    printf("  Hero: id=%u, hp=%u, attack=%u, defense=%u, is_alive=%u\n",
           s_units[0].id, s_units[0].hp, s_units[0].attack, s_units[0].defense, s_units[0].is_alive);
    printf("  Enemy: id=%u, hp=%u, attack=%u, defense=%u, is_alive=%u\n",
           s_units[1].id, s_units[1].hp, s_units[1].attack, s_units[1].defense, s_units[1].is_alive);
    printf("  death_flag=%u\n\n", s_action_state.death_flag);

    /* Attack */
    battle_action_attack(&ctx);

    printf("After attack:\n");
    printf("  Enemy: hp=%u, is_alive=%u\n", s_units[1].hp, s_units[1].is_alive);
    printf("  death_flag=%u\n\n", s_action_state.death_flag);

    /* Check results */
    printf("Conditions:\n");
    printf("  enemy->hp == 0: %s (actual: %u)\n", s_units[1].hp == 0 ? "PASS" : "FAIL", s_units[1].hp);
    printf("  enemy->is_alive == 0: %s (actual: %u)\n", s_units[1].is_alive == 0 ? "PASS" : "FAIL", s_units[1].is_alive);
    printf("  death_flag == 1: %s (actual: %u)\n", s_action_state.death_flag == 1 ? "PASS" : "FAIL", s_action_state.death_flag);

    enemy = &s_units[1];
    result = enemy->hp == 0 && enemy->is_alive == 0 && s_action_state.death_flag == 1;

    printf("\nFinal result: %s\n", result ? "PASS" : "FAIL");

    return result ? 0 : 1;
}
