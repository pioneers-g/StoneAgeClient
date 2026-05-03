/*
 * Stone Age Client - Scene Manager Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef SCENE_H
#define SCENE_H

#include "types.h"

/* Constants */
#define MAX_SCENES         16
#define MAX_SCENE_STACK    8

/* Scene IDs */
typedef enum {
    SCENE_NONE = 0,
    SCENE_LOGIN = 1,
    SCENE_CHARACTER_SELECT = 2,
    SCENE_GAME = 3,
    SCENE_BATTLE = 4,
    SCENE_LOADING = 5,
    SCENE_OPTIONS = 6,
    SCENE_SHOP = 7,
    SCENE_INVENTORY = 8,
    SCENE_SKILL = 9,
    SCENE_QUEST = 10,
    SCENE_GUILD = 11,
    SCENE_MAIL = 12,
    SCENE_PARTY = 13,
    SCENE_TRADE = 14,
    SCENE_CUSTOM_START = 100
} SceneID;

/* Scene callback types */
typedef void (*SceneEnterFunc)(void);
typedef void (*SceneExitFunc)(void);
typedef void (*SceneUpdateFunc)(u32 delta_time);
typedef void (*SceneRenderFunc)(void);
typedef void (*SceneInputFunc)(u32 msg, u32 wparam, u32 lparam);
typedef void (*ScenePauseFunc)(void);
typedef void (*SceneResumeFunc)(void);

/* Scene structure */
typedef struct {
    char name[32];
    SceneEnterFunc enter;
    SceneExitFunc exit;
    SceneUpdateFunc update;
    SceneRenderFunc render;
    SceneInputFunc handle_input;
    ScenePauseFunc pause;
    SceneResumeFunc resume;
    void* user_data;

} Scene;

/* Scene context */
typedef struct {
    /* Registered scenes */
    Scene scenes[MAX_SCENES];
    int scene_count;

    /* Current scene */
    SceneID current_scene;
    SceneID next_scene;
    u32 scene_start_time;
    void* scene_data;

    /* Transition */
    int transitioning;

    /* Pause state */
    int paused;

    /* Scene stack (for overlays) */
    SceneID scene_stack[MAX_SCENE_STACK];
    int stack_count;

} SceneContext;

/* Global scene context */
extern SceneContext g_scene;

/* Initialization */
int scene_init(void);
void scene_shutdown(void);

/* Scene registration */
int scene_register(SceneID id, Scene* scene);
int scene_unregister(SceneID id);

/* Scene management */
int scene_change(SceneID id);
int scene_request_change(SceneID id);
void scene_update(u32 delta_time);
void scene_render(void);
void scene_handle_input(u32 msg, u32 wparam, u32 lparam);

/* Query */
SceneID scene_get_current(void);
Scene* scene_get_current_scene(void);
Scene* scene_get(SceneID id);
int scene_is_transitioning(void);
u32 scene_get_time(void);
int scene_get_count(void);

/* Scene data */
void scene_set_data(void* data);
void* scene_get_data(void);

/* Pause/resume */
void scene_pause(void);
void scene_resume(void);
int scene_is_paused(void);

/* Scene stack (for overlays/menus) */
int scene_push(SceneID id);
int scene_pop(void);
int scene_get_stack_depth(void);
void scene_clear_stack(void);

/* Utility */
SceneID scene_name_to_id(const char* name);
const char* scene_id_to_name(SceneID id);

#endif /* SCENE_H */
