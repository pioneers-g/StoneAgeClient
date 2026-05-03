/*
 * Stone Age Client - Battle Action Executor Implementation
 * Reverse engineered from sa_9061.exe (FUN_00424f50)
 *
 * Battle action execution dispatcher for rendering and processing
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "battle_action_exec.h"
#include "battle.h"
#include "render.h"
#include "uiwidget.h"
#include "logger.h"

/* ========================================
 * Global State
 * ======================================== */

BattleActionExecState g_battle_exec = {0};

/* External network socket */
extern void* gSocket;

/* Button text labels - from DAT_004b91dc region */
static const char* s_action_button_labels[6] = {
    "Attack",   /* DAT_004b91dc */
    "Cancel",   /* s_CANCEL_004b91d4 */
    "Skill",    /* DAT_004aaa38 */
    "Item",     /* DAT_004aaa30 */
    "Defend",   /* DAT_004b91cc */
    "Escape"    /* DAT_004b91c4 */
};

/* ========================================
 * Initialization
 * ======================================== */

void battle_action_exec_init(void) {
    memset(&g_battle_exec, 0, sizeof(BattleActionExecState));
    g_battle_exec.button_height = 20;  /* DAT_0455b0dc = 0x14 */
    LOG_INFO("Battle action executor initialized");
}

void battle_action_exec_shutdown(void) {
    /* Cleanup any active windows */
    if (g_battle_exec.action_window_id) {
        /* widget_destroy(g_battle_exec.action_window_id); */
        g_battle_exec.action_window_id = 0;
    }
    if (g_battle_exec.skill_window_id) {
        /* widget_destroy(g_battle_exec.skill_window_id); */
        g_battle_exec.skill_window_id = 0;
    }

    memset(&g_battle_exec, 0, sizeof(BattleActionExecState));
    LOG_INFO("Battle action executor shutdown");
}

void battle_action_set_type(u32 action_type) {
    g_battle_exec.current_action = action_type;
}

u32 battle_action_get_type(void) {
    return g_battle_exec.current_action;
}

/* ========================================
 * Position Check - from FUN_00424f50
 * ======================================== */

int battle_action_check_position(void) {
    int dx, dy;

    /* Calculate distance from target position */
    dx = g_battle_exec.player_target_x - g_battle_exec.player_current_x;
    if (dx < 0) dx = -dx;

    dy = g_battle_exec.player_target_y - g_battle_exec.player_current_y;
    if (dy < 0) dy = -dy;

    /* Return 1 if within range (distance < 2) */
    return (dx < 2 && dy < 2);
}

/* ========================================
 * Main Action Executor - FUN_00424f50
 * ======================================== */

void battle_action_execute(void) {
    /* Check for death flag - DAT_0455efa0 */
    /* If set, send death notification to server */
    /* FUN_00464ac0(gSocket, 0, 1, -1, -1, &DAT_04554270) */

    /* Check if action is valid (DAT_004b83ec >= 0) */
    if ((s32)g_battle_exec.current_action < 0) {
        return;
    }

    /* Check position - from FUN_00424f50 */
    if (!battle_action_check_position()) {
        g_battle_exec.scroll_active = 1;
    }

    /* Main dispatcher switch - matches FUN_00424f50 */
    switch (g_battle_exec.current_action) {
        /* Attack actions */
        case 0:
        case 10:
            battle_action_render_attack(0);
            break;

        case 1:
        case 11:
            battle_action_render_attack(1);
            break;

        /* Skill selection */
        case 2:
            battle_action_render_skill_select();
            break;

        /* Counter setup */
        case 3:
            battle_action_render_counter_setup();
            break;

        /* Combo attacks */
        case 4:
            battle_action_render_combo(0);
            break;

        case 5:
            battle_action_render_combo(1);
            break;

        /* Counter/Combo execution */
        case 6:
        case 7:
        case 8:
            battle_action_render_counter_exec();
            break;

        /* Item usage */
        case 9:
            battle_action_render_item_select();
            break;

        /* Capture/End */
        case 12:
        case 13:
            battle_action_render_capture();
            break;

        /* Pet actions */
        case 14:
            battle_action_render_pet_attack();
            break;

        case 15:
            battle_action_render_pet_skill();
            break;

        /* Skill variations */
        case 18:
            battle_action_render_skill_chain();
            break;

        case 19:
        case 20:
            battle_action_render_skill_area();
            break;

        case 21:
            battle_action_render_skill_special();
            break;

        /* Summon/Recall */
        case 22:
            battle_action_render_summon(0);
            break;

        case 23:
            battle_action_render_recall(0);
            break;

        case 24:
            battle_action_render_pet_wait();
            break;

        case 25:
            battle_action_render_pet_defend();
            break;

        case 26:
            battle_action_render_pet_escape();
            break;

        case 27:
            battle_action_render_pet_item();
            break;

        /* Special actions */
        case 29:
            battle_action_render_death();
            break;

        case 30:
            battle_action_render_revive();
            break;

        case 31:
            battle_action_render_berserk();
            break;

        case 32:
            battle_action_render_transform();
            break;

        case 33:
            battle_action_render_chain();
            break;

        case 34:
            battle_action_render_combo_finish();
            break;

        case 35:
            battle_action_render_special_move();
            break;

        /* Target selection */
        case 36:
        case 39:
            battle_action_render_target_select(0);
            break;

        case 37:
        case 38:
            battle_action_render_target_select(1);
            break;

        /* Defense and escape */
        case 40:
            battle_action_render_defend();
            break;

        case 41:
            battle_action_render_escape(1);
            break;

        case 42:
            battle_action_render_escape(0);
            break;

        /* Pet variations */
        case 43:
            battle_action_render_recall(1);
            break;

        case 44:
            battle_action_render_summon(1);
            break;

        case 45:
            battle_action_render_attack(2);
            break;

        /* Pet battle system */
        case 103:
            battle_action_render_pet_battle_end();
            break;

        case 104:
            battle_action_render_pet_swap();
            break;

        case 105:
            battle_action_render_pet_battle_action();
            break;

        default:
            LOG_WARN("Unknown action type: %d", g_battle_exec.current_action);
            break;
    }
}

/* ========================================
 * Attack UI - FUN_00426380
 * ======================================== */

void battle_action_render_attack(int attack_type) {
    int window_x, window_y;
    int button_y;
    int row_height;
    int visible_rows;
    int button_count;
    int i;
    u32 action_bitmask;
    u32 bit_check;
    int buttons_placed;

    /* Check if window already created */
    if (g_battle_exec.action_window_id == 0) {
        /* Calculate window position - from FUN_00426380 */
        window_x = (g_battle_exec.window_width * -0x30 + 0x1c8) / 2;
        window_y = ((10 - g_battle_exec.window_height) * 0x40) / 2;

        /* Create window - FUN_00448610 */
        /* g_battle_exec.action_window_id = ui_window_create(window_x, window_y, width, height, 0, 1); */

        row_height = g_battle_exec.button_height;  /* 0x14 = 20 */
        button_y = (g_battle_exec.window_height * 0x30 - 0x38) / row_height;

        /* Count available actions from bitmask */
        action_bitmask = g_battle_exec.action_flags;
        button_count = 0;
        bit_check = 1;

        for (i = 0; i < 6; i++) {
            if ((bit_check & action_bitmask) != 0) {
                button_count++;
                if (button_count == 4) break;
            }
            bit_check <<= 1;
        }

        /* Store button count */
        g_battle_exec.skill_count = button_count;

        /* Calculate button positions */
        if (button_count > 0) {
            int spacing = (g_battle_exec.window_width << 6) / (button_count + 1);
            int start_x = spacing - 0x1b;

            for (i = 0; i < button_count; i++) {
                g_battle_exec.action_buttons[i].x = start_x;
                g_battle_exec.action_buttons[i].y = (button_y / row_height) * (row_height - 1) + 8 +
                                                     (g_battle_exec.window_height * 0x30 - row_height * g_battle_exec.button_height) / 2;
                start_x += spacing;
            }
        }

        /* Set up specific mode handling */
        if (attack_type == 0 || attack_type == 2) {
            /* Normal attack mode */
            g_battle_exec.skill_selected = 0;
            g_battle_exec.skill_scroll = 0;
        } else if (attack_type == 1) {
            /* Ranged/special attack mode */
            g_battle_exec.skill_selected = 0;
            g_battle_exec.skill_scroll = 0;
        }

        LOG_DEBUG("Attack UI created: type=%d, buttons=%d", attack_type, button_count);
    }

    /* Process button clicks - FUN_00421080 */
    /* int clicked = ui_widget_check_click(g_battle_exec.action_buttons, 6); */

    /* Check if action should be cancelled */
    /* if cancel or scroll active, end action */

    /* Render action buttons */
    battle_action_render_buttons();
}

/* ========================================
 * Skill Selection UI - FUN_00426850
 * ======================================== */

void battle_action_render_skill_select(void) {
    int i;
    int skill_start_y;
    int row_count;
    int visible_skills;

    /* Check if window already created */
    if (g_battle_exec.action_window_id == 0) {
        /* Create skill selection window */
        g_battle_exec.window_width = 7;
        g_battle_exec.window_height = 5;
        g_battle_exec.button_height = 0x60;  /* 96 */
        g_battle_exec.skill_count = 10;      /* DAT_0454efcc */

        /* Create window - FUN_00448610(0x60, 0x6c, 7, 5, 0, 1) */
        /* g_battle_exec.action_window_id = ui_window_create(0x60, 0x6c, 7, 5, 0, 1); */

        /* Count available actions */
        u32 action_bitmask = g_battle_exec.action_flags;
        int button_count = 0;
        u32 bit_check = 1;

        for (i = 0; i < 6; i++) {
            if ((bit_check & action_bitmask) != 0) {
                button_count++;
                if (button_count == 4) break;
            }
            bit_check <<= 1;
        }

        g_battle_exec.skill_selected = button_count;

        LOG_DEBUG("Skill selection UI created, buttons=%d", button_count);
    }

    /* Render skill list */
    battle_action_render_skill_list();

    /* Render action buttons */
    battle_action_render_buttons();
}

/* ========================================
 * Render Helper Functions
 * ======================================== */

void battle_action_render_buttons(void) {
    int i;
    int button_count;
    u32 action_bitmask;
    u32 bit_check;
    int buttons_placed;

    if (g_battle_exec.action_window_id == 0) return;
    if (*(int*)(g_battle_exec.action_window_id + 0x78) <= 0) return;  /* Check widget child count */

    action_bitmask = g_battle_exec.action_flags;
    buttons_placed = 0;

    for (i = 0; i < 6; i++) {
        /* Clear button widget ID */
        g_battle_exec.action_buttons[i].widget_id = 0xFFFFFFFE;

        if ((bit_check & action_bitmask) != 0 && buttons_placed < g_battle_exec.skill_selected) {
            /* Create button widget - FUN_0041d860 */
            /* widget_id = ui_button_create(x, y, 1, 4, label, 2); */
            g_battle_exec.action_buttons[i].widget_id = i;  /* Placeholder */
            buttons_placed++;
        }

        bit_check <<= 1;
    }
}

void battle_action_render_skill_list(void) {
    int i;
    int row_y;

    if (g_battle_exec.action_window_id == 0) return;
    if (*(int*)(g_battle_exec.action_window_id + 0x78) <= 0) return;

    row_y = 0x15;  /* 21 - starting Y offset */

    /* Render skill names */
    for (i = g_battle_exec.skill_scroll; i < g_battle_exec.skill_count && i < 10; i++) {
        if (g_battle_exec.skills[i].name[0] != '\0') {
            /* Render skill text - FUN_0041d860 */
            /* render_text(x + 0x2c, row_y + offset, skill_name, 0); */
        }
        row_y += 0x15;  /* 21 - row height */
    }
}

/* ========================================
 * Stub Implementations for Other Actions
 * ======================================== */

void battle_action_render_counter_setup(void) {
    LOG_DEBUG("Counter setup UI");
    /* TODO: Implement FUN_00426cc0 */
}

void battle_action_render_combo(int combo_phase) {
    LOG_DEBUG("Combo UI: phase=%d", combo_phase);
    /* TODO: Implement FUN_00427190, FUN_004276a0 */
}

void battle_action_render_counter_exec(void) {
    LOG_DEBUG("Counter/Combo execution UI");
    /* TODO: Implement FUN_00428280 */
}

void battle_action_render_item_select(void) {
    LOG_DEBUG("Item selection UI");
    /* TODO: Implement FUN_0042acf0 */
}

void battle_action_render_capture(void) {
    LOG_DEBUG("Capture UI");
    /* TODO: Implement FUN_0042ce40 */
}

void battle_action_render_pet_attack(void) {
    LOG_DEBUG("Pet attack UI");
    /* TODO: Implement FUN_0042e8f0 */
}

void battle_action_render_pet_skill(void) {
    LOG_DEBUG("Pet skill UI");
    /* TODO: Implement FUN_0042f370 */
}

void battle_action_render_skill_chain(void) {
    LOG_DEBUG("Skill chain UI");
    /* TODO: Implement FUN_00430b50 */
}

void battle_action_render_skill_area(void) {
    LOG_DEBUG("Area skill UI");
    /* TODO: Implement FUN_00431560 */
}

void battle_action_render_skill_special(void) {
    LOG_DEBUG("Special skill UI");
    /* TODO: Implement FUN_00431d60 */
}

void battle_action_render_summon(int is_alt) {
    LOG_DEBUG("Summon UI: alt=%d", is_alt);
    /* TODO: Implement FUN_00432a10 */
}

void battle_action_render_recall(int is_alt) {
    LOG_DEBUG("Recall UI: alt=%d", is_alt);
    /* TODO: Implement FUN_00433030 */
}

void battle_action_render_pet_wait(void) {
    LOG_DEBUG("Pet wait UI");
    /* TODO: Implement FUN_00436af0 */
}

void battle_action_render_pet_defend(void) {
    LOG_DEBUG("Pet defend UI");
    /* TODO: Implement FUN_004338d0 */
}

void battle_action_render_pet_escape(void) {
    LOG_DEBUG("Pet escape UI");
    /* TODO: Implement FUN_004340a0 */
}

void battle_action_render_pet_item(void) {
    LOG_DEBUG("Pet item UI");
    /* TODO: Implement FUN_00434610 */
}

void battle_action_render_death(void) {
    LOG_DEBUG("Death UI");
    /* TODO: Implement FUN_00434d60 */
}

void battle_action_render_revive(void) {
    LOG_DEBUG("Revive UI");
    /* TODO: Implement FUN_004354f0 */
}

void battle_action_render_berserk(void) {
    LOG_DEBUG("Berserk UI");
    /* TODO: Implement FUN_004366b0 */
}

void battle_action_render_transform(void) {
    LOG_DEBUG("Transform UI");
    /* TODO: Implement FUN_00435b40 */
}

void battle_action_render_chain(void) {
    LOG_DEBUG("Chain attack UI");
    /* TODO: Implement FUN_00435f70 */
}

void battle_action_render_combo_finish(void) {
    LOG_DEBUG("Combo finish UI");
    /* TODO: Implement FUN_00436220 */
}

void battle_action_render_special_move(void) {
    LOG_DEBUG("Special move UI");
    /* TODO: Implement FUN_00437200 */
}

void battle_action_render_target_select(int selection_type) {
    LOG_DEBUG("Target selection UI: type=%d", selection_type);
    /* TODO: Implement FUN_00438080, FUN_00438880 */
}

void battle_action_render_defend(void) {
    LOG_DEBUG("Defend UI");
    /* TODO: Implement FUN_00427cc0 */
}

void battle_action_render_escape(int success) {
    LOG_DEBUG("Escape UI: success=%d", success);
    /* TODO: Implement FUN_0042bb40, FUN_0042b150 */
}

void battle_action_render_pet_battle_end(void) {
    LOG_DEBUG("Pet battle end UI");
    /* TODO: Implement FUN_00439a00 */
}

void battle_action_render_pet_swap(void) {
    LOG_DEBUG("Pet swap UI");
    /* TODO: Implement FUN_0043a100 */
}

void battle_action_render_pet_battle_action(void) {
    LOG_DEBUG("Pet battle action UI");
    /* TODO: Implement FUN_0043a420 */
}

/* ========================================
 * Server Communication
 * ======================================== */

void battle_action_send_to_server(u32 action, u32 param1, u32 param2, u32 param3, const char* message) {
    /* Send action to server - FUN_0043b980 */
    LOG_DEBUG("Send action to server: %d, %d, %d, %d, %s", action, param1, param2, param3, message ? message : "");

    /* TODO: Implement actual network send */
}

void battle_action_update_scroll(void) {
    /* Update scroll/movement state */
    g_battle_exec.scroll_active = 0;
}
