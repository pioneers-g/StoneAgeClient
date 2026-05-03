/*
 * Stone Age Client - Battle Render UI Components
 * Split from battle_render.c
 *
 * Contains complex UI render functions:
 * - Attack UI (action 0)
 * - Skill UI (action 2)
 * - Capture UI (actions 0xc, 0xd)
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "battle_render.h"
#include "battle.h"
#include "render.h"
#include "fade.h"
#include "logger.h"

/* Memory Locations */
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

/* Skill UI state */
#define DAT_0455b118 (*(int*)0x0455b118)
#define _DAT_04552e34 (*(int*)0x04552e34)
#define DAT_04556404 (*(int*)0x04556404)
#define DAT_04552b30 (*(int*)0x04552b30)
#define DAT_004b8408 (*(int*)0x004b8408)
#define DAT_04556478 (*(int*)0x04556478)
#define DAT_0454efcc (*(int*)0x0454efcc)
#define DAT_0454f5b0 (*(int*)0x0454f5b0)

/* Capture state */
#define DAT_0455a0fc (*(int*)0x0455a0fc)
#define DAT_0454f25c (*(int*)0x0454f25c)
#define DAT_04558c98 (*(int*)0x04558c98)

/* Packet buffer */
static char DAT_0454efe0[256];

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

/* String constants */
static const char* g_attack_strings[] = {
    "ATTACK", "CANCEL", "SKILL", "ITEM", "DEFEND", "ESCAPE"
};

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

/* Helper: Count targets from bitmask */
static int count_targets(u32 bitmask) {
    int count = 0;
    u32 mask = 1;
    int i;
    for (i = 0; i < 6; i++) {
        if ((mask & bitmask) != 0) {
            count++;
            if (count == 4) break;
        }
        mask <<= 1;
    }
    return count;
}

/*
 * Attack render - FUN_00426380
 * Actions 0, 1, 10, 0x0b, 0x2d
 */
void battle_render_action_0(int variant) {
    int window;
    int target_count;
    int row_height;
    int i;
    u32 mask;
    int button_result;
    int state_check;
    char packet_data[256];
    char buffer[256];

    LOG_DEBUG("Render attack variant %d", variant);

    if (DAT_0455ef98 == 0) {
        DAT_0455eec8 = DAT_045541dc;
        DAT_0455af84 = DAT_045541a4;

        if (variant == 2) {
            DAT_0455af84 = 7;
        }

        DAT_0454f3c8 = (DAT_0455af84 * -0x30 + 0x1c8) / 2;
        DAT_0455796c = ((10 - DAT_045541dc) * 0x40) / 2;

        DAT_0455ef98 = FUN_00448610(DAT_0455796c, DAT_0454f3c8,
                                     DAT_045541dc, DAT_0455af84, 0, 1);

        row_height = DAT_0455af84 * 0x30;
        DAT_0455b0dc = 0x14;

        for (i = 0; i < 6; i++) {
            (&DAT_004b83f0)[i] = -2;
        }

        target_count = count_targets(DAT_0455ef94);
        DAT_04552e38 = target_count;
        DAT_0455641c = (row_height - 0x38) / 0x14;

        if (target_count > 0) {
            int spacing = (DAT_0455eec8 << 6) / (target_count + 1);
            int btn_x = spacing - 0x1b;
            int btn_y = (row_height - 0x38) / DAT_0455641c;

            for (i = 0; i < target_count; i++) {
                ((int*)0x04552e40)[i * 2] = btn_x;
                ((int*)0x04552e40)[i * 2 + 1] = btn_y * (DAT_0455641c - 1) + 8 +
                                                  (row_height - DAT_0455641c * 0x14) / 2;
                btn_x += spacing;
            }
        }

        if (variant == 0 || variant == 2) {
            *(int*)0x04552d88 = DAT_0455641c - 1;
            *(int*)0x0454efe0 = 0;
        } else if (variant == 1) {
            *(int*)0x04552d88 = DAT_0455641c - 2;
            *(int*)0x0454efe0 = 0;
            *(char*)0x0454f0e7 = 0;
            *(char*)0x0454f0e9 = 0;
        }

        FUN_00488190(0xca, 0x140, 0xf0);

        if (DAT_0455ef98 == 0) {
            return;
        }
    }

    button_result = FUN_00421080(&DAT_004b83f0, 6);
    state_check = FUN_0044b030();

    if (state_check == 0 &&
        (DAT_04ebe490 & 0x80000000) == 0 &&
        DAT_005676a0 != 1 &&
        DAT_00564e62 != 1 &&
        DAT_0461b420 != 1 &&
        DAT_0455ef9c != 1) {

        if (button_result >= 0 && button_result < 6) {
            FUN_0048a200(&DAT_0454efe0, packet_data, 0xff);

            if (DAT_0461b658 == 0) {
                FUN_0043b980(gSocket, DAT_04581d3c, DAT_04581d40,
                             DAT_04558c34, DAT_0455b5ac,
                             1 << button_result, packet_data);
            }

            DAT_004b83ec = -1;
        }

        if (button_result < 0) {
            goto render_menu;
        }
    } else {
        DAT_004b83ec = -1;
        DAT_0455ef9c = 0;
        goto cleanup;
    }

    return;

render_menu:
    if (DAT_0455ef98 != 0 && *(int*)(DAT_0455ef98 + 0x78) > 0) {
        for (i = 0; i < DAT_04552e38 && i < *(int*)0x04552d88; i++) {
            char* text = (char*)(0x04556678 + i * 100);
            if (text[0] != '\0') {
                int text_x = (DAT_045541dc * 0x40 - 9 * *(int*)0x04552fb0) / 2 + DAT_0455796c;
                int text_y = ((DAT_0455af84 * 0x30 - 0x38) / DAT_0455641c) * i +
                             DAT_0454f3c8 + (DAT_0455af84 * 0x30 - DAT_0455641c * DAT_0455b0dc) / 2;
                FUN_0041d860(text_x, text_y, 1, 0, text, 0);
            }
        }

        mask = 1;
        for (i = 0; i < 6; i++) {
            int* btn_positions = (int*)0x04552e40;
            (&DAT_004b83f0)[i] = -2;

            if ((mask & DAT_0455ef94) != 0) {
                int btn_handle = FUN_0041d860(
                    btn_positions[i * 2] + DAT_0455796c,
                    btn_positions[i * 2 + 1] + DAT_0454f3c8,
                    1, 4, g_attack_strings[i], 2);
                (&DAT_004b83f0)[i] = btn_handle;
            }
            mask <<= 1;
        }
    }
    return;

cleanup:
    FUN_004011c0(DAT_0455ef98);
    DAT_0455ef98 = 0;
}

/*
 * Skill render - FUN_00426850
 * Action 2
 */
void battle_render_action_2(void) {
    int target_count;
    int i;
    u32 mask;
    int button_result;
    int list_result;
    char packet_data[256];
    char buffer[256];
    int target_mask;

    LOG_DEBUG("Render skill action");

    if (DAT_0455ef98 == 0) {
        DAT_0455b118 = 7;
        _DAT_04552e34 = 5;
        DAT_04556404 = 0x60;
        DAT_04552b30 = 0x6c;

        DAT_0455ef98 = FUN_00448610(0x60, 0x6c, 7, 5, 0, 1);

        for (i = 0; i < 6; i++) {
            (&DAT_004b8408)[i] = -2;
        }

        target_count = count_targets(DAT_0455ef94);
        DAT_04556478 = target_count;

        if (target_count > 0) {
            int spacing = (DAT_0455b118 << 6) / (target_count + 1);
            int btn_x = spacing - 0x1b;

            for (i = 0; i < target_count; i++) {
                ((int*)0x0454bb5c)[i * 2] = btn_x;
                ((int*)0x0454bb5c)[i * 2 + 1] = 0xc4;
                btn_x += spacing;
            }
        }

        DAT_0454efcc = 10;
        FUN_00488190(0xca, 0x140, 0xf0);

        if (DAT_0455ef98 == 0) {
            return;
        }
    }

    list_result = -1;
    button_result = FUN_00421080(&DAT_004b8408, 6);
    int state_check = FUN_0044b030();

    if (state_check == 0 &&
        (DAT_04ebe490 & 0x80000000) == 0 &&
        DAT_005676a0 != 1 &&
        DAT_00564e62 != 1 &&
        DAT_0461b420 != 1 &&
        DAT_0455ef9c != 1) {

        if (button_result == 100) {
            target_mask = 2;
        } else if (button_result >= 0 && button_result <= 5) {
            target_mask = 1 << button_result;
        } else if (list_result >= 0 && list_result < 10) {
            target_mask = 1;
        } else {
            goto render_menu;
        }

        FUN_004923a7(buffer, (void*)0x004aa1c4);
        FUN_0048a200(packet_data, buffer, 0xff, buffer, (void*)0x004aa1c4, list_result);

        if (DAT_0461b658 == 0) {
            FUN_0043b980(gSocket, DAT_04581d3c, DAT_04581d40,
                         DAT_04558c34, DAT_0455b5ac, target_mask, packet_data);
        } else if (DAT_046308b8 == 0) {
            FUN_0048fdc0(gSocket, DAT_04581d3c, DAT_04581d40,
                         DAT_04558c34, DAT_0455b5ac, target_mask, packet_data);
        } else {
            FUN_0048fdc0(gSocket, DAT_04581d3c - DAT_045829d8,
                         DAT_04581d40 - DAT_045829dc,
                         DAT_04558c34, DAT_0455b5ac, target_mask, packet_data);
        }

        DAT_004b83ec = -1;
    } else {
        DAT_004b83ec = -1;
        DAT_0455ef9c = 0;
        goto cleanup;
    }

render_menu:
    if (button_result < 0 && list_result < 0) {
        if (DAT_0455ef98 != 0 && *(int*)(DAT_0455ef98 + 0x78) > 0) {
            for (i = 0; i < DAT_0454efcc; i++) {
                char* text = (char*)(0x04556678 + i * 100);
                if (text[0] != '\0') {
                    int text_x = DAT_04556404 + 0x2c;
                    int text_y = 0x15 * i + DAT_04552b30;
                    FUN_0041d860(text_x, text_y, 1, 0, text, 0);
                }
            }

            mask = 1;
            for (i = 0; i < 6; i++) {
                int* btn_positions = (int*)0x0454bb5c;
                (&DAT_004b8408)[i] = -2;

                if ((mask & DAT_0455ef94) != 0) {
                    int btn_handle = FUN_0041d860(
                        btn_positions[i * 2] + DAT_04556404,
                        btn_positions[i * 2 + 1] + DAT_04552b30,
                        1, 4, g_attack_strings[i], 2);
                    (&DAT_004b8408)[i] = btn_handle;
                }
                mask <<= 1;
            }
        }
    }
    return;

cleanup:
    FUN_004011c0(DAT_0455ef98);
    DAT_0455ef98 = 0;
}

/*
 * Capture render - FUN_0042ce40
 * Actions 0xc, 0xd
 */
void battle_render_action_cd(void) {
    int result;
    char packet_data[256];
    char buffer[256];
    short item_id;

    LOG_DEBUG("Render capture state=%d", DAT_0455a0fc);

    switch (DAT_0455a0fc) {
        case 0:
            DAT_0455a0fc++;
            FUN_00488190(0xca, 0x140, 0xf0);
            break;

        case 1:
            break;

        case 10:
            DAT_0455a0fc++;
            FUN_00488190(0xca, 0x140, 0xf0);
            break;

        case 11:
            result = 0;
            if (result == 1) {
                FUN_004923a7(buffer, (void*)0x004b92e8);
                FUN_0048a200(buffer, packet_data, 0xff);
                if (DAT_0461b658 == 0) {
                    FUN_0043b980(gSocket, DAT_04581d3c, DAT_04581d40,
                                 DAT_04558c34, DAT_0455b5ac, 0, packet_data);
                }
                DAT_004b83ec = -1;
                return;
            } else if (result == 2) {
                DAT_0455a0fc = 0x14;
            }
            break;

        case 0x14:
            DAT_0455a0fc++;
            FUN_00488190(0xca, 0x140, 0xf0);
            break;

        case 0x15:
            result = 0;
            if (result == 1) {
                item_id = *(short*)(0x0455b5de + DAT_0454f25c * 0x44);
                FUN_004923a7(buffer, (void*)0x004aa1c4);
                FUN_0048a200(buffer, packet_data, 0xff, buffer, (void*)0x004aa1c4, item_id);

                if (DAT_0461b658 == 0) {
                    FUN_0043b980(gSocket, DAT_04581d3c, DAT_04581d40,
                                 DAT_04558c34, DAT_0455b5ac, 0, packet_data);
                }

                DAT_04558c98--;
                if (DAT_04558c98 < 0) DAT_04558c98 = 0;
            } else if (result == 2) {
                DAT_0455a0fc = 10;
            }
            break;

        case 100:
            DAT_0455a0fc++;
            FUN_00488190(0xca, 0x140, 0xf0);
            break;

        case 0x65:
            result = 0;
            if (result == 1) {
                FUN_004923a7(buffer, (void*)0x004b92e8);
                FUN_0048a200(buffer, packet_data, 0xff);
                if (DAT_0461b658 == 0) {
                    FUN_0043b980(gSocket, DAT_04581d3c, DAT_04581d40,
                                 DAT_04558c34, DAT_0455b5ac, 0, packet_data);
                }
                DAT_004b83ec = -1;
                return;
            } else if (result == 2) {
                DAT_0455a0fc = 0x6e;
            } else if (result == 3) {
                DAT_0455a0fc = 0x78;
                return;
            }
            break;

        default:
            break;
    }
}
