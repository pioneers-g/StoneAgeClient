/*
 * Stone Age Client - NPC Dialog System Header
 * Split from npc.h for code organization
 */

#ifndef NPC_DIALOG_H
#define NPC_DIALOG_H

#include "types.h"
#include "npc.h"

/* Maximum dialog entities */
#define MAX_DIALOG_ENTITIES     4

/* Dialog action processing - FUN_004781f0 */
void npc_process_dialog_action(DialogContext* ctx, DialogAction action);

/* Special action handler - FUN_00478190 */
void npc_handle_special_action(int param);

/* Multi-spawn handler - FUN_004781f0 case 0x3c */
void npc_handle_multi_spawn(DialogContext* ctx);

/* Dialog entity management */
void npc_create_dialog_entity(u32 sprite_id, int x, int y, u16 action);
void npc_spawn_dialog_effect(u32 effect_id, int x, int y, u8 effect_type);
void npc_clear_dialog_entities(void);

/* Dialog display */
void npc_show_dialog(u32 npc_id, const char* text);

#endif /* NPC_DIALOG_H */
