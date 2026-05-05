/*
 * Stone Age Client - Text Queue Rendering Header
 * Reverse engineered from sa_9061.exe FUN_0041d7c0, FUN_00414820
 */

#ifndef RENDER_TEXT_H
#define RENDER_TEXT_H

#include "types.h"

/* Maximum text queue entries - 0x3ff from original */
#define MAX_TEXT_QUEUE_ENTRIES  1024

/*
 * Text queue entry structure - 0x110 (272) bytes per entry
 * Reverse engineered from FUN_0041d7c0 and FUN_00414820
 * Memory layout matches DAT_005676f8 array
 */
typedef struct {
    /* Offset 0x00-0x03: Position */
    s16 x;                  /* Offset 0x00: X position */
    s16 y;                  /* Offset 0x02: Y position */

    /* Offset 0x04: Flags/secondary index */
    u8  flags;              /* Offset 0x04: Flags (checked at pCVar8[-1] in FUN_00414820) */

    /* Offset 0x05-0x104: Text buffer */
    char text[256];         /* Offset 0x05: Text string (256 bytes to 0x105) */

    /* Offset 0x105-0x10f: Rendering parameters */
    u8  color_type;         /* Offset 0x105: Color index (checked at pCVar8[0x100]) */
    u8  padding[2];         /* Offset 0x106-0x107: Padding */
    u32 param1;             /* Offset 0x108: Font size or custom parameter */
    u32 param2;             /* Offset 0x10c: Additional parameter */
} TextQueueEntry;           /* Total: 0x110 = 272 bytes */

/* Initialization and shutdown */
void text_queue_init(void);
void text_queue_shutdown(void);

/* Mode settings */
void text_queue_set_half_resolution(int enabled);
void text_queue_set_dialog_offset(int offset);
void text_queue_set_dialog_flag(int flag);

/* Queue operations - FUN_0041d7c0 */
int text_queue_add(int x, int y, u8 color_type, u8 flags, const char* text, u32 param1, u32 param2);

/* Convenience wrapper - FUN_0041d860 (calls text_queue_add with param2=0) */
int text_queue_add_simple(int x, int y, u8 color_type, u8 flags, const char* text, u32 param1);

/* Queue management */
void text_queue_clear(void);
int text_queue_get_count(void);

/* Rendering - FUN_00414820(group)
 * group parameter filters which text entries to render by their color_type field:
 *   0 = general text (rendered at layer > 0x67)
 *   1 = second text layer (rendered at layer > 0x6c)
 *   2 = third text layer (rendered at layer > 0x6e)
 *   3 = special text / dialog / IME (rendered at layer > 0x67)
 * Pass -1 to render all groups (for final flush).
 */
void text_queue_process(int group);

/* Backward compatible aliases */
#define render_text_queue_add    text_queue_add
#define render_text_queue_clear  text_queue_clear
#define render_text_queue_get_count  text_queue_get_count
#define render_text_queue_process    text_queue_process

#endif /* RENDER_TEXT_H */
