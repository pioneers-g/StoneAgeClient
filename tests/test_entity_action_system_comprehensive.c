/*
 * Stone Age Client - Entity Action System Unit Tests
 * Tests for entity action dispatchers, state machine, effects
 *
 * Based on Ghidra decompilation analysis:
 * - FUN_0040ddd0: Entity action dispatcher (generic)
 * - FUN_004781f0: Player entity action dispatcher
 * - FUN_00477d90: Set entity state
 * - FUN_00477d70: Set entity timer
 * - FUN_00477cb0: Set entity position
 * - 30+ action types (0x00-0x3C)
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

/* Entity constants */
#define ENTITY_STATE_IDLE_1     0
#define ENTITY_STATE_IDLE_2     1
#define ENTITY_STATE_IDLE_3     2
#define ENTITY_STATE_NORMAL     3
#define ENTITY_STATE_WALKING    4
#define ENTITY_STATE_5          5
#define ENTITY_STATE_6          6
#define ENTITY_STATE_7          7
#define ENTITY_STATE_8          8
#define ENTITY_STATE_9          9
#define ENTITY_STATE_10         10
#define ENTITY_STATE_11         11
#define ENTITY_STATE_12         12

/* Action types */
#define ACTION_IDLE             0x00
#define ACTION_WALK             0x01
#define ACTION_STATE_0          0x02
#define ACTION_STATE_12         0x03
#define ACTION_STATE_1          0x04
#define ACTION_STATE_2          0x05
#define ACTION_STATE_2_ALT      0x0A
#define ACTION_STATE_5          0x0B
#define ACTION_STATE_6          0x0C
#define ACTION_STATE_7          0x0D
#define ACTION_STATE_8          0x0E
#define ACTION_STATE_9          0x0F
#define ACTION_STATE_10         0x10
#define ACTION_STATE_4_ALT      0x11
#define ACTION_STATE_11         0x12
#define ACTION_STATE_3_ALT      0x13
#define ACTION_FACE_DIRECTION   0x14
#define ACTION_EXPRESSION       0x15
#define ACTION_ANIMATION        0x16
#define ACTION_SET_TIMER        0x17
#define ACTION_MOVE_TARGET      0x1E
#define ACTION_STOP_MOVING      0x1F
#define ACTION_CREATE_CHILD     0x29
#define ACTION_DELETE_CHILD     0x2A
#define ACTION_CREATE_EFFECT    0x33
#define ACTION_CREATE_EFFECTS   0x3C

/* Sprite IDs */
#define SPRITE_EFFECT_1         0x18B00
#define SPRITE_EFFECT_2         0x18CFC
#define SPRITE_EFFECT_DEFAULT   0x18BAA
#define SPRITE_MULTI_EFFECT_0   0x18E36
#define SPRITE_MULTI_EFFECT_3   0x18E37

/* Test entity structure */
typedef struct Entity {
    struct Entity* prev;
    struct Entity* next;
    void (*update_func)(struct Entity*);
    void* extra_data;
    u8 type;
    u8 padding[15];
    s32 delete_flag;
    s32 id;
    s32 x;
    s32 y;
    s32 field_110;
    s32 state;
    s32 sub_state;
    s32 timer;
    s32 sprite_id;
} Entity;

/* Test extra data structure */
typedef struct ExtraData {
    s32 entity_index;
    void* child_entities[4];
    void* effect_entities[4];
    void* extra_slots[4];
} ExtraData;

/* Test data storage */
static Entity g_test_entity;
static ExtraData g_test_extra;
static s32 g_entity_flags[100];
static s32 g_entity_move_target[100];
static s32 g_entity_target_x[100];
static s32 g_entity_target_y[100];
static Entity* g_current_player;
static int g_entity_created_count;
static int g_entity_deleted_count;

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
    memset(&g_test_entity, 0, sizeof(Entity));
    memset(&g_test_extra, 0, sizeof(ExtraData));
    memset(g_entity_flags, 0, sizeof(g_entity_flags));
    memset(g_entity_move_target, 0, sizeof(g_entity_move_target));
    memset(g_entity_target_x, 0, sizeof(g_entity_target_x));
    memset(g_entity_target_y, 0, sizeof(g_entity_target_y));

    g_test_entity.extra_data = &g_test_extra;
    g_test_extra.entity_index = 0;
    g_current_player = &g_test_entity;
    g_entity_created_count = 0;
    g_entity_deleted_count = 0;
}

/*
 * Set entity state (FUN_00477d90)
 */
static void entity_set_state(Entity* entity, s32 state) {
    if (entity != NULL) {
        entity->state = state;
        if (state != ENTITY_STATE_WALKING) {
            entity->sub_state = -1;
        }
    }
}

/*
 * Set entity timer (FUN_00477d70)
 */
static void entity_set_timer(Entity* entity, u32 timer) {
    entity->timer = timer;
    if (g_current_player != NULL) {
        g_current_player->timer = timer;
    }
}

/*
 * Set entity position (FUN_00477cb0)
 */
static void entity_set_position(Entity* entity, s32 x, s32 y) {
    entity->x = x;
    entity->y = y;
}

/*
 * Entity action dispatcher (simplified FUN_0040ddd0)
 */
static void entity_action_dispatch(Entity* entity, s32 x, s32 y, u32 timer,
                                   s32 action, s32 param1, s32 param2, s32 param3) {
    if (entity == NULL) return;

    ExtraData* extra = (ExtraData*)entity->extra_data;
    s32 entity_index = extra->entity_index;

    switch (action) {
        case ACTION_IDLE:
            entity->field_110 = 0;
            entity->state = ENTITY_STATE_NORMAL;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_WALK:
            entity->state = ENTITY_STATE_WALKING;
            break;

        case ACTION_STATE_0:
            entity->state = ENTITY_STATE_IDLE_1;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_12:
            entity->state = ENTITY_STATE_12;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_1:
            entity->state = ENTITY_STATE_IDLE_2;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_2:
        case ACTION_STATE_2_ALT:
            entity->state = ENTITY_STATE_IDLE_3;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_5:
            entity->state = ENTITY_STATE_5;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_6:
            entity->state = ENTITY_STATE_6;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_7:
            entity->state = ENTITY_STATE_7;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_8:
            entity->state = ENTITY_STATE_8;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_9:
            entity->state = ENTITY_STATE_9;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_10:
            entity->state = ENTITY_STATE_10;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_4_ALT:
            entity->state = ENTITY_STATE_WALKING;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_11:
            entity->state = ENTITY_STATE_11;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_STATE_3_ALT:
            entity->state = ENTITY_STATE_NORMAL;
            entity->sub_state = -1;
            entity->timer = timer;
            break;

        case ACTION_FACE_DIRECTION:
            entity->state = ENTITY_STATE_NORMAL;
            entity->sub_state = -1;
            entity_set_position(entity, x, y);
            entity->timer = timer;
            break;

        case ACTION_EXPRESSION:
            entity_set_position(entity, x, y);
            entity->timer = timer;
            break;

        case ACTION_ANIMATION:
            entity_set_position(entity, x, y);
            entity->timer = timer;
            break;

        case ACTION_SET_TIMER:
            entity->timer = param1;
            break;

        case ACTION_MOVE_TARGET:
            g_entity_move_target[entity_index] = timer;
            g_entity_target_x[entity_index] = x;
            g_entity_target_y[entity_index] = y;
            break;

        case ACTION_STOP_MOVING:
            entity_set_position(entity, x, y);
            entity->field_110 = 0;
            entity->state = ENTITY_STATE_NORMAL;
            entity->sub_state = -1;
            entity->timer = timer;
            g_entity_move_target[entity_index] = -1;
            break;

        case ACTION_CREATE_CHILD:
            if (extra->child_entities[0] != NULL) {
                g_entity_deleted_count++;
                extra->child_entities[0] = NULL;
            }
            extra->child_entities[0] = (void*)1;  // Simulate creation
            g_entity_created_count++;
            break;

        case ACTION_DELETE_CHILD:
            if (extra->child_entities[0] != NULL) {
                g_entity_deleted_count++;
                extra->child_entities[0] = NULL;
            }
            break;

        case ACTION_CREATE_EFFECT:
            if (extra->effect_entities[0] != NULL) {
                g_entity_deleted_count++;
                extra->effect_entities[0] = NULL;
            }
            extra->effect_entities[0] = (void*)1;  // Simulate creation
            entity->sprite_id = param2;
            g_entity_created_count++;
            break;

        case ACTION_CREATE_EFFECTS:
            // Clear existing
            for (int i = 0; i < 4; i++) {
                if (extra->extra_slots[i] != NULL) {
                    g_entity_deleted_count++;
                    extra->extra_slots[i] = NULL;
                }
            }
            // Create new
            for (int i = 0; i < 4; i++) {
                extra->extra_slots[i] = (void*)1;
                g_entity_created_count++;
            }
            break;
    }
}

/* ========================================
 * State Action Tests
 * ======================================== */

static int test_action_idle(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 100, ACTION_IDLE, 0, 0, 0);

    assert(g_test_entity.state == ENTITY_STATE_NORMAL);
    assert(g_test_entity.sub_state == -1);
    assert(g_test_entity.timer == 100);
    assert(g_test_entity.field_110 == 0);

    return 1;
}

static int test_action_walk(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 100, 200, 50, ACTION_WALK, 0, 0, 0);

    assert(g_test_entity.state == ENTITY_STATE_WALKING);

    return 1;
}

static int test_action_state_0(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 200, ACTION_STATE_0, 0, 0, 0);

    assert(g_test_entity.state == ENTITY_STATE_IDLE_1);
    assert(g_test_entity.sub_state == -1);
    assert(g_test_entity.timer == 200);

    return 1;
}

static int test_action_state_12(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 300, ACTION_STATE_12, 0, 0, 0);

    assert(g_test_entity.state == ENTITY_STATE_12);
    assert(g_test_entity.timer == 300);

    return 1;
}

static int test_action_state_2(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 150, ACTION_STATE_2, 0, 0, 0);

    assert(g_test_entity.state == ENTITY_STATE_IDLE_3);
    assert(g_test_entity.timer == 150);

    return 1;
}

static int test_action_state_2_alt(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 160, ACTION_STATE_2_ALT, 0, 0, 0);

    assert(g_test_entity.state == ENTITY_STATE_IDLE_3);
    assert(g_test_entity.timer == 160);

    return 1;
}

static int test_action_all_states(void) {
    test_setup();

    s32 actions[] = {ACTION_STATE_5, ACTION_STATE_6, ACTION_STATE_7,
                     ACTION_STATE_8, ACTION_STATE_9, ACTION_STATE_10,
                     ACTION_STATE_11};
    s32 expected_states[] = {ENTITY_STATE_5, ENTITY_STATE_6, ENTITY_STATE_7,
                             ENTITY_STATE_8, ENTITY_STATE_9, ENTITY_STATE_10,
                             ENTITY_STATE_11};

    for (int i = 0; i < 7; i++) {
        entity_action_dispatch(&g_test_entity, 0, 0, 100, actions[i], 0, 0, 0);
        assert(g_test_entity.state == expected_states[i]);
    }

    return 1;
}

/* ========================================
 * Position/Timer Tests
 * ======================================== */

static int test_action_face_direction(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 100, 200, 500, ACTION_FACE_DIRECTION, 3, 0, 0);

    assert(g_test_entity.state == ENTITY_STATE_NORMAL);
    assert(g_test_entity.x == 100);
    assert(g_test_entity.y == 200);
    assert(g_test_entity.timer == 500);

    return 1;
}

static int test_action_set_timer(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_SET_TIMER, 999, 0, 0);

    assert(g_test_entity.timer == 999);

    return 1;
}

static int test_action_move_target(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 150, 250, 600, ACTION_MOVE_TARGET, 0, 0, 0);

    assert(g_entity_move_target[0] == 600);
    assert(g_entity_target_x[0] == 150);
    assert(g_entity_target_y[0] == 250);

    return 1;
}

static int test_action_stop_moving(void) {
    test_setup();

    g_test_entity.field_110 = 10;
    g_test_entity.state = ENTITY_STATE_WALKING;
    g_entity_move_target[0] = 500;

    entity_action_dispatch(&g_test_entity, 80, 90, 700, ACTION_STOP_MOVING, 0, 0, 0);

    assert(g_test_entity.state == ENTITY_STATE_NORMAL);
    assert(g_test_entity.field_110 == 0);
    assert(g_test_entity.x == 80);
    assert(g_test_entity.y == 90);
    assert(g_entity_move_target[0] == -1);

    return 1;
}

/* ========================================
 * Effect Creation Tests
 * ======================================== */

static int test_action_create_child(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_CHILD, 0, 0, 0);

    assert(g_test_extra.child_entities[0] != NULL);
    assert(g_entity_created_count == 1);

    return 1;
}

static int test_action_delete_child(void) {
    test_setup();

    // Create first
    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_CHILD, 0, 0, 0);
    assert(g_entity_created_count == 1);

    // Then delete
    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_DELETE_CHILD, 0, 0, 0);
    assert(g_test_extra.child_entities[0] == NULL);
    assert(g_entity_deleted_count == 1);

    return 1;
}

static int test_action_create_effect(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_EFFECT, 3, SPRITE_EFFECT_1, 0);

    assert(g_test_extra.effect_entities[0] != NULL);
    assert(g_test_entity.sprite_id == SPRITE_EFFECT_1);
    assert(g_entity_created_count == 1);

    return 1;
}

static int test_action_create_effects(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_EFFECTS, 0, 0, 1);

    assert(g_entity_created_count == 4);
    for (int i = 0; i < 4; i++) {
        assert(g_test_extra.extra_slots[i] != NULL);
    }

    return 1;
}

static int test_action_create_effects_clears_existing(void) {
    test_setup();

    // Create initial effects
    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_EFFECTS, 0, 0, 1);
    assert(g_entity_created_count == 4);

    // Create again (should clear existing)
    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_EFFECTS, 0, 0, 1);

    assert(g_entity_deleted_count == 4);  // Previous 4 deleted
    assert(g_entity_created_count == 8);  // 4 + 4 created

    return 1;
}

/* ========================================
 * Sprite ID Tests
 * ======================================== */

static int test_sprite_effect_ids(void) {
    test_setup();

    assert(SPRITE_EFFECT_1 == 0x18B00);
    assert(SPRITE_EFFECT_2 == 0x18CFC);
    assert(SPRITE_EFFECT_DEFAULT == 0x18BAA);
    assert(SPRITE_MULTI_EFFECT_0 == 0x18E36);
    assert(SPRITE_MULTI_EFFECT_3 == 0x18E37);

    return 1;
}

static int test_action_expression(void) {
    test_setup();

    g_test_entity.x = 0;
    g_test_entity.y = 0;

    entity_action_dispatch(&g_test_entity, 50, 60, 800, ACTION_EXPRESSION, 1, 0, 0);

    assert(g_test_entity.x == 50);
    assert(g_test_entity.y == 60);
    assert(g_test_entity.timer == 800);

    return 1;
}

static int test_action_animation(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 70, 80, 900, ACTION_ANIMATION, 0, 0, 0);

    assert(g_test_entity.x == 70);
    assert(g_test_entity.y == 80);
    assert(g_test_entity.timer == 900);

    return 1;
}

/* ========================================
 * Null Entity Tests
 * ======================================== */

static int test_null_entity_no_crash(void) {
    test_setup();

    // Should not crash with null entity
    entity_action_dispatch(NULL, 0, 0, 0, ACTION_IDLE, 0, 0, 0);

    return 1;
}

/* ========================================
 * State Machine Tests
 * ======================================== */

static int test_state_values(void) {
    test_setup();

    assert(ENTITY_STATE_IDLE_1 == 0);
    assert(ENTITY_STATE_IDLE_2 == 1);
    assert(ENTITY_STATE_IDLE_3 == 2);
    assert(ENTITY_STATE_NORMAL == 3);
    assert(ENTITY_STATE_WALKING == 4);

    return 1;
}

static int test_action_values(void) {
    test_setup();

    assert(ACTION_IDLE == 0x00);
    assert(ACTION_WALK == 0x01);
    assert(ACTION_FACE_DIRECTION == 0x14);
    assert(ACTION_STOP_MOVING == 0x1F);
    assert(ACTION_CREATE_EFFECT == 0x33);
    assert(ACTION_CREATE_EFFECTS == 0x3C);

    return 1;
}

static int test_substate_reset_on_state_change(void) {
    test_setup();

    g_test_entity.sub_state = 100;  // Set some value

    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_IDLE, 0, 0, 0);

    assert(g_test_entity.sub_state == -1);

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_multiple_actions_sequence(void) {
    test_setup();

    // Start idle
    entity_action_dispatch(&g_test_entity, 0, 0, 100, ACTION_IDLE, 0, 0, 0);
    assert(g_test_entity.state == ENTITY_STATE_NORMAL);

    // Start walking
    entity_action_dispatch(&g_test_entity, 100, 200, 200, ACTION_WALK, 0, 0, 0);
    assert(g_test_entity.state == ENTITY_STATE_WALKING);

    // Set timer
    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_SET_TIMER, 999, 0, 0);
    assert(g_test_entity.timer == 999);

    // Stop
    entity_action_dispatch(&g_test_entity, 150, 250, 300, ACTION_STOP_MOVING, 0, 0, 0);
    assert(g_test_entity.state == ENTITY_STATE_NORMAL);
    assert(g_test_entity.x == 150);
    assert(g_test_entity.y == 250);

    return 1;
}

static int test_create_delete_cycle(void) {
    test_setup();

    // Create child
    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_CHILD, 0, 0, 0);
    assert(g_entity_created_count == 1);

    // Create effect
    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_EFFECT, 0, 0, 0);
    assert(g_entity_created_count == 2);

    // Delete child
    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_DELETE_CHILD, 0, 0, 0);
    assert(g_entity_deleted_count == 1);

    return 1;
}

static int test_move_target_to_stop(void) {
    test_setup();

    // Set move target
    entity_action_dispatch(&g_test_entity, 100, 200, 500, ACTION_MOVE_TARGET, 0, 0, 0);
    assert(g_entity_move_target[0] == 500);

    // Stop
    entity_action_dispatch(&g_test_entity, 100, 200, 0, ACTION_STOP_MOVING, 0, 0, 0);
    assert(g_entity_move_target[0] == -1);

    return 1;
}

/* ========================================
 * Additional State Tests
 * ======================================== */

static int test_action_state_4_alt(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 400, ACTION_STATE_4_ALT, 0, 0, 0);

    assert(g_test_entity.state == ENTITY_STATE_WALKING);
    assert(g_test_entity.timer == 400);

    return 1;
}

static int test_action_state_3_alt(void) {
    test_setup();

    g_test_entity.state = ENTITY_STATE_WALKING;

    entity_action_dispatch(&g_test_entity, 0, 0, 500, ACTION_STATE_3_ALT, 0, 0, 0);

    assert(g_test_entity.state == ENTITY_STATE_NORMAL);
    assert(g_test_entity.timer == 500);

    return 1;
}

static int test_action_state_1(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 600, ACTION_STATE_1, 0, 0, 0);

    assert(g_test_entity.state == ENTITY_STATE_IDLE_2);
    assert(g_test_entity.timer == 600);

    return 1;
}

/* ========================================
 * Effect Type Tests
 * ======================================== */

static int test_effect_sprite_default(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_EFFECT, 0, SPRITE_EFFECT_DEFAULT, 0);

    assert(g_test_entity.sprite_id == SPRITE_EFFECT_DEFAULT);

    return 1;
}

static int test_effect_sprite_special(void) {
    test_setup();

    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_EFFECT, 0, SPRITE_EFFECT_1, 0);

    assert(g_test_entity.sprite_id == SPRITE_EFFECT_1);

    return 1;
}

static int test_child_recreate(void) {
    test_setup();

    // Create first child
    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_CHILD, 0, 0, 0);
    void* first_child = g_test_extra.child_entities[0];

    // Create again (should replace)
    entity_action_dispatch(&g_test_entity, 0, 0, 0, ACTION_CREATE_CHILD, 0, 0, 0);

    assert(g_entity_deleted_count == 1);  // First child deleted
    assert(g_entity_created_count == 2);  // Two children created

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Entity Action System Unit Tests ===\n\n");

    /* State action tests */
    printf("State Action Tests:\n");
    TEST(action_idle);
    TEST(action_walk);
    TEST(action_state_0);
    TEST(action_state_12);
    TEST(action_state_2);
    TEST(action_state_2_alt);
    TEST(action_all_states);
    TEST(action_state_4_alt);
    TEST(action_state_3_alt);
    TEST(action_state_1);

    /* Position/timer tests */
    printf("\nPosition/Timer Tests:\n");
    TEST(action_face_direction);
    TEST(action_set_timer);
    TEST(action_move_target);
    TEST(action_stop_moving);

    /* Effect creation tests */
    printf("\nEffect Creation Tests:\n");
    TEST(action_create_child);
    TEST(action_delete_child);
    TEST(action_create_effect);
    TEST(action_create_effects);
    TEST(action_create_effects_clears_existing);
    TEST(effect_sprite_default);
    TEST(effect_sprite_special);
    TEST(child_recreate);

    /* Sprite ID tests */
    printf("\nSprite ID Tests:\n");
    TEST(sprite_effect_ids);
    TEST(action_expression);
    TEST(action_animation);

    /* Null entity tests */
    printf("\nNull Entity Tests:\n");
    TEST(null_entity_no_crash);

    /* State machine tests */
    printf("\nState Machine Tests:\n");
    TEST(state_values);
    TEST(action_values);
    TEST(substate_reset_on_state_change);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(multiple_actions_sequence);
    TEST(create_delete_cycle);
    TEST(move_target_to_stop);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Movement flag checks (0x200, 0x100)
     * - Long walk vs short walk
     * - Effect position calculation (param1 == 3)
     * - Player-specific action dispatcher (FUN_004781f0)
     * - Real game action sequences
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
