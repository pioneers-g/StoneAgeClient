/*
 * Stone Age Client - Battle Action Render/Execute Module
 * Reverse engineered from sa_9061.exe (FUN_00424f50)
 *
 * Action execution and rendering dispatcher.
 * Separated from core to allow independent updates.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "battle_action.h"
#include "battle.h"
#include "logger.h"

/* External state from core module */
extern BattleActionState g_battle_action;

/* Position data - matches DAT_045528c8, DAT_0454fe98 */
static u16 g_player_x = 0;
static u16 g_player_y = 0;

/* Death message buffer - matches DAT_04554270 */
static char s_death_message[256] = {0};

/* Effect and sound implementation */
static BattleEffect s_effects[MAX_BATTLE_EFFECTS];
static u32 s_effect_count = 0;

/*
 * Show visual effect
 * Effect IDs: 1=hit, 2=heal, 3=death, 4=revive, 5=skill, 6=summon, 7=recall
 */
void action_show_effect(u32 effect_id, u32 x, u32 y) {
    BattleEffect* effect;

    if (s_effect_count >= MAX_BATTLE_EFFECTS) {
        return;  /* Effect queue full */
    }

    effect = &s_effects[s_effect_count++];
    effect->id = (u16)effect_id;
    effect->x = (u16)x;
    effect->y = (u16)y;
    effect->timer = 30;  /* Default 30 frames */
    effect->frame = 0;
    effect->active = 1;

    LOG_DEBUG("Effect %d at (%d, %d)", effect_id, x, y);
}

/*
 * Play sound effect
 * Sound IDs: 1=attack, 2=skill, 3=item, 4=capture, 5=capture_fail, 6=summon, 7=recall, 8=death, 9=revive
 */
void action_play_sound(u32 sound_id) {
    /* Integrate with sound system */
    /* Actual implementation would call audio_play_sound() */

    /* Sound file mapping:
     * 1 -> hit.wav
     * 2 -> skill.wav
     * 3 -> item.wav
     * 4 -> capture.wav
     * 5 -> capture_fail.wav
     * 6 -> summon.wav
     * 7 -> recall.wav
     * 8 -> death.wav
     * 9 -> revive.wav
     */

    LOG_DEBUG("Play sound %d", sound_id);
}

/*
 * Check if player position is close to action position
 */
int battle_action_check_position(void) {
    int dx = g_battle_action.player_x - g_player_x;
    int dy = g_battle_action.player_y - g_player_y;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    /* If within 2 tiles, position is valid */
    if (dx >= 2 || dy >= 2) {
        return 0;  /* Position too far */
    }

    return 1;  /* Position valid */
}

/*
 * Get effect queue for rendering
 */
BattleEffect* battle_action_get_effects(u32* count) {
    if (count) {
        *count = s_effect_count;
    }
    return s_effects;
}

/*
 * Clear effect queue
 */
void battle_action_clear_effects(void) {
    s_effect_count = 0;
    memset(s_effects, 0, sizeof(s_effects));
}

/*
 * Action execution dispatcher - FUN_00424f50
 * Executes the rendering/animation for the current battle action
 */
void battle_action_execute(void) {
    u32 action = g_battle_action.current_action;

    /* Check death flag - if set, send death message */
    if (g_battle_action.death_flag != 0) {
        g_battle_action.death_flag = 0;
        /* FUN_00464ac0: Send death message packet */
        LOG_DEBUG("Death flag set, sending death message");
    }

    /* Check if action is valid (not negative/terminated) */
    if ((s32)action < 0) {
        return;
    }

    /* Position check */
    if (!battle_action_check_position()) {
        LOG_DEBUG("Player position changed during action");
    }

    /* Main action execution switch - matches FUN_00424f50 */
    switch (action) {
        /* Basic attacks */
        case 0:
        case 10:
            /* battle_render_action_0(0); */
            break;

        case 1:
        case 0x0b:
            /* battle_render_action_0(1); */
            break;

        case 0x2d:
            /* battle_render_action_0(2); */
            break;

        /* Skill */
        case 2:
            /* battle_render_action_2(); */
            break;

        /* Counter */
        case 3:
            /* battle_render_action_3(); */
            break;

        /* Defend */
        case 4:
            /* battle_render_action_4(); */
            break;

        /* Charge */
        case 5:
            /* battle_render_action_5(); */
            break;

        /* Combo attacks */
        case 6:
        case 7:
        case 8:
            /* battle_render_action_678(); */
            break;

        /* Item */
        case 9:
            /* battle_render_action_9(); */
            break;

        /* Capture */
        case 0x0c:
        case 0x0d:
            /* battle_render_action_cd(); */
            break;

        /* Pet actions */
        case 0x0e:
            /* battle_render_action_e(); */
            break;

        case 0x0f:
            /* battle_render_action_f(); */
            break;

        /* Skills */
        case 0x12:
            /* battle_render_action_12(); */
            break;

        case 0x13:
        case 0x14:
            /* battle_render_action_1314(); */
            break;

        case 0x15:
            /* battle_render_action_15(); */
            break;

        /* Summon/Recall */
        case 0x16:
            /* battle_render_action_16(0); */
            break;

        case 0x17:
            /* battle_render_action_17(0); */
            break;

        case 0x2c:
            /* battle_render_action_16(1); */
            break;

        case 0x2b:
            /* battle_render_action_17(1); */
            break;

        /* Special actions */
        case 0x18:
            /* battle_render_action_18(); */
            break;

        case 0x19:
            /* battle_render_action_19(0); */
            break;

        case 0x1a:
            /* battle_render_action_1a(0); */
            break;

        case 0x1b:
            /* battle_render_action_1b(0); */
            break;

        case 0x1d:
            /* battle_render_action_1d(); */
            break;

        case 0x1e:
            /* battle_render_action_1e(); */
            break;

        case 0x1f:
            /* battle_render_action_1f(); */
            break;

        case 0x20:
            /* battle_render_action_20(); */
            break;

        case 0x21:
            /* battle_render_action_21(); */
            break;

        case 0x22:
            /* battle_render_action_22(); */
            break;

        case 0x23:
            /* battle_render_action_23(); */
            break;

        /* Target selection */
        case 0x24:
        case 0x27:
            /* battle_render_action_2427(); */
            break;

        case 0x25:
        case 0x26:
            /* battle_render_action_2526(); */
            break;

        case 0x28:
            /* battle_render_action_28(); */
            break;

        case 0x29:
            /* battle_render_action_29(); */
            break;

        case 0x2a:
            /* battle_render_action_2a(); */
            break;

        /* Pet escape/swap */
        case 0x67:
            /* battle_render_action_67(); */
            break;

        case 0x68:
            /* battle_render_action_68(); */
            break;

        case 0x69:
            /* battle_render_action_69(); */
            break;

        default:
            LOG_DEBUG("Unknown action render type: 0x%02x", action);
            break;
    }
}
