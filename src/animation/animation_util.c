/*
 * Stone Age Client - Animation Utility Functions
 * Split from animation.c for code organization
 *
 * Handles callbacks, frame data access, and utility functions
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "animation.h"
#include "logger.h"

/* External global animation context */
extern AnimationContext g_anim;

/* External functions from animation_core.c */
extern AnimationData* anim_get(u16 anim_id);

/*
 * Set complete callback
 */
void anim_set_complete_callback(u32 entity_id, void (*callback)(u32)) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            g_anim.active[i].on_complete = callback;
            return;
        }
    }
}

/*
 * Set frame event callback
 */
void anim_set_frame_callback(u32 entity_id, void (*callback)(u32, u8, u8)) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            g_anim.active[i].on_frame_event = callback;
            return;
        }
    }
}

/*
 * Get current frame data
 */
AnimFrame* anim_get_current_frame_data(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            if (anim) {
                return &anim->frames[g_anim.active[i].current_frame][g_anim.active[i].current_dir];
            }
        }
    }

    return NULL;
}

/*
 * Get frame sprite
 */
u16 anim_get_frame_sprite(u32 entity_id) {
    AnimFrame* frame = anim_get_current_frame_data(entity_id);
    return frame ? frame->sprite_id : 0;
}

/*
 * Get frame duration
 */
u16 anim_get_frame_duration(u32 entity_id) {
    AnimFrame* frame = anim_get_current_frame_data(entity_id);
    return frame ? frame->duration : 0;
}

/*
 * Trigger frame event
 */
void anim_trigger_frame_event(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            if (anim && g_anim.active[i].on_frame_event) {
                AnimFrame* frame = &anim->frames[g_anim.active[i].current_frame][g_anim.active[i].current_dir];
                if (frame->event_type != 0) {
                    g_anim.active[i].on_frame_event(entity_id, frame->event_type, frame->event_param);
                }
            }
            return;
        }
    }
}

/*
 * Clear events
 */
void anim_clear_events(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            g_anim.active[i].on_complete = NULL;
            g_anim.active[i].on_frame_event = NULL;
            return;
        }
    }
}

/*
 * Animation type to string
 */
const char* anim_type_to_string(AnimationType type) {
    static const char* type_names[] = {
        "Idle", "Walk", "Run", "Attack", "Skill", "Cast",
        "Damage", "Death", "Sit", "Stand", "Emote", "Victory",
        "Defend", "Special"
    };

    if (type < sizeof(type_names) / sizeof(type_names[0])) {
        return type_names[type];
    }
    return "Unknown";
}

/*
 * Direction to string - FUN_00443e80 verified
 */
const char* anim_dir_to_string(AnimationDirection dir) {
    static const char* dir_names[] = {
        "SW",   /* 0: Southwest diagonal (-1, 1) */
        "W",    /* 1: Pure West (-1, 0) */
        "NW",   /* 2: Northwest diagonal (-1, -1) */
        "N",    /* 3: Pure North (0, -1) */
        "NE",   /* 4: Northeast diagonal (1, -1) */
        "E",    /* 5: Pure East (1, 0) */
        "SE",   /* 6: Southeast diagonal (1, 1) */
        "S"     /* 7: Pure South (0, 1) */
    };

    if (dir < sizeof(dir_names) / sizeof(dir_names[0])) {
        return dir_names[dir];
    }
    return "Unknown";
}

/*
 * Blend between two animations
 */
int anim_blend_to(u32 entity_id, u16 new_anim_id, u8 direction, float blend_time) {
    /* For now, just play the new animation */
    /* Full blending would require interpolation between frames */
    return anim_play(entity_id, new_anim_id, direction);
}

/*
 * Get animation type from ID
 */
AnimationType anim_get_type(u16 anim_id) {
    AnimationData* anim = anim_get(anim_id);
    return anim ? anim->type : ANIM_TYPE_IDLE;
}

/*
 * Check if animation type is interruptible
 */
int anim_is_interruptible(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            if (anim) {
                return (anim->flags & ANIM_FLAG_INTERRUPTIBLE) != 0;
            }
        }
    }

    return 1;  /* Default to interruptible */
}

/*
 * Get animation position offset for current frame
 */
void anim_get_frame_offset(u32 entity_id, s16* offset_x, s16* offset_y) {
    AnimFrame* frame = anim_get_current_frame_data(entity_id);

    if (frame) {
        if (offset_x) *offset_x = frame->offset_x;
        if (offset_y) *offset_y = frame->offset_y;
    } else {
        if (offset_x) *offset_x = 0;
        if (offset_y) *offset_y = 0;
    }
}

/*
 * Get animation sound ID for current frame
 */
u16 anim_get_frame_sound(u32 entity_id) {
    AnimFrame* frame = anim_get_current_frame_data(entity_id);
    return frame ? frame->sound_id : 0;
}
