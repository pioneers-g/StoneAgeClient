/*
 * Stone Age Client - Scene Manager Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "scene.h"
#include "logger.h"

/* Global scene context */
SceneContext g_scene = {0};

/*
 * Initialize scene manager
 */
int scene_init(void) {
    memset(&g_scene, 0, sizeof(SceneContext));

    g_scene.current_scene = SCENE_NONE;
    g_scene.next_scene = SCENE_NONE;
    g_scene.transitioning = 0;

    LOG_INFO("Scene manager initialized");
    return 1;
}

/*
 * Shutdown scene manager
 */
void scene_shutdown(void) {
    /* Exit current scene */
    if (g_scene.current_scene != SCENE_NONE && g_scene.scenes[g_scene.current_scene].exit) {
        g_scene.scenes[g_scene.current_scene].exit();
    }

    memset(&g_scene, 0, sizeof(SceneContext));
    LOG_INFO("Scene manager shutdown");
}

/*
 * Register a scene
 */
int scene_register(SceneID id, Scene* scene) {
    if (id >= MAX_SCENES || !scene) {
        return 0;
    }

    memcpy(&g_scene.scenes[id], scene, sizeof(Scene));
    g_scene.scene_count++;

    LOG_DEBUG("Registered scene: %s (id=%d)", scene->name, id);
    return 1;
}

/*
 * Unregister a scene
 */
int scene_unregister(SceneID id) {
    if (id >= MAX_SCENES) {
        return 0;
    }

    memset(&g_scene.scenes[id], 0, sizeof(Scene));
    g_scene.scene_count--;

    return 1;
}

/*
 * Change scene immediately
 */
int scene_change(SceneID id) {
    Scene* old_scene;
    Scene* new_scene;

    if (id >= MAX_SCENES || !g_scene.scenes[id].enter) {
        LOG_ERROR("Invalid scene: %d", id);
        return 0;
    }

    LOG_INFO("Changing scene: %d -> %d", g_scene.current_scene, id);

    /* Exit current scene */
    if (g_scene.current_scene != SCENE_NONE) {
        old_scene = &g_scene.scenes[g_scene.current_scene];
        if (old_scene->exit) {
            old_scene->exit();
        }
    }

    /* Enter new scene */
    new_scene = &g_scene.scenes[id];
    g_scene.current_scene = id;
    g_scene.scene_start_time = timeGetTime();

    if (new_scene->enter) {
        new_scene->enter();
    }

    return 1;
}

/*
 * Request scene change (deferred)
 */
int scene_request_change(SceneID id) {
    if (id >= MAX_SCENES) {
        return 0;
    }

    g_scene.next_scene = id;
    g_scene.transitioning = 1;

    return 1;
}

/*
 * Update scene manager
 */
void scene_update(u32 delta_time) {
    Scene* scene;

    /* Process pending scene change */
    if (g_scene.transitioning) {
        scene_change(g_scene.next_scene);
        g_scene.transitioning = 0;
        g_scene.next_scene = SCENE_NONE;
    }

    /* Update current scene */
    if (g_scene.current_scene != SCENE_NONE) {
        scene = &g_scene.scenes[g_scene.current_scene];
        if (scene->update) {
            scene->update(delta_time);
        }
    }
}

/*
 * Render current scene
 */
void scene_render(void) {
    Scene* scene;

    if (g_scene.current_scene != SCENE_NONE) {
        scene = &g_scene.scenes[g_scene.current_scene];
        if (scene->render) {
            scene->render();
        }
    }
}

/*
 * Handle input for current scene
 */
void scene_handle_input(u32 msg, u32 wparam, u32 lparam) {
    Scene* scene;

    if (g_scene.current_scene != SCENE_NONE) {
        scene = &g_scene.scenes[g_scene.current_scene];
        if (scene->handle_input) {
            scene->handle_input(msg, wparam, lparam);
        }
    }
}

/*
 * Get current scene ID
 */
SceneID scene_get_current(void) {
    return g_scene.current_scene;
}

/*
 * Get current scene pointer
 */
Scene* scene_get_current_scene(void) {
    if (g_scene.current_scene == SCENE_NONE) {
        return NULL;
    }
    return &g_scene.scenes[g_scene.current_scene];
}

/*
 * Get scene by ID
 */
Scene* scene_get(SceneID id) {
    if (id >= MAX_SCENES) {
        return NULL;
    }
    return &g_scene.scenes[id];
}

/*
 * Check if in transition
 */
int scene_is_transitioning(void) {
    return g_scene.transitioning;
}

/*
 * Get scene time (time spent in current scene)
 */
u32 scene_get_time(void) {
    return timeGetTime() - g_scene.scene_start_time;
}

/*
 * Set scene data
 */
void scene_set_data(void* data) {
    g_scene.scene_data = data;
}

/*
 * Get scene data
 */
void* scene_get_data(void) {
    return g_scene.scene_data;
}

/*
 * Get scene count
 */
int scene_get_count(void) {
    return g_scene.scene_count;
}

/*
 * Pause current scene
 */
void scene_pause(void) {
    Scene* scene;

    if (g_scene.current_scene != SCENE_NONE) {
        scene = &g_scene.scenes[g_scene.current_scene];
        if (scene->pause) {
            scene->pause();
        }
        g_scene.paused = 1;
    }
}

/*
 * Resume current scene
 */
void scene_resume(void) {
    Scene* scene;

    if (g_scene.current_scene != SCENE_NONE && g_scene.paused) {
        scene = &g_scene.scenes[g_scene.current_scene];
        if (scene->resume) {
            scene->resume();
        }
        g_scene.paused = 0;
    }
}

/*
 * Check if scene is paused
 */
int scene_is_paused(void) {
    return g_scene.paused;
}

/*
 * Scene name to ID
 */
SceneID scene_name_to_id(const char* name) {
    int i;

    for (i = 0; i < MAX_SCENES; i++) {
        if (g_scene.scenes[i].name[0] != '\0' &&
            strcmp(g_scene.scenes[i].name, name) == 0) {
            return i;
        }
    }

    return SCENE_NONE;
}

/*
 * Scene ID to name
 */
const char* scene_id_to_name(SceneID id) {
    if (id >= MAX_SCENES) {
        return "Unknown";
    }
    return g_scene.scenes[id].name;
}

/*
 * Push scene (for overlay)
 */
int scene_push(SceneID id) {
    if (g_scene.stack_count >= MAX_SCENE_STACK) {
        return 0;
    }

    /* Pause current scene */
    if (g_scene.current_scene != SCENE_NONE) {
        Scene* scene = &g_scene.scenes[g_scene.current_scene];
        if (scene->pause) {
            scene->pause();
        }
    }

    /* Push to stack */
    g_scene.scene_stack[g_scene.stack_count++] = g_scene.current_scene;

    /* Change to new scene */
    scene_change(id);

    return 1;
}

/*
 * Pop scene
 */
int scene_pop(void) {
    SceneID prev_scene;

    if (g_scene.stack_count == 0) {
        return 0;
    }

    /* Exit current scene */
    if (g_scene.current_scene != SCENE_NONE) {
        Scene* scene = &g_scene.scenes[g_scene.current_scene];
        if (scene->exit) {
            scene->exit();
        }
    }

    /* Pop from stack */
    prev_scene = g_scene.scene_stack[--g_scene.stack_count];
    g_scene.current_scene = prev_scene;

    /* Resume previous scene */
    if (prev_scene != SCENE_NONE) {
        Scene* scene = &g_scene.scenes[prev_scene];
        if (scene->resume) {
            scene->resume();
        }
    }

    return 1;
}

/*
 * Get stack depth
 */
int scene_get_stack_depth(void) {
    return g_scene.stack_count;
}

/*
 * Clear scene stack
 */
void scene_clear_stack(void) {
    g_scene.stack_count = 0;
}
