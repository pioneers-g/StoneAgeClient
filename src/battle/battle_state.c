/*
 * Stone Age Client - Battle State Machine Implementation
 * Reverse engineered from sa_9061.exe FUN_0040a1a0
 *
 * Contains the 9-state battle machine:
 * - States 0-1: Initialization
 * - State 2: Main loop (action selection)
 * - State 3: Action processing
 * - State 4: Execute action
 * - State 5: Result processing
 * - State 6: Message display
 * - State 7-8: Battle end
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "battle.h"
#include "battle_state.h"
#include "sound.h"
#include "gamestate.h"
#include "network.h"
#include "logger.h"

/* External battle context */
extern BattleContext g_battle;

/* External functions from battle.c */
extern void battle_clear_entity_queue(void);
extern void battle_reset_ai_state(void);
extern void battle_clear_unit_flags(void);
extern void battle_clear_render_queue(void);
extern void battle_init_render_queue(void);
extern int battle_is_special_map(u32 map_id);
extern u32 battle_create_dungeon_entity(u32 index);
extern int battle_load_field(u32 field_id);
extern void battle_menu_init(void);
extern u32 battle_get_bgm_for_map(u32 map_id);
extern void battle_start_fade(u32 fade_type);
extern void battle_process_network(void);
extern void battle_handle_unit_action_response(u32 unit_index);
extern u32 battle_get_action_type(void);
extern void battle_execute_pending_action(void);
extern void battle_update_state(void);
extern void battle_handle_action_results(void);
extern int battle_check_end_condition(void);
extern void battle_ai_execute_action(void);
extern void battle_ai_update(void);
extern void battle_player_execute_action(void);
extern void battle_update_unit_display(u32 unit_index);
extern void battle_update_unit_displays(void);
extern void battle_quick_update(void);
extern void battle_send_party_update(void);
extern void battle_free_entity(u32 entity_id);
extern void battle_clear_state(void);
extern int battle_find_rideable_pet(void);
extern void battle_draw_text(u32 x, u32 y, u32 a, u32 b, const char* text, u32 c);
extern void sound_stop_bgm(void);
extern void network_send_pvp_battle_end(void);
extern void network_send_battle_end(void);
extern void network_send_pvp_ride_request(u32 pet_index);
extern void network_send_ride_request(u32 pet_index);
extern void network_send_pvp_special_command(void);
extern void network_send_special_command(void);
extern void render_process_queue(void);
extern u32 ui_window_create(u32 x, u32 y, u32 w, u32 h, u32 flags, u32 visible);

/*
 * Battle State 0 - Initialize - FUN_0040a1a0 case 0
 * Clears NPC data, initializes battle systems, loads field, plays BGM
 */
void battle_state_init(u32* state, u32* action_result) {
    u32 i;
    u32 bgm_id;

    /* Clear action result */
    *action_result = 0;

    /* Initialize UI state - FUN_00418330 pattern */
    g_battle.ui_flag1 = 0;
    g_battle.ui_flag2 = 0;
    g_battle.ui_flag3 = 0;
    g_battle.ui_flag4 = 0;
    g_battle.action_state = 0;
    g_battle.battle_coord_x = 0;
    g_battle.battle_coord_y = 0;
    g_battle.state_flag = 0;

    /* Clear entity references */
    g_battle.entity_ref1 = 0;
    g_battle.entity_ref2 = 0;
    g_battle.entity_ref3 = 0;

    /* Clear battle systems */
    battle_clear_entity_queue();
    battle_reset_ai_state();
    battle_clear_unit_flags();
    battle_clear_render_queue();

    /* Check for special map IDs - dungeon maps */
    if (battle_is_special_map(g_battle.map_id)) {
        /* Initialize dungeon-specific entities */
        for (i = 0; i < 9; i++) {
            if (g_battle.dungeon_entities[i] == 0) {
                /* Create dungeon entity - FUN_004472e0 pattern */
                u32 entity = battle_create_dungeon_entity(i);
                g_battle.dungeon_entities[i] = entity;
            }
        }
    }

    /* Clear counters */
    g_battle.message_index = 0;
    g_battle.action_queue_pos = 0;

    /* Initialize battle systems */
    battle_init_render_queue();
    battle_load_field(g_battle.field_id);

    /* Initialize UI */
    battle_ui_init();
    battle_cursor_init();
    battle_menu_init();

    /* Clear render flag */
    g_battle.render_phase = 0;

    /* Process initial render queue */
    battle_render_queue_process();

    /* Set render phase active */
    g_battle.render_phase = 1;

    /* Clear counters again */
    g_battle.message_index = 0;
    g_battle.action_queue_pos = 0;

    /* Determine BGM based on map type and time */
    bgm_id = battle_get_bgm_for_map(g_battle.map_id);
    if (bgm_id != 0) {
        sound_play_bgm(bgm_id);
    }

    /* Record start time */
    g_battle.animation_timer = timeGetTime();
    g_battle.intro_phase = 2;

    /* Start fade in effect - FUN_0047bde0(0x11) */
    battle_start_fade(0x11);

    /* Initialize cursors and field */
    battle_cursor_init();
    battle_field_render();

    /* Set battle active flag */
    g_battle.active = 1;

    /* Clear entity queue positions */
    for (i = 0; i < 20; i++) {
        if (g_battle.units[i].hp == 0) {
            /* Clear selection if entity was removed */
            if (i == g_battle.selected_index) {
                g_battle.selected_index = -1;
            }
            if (i == g_battle.target_index) {
                g_battle.target_index = -1;
            }
            g_battle.entity_queue[i] = 0;
        }
    }

    /* Initialize battle menu */
    battle_menu_init();

    /* Advance to state 1 */
    (*state)++;
}

/*
 * Battle State 2 - Main Loop - FUN_0040a1a0 case 2
 * Handles action selection, network messages, and turn processing
 */
void battle_state_main(u32* state, u32* action_result) {
    u32 i;
    char* message_ptr;

    /* Update battle UI */
    battle_ui_init();
    battle_cursor_init();
    battle_menu_update();
    battle_field_render();

    /* Handle copy message flag */
    if (g_battle.copy_message_flag) {
        /* Copy message to display buffer */
        strncpy(g_battle.display_message, g_battle.source_message,
                sizeof(g_battle.display_message) - 1);
    }

    /* Check for network action if not processing */
    if (g_battle.action_state == 0) {
        /* Update frame counter */
        g_battle.frame_counter = (g_battle.frame_counter + 1) & 0x3F;

        /* Process network messages */
        battle_process_network();

        /* Handle action results */
        if (g_battle.action_result_count > 0) {
            /* Process action result from queue */
            message_ptr = g_battle.action_results[g_battle.action_result_read];

            /* Copy result message */
            strncpy(g_battle.current_message, message_ptr,
                    sizeof(g_battle.current_message) - 1);

            g_battle.action_result_read = (g_battle.action_result_read + 1) & 3;
            g_battle.action_state = 0;

            /* Copy message to display */
            strncpy(g_battle.display_message, g_battle.current_message,
                    sizeof(g_battle.display_message) - 1);

            /* Handle unit action response */
            if (g_battle.current_unit_index < 20) {
                battle_handle_unit_action_response(g_battle.current_unit_index);
            }

            /* Get action type */
            g_battle.action_type = battle_get_action_type();

            /* Advance to action state */
            (*state)++;
        }
    }

    /* Show waiting dialog if all units ready */
    if (g_battle.current_unit_index >= 20 && g_battle.wait_dialog_id == 0) {
        /* Create wait dialog */
        g_battle.wait_dialog_id = ui_window_create(0x1bc, 4, 3, 2, 0, 1);
        /* Play sound */
        sound_play_effect(0xca, 320, 240);
    }
}

/*
 * Battle State 3 - Action Processing - FUN_0040a1a0 case 3
 * Executes selected actions
 */
void battle_state_action(u32* state, u32* action_result) {
    /* Update frame counter */
    g_battle.frame_counter = (g_battle.frame_counter + 1) & 0x3F;

    /* Execute pending action */
    battle_execute_pending_action();

    /* Update battle state */
    battle_update_state();

    /* Process network */
    battle_process_network();

    /* Handle action results */
    battle_handle_action_results();

    /* Update UI */
    battle_ui_init();
    battle_cursor_init();
    battle_menu_update();
    battle_field_render();

    /* Check for action completion */
    if (*action_result == 3) {
        /* Action complete, check for battle end */
        *action_result = 0;
        g_battle.copy_message_flag = 0;

        if (battle_check_end_condition()) {
            /* Battle ended */
            *state = 7;
        } else {
            /* Continue to next phase */
            (*state)++;
        }
    }
}

/*
 * Battle State 4 - Execute Action Animation - FUN_0040a1a0 case 4
 * Shows action animations and effects
 */
void battle_state_execute(u32* state) {
    /* Update frame counter */
    g_battle.frame_counter = (g_battle.frame_counter + 1) & 0x3F;

    /* Check AI mode for different execution */
    if (g_battle.ai_mode == 3) {
        /* AI-controlled execution */
        battle_ai_execute_action();
        battle_ai_update();
    } else {
        /* Player-controlled execution */
        battle_player_execute_action();
    }

    /* Update battle state */
    battle_update_state();
    battle_process_network();

    /* Update UI */
    battle_ui_init();
    battle_cursor_init();
    battle_menu_update();
    battle_field_render();
}

/*
 * Battle State 5 - Result Processing - FUN_0040a1a0 case 5
 * Shows battle results and transitions to next turn
 */
void battle_state_result(u32* state, u32* action_result) {
    u32 i;
    char* message_ptr;

    /* Update frame counter */
    g_battle.frame_counter = (g_battle.frame_counter + 1) & 0x3F;

    /* Update state */
    battle_update_state();
    battle_process_network();

    /* Update UI */
    battle_ui_init();
    battle_cursor_init();
    battle_menu_update();
    battle_field_render();

    /* Check for queued result message */
    if (g_battle.result_queue_count > 0) {
        /* Get message from queue */
        message_ptr = g_battle.result_queue[g_battle.result_queue_read];

        /* Copy to current message */
        strncpy(g_battle.current_message, message_ptr,
                sizeof(g_battle.current_message) - 1);

        /* Clear action result */
        *action_result = 0;
        g_battle.wait_dialog_id = 0;

        g_battle.result_queue_read = (g_battle.result_queue_read + 1) & 3;

        /* Advance to message state */
        (*state)++;
    }

    /* Handle action type display */
    if (g_battle.action_type == 2) {
        /* Copy defeat message */
        strncpy(g_battle.display_message, g_battle.defeat_message,
                sizeof(g_battle.display_message) - 1);
    }

    /* Update unit displays */
    if (g_battle.current_unit_index < 20) {
        /* Update specific units */
        if (g_battle.current_unit_index < 10) {
            /* Player side */
            for (i = 0; i < 10; i++) {
                battle_update_unit_display(i);
            }
        } else {
            /* Enemy side */
            for (i = 10; i < 20; i++) {
                battle_update_unit_display(i);
            }
        }
    } else {
        /* Update all units */
        for (i = 0; i < 20; i++) {
            battle_update_unit_display(i);
        }
    }
}

/*
 * Battle State 6 - Message Display - FUN_0040a1a0 case 6
 * Shows messages and waits for player input
 */
void battle_state_message(u32* state, u32* action_result) {
    /* Display message if set */
    if (g_battle.display_message[0] != '\0') {
        /* Calculate message display position */
        u32 x = (strlen(g_battle.display_message) - 1) * (-7) + 0x27c;
        battle_draw_text(x, 0x1b8, 1, 4, g_battle.display_message, 0);
    }

    /* Check action result for state transition */
    if (*action_result == 0) {
        /* Update frame counter */
        g_battle.frame_counter = (g_battle.frame_counter + 1) & 0x3F;

        /* Process normal update */
        battle_process_network();
        battle_update_unit_displays();
    } else {
        /* Slower update for waiting state */
        if ((g_battle.frame_counter & 0x1F) == 0) {
            g_battle.frame_counter = (g_battle.frame_counter + 1) & 0x3F;
        }

        /* Only update every 4 frames */
        if ((g_battle.frame_counter & 3) == 0) {
            battle_process_network();
            battle_update_unit_displays();
        } else {
            /* Quick update */
            battle_quick_update();
        }
    }

    /* Check for party update */
    if (g_battle.party_update_pending) {
        battle_send_party_update();
    }

    /* Handle action result transitions */
    if (*action_result == 1) {
        /* Return to main state */
        *state = 2;
        g_battle.turn_count++;
        *action_result = 0;
        g_battle.current_message[0] = 0;
        g_battle.active = 1;
    } else if (*action_result == 2) {
        /* Go to end state */
        *state = 7;
        *action_result = 0;
        g_battle.current_message[0] = 0;
    }
}

/*
 * Battle State 7 - End Start - FUN_0040a1a0 case 7
 * Begins battle end sequence
 */
void battle_state_end_start(u32* state) {
    u32 i;

    /* Stop BGM */
    sound_stop_bgm();

    /* Start fade out effect */
    battle_start_fade(0x0C);

    /* Set fade state */
    g_battle.intro_phase = 2;
    g_battle.fade_active = 0;
    g_battle.ended = 0;

    /* Clear dungeon entities */
    for (i = 0; i < 9; i++) {
        if (g_battle.dungeon_entities[i] != 0) {
            battle_free_entity(g_battle.dungeon_entities[i]);
            g_battle.dungeon_entities[i] = 0;
        }
    }

    /* Advance to final state */
    (*state)++;
}

/*
 * Battle State 8 - End Complete - FUN_0040a1a0 case 8
 * Final cleanup and return to field
 */
void battle_state_end_complete(u32* state) {
    int fade_result;

    /* Check if fade is complete */
    fade_result = battle_fade_check();
    if (fade_result == 1) {
        /* Return to field state */
        gamestate_set_state(GAME_STATE_PLAYING, 1);

        /* Clear battle state */
        battle_clear_state();
        g_battle.active = 0;

        /* Clear player destination */
        g_battle.player_dest = 0;

        /* Send battle end to server */
        if (g_battle.is_pvp) {
            /* PvP battle end */
            network_send_pvp_battle_end();
        } else {
            /* Normal battle end */
            network_send_battle_end();
        }

        /* Handle after-battle positioning */
        if (g_battle.after_battle_delay > 0) {
            g_battle.after_battle_state = (int)g_battle.after_battle_delay;
            g_battle.after_battle_delay = 0;
        }

        /* Return if no special handling */
        if (g_battle.return_flag == 0) {
            return;
        }

        /* Check for pet ride after battle */
        if (g_battle.ai_mode == 3 && g_battle.return_flag != 0) {
            /* Find active pet and send ride request */
            int pet_index = battle_find_rideable_pet();
            if (pet_index >= 0) {
                if (g_battle.is_pvp) {
                    network_send_pvp_ride_request(pet_index + 9);
                } else {
                    network_send_ride_request(pet_index + 9);
                }
            }
        }
    }
}

/*
 * Check if fade effect is complete
 */
int battle_fade_check(void) {
    /* Check if fade is active - FUN_0047bde0 pattern */
    return g_battle.fade_complete ? 1 : 0;
}

/*
 * Process render queue
 */
void battle_render_queue_process(void) {
    /* FUN_0047dc60 - render queue processor */
    render_process_queue();
}

/*
 * Initialize battle UI
 */
void battle_ui_init(void) {
    /* FUN_00410850 - battle UI init */
}

/*
 * Initialize battle cursor
 */
void battle_cursor_init(void) {
    /* FUN_004117e0 - cursor init */
    /* FUN_00410b00 - cursor update */
}

/*
 * Render battle field
 */
void battle_field_render(void) {
    /* FUN_0044b0b0 - unit render */
    /* FUN_0041f1e0 - effect render */
    /* FUN_004199c0 - field render */
}

/*
 * Check special battle conditions
 */
void battle_check_special_conditions(u32* state) {
    /* Handle escape request during battle */
    if (g_battle.intro_phase != 2 && g_battle.escape_request) {
        *state = 7;
        g_battle.escape_request = 0;
    }

    /* Handle special dialog during battle */
    if ((g_battle.special_flags & 2) != 0 && g_battle.current_unit_index >= 20) {
        if (g_battle.is_pvp) {
            network_send_pvp_special_command();
        } else {
            network_send_special_command();
        }
    }

    /* Handle wait dialog buttons */
    if (g_battle.wait_dialog_id != 0 && g_battle.wait_dialog_button_count > 0) {
        /* Draw dialog buttons */
        battle_draw_dialog_buttons(g_battle.wait_dialog_id);
    }
}

/*
 * Draw dialog buttons
 */
void battle_draw_dialog_buttons(u32 dialog_id) {
    /* Stub implementation */
    (void)dialog_id;
}

/*
 * Update battle menu
 */
void battle_menu_update(void) {
    /* Stub implementation */
}

/*
 * Initialize turn order for battle
 */
void battle_init_turn_order(void) {
    u32 i, j;
    u32 temp_idx;
    u16 temp_speed;
    u16 speed_i, speed_j;

    /* Sort units by speed (descending) */
    /* turn_order stores unit indices; look up speed from units array */
    for (i = 0; i < g_battle.unit_count - 1; i++) {
        for (j = i + 1; j < g_battle.unit_count; j++) {
            speed_i = g_battle.units[g_battle.turn_order[i]].speed;
            speed_j = g_battle.units[g_battle.turn_order[j]].speed;

            if (speed_j > speed_i) {
                /* Swap indices */
                temp_idx = g_battle.turn_order[i];
                g_battle.turn_order[i] = g_battle.turn_order[j];
                g_battle.turn_order[j] = temp_idx;
            }
        }
    }

    g_battle.current_turn = 0;
}

/*
 * Reset all battle animations
 */
void battle_reset_animations(void) {
    u32 i;

    g_battle.animation_timer = 0;
    g_battle.animation_id = 0;

    for (i = 0; i < 20; i++) {
        g_battle.units[i].animation = 0;
        g_battle.units[i].frame = 0;
        g_battle.units[i].effect_timer = 0;
    }
}

/*
 * Show battle message
 */
void battle_show_message(const char* msg, int type) {
    if (msg == NULL) return;

    strncpy(g_battle.display_message, msg, sizeof(g_battle.display_message) - 1);
    g_battle.display_message[sizeof(g_battle.display_message) - 1] = '\0';
    g_battle.message_type = type;
}

/*
 * Show result animation
 */
void battle_show_result_animation(u32 p2, u32 p3, u32 p4, int result) {
    /* Stub - would show victory/defeat animation */
    (void)p2; (void)p3; (void)p4; (void)result;
}

/*
 * Show damage numbers
 */
void battle_show_damage(u32 attacker, u32 target, u16 damage, u16 flags, u32 skill_id) {
    /* Stub - would display floating damage numbers */
    (void)attacker; (void)target; (void)damage; (void)flags; (void)skill_id;
}

/*
 * Clear capture UI elements
 */
void battle_clear_capture_ui(void) {
    g_battle.capture_result[0] = 0;
    g_battle.capture_result[1] = 0;
    g_battle.capture_pet_id = 0;
    g_battle.capture_timer = 0;
}
