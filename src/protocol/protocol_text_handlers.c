/*
 * Stone Age Client - Text Protocol Handlers
 * Split from protocol_text.c for code organization
 *
 * Contains handler functions for text protocol commands
 * Each handler corresponds to a server command (EV, EN, DU, etc.)
 */

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "protocol.h"
#include "protocol_text.h"
#include "gamestate.h"
#include "logger.h"

/* External game module functions */
extern void gamestate_set_state(int state, int param);
extern void login_handle_text_result(const char* password);
extern void login_handle_create_text_result(const char* name, const char* data);
extern void login_handle_delete_text_result(const char* name, const char* data);
extern void login_handle_char_login_result(const char* name, const char* data);
extern void login_handle_char_list_text(const char* data1, const char* data2);
extern void battle_handle_start_text(int p1, int p2, int p3, int p4);
extern void battle_handle_turn_text(int p1);
extern void battle_handle_action_text(int p1, int p2, int p3, int p4, int p5, const char* str);
extern void battle_handle_result_text(int p1, int p2, const char* str);
extern void battle_handle_end_text(int p1, int p2, int p3, int p4);
extern void chat_handle_system_message(const char* str);
extern void chat_handle_message(const char* str);
extern void trade_handle_request(const char* str);
extern void friend_handle_message(const char* str);
extern void party_handle_update(int p1);
extern void skill_handle_effect_text(int p1, int p2, int p3, int p4);
extern void npc_handle_dialog_text(const char* str);
extern void npc_handle_shop_text(const char* str);
extern void map_handle_enter_field(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, const char* str);
extern void character_handle_spawn_text(int p1, int p2);
extern void character_handle_remove_text(int p1);
extern void character_handle_update_text(int p1, int p2);
extern void pet_handle_spawn_text(int p1, int p2);
extern void pet_handle_action_text(int p1, int p2);

/* ========================================
 * Short Command Handlers (2-3 character)
 * ======================================== */

/*
 * XYD handler - FUN_00465400
 * Map/coordinate with 3 int
 */
void handler_xyd(int p1, int p2, int p3) {
    LOG_DEBUG("XYD: %d %d %d", p1, p2, p3);
}

/*
 * EV handler - FUN_00463e70
 * Event with 2 int
 */
void handler_ev(int p1, int p2) {
    LOG_DEBUG("EV: %d %d", p1, p2);
    character_handle_spawn_text(p1, p2);
}

/*
 * EN handler - FUN_00464e10
 * Entity/name with 2 int
 */
void handler_en(int p1, int p2) {
    LOG_DEBUG("EN: %d %d", p1, p2);
    character_handle_update_text(p1, p2);
}

/*
 * DU handler - FUN_004643f0
 * Dialog/update with 1 string
 */
void handler_du(const char* str) {
    LOG_DEBUG("DU: %s", str);
    npc_handle_dialog_text(str);
}

/*
 * EO handler - FUN_00464610
 * Entity/object with 1 string
 */
void handler_eo(const char* str) {
    LOG_DEBUG("EO: %s", str);
}

/*
 * EB handler - FUN_00464ef0
 * Chat/message with 1 string
 */
void handler_eb(const char* str) {
    LOG_DEBUG("EB: %s", str);
    chat_handle_message(str);
}

/*
 * EC handler - FUN_00464670
 * Entity/command with 1 string
 */
void handler_ec(const char* str) {
    LOG_DEBUG("EC: %s", str);
}

/*
 * ED handler - FUN_00464650
 * Entity/delete with 2 int
 */
void handler_ed(int p1, int p2) {
    LOG_DEBUG("ED: %d %d", p1, p2);
    character_handle_remove_text(p1);
}

/*
 * EE handler - FUN_00465170
 * Entity/effect with 2 int, 1 string, 1 int
 */
void handler_ee(int p1, const char* str, int p3) {
    LOG_DEBUG("EE: %d %s %d", p1, str, p3);
}

/*
 * EF handler - FUN_00464af0
 * Enter field with 8 int, 1 string
 */
void handler_ef(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, const char* str) {
    LOG_DEBUG("EF: %d %d %d %d %d %d %d %d %s", p1, p2, p3, p4, p5, p6, p7, p8, str);
    map_handle_enter_field(p1, p2, p3, p4, p5, p6, p7, p8, str);
}

/*
 * EG handler - FUN_00463f00
 * Entity/group with 1 string
 */
void handler_eg(const char* str) {
    LOG_DEBUG("EG: %s", str);
}

/*
 * EH handler - FUN_00464190
 * Entity/health with 1 int
 */
void handler_eh(int p1) {
    LOG_DEBUG("EH: %d", p1);
}

/*
 * EI handler - FUN_00463790
 * Entity/item with 2 int
 */
void handler_ei(int p1, int p2) {
    LOG_DEBUG("EI: %d %d", p1, p2);
}

/*
 * EJ handler - FUN_00462010
 * Entity/join with 1 int
 */
void handler_ej(int p1) {
    LOG_DEBUG("EJ: %d", p1);
    pet_handle_spawn_text(p1, 0);
}

/*
 * EK handler - FUN_00462200
 * Entity/kick with 1 int
 */
void handler_ek(int p1) {
    LOG_DEBUG("EK: %d", p1);
}

/*
 * EL handler - FUN_00462590
 * Entity/leave with 2 int
 */
void handler_el(int p1, int p2) {
    LOG_DEBUG("EL: %d %d", p1, p2);
}

/*
 * EM handler - FUN_00462f60
 * Entity/message with 1 string
 */
void handler_em(const char* str) {
    LOG_DEBUG("EM: %s", str);
    npc_handle_dialog_text(str);
}

/*
 * EN2 handler - FUN_00463380
 * Entity/shop with 1 string
 */
void handler_en2(const char* str) {
    LOG_DEBUG("EN2: %s", str);
    npc_handle_shop_text(str);
}

/*
 * EO2 handler - FUN_00465460
 * Generic handler with 1 string
 */
void handler_eo2(const char* str) {
    LOG_DEBUG("EO2: %s", str);
}

/*
 * EP handler - FUN_0045ffb0
 * Binary packet wrapper with 1 string
 */
void handler_ep(const char* str) {
    LOG_DEBUG("EP: binary packet");
}

/*
 * EQ handler - FUN_00465460
 * Equipment with 3 int, 1 string
 */
void handler_eq(int p1, int p2, int p3, const char* str) {
    LOG_DEBUG("EQ: %d %d %d %s", p1, p2, p3, str);
}

/*
 * ER handler - FUN_00463ee0
 * Entity/result with 1 int
 */
void handler_er(int p1) {
    LOG_DEBUG("ER: %d", p1);
}

/*
 * ES handler - FUN_00464ee0
 * Entity/status with 1 int
 */
void handler_es(int p1) {
    LOG_DEBUG("ES: %d", p1);
}

/*
 * ET handler - FUN_00463d80
 * Entity/target with 2 int
 */
void handler_et(int p1, int p2) {
    LOG_DEBUG("ET: %d %d", p1, p2);
}

/*
 * EU handler - FUN_00465060
 * Entity/update with 2 int
 */
void handler_eu(int p1, int p2) {
    LOG_DEBUG("EU: %d %d", p1, p2);
}

/*
 * EV2 handler - FUN_00465390
 * Event with 4 int (battle start)
 */
void handler_ev2(int p1, int p2, int p3, int p4) {
    LOG_DEBUG("EV2: %d %d %d %d", p1, p2, p3, p4);
    battle_handle_start_text(p1, p2, p3, p4);
}

/*
 * EW handler - FUN_00465160
 * Entity/weapon with 1 int (battle turn)
 */
void handler_ew(int p1) {
    LOG_DEBUG("EW: %d", p1);
    battle_handle_turn_text(p1);
}

/*
 * EX handler - FUN_00464ac0
 * Entity/exit with 4 int, 1 string (battle action)
 */
void handler_ex(int p1, int p2, int p3, int p4, const char* str) {
    LOG_DEBUG("EX: %d %d %d %d %s", p1, p2, p3, p4, str);
    battle_handle_action_text(p1, p2, p3, p4, 0, str);
}

/*
 * EY handler - FUN_00464db0
 * Entity/yield with 2 int, 1 string (battle result)
 */
void handler_ey(int p1, int p2, const char* str) {
    LOG_DEBUG("EY: %d %d %s", p1, p2, str);
    battle_handle_result_text(p1, p2, str);
}

/*
 * EZ handler - FUN_004653d0
 * Entity/zone with 4 int (battle end)
 */
void handler_ez(int p1, int p2, int p3, int p4) {
    LOG_DEBUG("EZ: %d %d %d %d", p1, p2, p3, p4);
    battle_handle_end_text(p1, p2, p3, p4);
}

/*
 * Echo handler - FUN_00465460
 * System echo with 1 string
 */
void handler_echo(const char* str) {
    LOG_DEBUG("Echo: %s", str);
    chat_handle_system_message(str);
}

/*
 * TD handler - FUN_0045a9a0
 * Trade/dialog with 1 string
 */
void handler_td(const char* str) {
    LOG_DEBUG("TD: %s", str);
    trade_handle_request(str);
}

/*
 * FM handler - FUN_00465470
 * Friend/message with 1 string
 */
void handler_fm(const char* str) {
    LOG_DEBUG("FM: %s", str);
    friend_handle_message(str);
}

/*
 * NU handler - FUN_00465440
 * Number/user with 1 int
 */
void handler_nu(int p1) {
    LOG_DEBUG("NU: %d", p1);
    party_handle_update(p1);
}

/*
 * SE handler - FUN_004653d0
 * Skill/effect with 4 int
 */
void handler_se(int p1, int p2, int p3, int p4) {
    LOG_DEBUG("SE: %d %d %d %d", p1, p2, p3, p4);
    skill_handle_effect_text(p1, p2, p3, p4);
}

/*
 * EF2 handler - FUN_00464db0
 * Effect with 2 int, 1 string
 */
void handler_ef2(int p1, int p2, const char* str) {
    LOG_DEBUG("EF2: %d %d %s", p1, p2, str);
}

/*
 * R handler - FUN_00465460
 * Result with 1 string
 */
void handler_r(const char* str) {
    LOG_DEBUG("R: %s", str);
}

/* ========================================
 * Long Command Handlers
 * ======================================== */

/*
 * ClientLogin handler - FUN_0045fa40
 */
void handler_client_login(const char* password) {
    LOG_DEBUG("ClientLogin");
    login_handle_text_result(password);
}

/*
 * CreateNewChar handler - FUN_00463c20
 */
void handler_create_new_char(const char* name, const char* data) {
    LOG_DEBUG("CreateNewChar: %s", name);
    login_handle_create_text_result(name, data);
}

/*
 * CharDelete handler - FUN_00463d20
 */
void handler_char_delete(const char* name, const char* data) {
    LOG_DEBUG("CharDelete: %s", name);
    login_handle_delete_text_result(name, data);
}

/*
 * CharLogin handler - FUN_0045fdc0
 */
void handler_char_login(const char* name, const char* data) {
    LOG_DEBUG("CharLogin: %s", name);
    login_handle_char_login_result(name, data);
}

/*
 * CharList handler - FUN_0045fb80
 */
void handler_char_list(const char* data1, const char* data2) {
    LOG_DEBUG("CharList");
    login_handle_char_list_text(data1, data2);
}

/*
 * CharLogout handler - FUN_0045ff50
 */
void handler_char_logout(const char* data1, const char* data2) {
    LOG_DEBUG("CharLogout");
    gamestate_set_state(GAME_STATE_CHAR_SELECT, 0);
    (void)data1;
    (void)data2;
}

/*
 * ProcGet handler - FUN_00465460
 */
void handler_proc_get(const char* data) {
    LOG_DEBUG("ProcGet: %s", data);
}

/*
 * PlayerNumGet handler - FUN_00465460
 */
void handler_player_num_get(int p1, int p2) {
    LOG_DEBUG("PlayerNumGet: %d %d", p1, p2);
}
