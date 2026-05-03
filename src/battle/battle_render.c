/*
 * Stone Age Client - Battle Render Core
 * Reverse engineered from sa_9061.exe
 *
 * Core battle rendering definitions and memory locations.
 * Complex UI renders: battle_render_ui.c
 * Simple action stubs: battle_render_actions.c
 */

#include <windows.h>
#include "types.h"
#include "battle_render.h"
#include "logger.h"

/* ========================================
 * Memory Locations from Binary
 * ======================================== */

/* Attack UI state - FUN_00426380 */
#define DAT_0455ef98 (*(int*)0x0455ef98)
#define DAT_0455ef94 (*(u32*)0x0455ef94)
#define DAT_0455eec8 (*(int*)0x0455eec8)
#define DAT_0455af84 (*(int*)0x0455af84)
#define DAT_045541dc (*(int*)0x045541dc)
#define DAT_045541a4 (*(int*)0x045541a4)
#define DAT_0454f3c8 (*(int*)0x0454f3c8)
#define DAT_0455796c (*(int*)0x0455796c)
#define DAT_04552e38 (*(int*)0x04552e38)
#define DAT_0455641c (*(int*)0x0455641c)
#define DAT_0455b0dc (*(int*)0x0455b0dc)
#define DAT_004b83f0 (*(int*)0x004b83f0)
#define DAT_004b83ec (*(int*)0x004b83ec)

/* Skill UI state - FUN_00426850 */
#define DAT_0455b118 (*(int*)0x0455b118)
#define _DAT_04552e34 (*(int*)0x04552e34)
#define DAT_04556404 (*(int*)0x04556404)
#define DAT_04552b30 (*(int*)0x04552b30)
#define DAT_004b8408 (*(int*)0x004b8408)
#define DAT_04556478 (*(int*)0x04556478)
#define DAT_0454efcc (*(int*)0x0454efcc)
#define DAT_0454f5b0 (*(int*)0x0454f5b0)

/* Capture state - FUN_0042ce40 */
#define DAT_0455a0fc (*(int*)0x0455a0fc)
#define DAT_0454f25c (*(int*)0x0454f25c)
#define DAT_04558c98 (*(int*)0x04558c98)

/* External globals */
extern int gSocket;
extern int DAT_04581d3c, DAT_04581d40;
extern int DAT_04558c34, DAT_0455b5ac;
extern u32 DAT_04ebe490;
extern int DAT_005676a0, DAT_00564e62, DAT_0461b420;
extern int DAT_0455ef9c;
extern int DAT_0461b658;
extern int DAT_046308b8;
extern int DAT_045829d8, DAT_045829dc;

/* External functions */
extern void FUN_0043b980(int socket, int x, int y, int ctx1, int ctx2, int target, void* data);
extern void FUN_0048fdc0(int socket, int x, int y, int ctx1, int ctx2, int target, void* data);
extern int FUN_00448610(int x, int y, int width, int height, int flags, int visible);
extern void FUN_004011c0(int window);
extern int FUN_0041d860(int x, int y, int font, int style, const char* text, int color);
extern int FUN_00421080(int* buttons, int count);
extern int FUN_0044b030(void);
extern void FUN_0048a200(void* dest, void* src, int size, ...);
extern void FUN_004923a7(void* dest, void* src);
extern void FUN_00488190(int r, int g, int b);

/* ========================================
 * Animation State Helper
 * ======================================== */

/*
 * Initialize animation state
 */
void battle_anim_init(BattleAnimState* state) {
    memset(state, 0, sizeof(BattleAnimState));
    state->frame = 0;
    state->timer = 0;
    state->active = 1;
}

/*
 * Update animation state
 */
int battle_anim_update(BattleAnimState* state) {
    if (!state->active) return 0;

    state->timer++;

    /* Check for frame advance */
    if (state->timer >= state->frame_delay) {
        state->timer = 0;
        state->frame++;

        if (state->frame >= state->total_frames) {
            if (state->loop) {
                state->frame = 0;
            } else {
                state->active = 0;
                return 0;
            }
        }
    }

    return 1;
}

/*
 * Check if animation is complete
 */
int battle_anim_is_complete(BattleAnimState* state) {
    return !state->active;
}

/* ========================================
 * External Render Function Declarations
 * Implemented in battle_render_ui.c and battle_render_actions.c
 * ======================================== */

extern void battle_render_action_0(int variant);
extern void battle_render_action_2(void);
extern void battle_render_action_3(void);
extern void battle_render_action_4(void);
extern void battle_render_action_5(void);
extern void battle_render_action_678(void);
extern void battle_render_action_9(void);
extern void battle_render_action_cd(void);
extern void battle_render_action_e(void);
extern void battle_render_action_f(void);
extern void battle_render_action_12(void);
extern void battle_render_action_1314(void);
extern void battle_render_action_15(void);
extern void battle_render_action_16(int variant);
extern void battle_render_action_17(int variant);
extern void battle_render_action_18(void);
extern void battle_render_action_19(int param);
extern void battle_render_action_1a(int param);
extern void battle_render_action_1b(int param);
extern void battle_render_action_1d(void);
extern void battle_render_action_1e(void);
extern void battle_render_action_1f(void);
extern void battle_render_action_20(void);
extern void battle_render_action_21(void);
extern void battle_render_action_22(void);
extern void battle_render_action_23(void);
extern void battle_render_action_2427(void);
extern void battle_render_action_2526(void);
extern void battle_render_action_28(void);
extern void battle_render_action_29(void);
extern void battle_render_action_2a(void);
extern void battle_render_action_67(void);
extern void battle_render_action_68(void);
extern void battle_render_action_69(void);

/* ========================================
 * Main Action Render Dispatcher
 * ======================================== */

/*
 * Dispatch to appropriate action render function
 * Called from FUN_00424f50 (battle_action_execute)
 */
void battle_render_dispatch_action(int action_type, int param) {
    switch (action_type) {
        case 0:
        case 1:
        case 10:
        case 0x0b:
        case 0x2d:
            battle_render_action_0(param);
            break;

        case 2:
            battle_render_action_2();
            break;

        case 3:
            battle_render_action_3();
            break;

        case 4:
            battle_render_action_4();
            break;

        case 5:
            battle_render_action_5();
            break;

        case 6:
        case 7:
        case 8:
            battle_render_action_678();
            break;

        case 9:
            battle_render_action_9();
            break;

        case 0x0c:
        case 0x0d:
            battle_render_action_cd();
            break;

        case 0x0e:
            battle_render_action_e();
            break;

        case 0x0f:
            battle_render_action_f();
            break;

        case 0x12:
            battle_render_action_12();
            break;

        case 0x13:
        case 0x14:
            battle_render_action_1314();
            break;

        case 0x15:
            battle_render_action_15();
            break;

        case 0x16:
        case 0x2c:
            battle_render_action_16(param);
            break;

        case 0x17:
        case 0x2b:
            battle_render_action_17(param);
            break;

        case 0x18:
            battle_render_action_18();
            break;

        case 0x19:
            battle_render_action_19(param);
            break;

        case 0x1a:
            battle_render_action_1a(param);
            break;

        case 0x1b:
            battle_render_action_1b(param);
            break;

        case 0x1d:
            battle_render_action_1d();
            break;

        case 0x1e:
            battle_render_action_1e();
            break;

        case 0x1f:
            battle_render_action_1f();
            break;

        case 0x20:
            battle_render_action_20();
            break;

        case 0x21:
            battle_render_action_21();
            break;

        case 0x22:
            battle_render_action_22();
            break;

        case 0x23:
            battle_render_action_23();
            break;

        case 0x24:
        case 0x27:
            battle_render_action_2427();
            break;

        case 0x25:
        case 0x26:
            battle_render_action_2526();
            break;

        case 0x28:
            battle_render_action_28();
            break;

        case 0x29:
            battle_render_action_29();
            break;

        case 0x2a:
            battle_render_action_2a();
            break;

        case 0x67:
            battle_render_action_67();
            break;

        case 0x68:
            battle_render_action_68();
            break;

        case 0x69:
            battle_render_action_69();
            break;

        default:
            LOG_DEBUG("Unknown battle action render: 0x%x", action_type);
            break;
    }
}
