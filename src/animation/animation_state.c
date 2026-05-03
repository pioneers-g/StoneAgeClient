/*
 * Stone Age Client - Animation State Management
 * Split from animation.c for code organization
 *
 * Handles animation updates, frame advancement, and state queries
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "animation.h"
#include "logger.h"

/* External global animation context */
extern AnimationContext g_anim;

/* External functions from animation_core.c */
extern AnimationData* anim_get(u16 anim_id);

/*
 * Update all animations
 */
void anim_update(u32 delta_time) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].playing) {
            anim_update_entity(g_anim.active[i].entity_id, delta_time);
        }
    }
}

/*
 * Update single entity animation
 */
void anim_update_entity(u32 entity_id, u32 delta_time) {
    ActiveAnimation* active = NULL;
    AnimationData* anim;
    int i;

    /* Find active animation */
    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            active = &g_anim.active[i];
            break;
        }
    }

    if (!active || !active->playing) return;

    anim = anim_get(active->anim_id);
    if (!anim) {
        active->playing = 0;
        return;
    }

    /* Apply speed modifier */
    u32 adjusted_delta = (delta_time * active->speed * g_anim.global_speed) / 10000;

    active->elapsed += adjusted_delta;
    active->total_elapsed += adjusted_delta;

    /* Check current frame duration */
    AnimFrame* frame = &anim->frames[active->current_frame][active->current_dir];

    if (active->elapsed >= frame->duration) {
        /* Advance to next frame */
        active->elapsed = 0;

        /* Trigger frame event */
        if (frame->event_type != 0 && active->on_frame_event) {
            active->on_frame_event(entity_id, frame->event_type, frame->event_param);
        }

        active->current_frame++;

        if (active->current_frame >= anim->frame_count) {
            /* End of animation */
            if (anim->flags & ANIM_FLAG_LOOP || active->loop_count != 0) {
                if (anim->flags & ANIM_FLAG_PINGPONG) {
                    active->pingpong_forward = !active->pingpong_forward;
                    active->current_frame = active->pingpong_forward ? 0 : anim->frame_count - 1;
                } else {
                    active->current_frame = 0;
                }

                if (active->loop_count > 0) {
                    active->loop_count--;
                }
            } else {
                /* Animation complete */
                active->playing = 0;

                if (active->on_complete) {
                    active->on_complete(entity_id);
                }
            }
        }
    }
}

/*
 * Advance frame manually
 */
int anim_advance_frame(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            if (anim) {
                g_anim.active[i].current_frame++;
                if (g_anim.active[i].current_frame >= anim->frame_count) {
                    g_anim.active[i].current_frame = 0;
                }
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Go to specific frame
 */
int anim_goto_frame(u32 entity_id, u8 frame) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            if (anim && frame < anim->frame_count) {
                g_anim.active[i].current_frame = frame;
                g_anim.active[i].elapsed = 0;
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Set animation speed
 */
int anim_set_speed(u32 entity_id, u8 speed) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            g_anim.active[i].speed = speed;
            return 1;
        }
    }

    return 0;
}

/*
 * Get current frame
 */
u8 anim_get_current_frame(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            return g_anim.active[i].current_frame;
        }
    }

    return 0;
}

/*
 * Get current direction
 */
u8 anim_get_current_dir(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            return g_anim.active[i].current_dir;
        }
    }

    return 0;
}

/*
 * Check if animation complete
 */
int anim_is_complete(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            return !g_anim.active[i].playing;
        }
    }

    return 1;
}

/*
 * Check if animation looping
 */
int anim_is_looping(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            return anim && (anim->flags & ANIM_FLAG_LOOP);
        }
    }

    return 0;
}

/*
 * Get current animation ID
 */
u16 anim_get_current_id(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            return g_anim.active[i].anim_id;
        }
    }

    return 0;
}

/*
 * Get remaining time
 */
u32 anim_get_remaining_time(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            if (anim) {
                u32 total = anim_get_total_duration(g_anim.active[i].anim_id);
                return total > g_anim.active[i].total_elapsed ?
                       total - g_anim.active[i].total_elapsed : 0;
            }
        }
    }

    return 0;
}

/*
 * Get progress (0-100)
 */
u8 anim_get_progress(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            if (anim && anim->total_duration > 0) {
                return (u8)((g_anim.active[i].total_elapsed * 100) / anim->total_duration);
            }
        }
    }

    return 0;
}

/*
 * Set direction
 */
void anim_set_direction(u32 entity_id, u8 direction) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            g_anim.active[i].current_dir = direction % 8;
            return;
        }
    }
}

/*
 * Get total animation duration
 */
u32 anim_get_total_duration(u16 anim_id) {
    AnimationData* anim = anim_get(anim_id);
    return anim ? anim->total_duration : 0;
}

/*
 * Check if animation has completed specific number of loops
 */
int anim_has_looped(u32 entity_id, int loop_count) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            /* Check if we've looped enough times */
            return g_anim.active[i].total_elapsed >=
                   anim_get_total_duration(g_anim.active[i].anim_id) * loop_count;
        }
    }

    return 0;
}

/*
 * Force animation change (interrupts current)
 */
int anim_force_play(u32 entity_id, u16 anim_id, u8 direction) {
    /* Stop current animation */
    anim_stop(entity_id);

    /* Play new animation */
    return anim_play(entity_id, anim_id, direction);
}

/*
 * Queue animation to play after current completes
 */
int anim_queue_next(u32 entity_id, u16 next_anim_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            if (anim) {
                anim->next_anim = next_anim_id;
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Check if animation has next queued
 */
int anim_has_next(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            if (anim && anim->next_anim != 0) {
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Play next queued animation
 */
int anim_play_next(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            if (anim && anim->next_anim != 0) {
                u16 next = anim->next_anim;
                u8 dir = g_anim.active[i].current_dir;
                anim->next_anim = 0;  /* Clear queue */
                return anim_play(entity_id, next, dir);
            }
        }
    }

    return 0;
}

/*
 * Sync animation with character movement
 */
void anim_sync_movement(u32 entity_id, s16 x, s16 y) {
    /* Update direction based on movement */
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            /* Calculate direction from stored position */
            AnimationData* anim = anim_get(g_anim.active[i].anim_id);
            if (anim && (anim->flags & ANIM_FLAG_MOVE_CHARACTER)) {
                /* Movement will be handled by animation */
            }
            break;
        }
    }
}
