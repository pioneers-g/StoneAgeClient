/*
 * Stone Age Client - Battle Transition Functions
 * Reverse engineered from sa_9061.exe
 *
 * Functions for battle state transitions and initialization.
 * FUN_0047a3f0, FUN_0044aff0, FUN_0047e440, FUN_0041b870
 */

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include "types.h"
#include "battle_transition.h"
#include "battle.h"
#include "render.h"
#include "logger.h"

/* ========================================
 * Memory Locations from Binary
 * ======================================== */

/* Render queue */
#define DAT_0464f488 (*(u32*)0x0464f488)  /* Render queue count */
#define DAT_0464b488 ((short*)0x0464b488)  /* Render queue sprite indices */
#define DAT_04633488 ((int*)0x04633488)    /* Render queue entries */

/* Entity list */
#define DAT_004d7e3c (*(int*)0x004d7e3c)   /* Entity list head */
#define DAT_004d7e38 (*(int*)0x004d7e38)   /* Entity list tail */

/* State flags */
#define DAT_045e8ce0 (*(u32*)0x045e8ce0)   /* Game state flags */
#define DAT_045e19b0 (*(int*)0x045e19b0)   /* Battle active flag */
#define DAT_005ab6f8 (*(int*)0x005ab6f8)   /* Transition state */

/* Transition message */
#define DAT_005676b4 (*(int*)0x005676b4)   /* Transition timer */
#define DAT_005676cc (*(int*)0x005676cc)   /* Transition window handle */
#define DAT_04560228 (*(char*)0x04560228)  /* Transition message text */

/* Window dimensions */
#define DAT_0054c8e0 (*(int*)0x0054c8e0)
#define DAT_0054cd64 (*(int*)0x0054cd64)
#define DAT_0054ca10 (*(int*)0x0054ca10)
#define DAT_0054d174 (*(int*)0x0054d174)

/* UI window handles to clear */
#define DAT_045e7af0 (*(int*)0x045e7af0)
#define DAT_00564e5c (*(int*)0x00564e5c)
#define DAT_045f1a3b (*(char*)0x045f1a3b)
#define DAT_0458f620 (*(int*)0x0458f620)
#define DAT_045f1a3a (*(char*)0x045f1a3a)
#define DAT_045e859c (*(int*)0x045e859c)
#define DAT_045e19b4 (*(int*)0x045e19b4)
#define DAT_045e1850 (*(int*)0x045e1850)
#define DAT_045e7b14 (*(int*)0x045e7b14)
#define DAT_04583108 (*(int*)0x04583108)
#define DAT_045e19b8 (*(int*)0x045e19b8)
#define DAT_04583240 (*(int*)0x04583240)
#define DAT_045e85a0 (*(int*)0x045e85a0)
#define DAT_04582cb4 (*(int*)0x04582cb4)
#define DAT_0458f718 (*(int*)0x0458f718)
#define DAT_0458f714 (*(int*)0x0458f714)
#define DAT_045967a0 (*(int*)0x045967a0)
#define DAT_045e7c4c (*(int*)0x045e7c4c)
#define DAT_045e19bc (*(int*)0x045e19bc)
#define DAT_0458f70c (*(int*)0x0458f70c)
#define DAT_045f194e (*(short*)0x045f194e)
#define DAT_045f194f (*(char*)0x045f194f)
#define DAT_004bd3dc (*(int*)0x004bd3dc)
#define DAT_004bd3d8 (*(int*)0x004bd3d8)
#define DAT_004bd3d4 (*(int*)0x004bd3d4)
#define DAT_004e21dc (*(int*)0x004e21dc)
#define DAT_045f1be0 (*(int*)0x045f1be0)
#define DAT_04582a24 (*(short*)0x04582a24)
#define DAT_04582a22 (*(short*)0x04582a22)
#define DAT_04582a28 (*(short*)0x04582a28)

/* External state */
extern int DAT_005676a0, DAT_00564e62;
extern int DAT_004b83ec;
extern int gSocket;

/* External functions */
extern void FUN_00418370(void);  /* field_update */
extern void FUN_00424f50(void);  /* battle_action_execute */
extern void FUN_004011c0(int window);
extern int FUN_00448610(int x, int y, int w, int h, int flags, int visible);
extern void FUN_00488190(int r, int g, int b);
extern int FUN_0041d860(int x, int y, int font, int style, const char* text, int color);
extern int FUN_0044b030(void);
extern void FUN_00411990(void* ptr);
extern void FUN_0041adf0(void);
extern void FUN_0041a8d0(void);
extern void FUN_0041bba0(void);
extern void FUN_0044ac00(void);
extern void FUN_004445c0(void);
extern void FUN_00445070(void);
extern void FUN_004419a0(void);
extern void FUN_00440e90(void);
extern void FUN_00410850(void);
extern void FUN_004117e0(void);
extern void FUN_0047d8c0(void);
extern void FUN_00412a40(void);
extern void FUN_0047dc60(void);
extern void FUN_00401170(void);

/* ========================================
 * Implementation
 * ======================================== */

/*
 * Clear battle UI windows - part 1
 * FUN_0044ac50
 */
void battle_clear_windows_1(void) {
    /* Destroy main UI windows */
    if (DAT_045e7af0 != 0) {
        FUN_004011c0(DAT_045e7af0);
        DAT_045e7af0 = 0;
    }
    DAT_00564e5c = 0;
    DAT_045f1a3b = 0;

    if (DAT_0458f620 != 0) {
        FUN_004011c0(DAT_0458f620);
        DAT_0458f620 = 0;
    }
    DAT_045f1a3a = 0;

    if (DAT_045e859c != 0) {
        FUN_004011c0(DAT_045e859c);
        DAT_045e859c = 0;
    }

    if (DAT_045e19b4 != 0) {
        FUN_004011c0(DAT_045e19b4);
        DAT_045e19b4 = 0;
    }

    /* Clear cursor sprite */
    FUN_00411990((void*)0x0054a3c8);

    if (DAT_045e1850 != 0) {
        FUN_004011c0(DAT_045e1850);
        DAT_045e1850 = 0;
    }

    if (DAT_045e7b14 != 0) {
        FUN_004011c0(DAT_045e7b14);
        DAT_045e7b14 = 0;
    }

    if (DAT_04583108 != 0) {
        FUN_004011c0(DAT_04583108);
        DAT_04583108 = 0;
    }

    /* Clear additional resources */
    FUN_0044ac00();

    /* Clear optional windows */
    if (DAT_045e19b8 != 0) {
        FUN_004011c0(DAT_045e19b8);
        DAT_045e19b8 = 0;
    }

    if (DAT_04583240 != 0) {
        FUN_004011c0(DAT_04583240);
        DAT_04583240 = 0;
    }

    if (DAT_045e85a0 != 0) {
        FUN_004011c0(DAT_045e85a0);
        DAT_045e85a0 = 0;
    }

    if (DAT_04582cb4 != 0) {
        FUN_004011c0(DAT_04582cb4);
        DAT_04582cb4 = 0;
    }

    if (DAT_0458f718 != 0) {
        FUN_004011c0(DAT_0458f718);
        DAT_0458f718 = 0;
    }

    if (DAT_0458f714 != 0) {
        FUN_004011c0(DAT_0458f714);
        DAT_0458f714 = 0;
    }

    if (DAT_045967a0 != 0) {
        FUN_004011c0(DAT_045967a0);
        DAT_045967a0 = 0;
    }

    if (DAT_045e7c4c != 0) {
        FUN_004011c0(DAT_045e7c4c);
        DAT_045e7c4c = 0;
    }
}

/*
 * Clear battle UI windows - part 2
 * FUN_0044adc0
 */
void battle_clear_windows_2(void) {
    if (DAT_045e19bc != 0) {
        FUN_004011c0(DAT_045e19bc);
        FUN_00411990((void*)0x0054a3c8);
        DAT_045f194e = 0;
        DAT_045e19bc = 0;
    }

    if (DAT_0458f70c != 0) {
        FUN_004011c0(DAT_0458f70c);
        DAT_0458f70c = 0;
    }

    DAT_004bd3dc = -1;

    if (DAT_045e19b8 != 0) {
        FUN_004011c0(DAT_045e19b8);
    }

    DAT_00564e5c = 0;
    DAT_045f1a3b = 0;

    if (DAT_0458f620 != 0) {
        FUN_004011c0(DAT_0458f620);
        DAT_0458f620 = 0;
    }
    DAT_045f1a3a = 0;

    if (DAT_045e859c != 0) {
        FUN_004011c0(DAT_045e859c);
        DAT_045e859c = 0;
    }

    if (DAT_04583240 != 0) {
        FUN_004011c0(DAT_04583240);
        DAT_04583240 = 0;
    }

    if (DAT_045e85a0 != 0) {
        FUN_004011c0(DAT_045e85a0);
        DAT_045e85a0 = 0;
    }

    if (DAT_04582cb4 != 0) {
        FUN_004011c0(DAT_04582cb4);
        DAT_04582cb4 = 0;
    }

    DAT_045f194f = 0;
    DAT_045e19b8 = 0;

    if (DAT_0458f718 != 0) {
        FUN_004011c0(DAT_0458f718);
        DAT_0458f718 = 0;
    }

    if (DAT_0458f714 != 0) {
        FUN_004011c0(DAT_0458f714);
        DAT_0458f714 = 0;
    }

    if (DAT_045967a0 != 0) {
        FUN_004011c0(DAT_045967a0);
        DAT_045967a0 = 0;
    }

    if (DAT_045e7c4c != 0) {
        FUN_004011c0(DAT_045e7c4c);
        DAT_045e7c4c = 0;
    }
}

/*
 * Clear battle UI state
 * FUN_0044aff0
 */
void battle_clear_ui_state(void) {
    DAT_045e8ce0 = 0;
    battle_clear_windows_1();
    battle_clear_windows_2();
    DAT_004e21dc = 0;
    DAT_045f1be0 = -1;
    DAT_004bd3d8 = -1;
    DAT_004bd3d4 = -1;
}

/*
 * Build entity render queue
 * FUN_0047e440
 *
 * Iterates through entity list and adds visible entities to render queue.
 * Each entry contains position, sprite info, and render priority.
 */
void battle_build_entity_queue(void) {
    int entity;
    int* queue_entry;
    short* sprite_entry;
    u32 flags;
    int render_type;
    int entity_type;

    sprite_entry = &DAT_0464b488[DAT_0464f488 * 2];
    entity = *(int*)(DAT_004d7e3c + 4);  /* First entity in list */
    queue_entry = &DAT_04633488[DAT_0464f488 * 6];

    if (entity == DAT_004d7e38) {
        return;  /* Empty list */
    }

    while (entity != DAT_004d7e38) {
        if (DAT_0464f488 > 0xfff) {
            return;  /* Queue full */
        }

        /* Check if entity should be rendered */
        int field_9c = *(int*)(entity + 0x9c);
        int field_24 = *(int*)(entity + 0x24);
        u32 field_a0 = *(u32*)(entity + 0xa0);

        /* Visibility check: field_9c < -1 or > 99, not dead, not hidden */
        if ((field_9c < -1 || field_9c > 99) && field_24 != 1 && (field_a0 & 2) == 0) {
            /* Set sprite direction */
            *(sprite_entry + 1) = *(unsigned char*)(entity + 0x15);
            *sprite_entry = DAT_0464f488;

            /* Set position (with offset) */
            queue_entry[0] = *(int*)(entity + 0x160) + *(int*)(entity + 0x18);
            queue_entry[1] = *(int*)(entity + 0x164) + *(int*)(entity + 0x1c);
            queue_entry[2] = field_9c;
            queue_entry[3] = entity;  /* Entity pointer */

            /* Determine render type from flags */
            flags = field_a0;

            if (flags & 0x200000) {
                render_type = 10;
            } else if (flags & 0x400000) {
                render_type = 11;
            } else if (flags & 0x800000) {
                render_type = 12;
            } else if (flags & 0x1000000) {
                render_type = 13;
            } else if (flags & 0x20000) {
                render_type = 6;
            } else if (flags & 0x40000) {
                render_type = 7;
            } else if (flags & 0x80000) {
                render_type = 8;
            } else if (flags & 0x100000) {
                render_type = 9;
            } else if (flags & 0x8000) {
                render_type = 5;
            } else if (flags & 0x4000) {
                render_type = 4;
            } else if (flags & 0x2000) {
                render_type = 3;
            } else if (flags & 8) {
                render_type = 2;
            } else if (flags & 4) {
                render_type = 1;
            } else {
                render_type = 0;
            }

            queue_entry[4] = render_type;

            /* Set entity type indicator */
            entity_type = *(int*)(entity + 0x90);
            if (entity_type == 4) {
                *(char*)(queue_entry + 5) = 3;
            } else if (entity_type == 1) {
                *(char*)(queue_entry + 5) = 4;
            } else {
                *(char*)(queue_entry + 5) = 0;
            }

            /* Special case for certain entity IDs */
            if (field_9c > 499999 && field_9c < 550000) {
                int lookup = *(int*)(0x81c804 + (field_9c * 5 - 2500000) * 8);
                if (lookup == 1) {
                    *(char*)(queue_entry + 5) = 2;
                }
            }

            /* Store queue index in entity */
            *(int*)(entity + 0x20) = DAT_0464f488;

            /* Advance queue */
            DAT_0464f488++;
            sprite_entry += 2;
            queue_entry += 6;
        }

        entity = *(int*)(entity + 4);  /* Next entity */
    }
}

/*
 * Display battle transition message
 * FUN_0041b870
 *
 * Shows "BATTLE START" or similar message during transition.
 * Creates a temporary window with the message text.
 */
void battle_transition_message(void) {
    int state_check;
    DWORD current_time;
    u32 text_len;
    char* text_ptr;

    if (DAT_005676b4 == 0) {
        /* No transition in progress */
        if (DAT_005676cc != 0) {
            FUN_004011c0(DAT_005676cc);
            DAT_005676cc = 0;
        }
        return;
    }

    if (DAT_045e19b0 != 0) {
        /* Battle already active */
        return;
    }

    /* Create transition window if needed */
    if (DAT_005676cc == 0) {
        state_check = FUN_0044b030();
        if (state_check != 0) {
            if (DAT_045e8ce0 & 0x40000000) {
                DAT_045e19b0 = 1;
            }
            battle_clear_ui_state();
        }

        /* Initialize transition UI */
        FUN_0041adf0();
        FUN_0041a8d0();
        FUN_0041bba0();

        DAT_0054c8e0 = 4;
        DAT_0054cd64 = 2;
        DAT_0054ca10 = 0xc0;
        DAT_0054d174 = 0xb4;

        /* Create window: 4x2 at (0xc0, 0xb4) */
        DAT_005676cc = FUN_00448610(0xc0, 0xb4, 4, 2, 0, 3);
        FUN_00488190(0xca, 0x140, 0xf0);

        if (DAT_005676cc == 0) {
            return;
        }
    }

    /* Check if should display message */
    current_time = timeGetTime();

    if (current_time < DAT_005676b4 + 2000 &&
        FUN_0044b030() == 0 &&
        DAT_004b83ec > 0x7fffffff &&
        DAT_005676a0 != 1 &&
        DAT_00564e62 != 1) {

        /* Render transition message */
        if (*(int*)(DAT_005676cc + 0x78) < 1) {
            return;
        }

        /* Calculate text length */
        text_len = 0xffffffff;
        text_ptr = (char*)&DAT_04560228;
        while (*text_ptr != '\0' && text_len != 0) {
            text_len--;
            text_ptr++;
        }
        text_len = ~text_len;

        /* Render centered text */
        int text_x = ((DAT_0054c8e0 * 0x40 + (text_len - 1) / 2 * -0x11) >> 1) + DAT_0054ca10;
        int text_y = ((DAT_0054cd64 * 0x30 - 0x10) >> 1) + DAT_0054d174;

        FUN_0041d860(text_x, text_y, 1, 0, (char*)&DAT_04560228, 0);
        return;
    }

    /* Transition complete - cleanup */
    FUN_004011c0(DAT_005676cc);
    DAT_005676cc = 0;
    DAT_005676b4 = 0;
}

/*
 * Initialize battle transition state
 * FUN_0047a3f0
 */
void battle_transition_init(void) {
    /* Copy state variables */
    DAT_04582a24 = DAT_04582a22;
    DAT_04582a28 = *(short*)&DAT_04582a28;

    /* Clear render queue */
    DAT_0464f488 = 0;
    DAT_005ab6f8 = 0;

    /* Check battle flag */
    if (DAT_045e8ce0 & 0x40000000) {
        DAT_045e19b0 = 1;
    }

    /* Call subsystem initializers */
    battle_clear_ui_state();       /* FUN_0044aff0 */
    FUN_00418370();                /* field_update */
    FUN_004445c0();                /* Unknown */
    FUN_00424f50();                /* battle_action_execute */

    DAT_005676b4 = 0;

    battle_transition_message();   /* FUN_0041b870 */
    FUN_00401170();                /* Unknown */

    battle_build_entity_queue();   /* FUN_0047e440 */
    FUN_00445070();                /* Unknown */
    FUN_004419a0();                /* Unknown */
    FUN_00440e90();                /* Unknown */
    FUN_00410850();                /* Unknown */
    FUN_004117e0();                /* Unknown */
    FUN_0047d8c0();                /* Unknown */
    FUN_00412a40();                /* Unknown */
    FUN_0047dc60();                /* render_queue_process */
}

/*
 * Get render queue count
 */
u32 battle_get_render_queue_count(void) {
    return DAT_0464f488;
}
