/*
 * Stone Age Client - Animation Core System
 * Split from animation.c for code organization
 *
 * Handles initialization, database loading, and basic animation control
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "animation.h"
#include "logger.h"

/* Global animation context */
AnimationContext g_anim = {0};

/* Animation timing constants */
#define ANIM_FRAME_INTERVAL    100    /* Default frame duration in ms */
#define ANIM_IDLE_INTERVAL     200    /* Idle animation frame interval */
#define ANIM_WALK_INTERVAL     80     /* Walk animation frame interval */
#define ANIM_RUN_INTERVAL      60     /* Run animation frame interval */
#define ANIM_ATTACK_INTERVAL   50     /* Attack animation frame interval */

/*
 * Initialize animation system
 */
int anim_init(void) {
    memset(&g_anim, 0, sizeof(AnimationContext));

    g_anim.global_speed = 100;

    /* Set default animation IDs */
    g_anim.default_anims[ANIM_TYPE_IDLE] = 1;
    g_anim.default_anims[ANIM_TYPE_WALK] = 2;
    g_anim.default_anims[ANIM_TYPE_RUN] = 3;
    g_anim.default_anims[ANIM_TYPE_ATTACK] = 4;
    g_anim.default_anims[ANIM_TYPE_SKILL] = 5;
    g_anim.default_anims[ANIM_TYPE_DEATH] = 6;

    LOG_INFO("Animation system initialized");
    return 1;
}

/*
 * Shutdown animation system
 */
void anim_shutdown(void) {
    if (g_anim.anim_db) {
        free(g_anim.anim_db);
    }

    memset(&g_anim, 0, sizeof(AnimationContext));
    LOG_INFO("Animation system shutdown");
}

/*
 * Load animation database
 */
int anim_load_database(void) {
    FILE* fp;
    char path[MAX_PATH];
    u32 i, j, k;
    u16 magic;
    u16 anim_count;
    AnimationData* anim;

    /* Build path to animation data file */
    GetModuleFileNameA(NULL, path, MAX_PATH);
    {
        char* last_slash = strrchr(path, '\\');
        if (last_slash) {
            strcpy(last_slash + 1, "data\\anim.bin");
        } else {
            strcpy(path, "data\\anim.bin");
        }
    }

    fp = fopen(path, "rb");
    if (!fp) {
        LOG_DEBUG("Animation database file not found, using defaults");
        /* Create default animation entries */
        g_anim.anim_db = (AnimationData*)malloc(MAX_ANIMATIONS * sizeof(AnimationData));
        if (!g_anim.anim_db) return 0;

        memset(g_anim.anim_db, 0, MAX_ANIMATIONS * sizeof(AnimationData));
        g_anim.db_count = MAX_ANIMATIONS;
        g_anim.db_capacity = MAX_ANIMATIONS;

        /* Initialize default animations for each type */
        for (i = 0; i < 14; i++) {
            anim = &g_anim.anim_db[i + 1];
            anim->id = (u16)(i + 1);
            anim->type = (AnimationType)i;
            anim->frame_count = 8;
            anim->direction_count = 8;
            anim->total_duration = 800;  /* 100ms per frame */
            anim->flags = (i == ANIM_TYPE_IDLE) ? ANIM_FLAG_LOOP : ANIM_FLAG_NONE;

            /* Setup frames for each direction */
            for (j = 0; j < 8; j++) {
                for (k = 0; k < 8; k++) {
                    anim->frames[k][j].sprite_id = (u16)(0x20000 + i * 100 + k * 10 + j);
                    anim->frames[k][j].duration = 100;
                    anim->frames[k][j].offset_x = 0;
                    anim->frames[k][j].offset_y = 0;
                    anim->frames[k][j].event_type = 0;
                    anim->frames[k][j].event_param = 0;
                }
            }
        }

        LOG_INFO("Created %d default animations", 14);
        return 1;
    }

    /* Read header */
    if (fread(&magic, 2, 1, fp) != 1 || magic != 0x414E) {  /* "NA" */
        LOG_WARN("Invalid animation database file");
        fclose(fp);
        return 0;
    }

    if (fread(&anim_count, 2, 1, fp) != 1) {
        fclose(fp);
        return 0;
    }

    if (anim_count > MAX_ANIMATIONS) {
        anim_count = MAX_ANIMATIONS;
    }

    /* Allocate database */
    g_anim.anim_db = (AnimationData*)malloc(anim_count * sizeof(AnimationData));
    if (!g_anim.anim_db) {
        fclose(fp);
        return 0;
    }

    memset(g_anim.anim_db, 0, anim_count * sizeof(AnimationData));
    g_anim.db_capacity = anim_count;

    /* Read animation entries */
    for (i = 0; i < anim_count; i++) {
        anim = &g_anim.anim_db[i];

        /* Read header */
        fread(&anim->id, 2, 1, fp);
        fread(anim->name, 1, 24, fp);
        fread(&anim->type, 2, 1, fp);
        fread(&anim->total_duration, 2, 1, fp);
        fread(&anim->frame_count, 2, 1, fp);
        fread(&anim->direction_count, 1, 1, fp);
        fread(&anim->flags, 1, 1, fp);

        /* Read frames for each direction */
        for (j = 0; j < anim->direction_count && j < 8; j++) {
            for (k = 0; k < anim->frame_count && k < MAX_ANIM_FRAMES; k++) {
                fread(&anim->frames[k][j].sprite_id, 2, 1, fp);
                fread(&anim->frames[k][j].duration, 2, 1, fp);
                fread(&anim->frames[k][j].offset_x, 2, 1, fp);
                fread(&anim->frames[k][j].offset_y, 2, 1, fp);
                fread(&anim->frames[k][j].sound_id, 2, 1, fp);
                fread(&anim->frames[k][j].event_type, 1, 1, fp);
                fread(&anim->frames[k][j].event_param, 1, 1, fp);
            }
        }

        /* Read layers */
        fread(&anim->layer_count, 1, 1, fp);
        for (j = 0; j < anim->layer_count && j < MAX_ANIM_LAYERS; j++) {
            fread(&anim->layers[j].z_order, 2, 1, fp);
            fread(&anim->layers[j].blend_mode, 1, 1, fp);
            fread(&anim->layers[j].alpha, 1, 1, fp);
            fread(&anim->layers[j].offset_x, 2, 1, fp);
            fread(&anim->layers[j].offset_y, 2, 1, fp);
        }

        /* Read transitions */
        fread(&anim->next_anim, 2, 1, fp);
        fread(&anim->transition_frame, 2, 1, fp);

        g_anim.db_count++;
    }

    fclose(fp);
    LOG_INFO("Loaded %d animations from database", g_anim.db_count);
    return 1;
}

/*
 * Get animation by ID
 */
AnimationData* anim_get(u16 anim_id) {
    int i;

    if (!g_anim.anim_db) return NULL;

    for (i = 0; i < g_anim.db_count; i++) {
        if (g_anim.anim_db[i].id == anim_id) {
            return &g_anim.anim_db[i];
        }
    }

    return NULL;
}

/*
 * Get animation by name
 */
AnimationData* anim_get_by_name(const char* name) {
    int i;

    if (!g_anim.anim_db || !name) return NULL;

    for (i = 0; i < g_anim.db_count; i++) {
        if (strcmp(g_anim.anim_db[i].name, name) == 0) {
            return &g_anim.anim_db[i];
        }
    }

    return NULL;
}

/*
 * Get default animation for type
 */
AnimationData* anim_get_default(AnimationType type) {
    if (type < 14) {
        return anim_get(g_anim.default_anims[type]);
    }
    return NULL;
}

/*
 * Play animation on entity
 */
int anim_play(u32 entity_id, u16 anim_id, u8 direction) {
    ActiveAnimation* active;
    int slot = -1;
    int i;

    /* Find existing or free slot */
    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            slot = i;
            break;
        }
        if (g_anim.active[i].entity_id == 0 && slot < 0) {
            slot = i;
        }
    }

    if (slot < 0) {
        if (g_anim.active_count < MAX_ANIMATIONS) {
            slot = g_anim.active_count++;
        } else {
            return 0;
        }
    }

    active = &g_anim.active[slot];
    active->anim_id = anim_id;
    active->entity_id = entity_id;
    active->current_dir = direction % 8;
    active->current_frame = 0;
    active->elapsed = 0;
    active->total_elapsed = 0;
    active->playing = 1;
    active->loop_count = 0;
    active->pingpong_forward = 1;
    active->speed = 100;
    active->on_complete = NULL;
    active->on_frame_event = NULL;

    return 1;
}

/*
 * Play animation type
 */
int anim_play_type(u32 entity_id, AnimationType type, u8 direction) {
    if (type >= 14) return 0;

    u16 anim_id = g_anim.default_anims[type];
    if (anim_id == 0) return 0;

    return anim_play(entity_id, anim_id, direction);
}

/*
 * Play looping animation
 */
int anim_play_loop(u32 entity_id, u16 anim_id, u8 direction, int loop_count) {
    if (!anim_play(entity_id, anim_id, direction)) {
        return 0;
    }

    /* Find the active animation */
    int i;
    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            g_anim.active[i].loop_count = loop_count > 0 ? loop_count : -1;
            return 1;
        }
    }

    return 0;
}

/*
 * Stop animation
 */
int anim_stop(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            memset(&g_anim.active[i], 0, sizeof(ActiveAnimation));

            /* Compact if last */
            if (i == g_anim.active_count - 1) {
                g_anim.active_count--;
            }

            return 1;
        }
    }

    return 0;
}

/*
 * Check if animation is playing
 */
int anim_is_playing(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id &&
            g_anim.active[i].playing) {
            return 1;
        }
    }

    return 0;
}

/*
 * Get frame interval based on animation type
 */
u32 anim_get_frame_interval(AnimationType type) {
    switch (type) {
        case ANIM_TYPE_IDLE:
            return ANIM_IDLE_INTERVAL;
        case ANIM_TYPE_WALK:
            return ANIM_WALK_INTERVAL;
        case ANIM_TYPE_RUN:
            return ANIM_RUN_INTERVAL;
        case ANIM_TYPE_ATTACK:
        case ANIM_TYPE_SKILL:
        case ANIM_TYPE_CAST:
            return ANIM_ATTACK_INTERVAL;
        default:
            return ANIM_FRAME_INTERVAL;
    }
}
