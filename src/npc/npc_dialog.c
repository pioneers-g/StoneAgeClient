/*
 * Stone Age Client - NPC Dialog System
 * Split from npc.c for code organization
 *
 * Handles dialog action processing and dialogue display
 *
 * TODO: Verify all action type values from FUN_004781f0 switch cases
 * TODO: Verify entity structure offsets (0x14, 0x1c, 0x20, 0x24, 0xa8-0x150)
 * TODO: Verify multi-spawn implementation from FUN_004781f0 case 0x3c
 * TODO: Verify special action handler FUN_00478190
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "npc.h"
#include "npc_dialog.h"
#include "logger.h"

/* External NPC context */
extern NPCContext g_npc;

/* Forward declarations for internal functions */
static void npc_set_dialog_mode(int mode);
static void npc_set_dialog_text(const char* text);
static void npc_set_position(int x, int y);

/* External functions from npc.c */
extern void npc_hide_talk_window(void);
extern int npc_show_talk_window(HINSTANCE hInstance);
extern void npc_set_dialog_position(int x, int y);

/*
 * Set dialog mode
 */
static void npc_set_dialog_mode(int mode) {
    g_npc.dialog.mode = mode;
}

/*
 * Set dialog text
 */
static void npc_set_dialog_text(const char* text) {
    if (text && g_npc.dialog.text) {
        strncpy(g_npc.dialog.text, text, MAX_DIALOGUE_TEXT - 1);
        g_npc.dialog.text[MAX_DIALOGUE_TEXT - 1] = '\0';
    }
}

/*
 * Set position
 */
static void npc_set_position(int x, int y) {
    g_npc.dialog.pos_x = x;
    g_npc.dialog.pos_y = y;
    npc_set_dialog_position(x, y);
}

/*
 * Process dialog action - FUN_004781f0 pattern
 * Handles different dialog types based on action code
 */
void npc_process_dialog_action(DialogContext* ctx, DialogAction action) {
    void** effect_ptr;
    void* new_entity;

    if (!ctx) {
        return;
    }

    switch (action) {
        case DIALOG_ACTION_CLOSE:
            /* Case 0: FUN_00477d90(3) - Close dialog */
            npc_set_dialog_mode(3);
            npc_hide_talk_window();
            break;

        case DIALOG_ACTION_OPEN:
            /* Case 1: FUN_00477d90(4) - Open dialog */
            npc_set_dialog_mode(4);
            npc_show_talk_window(GetModuleHandleA(NULL));
            break;

        case DIALOG_ACTION_SET_TEXT:
            /* Case 2: FUN_00477d70 + FUN_00477d90(0) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(0);
            break;

        case DIALOG_ACTION_SET_MENU:
            /* Case 3: FUN_00477d70 + FUN_00477d90(0xc) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(12);
            g_npc.dialog.has_options = 1;
            break;

        case DIALOG_ACTION_SET_OPTIONS:
            /* Case 4: FUN_00477d70 + FUN_00477d90(1) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(1);
            g_npc.dialog.has_options = 1;
            break;

        case DIALOG_ACTION_SET_VAR:
            /* Case 5: FUN_00477d70 + FUN_00477d90(2) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(2);
            break;

        case DIALOG_ACTION_SHOW_ITEM:
            /* Case 10 (0xa): FUN_00477d70 + FUN_00477d90(2) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(2);
            break;

        case DIALOG_ACTION_MENU_BUY:
            /* Case 0xb: FUN_00477d70 + FUN_00477d90(5) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(5);
            g_npc.shop_mode = 1;
            break;

        case DIALOG_ACTION_MENU_SELL:
            /* Case 0xc: FUN_00477d70 + FUN_00477d90(6) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(6);
            g_npc.shop_mode = 2;
            break;

        case DIALOG_ACTION_MENU_TRADE:
            /* Case 0xd: FUN_00477d70 + FUN_00477d90(7) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(7);
            g_npc.shop_mode = 3;
            break;

        case DIALOG_ACTION_MENU_STORAGE:
            /* Case 0xe: FUN_00477d70 + FUN_00477d90(8) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(8);
            g_npc.shop_mode = 4;
            break;

        case DIALOG_ACTION_MENU_HEAL:
            /* Case 0xf: FUN_00477d70 + FUN_00477d90(9) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(9);
            g_npc.shop_mode = 5;
            break;

        case DIALOG_ACTION_MENU_TELEPORT:
            /* Case 0x10: FUN_00477d70 + FUN_00477d90(10) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(10);
            g_npc.shop_mode = 6;
            break;

        case DIALOG_ACTION_MENU_QUEST:
            /* Case 0x11: FUN_00477d70 + FUN_00477d90(4) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(4);
            g_npc.in_quest = 1;
            break;

        case DIALOG_ACTION_SHOW_STATUS:
            /* Case 0x12: FUN_00477d70 + FUN_00477d90(0xb) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(11);
            break;

        case DIALOG_ACTION_SHOW_QUEST:
            /* Case 0x13: FUN_00477d70 + FUN_00477d90(3) */
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(3);
            break;

        case DIALOG_ACTION_SET_POSITION:
            /* Case 0x14: FUN_00477cb0 + FUN_00477d70 - Set position + text */
            npc_set_position((int)ctx->param1, (int)ctx->param2);
            npc_set_dialog_text(ctx->text);
            break;

        case DIALOG_ACTION_CONFIRM:
            /* Case 0x15: Position + text + conditional show/hide */
            npc_set_position((int)ctx->param1, (int)ctx->param2);
            npc_set_dialog_text(ctx->text);
            if (ctx->param3 == 1) {
                g_npc.dialog.show_confirm = 1;
            } else {
                g_npc.dialog.show_confirm = 0;
            }
            break;

        case DIALOG_ACTION_SHOW_GUILD:
            /* Case 0x16: Position + text + conditional guild display */
            npc_set_position((int)ctx->param1, (int)ctx->param2);
            npc_set_dialog_text(ctx->text);
            if (ctx->param3 == 1) {
                g_npc.dialog.show_guild = 1;
            } else {
                g_npc.dialog.show_guild = 0;
            }
            break;

        case DIALOG_ACTION_SPECIAL_PARAM:
            /* Case 0x17: FUN_00478190(param_5) - Special param action */
            npc_handle_special_action(ctx->param3);
            break;

        case DIALOG_ACTION_HIDE:
            /* Case 0x1e: FUN_00477d70 - Just set text */
            npc_set_dialog_text(ctx->text);
            break;

        case DIALOG_ACTION_SHOW_PARTY:
            /* Case 0x1f: Position + text + mode 3 */
            npc_set_position((int)ctx->param1, (int)ctx->param2);
            npc_set_dialog_text(ctx->text);
            npc_set_dialog_mode(3);
            break;

        case DIALOG_ACTION_QUEST_COMPLETE:
            /* Case 0x22: Position + text + conditional quest completion */
            npc_set_position((int)ctx->param1, (int)ctx->param2);
            npc_set_dialog_text(ctx->text);
            if (ctx->param3 == 1) {
                g_npc.in_quest = 0;
                g_npc.current_quest = 0;
            }
            break;

        case DIALOG_ACTION_ENTITY_EFFECT:
            /* Case 0x29: Entity effect at offset 0x14 */
            if (g_npc.current_entity) {
                void** effect = (void**)((char*)g_npc.current_entity + 0x14);
                if (*effect) {
                    free(*effect);
                }
                new_entity = calloc(1, 0x154);
                *effect = new_entity;
            }
            break;

        case DIALOG_ACTION_CLEAR_ENTITY:
            /* Case 0x2a: Clear entity effect at offset 0x14 */
            if (g_npc.current_entity) {
                void** effect = (void**)((char*)g_npc.current_entity + 0x14);
                if (*effect) {
                    free(*effect);
                    *effect = NULL;
                }
            }
            break;

        case DIALOG_ACTION_CREATE_ENTITY:
            /* Case 0x2b: Create entity with name storage */
            if (g_npc.current_entity) {
                /* Entity is already created in npc_handle_dialog_action */
                /* This just marks that the entity has been named */
                LOG_DEBUG("Entity created for NPC %s", ctx->npc_name);
            }
            break;

        case DIALOG_ACTION_CLEAR_EFFECT:
            /* Case 0x33: Clear effects at 0x1c/0x20, create new entity */
            if (g_npc.current_entity) {
                effect_ptr = (void**)((char*)g_npc.current_entity + 0x1c);
                if (*effect_ptr) {
                    free(*effect_ptr);
                    *effect_ptr = NULL;
                }
                effect_ptr = (void**)((char*)g_npc.current_entity + 0x20);
                if (*effect_ptr) {
                    free(*effect_ptr);
                    *effect_ptr = NULL;
                }
                new_entity = calloc(1, 0x154);
                effect_ptr = (void**)((char*)g_npc.current_entity + 0x1c);
                *effect_ptr = new_entity;

                if (new_entity) {
                    *(u32*)((char*)new_entity + 0xac) = ctx->param2;
                    *(s32*)((char*)new_entity + 0x140) = ctx->param1;
                    *(u32*)((char*)new_entity + 0x148) = 3;
                    *(u32*)((char*)new_entity + 0x150) = 1;
                    *(u8*)((char*)new_entity + 0x15) = 0x14;

                    if (ctx->param2 == 3) {
                        *(s32*)((char*)new_entity + 0xb0) = g_npc.base_x + 3;
                        *(s32*)((char*)new_entity + 0xb4) = g_npc.base_y - 3;
                    } else {
                        *(s32*)((char*)new_entity + 0xb0) = g_npc.base_x;
                        *(s32*)((char*)new_entity + 0xb4) = g_npc.base_y;
                    }
                    *(u32*)((char*)new_entity + 0xa8) = 0;
                }
            }
            break;

        case DIALOG_ACTION_MULTI_SPAWN:
            /* Case 0x3c: Multi-element spawn */
            npc_handle_multi_spawn(ctx);
            break;

        default:
            LOG_DEBUG("Unknown dialog action: %d", action);
            break;
    }
}

/*
 * Handle special action - FUN_00478190 pattern
 */
void npc_handle_special_action(int param) {
    switch (param) {
        case 0:
            /* Default action */
            break;
        case 1:
            /* Show special dialog */
            break;
        case 2:
            /* Hide elements */
            break;
        default:
            break;
    }
}

/*
 * Handle multi-spawn - FUN_004781f0 case 0x3c pattern
 */
void npc_handle_multi_spawn(DialogContext* ctx) {
    void* entity;
    void** entity_slots;
    int i;

    if (!g_npc.current_entity) {
        return;
    }

    entity_slots = (void**)((char*)g_npc.current_entity + 0x24);

    /* Clear existing entities (4 slots) */
    for (i = 0; i < 4; i++) {
        if (entity_slots[i]) {
            free(entity_slots[i]);
            entity_slots[i] = NULL;
        }
    }

    /* Spawn new entities based on params */
    for (i = 0; i < 4; i++) {
        entity = calloc(1, 0x154);
        if (entity) {
            entity_slots[i] = entity;

            /* Set position offset from base */
            *(s32*)((char*)entity + 0xb0) = g_npc.base_x + (i % 2) * 32;
            *(s32*)((char*)entity + 0xb4) = g_npc.base_y + (i / 2) * 32;
            *(u32*)((char*)entity + 0xa8) = 0;
        }
    }

    LOG_DEBUG("Multi-spawn: created %d entities", 4);
}

/*
 * Create dialog entity
 */
void npc_create_dialog_entity(u32 sprite_id, int x, int y, u16 action) {
    DialogEntity* entity;

    if (g_npc.entity_count >= MAX_DIALOG_ENTITIES) {
        return;
    }

    entity = &g_npc.entities[g_npc.entity_count];
    entity->sprite_id = sprite_id;
    entity->x = x;
    entity->y = y;
    entity->action_type = action;
    entity->anim_frame = 0;
    entity->visible = 1;

    g_npc.entity_count++;
}

/*
 * Spawn dialog effect
 */
void npc_spawn_dialog_effect(u32 effect_id, int x, int y, u8 effect_type) {
    /* Would spawn visual effect at position */
    LOG_DEBUG("Effect %u at (%d,%d) type %d", effect_id, x, y, effect_type);
}

/*
 * Clear dialog entities
 */
void npc_clear_dialog_entities(void) {
    g_npc.entity_count = 0;
    memset(g_npc.entities, 0, sizeof(g_npc.entities));
}

/*
 * Show dialog
 */
void npc_show_dialog(u32 npc_id, const char* text) {
    g_npc.current_npc = npc_id;
    g_npc.in_dialog = 1;

    if (text && g_npc.dialog.text) {
        strncpy(g_npc.dialog.text, text, MAX_DIALOGUE_TEXT - 1);
        g_npc.dialog.text[MAX_DIALOGUE_TEXT - 1] = '\0';
    }

    npc_show_talk_window(GetModuleHandleA(NULL));
}
