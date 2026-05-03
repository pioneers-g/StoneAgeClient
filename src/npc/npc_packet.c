/*
 * Stone Age Client - NPC Packet Handlers
 * Split from npc.c for code organization
 *
 * Contains packet handling functions for NPC system:
 * - Dialog packets
 * - Shop packets
 * - Object spawn packets
 * - Binary protocol handlers
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "npc.h"
#include "npc_shop.h"
#include "npc_dialog.h"
#include "network.h"
#include "logger.h"

/* External NPC context */
extern NPCContext g_npc;

/* External talk window state */
extern TalkWindow g_talk_window;

/* External send function */
extern void send_queue_add(const char* data, int len, int flags);

/* Forward declarations */
void npc_spawn_object(u32 obj_type, u32 obj_id, u16 param);

/* Forward declarations from npc.c */
extern int npc_parse_field(const char* str, char delimiter, int field_index, char* output, int max_len);
extern int npc_parse_field_int(const char* str, char delimiter, int field_index);
extern void npc_send_response(u32 npc_id);
extern void npc_render_dialog(void);
extern void npc_update_shop(void);
/* npc_find_by_id is declared in npc.h */

/*
 * Handle NPC dialog packet - FUN_00462f60 pattern
 */
void npc_handle_dialog_packet(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 npc_id;
    u16 portrait_id;
    u16 dialog_type;
    u16 action_type;
    char text[256];
    char name[32];
    int offset;
    int i;
    NPCData* check_id;

    if (g_npc.debug_mode || g_npc.in_event) {
        return;
    }

    /* Parse NPC ID */
    npc_id = *(u32*)ptr;
    ptr += 4;

    /* Parse portrait/dialog type */
    portrait_id = *(u16*)ptr;
    ptr += 2;

    dialog_type = *(u16*)ptr;
    ptr += 2;

    action_type = *(u16*)ptr;
    ptr += 2;

    /* Read NPC name */
    offset = 0;
    while (*ptr && offset < 31) {
        name[offset++] = *ptr++;
    }
    name[offset] = '\0';
    ptr++;

    /* Handle dialog based on type */
    if (action_type == 0x29) {
        /* Shop dialog */
        strncpy(text, (char*)ptr, 20);
        text[20] = '\0';

        /* Check if this is the current NPC */
        if (g_npc.current_npc == npc_id) {
            if (g_npc.player_data && g_npc.player_data->in_battle) {
                /* In battle - different handling */
                if (g_npc.shop_mode == 1) {
                    /* Initialize shop name */
                    strncpy(g_npc.shop_name, text, sizeof(g_npc.shop_name) - 1);
                    npc_update_shop();
                    npc_render_dialog();
                }
            } else {
                /* Normal NPC interaction */
                npc_send_response(npc_id);
                npc_update_shop();
            }
        } else {
            /* New NPC interaction */
            check_id = npc_find_by_id(npc_id);
            if (check_id == NULL) {
                /* Add to seen NPCs list */
                if (g_npc.seen_count < 4096) {
                    g_npc.seen_npcs[g_npc.seen_count++] = npc_id;
                }
                npc_send_response(npc_id);
            } else {
                /* Handle shop */
                strncpy(g_npc.shop_name, text, sizeof(g_npc.shop_name) - 1);
                npc_render_dialog();
            }
        }
    } else {
        /* Normal dialog */
        npc_render_dialog();
    }
}

/*
 * Handle NPC shop packet - FUN_00463380 pattern
 */
void npc_handle_shop_packet(void* data, u32 size) {
    char* ptr = (char*)data;
    char* token;
    int item_index;
    int i;
    u32 item_id;

    if (g_npc.debug_mode) {
        return;
    }

    item_index = 1;

    /* Parse comma-separated item IDs */
    token = strtok(ptr, ",");
    while (token != NULL) {
        item_id = atoi(token);

        /* Find item in shop */
        npc_find_shop_item(item_id);

        /* Clear item flags */
        for (i = 0; i < MAX_SHOP_ITEMS; i++) {
            if (g_npc.shop_items[i].id == item_id) {
                g_npc.shop_items[i].flags = 0;
                break;
            }
        }

        item_index++;
        token = strtok(NULL, ",");
    }
}

/*
 * Handle object spawn packet - PKT_SV_OBJECT_SPAWN (0x24)
 */
void npc_handle_object_spawn(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 obj_type;
    u32 obj_id;
    u16 param;

    if (size < 10) {
        return;
    }

    obj_type = *(u32*)ptr; ptr += 4;
    obj_id = *(u32*)ptr; ptr += 4;
    param = *(u16*)ptr;

    npc_spawn_object(obj_type, obj_id, param);
}

/*
 * Handle talk result from server
 * Called from PKT_SV_TALK_RESULT (0x29)
 */
void npc_handle_talk_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 npc_id;
    u16 result;
    char dialog_text[512];

    if (size < 6) return;

    npc_id = *(u32*)ptr; ptr += 4;
    result = *(u16*)ptr; ptr += 2;

    /* Extract dialog text if present */
    if (ptr < (u8*)data + size) {
        strncpy(dialog_text, (char*)ptr, sizeof(dialog_text) - 1);
        dialog_text[sizeof(dialog_text) - 1] = '\0';
    } else {
        dialog_text[0] = '\0';
    }

    /* Show dialog */
    if (result == 0 && dialog_text[0]) {
        npc_show_dialog(npc_id, dialog_text);
    }

    LOG_DEBUG("Talk result from NPC %u: %d", npc_id, result);
}

/*
 * Handle dialog from server
 * Wrapper for npc_handle_dialog_packet
 */
void npc_handle_dialog(void* data, u32 size) {
    npc_handle_dialog_packet(data, size);
}

/*
 * Handle shop from server
 * Wrapper for npc_handle_shop_packet
 */
void npc_handle_shop(void* data, u32 size) {
    npc_handle_shop_packet(data, size);
}

/*
 * Spawn object on map - protocol handler
 * Called from PKT_SV_OBJECT_SPAWN (0x24)
 */
void npc_spawn_object(u32 obj_type, u32 obj_id, u16 param) {
    /* Add to NPC list if it's an NPC */
    if (obj_type == 2) {
        /* Find free slot */
        u32 i;
        for (i = 0; i < g_npc.npc_count; i++) {
            if (g_npc.npcs[i].id == 0) {
                g_npc.npcs[i].id = obj_id;
                g_npc.npcs[i].sprite_id = param;
                break;
            }
        }

        if (i >= g_npc.npc_count && g_npc.npc_count < MAX_NPCS - 1) {
            g_npc.npcs[g_npc.npc_count].id = obj_id;
            g_npc.npcs[g_npc.npc_count].sprite_id = param;
            g_npc.npc_count++;
        }
    }

    LOG_DEBUG("Spawned object type %u, id %u", obj_type, obj_id);
}

/*
 * Handle dialog action packet - FUN_00462f60 enhanced pattern
 * Main NPC dialog packet handler that parses server messages
 * and dispatches appropriate dialog actions.
 *
 * Protocol format (pipe-delimited):
 * NPC_ID|Dialog_Type|Param1|Action_Type|Param2|Param3|[Name]|Text...
 *
 * Special handling for type 0x29 (dialog end with name storage)
 */
void npc_handle_dialog_action(void* data, u32 size) {
    char* ptr = (char*)data;
    char buffer[512];
    char name_buf[MAX_NPC_NAME];
    u32 npc_id;
    int dialog_type;
    int action_type;
    int field_idx;
    DialogContext ctx;
    int i;
    int field_count;
    int result;

    /* Skip if in debug mode or event */
    if (g_npc.debug_mode || g_npc.in_event) {
        return;
    }

    memset(&ctx, 0, sizeof(ctx));

    /* Parse NPC ID (field 0) */
    npc_id = npc_parse_field_int(ptr, '|', 0);

    /* Count total fields for iteration */
    field_count = 0;
    for (i = 0; i < 100; i++) {
        if (npc_parse_field(ptr, '|', i, buffer, sizeof(buffer)) < 0) {
            break;
        }
        field_count++;
    }

    /* Main parsing loop - FUN_00462f60 pattern */
    for (field_idx = 1; field_idx < field_count; ) {
        /* Field 1: Dialog type */
        dialog_type = npc_parse_field_int(ptr, '|', field_idx++);

        /* Store current NPC ID for tracking */
        ctx.npc_id = npc_id;

        /* Parse standard fields */
        ctx.param1 = npc_parse_field_int(ptr, '|', field_idx++);
        action_type = npc_parse_field_int(ptr, '|', field_idx++);
        ctx.action_type = action_type;
        ctx.param2 = npc_parse_field_int(ptr, '|', field_idx++);
        ctx.param3 = npc_parse_field_int(ptr, '|', field_idx++);

        /* Check for special type 0x29 - dialog end with name */
        if (action_type == 0x29) {
            /* Store NPC name at offset 0x1d4 */
            npc_parse_field(ptr, '|', field_idx++, name_buf, MAX_NPC_NAME);
            strncpy(ctx.npc_name, name_buf, MAX_NPC_NAME - 1);

            /* Check if this matches current interaction NPC */
            if (g_npc.current_npc == npc_id) {
                /* Check if we have an entity context */
                if (g_npc.current_entity) {
                    /* Clear and set name fields */
                    memset((char*)g_npc.current_entity + 0x1d4, 0, 0x15 + 4*5);
                    strncpy((char*)g_npc.current_entity + 0x1d4, name_buf, MAX_NPC_NAME - 1);

                    /* Trigger dialog spawn */
                    npc_process_dialog_action(&ctx, DIALOG_ACTION_CREATE_ENTITY);
                }
            }
        } else {
            /* Standard action processing */
            /* Parse additional text fields */
            buffer[0] = '\0';
            for (i = 0; i < 4 && field_idx < field_count; i++) {
                char temp[128];
                result = npc_parse_field(ptr, '|', field_idx++, temp, sizeof(temp));
                if (result > 0) {
                    if (buffer[0]) {
                        strcat(buffer, "\n");
                    }
                    strcat(buffer, temp);
                }
            }
            strncpy(ctx.text, buffer, MAX_DIALOGUE_TEXT - 1);

            /* Check if NPC matches current interaction */
            if (g_npc.current_npc == npc_id) {
                /* Process the action */
                npc_process_dialog_action(&ctx, (DialogAction)action_type);
            } else {
                /* Check for seen NPCs */
                int found = 0;
                for (i = 0; i < g_npc.seen_count && i < MAX_SEEN_NPCS; i++) {
                    if (g_npc.seen_npcs[i] == npc_id) {
                        found = 1;
                        break;
                    }
                }

                if (!found && g_npc.seen_count < MAX_SEEN_NPCS) {
                    /* Add to seen list and send acknowledgment */
                    g_npc.seen_npcs[g_npc.seen_count++] = npc_id;
                    npc_send_response(npc_id);
                }
            }
        }
    }

    LOG_DEBUG("Dialog action: NPC=%u, fields=%d, action=%d",
              npc_id, field_count, ctx.action_type);
}

/*
 * Send dialog acknowledgment in binary mode - FUN_0048f900 pattern
 * Used when g_protocol_mode != 0 (binary protocol active)
 */
void npc_send_ack_binary_impl(u32 npc_id) {
    /* Binary mode acknowledgment packet structure:
     * u16 size
     * u16 opcode (PKT_CLIENT_TALK = 0x28)
     * u16 checksum
     * u32 npc_id
     */
    u8 packet[12];
    u16 size = 10;
    u16 opcode = 0x28;  /* PKT_CLIENT_TALK */
    u16 checksum = 0;

    *(u16*)(packet + 0) = size;
    *(u16*)(packet + 2) = opcode;
    *(u16*)(packet + 4) = checksum;
    *(u32*)(packet + 6) = npc_id;

    network_send_raw(packet, size);

    LOG_DEBUG("Sent binary dialog ack for NPC %u", npc_id);
}

/*
 * Show dialog for NPC
 */
void npc_show_dialog(u32 npc_id, const char* text) {
    if (!text || !text[0]) return;

    g_npc.current_npc = npc_id;
    g_npc.in_dialog = 1;

    /* Add to dialogue entries */
    npc_add_dialogue(npc_id, text, 0, 0);

    LOG_DEBUG("Showing dialog for NPC %u", npc_id);
}
