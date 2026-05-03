/*
 * Stone Age Client - Animation System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef ANIMATION_H
#define ANIMATION_H

#include "types.h"

/* Constants */
#define MAX_ANIMATIONS      100
#define MAX_ANIM_FRAMES     32
#define MAX_ANIM_LAYERS     5

/* Animation types */
typedef enum {
    ANIM_TYPE_IDLE = 0,
    ANIM_TYPE_WALK = 1,
    ANIM_TYPE_RUN = 2,
    ANIM_TYPE_ATTACK = 3,
    ANIM_TYPE_SKILL = 4,
    ANIM_TYPE_CAST = 5,
    ANIM_TYPE_DAMAGE = 6,
    ANIM_TYPE_DEATH = 7,
    ANIM_TYPE_SIT = 8,
    ANIM_TYPE_STAND = 9,
    ANIM_TYPE_EMOTE = 10,
    ANIM_TYPE_VICTORY = 11,
    ANIM_TYPE_DEFEND = 12,
    ANIM_TYPE_SPECIAL = 13
} AnimationType;

/* Animation flags */
typedef enum {
    ANIM_FLAG_NONE = 0,
    ANIM_FLAG_LOOP = (1 << 0),
    ANIM_FLAG_PINGPONG = (1 << 1),
    ANIM_FLAG_REVERSE = (1 << 2),
    ANIM_FLAG_INTERRUPTIBLE = (1 << 3),
    ANIM_FLAG_MOVE_CHARACTER = (1 << 4),
    ANIM_FLAG_LOCK_INPUT = (1 << 5)
} AnimationFlags;

/* Animation direction - FUN_00443e80
 *
 * Isometric coordinate system with 8 directions:
 * - Even directions (0,2,4,6): Diagonal movements
 * - Odd directions (1,3,5,7): Cardinal (axis-aligned) movements
 *
 * Direction layout (from FUN_00443e80 binary analysis):
 *   0: dx=-1, dy=1   Southwest diagonal
 *   1: dx=-1, dy=0   Pure West
 *   2: dx=-1, dy=-1  Northwest diagonal
 *   3: dx=0,  dy=-1  Pure North
 *   4: dx=1,  dy=-1  Northeast diagonal
 *   5: dx=1,  dy=0   Pure East
 *   6: dx=1,  dy=1   Southeast diagonal
 *   7: dx=0,  dy=1   Pure South
 *
 * Visual representation (isometric):
 *        NW(2)   N(3)   NE(4)
 *          \      |      /
 *           \     |     /
 *            \    |    /
 *     W(1)----[CENTER]----E(5)
 *            /    |    \
 *           /     |     \
 *          /      |      \
 *        SW(0)   S(7)   SE(6)
 */
typedef enum {
    ANIM_DIR_SW = 0,        /* Southwest diagonal (-1, 1) */
    ANIM_DIR_W = 1,         /* Pure West (-1, 0) */
    ANIM_DIR_NW = 2,        /* Northwest diagonal (-1, -1) */
    ANIM_DIR_N = 3,         /* Pure North (0, -1) */
    ANIM_DIR_NE = 4,        /* Northeast diagonal (1, -1) */
    ANIM_DIR_E = 5,         /* Pure East (1, 0) */
    ANIM_DIR_SE = 6,        /* Southeast diagonal (1, 1) */
    ANIM_DIR_S = 7          /* Pure South (0, 1) */
} AnimationDirection;

/* Legacy aliases for backward compatibility - DEPRECATED */
#define ANIM_DIR_SOUTH     ANIM_DIR_S      /* Was incorrectly named */
#define ANIM_DIR_SOUTHWEST ANIM_DIR_SW
#define ANIM_DIR_WEST      ANIM_DIR_W      /* Was incorrectly named */
#define ANIM_DIR_NORTHWEST ANIM_DIR_NW
#define ANIM_DIR_NORTH     ANIM_DIR_N      /* Was incorrectly named */
#define ANIM_DIR_NORTHEAST ANIM_DIR_NE
#define ANIM_DIR_EAST      ANIM_DIR_E      /* Was incorrectly named */
#define ANIM_DIR_SOUTHEAST ANIM_DIR_SE

/* Animation frame */
typedef struct {
    u16 sprite_id;
    u16 duration;       /* Duration in ms */
    s16 offset_x;
    s16 offset_y;
    u16 sound_id;
    u8 event_type;      /* Event to trigger at this frame */
    u8 event_param;
} AnimFrame;

/* Animation layer */
typedef struct {
    u16 z_order;
    u8 blend_mode;
    u8 alpha;
    s16 offset_x;
    s16 offset_y;
} AnimLayer;

/* Animation data */
typedef struct {
    u16 id;
    char name[24];
    AnimationType type;
    u16 total_duration;
    u16 frame_count;
    u8 direction_count;
    u8 flags;

    /* Frames for each direction */
    AnimFrame frames[MAX_ANIM_FRAMES][8];

    /* Layers */
    AnimLayer layers[MAX_ANIM_LAYERS];
    u8 layer_count;

    /* Transitions */
    u16 next_anim;
    u16 transition_frame;

} AnimationData;

/* Active animation state */
typedef struct {
    u16 anim_id;
    u8 current_frame;
    u8 current_dir;
    u32 elapsed;
    u32 total_elapsed;
    u8 playing;
    u8 loop_count;
    u8 pingpong_forward;
    u8 speed;       /* 100 = normal speed */

    /* Callbacks */
    void (*on_complete)(u32 entity_id);
    void (*on_frame_event)(u32 entity_id, u8 event_type, u8 event_param);

    /* Entity reference */
    u32 entity_id;

} ActiveAnimation;

/* Animation context */
typedef struct {
    /* Animation database */
    AnimationData* anim_db;
    int db_count;
    int db_capacity;

    /* Active animations */
    ActiveAnimation active[MAX_ANIMATIONS];
    int active_count;

    /* Default animations per type */
    u16 default_anims[14];  /* One per AnimationType */

    /* Global speed multiplier */
    u16 global_speed;

    /* Entity animation tracking */
    u32 entity_anim_map[256];  /* Entity ID -> active anim index */

} AnimationContext;

/* Global animation context */
extern AnimationContext g_anim;

/* Initialization */
int anim_init(void);
void anim_shutdown(void);

/* Database */
int anim_load_database(void);
AnimationData* anim_get(u16 anim_id);
AnimationData* anim_get_by_name(const char* name);
AnimationData* anim_get_default(AnimationType type);

/* Play animation */
int anim_play(u32 entity_id, u16 anim_id, u8 direction);
int anim_play_type(u32 entity_id, AnimationType type, u8 direction);
int anim_play_loop(u32 entity_id, u16 anim_id, u8 direction, int loop_count);
int anim_stop(u32 entity_id);
int anim_is_playing(u32 entity_id);

/* Update */
void anim_update(u32 delta_time);
void anim_update_entity(u32 entity_id, u32 delta_time);

/* Frame control */
int anim_advance_frame(u32 entity_id);
int anim_goto_frame(u32 entity_id, u8 frame);
int anim_set_speed(u32 entity_id, u8 speed);
u8 anim_get_current_frame(u32 entity_id);
u8 anim_get_current_dir(u32 entity_id);

/* Query */
int anim_is_complete(u32 entity_id);
int anim_is_looping(u32 entity_id);
u16 anim_get_current_id(u32 entity_id);
u32 anim_get_remaining_time(u32 entity_id);
u8 anim_get_progress(u32 entity_id);  /* 0-100 */

/* Direction */
void anim_set_direction(u32 entity_id, u8 direction);
u8 anim_direction_from_delta(s16 dx, s16 dy);
u8 anim_direction_to_target(s16 from_x, s16 from_y, s16 to_x, s16 to_y);

/* Callbacks */
void anim_set_complete_callback(u32 entity_id, void (*callback)(u32));
void anim_set_frame_callback(u32 entity_id, void (*callback)(u32, u8, u8));

/* Frame info */
AnimFrame* anim_get_current_frame_data(u32 entity_id);
u16 anim_get_frame_sprite(u32 entity_id);
u16 anim_get_frame_duration(u32 entity_id);

/* Events */
void anim_trigger_frame_event(u32 entity_id);
void anim_clear_events(u32 entity_id);

/* Utility */
const char* anim_type_to_string(AnimationType type);
const char* anim_dir_to_string(AnimationDirection dir);
u32 anim_get_total_duration(u16 anim_id);

/* Frame timing */
u32 anim_get_frame_interval(AnimationType type);

/* Direction helpers */
void anim_get_direction_delta(u8 direction, s16* dx, s16* dy);
u8 anim_calculate_direction(s16 src_x, s16 src_y, s16 dst_x, s16 dst_y);
float anim_get_direction_angle(u8 direction);

/* Loop checking */
int anim_has_looped(u32 entity_id, int loop_count);

/* Blending */
int anim_blend_to(u32 entity_id, u16 new_anim_id, u8 direction, float blend_time);

/* Type query */
AnimationType anim_get_type(u16 anim_id);
int anim_is_interruptible(u32 entity_id);

/* Force play */
int anim_force_play(u32 entity_id, u16 anim_id, u8 direction);

/* Queue system */
int anim_queue_next(u32 entity_id, u16 next_anim_id);
int anim_has_next(u32 entity_id);
int anim_play_next(u32 entity_id);

/* Movement sync */
void anim_sync_movement(u32 entity_id, s16 x, s16 y);

/* Frame data */
void anim_get_frame_offset(u32 entity_id, s16* offset_x, s16* offset_y);
u16 anim_get_frame_sound(u32 entity_id);

#endif /* ANIMATION_H */
