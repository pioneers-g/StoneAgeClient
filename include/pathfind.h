/*
 * Stone Age Client - Auto Pathfinding System Header
 * Reverse engineered from sa_9061.exe
 * FUN_00443fe0 - Main path calculation with obstacle avoidance
 * FUN_00443e80 - Direction to delta conversion
 * FUN_00447150 - Angle calculation
 * FUN_00441ff0 - Walkability check
 */

#ifndef PATHFIND_H
#define PATHFIND_H

#include "types.h"

/* Constants */
#define MAX_PATH_LENGTH     256
#define MAX_WAYPOINTS       20
#define PATHFIND_GRID_SIZE  16
#define MAX_PATH_STEPS      100     /* From DAT_045829e4 limit in FUN_00443fe0 */

/* Pathfind status */
typedef enum {
    PATH_STATUS_IDLE = 0,
    PATH_STATUS_SEARCHING = 1,
    PATH_STATUS_MOVING = 2,
    PATH_STATUS_COMPLETE = 3,
    PATH_STATUS_BLOCKED = 4,
    PATH_STATUS_FAILED = 5
} PathStatus;

/* Path node */
typedef struct {
    u16 x;
    u16 y;
    u8 dir;         /* Direction to next node */
    u8 reserved;
} PathNode;

/* Waypoint for multi-destination paths */
typedef struct {
    u16 x;
    u16 y;
    u32 map_id;
    u8 type;        /* Waypoint type (NPC, item, etc) */
    u8 param;       /* Additional parameter */
} Waypoint;

/* Map link (for cross-map pathfinding) */
typedef struct {
    u16 from_x;
    u16 from_y;
    u32 from_map;
    u16 to_x;
    u16 to_y;
    u32 to_map;
    char link_name[32];
} MapLink;

/* Pathfind context */
typedef struct {
    /* Current path */
    PathNode path[MAX_PATH_LENGTH];
    int path_length;
    int current_node;

    /* Destination */
    u16 dest_x;
    u16 dest_y;
    u32 dest_map;

    /* Current position */
    u16 start_x;
    u16 start_y;
    u32 current_map;

    /* Status */
    PathStatus status;
    u8 auto_move;       /* Auto-move along path */
    u8 avoid_combat;    /* Avoid combat while moving */
    u32 last_update;
    u32 move_delay;

    /* Waypoints */
    Waypoint waypoints[MAX_WAYPOINTS];
    int waypoint_count;
    int current_waypoint;

    /* Map links database */
    MapLink* map_links;
    int link_count;

    /* Path cache */
    u32 cached_map;
    u16 cached_dest_x;
    u16 cached_dest_y;
    PathNode cached_path[MAX_PATH_LENGTH];
    int cached_path_length;

    /* Statistics */
    u32 total_distance;
    u32 estimated_time;
    u32 path_requests;
    u32 moves_sent;

} PathfindContext;

/* Global pathfind context */
extern PathfindContext g_pathfind;

/* Initialization */
int pathfind_init(void);
void pathfind_shutdown(void);

/* Main pathfinding */
int pathfind_to(u16 dest_x, u16 dest_y, u32 map_id);
int pathfind_to_waypoint(const Waypoint* waypoint);
int pathfind_to_npc(u32 npc_id);
int pathfind_to_player(const char* name);
int pathfind_to_coord(const char* coord_str);  /* "map,x,y" format */

/* Cross-map pathfinding */
int pathfind_cross_map(u32 from_map, u32 to_map);
int pathfind_add_waypoint(u16 x, u16 y, u32 map_id, u8 type);
void pathfind_clear_waypoints(void);

/* Update */
void pathfind_update(u16 current_x, u16 current_y);
PathStatus pathfind_get_status(void);
int pathfind_has_path(void);

/* Movement */
int pathfind_get_next_direction(void);
int pathfind_get_next_node(PathNode* node);
void pathfind_advance_node(void);
int pathfind_get_remaining_distance(void);

/* Cancel */
void pathfind_stop(void);
void pathfind_cancel(void);

/* Get path */
int pathfind_get_path(PathNode* path, int max_length);
int pathfind_get_length(void);
int pathfind_get_current_index(void);

/* Cache */
void pathfind_cache_path(void);
int pathfind_use_cached(u16 dest_x, u16 dest_y, u32 map_id);

/* Map links */
int pathfind_load_map_links(void);
MapLink* pathfind_find_link(u32 from_map, u32 to_map);
MapLink* pathfind_find_nearest_link(u32 map_id, u16 x, u16 y, u32 to_map);

/* Utility */
int pathfind_is_valid_pos(u16 x, u16 y, u32 map_id);
int pathfind_is_walkable(u16 x, u16 y);
u16 pathfind_distance(u16 x1, u16 y1, u16 x2, u16 y2);
u32 pathfind_estimate_time(u16 distance);

/* A* algorithm */
int pathfind_astar(u16 start_x, u16 start_y, u16 dest_x, u16 dest_y);

/* Query */
u16 pathfind_get_dest_x(void);
u16 pathfind_get_dest_y(void);
int pathfind_is_moving(void);

/* ========================================
 * Character Movement Path System
 * From FUN_00443fe0 analysis
 * ======================================== */

/* Movement path step - stored in arrays */
typedef struct {
    s16 direction;          /* Direction 0-7 */
    s16 x;                  /* Step X coordinate */
    s16 y;                  /* Step Y coordinate */
} MovementStep;

/* Movement path context - matches DAT_045811b4, DAT_04582804, DAT_045828cc regions */
typedef struct {
    MovementStep steps[MAX_PATH_STEPS];
    u32 step_count;                 /* DAT_045829e4 */
    u16 current_step;
    s16 last_direction;             /* DAT_004bb44c - last calculated direction */
} MovementPath;

/* Global movement path */
extern MovementPath g_movement_path;

/* Direction constants - matching FUN_00443e80
 *
 * TODO: The naming of these constants is INCORRECT!
 * Based on binary FUN_00443e80 analysis:
 *   Direction 0: dx=-1, dy=1  -> This is SOUTHWEST (not NORTH)
 *   Direction 1: dx=-1, dy=0  -> This is WEST (not NORTHWEST)
 *   Direction 2: dx=-1, dy=-1 -> This is NORTHWEST (not WEST)
 *   Direction 3: dx=0, dy=-1  -> This is NORTH (not SOUTHWEST)
 *   Direction 4: dx=1, dy=-1  -> This is NORTHEAST (not SOUTH)
 *   Direction 5: dx=1, dy=0   -> This is EAST (not SOUTHEAST)
 *   Direction 6: dx=1, dy=1   -> This is SOUTHEAST (not EAST)
 *   Direction 7: dx=0, dy=1   -> This is SOUTH (not NORTHEAST)
 *
 * FIX: Rename these constants to match actual directions.
 * The dx/dy values in comments are correct, but the names are wrong.
 */
#define MOVE_DIR_NORTH          0   /* dx=-1, dy=1 - WRONG NAME: actually Southwest */
#define MOVE_DIR_NORTHWEST      1   /* dx=-1, dy=0 - WRONG NAME: actually West */
#define MOVE_DIR_WEST           2   /* dx=-1, dy=-1 - WRONG NAME: actually Northwest */
#define MOVE_DIR_SOUTHWEST      3   /* dx=0, dy=-1 - WRONG NAME: actually North */
#define MOVE_DIR_SOUTH          4   /* dx=1, dy=-1 - WRONG NAME: actually Northeast */
#define MOVE_DIR_SOUTHEAST      5   /* dx=1, dy=0 - WRONG NAME: actually East */
#define MOVE_DIR_EAST           6   /* dx=1, dy=1 - WRONG NAME: actually Southeast */
#define MOVE_DIR_NORTHEAST      7   /* dx=0, dy=1 - WRONG NAME: actually South */

/* Direction to delta conversion - FUN_00443e80 */
void direction_get_delta(u32 direction, s32* dx, s32* dy);

/* Angle calculation - FUN_00447150 */
float calculate_angle(float dx, float dy);

/* Angle normalization - FUN_004470d0 */
void normalize_angle(float* angle);

/* Direction from delta - FUN_00443f80 */
u32 calculate_direction_from_delta(s32 from_x, s32 from_y, s32 to_x, s32 to_y);

/* Walkability check - FUN_00441ff0 */
int is_tile_walkable(s32 x, s32 y);

/* Main path calculation - FUN_00443fe0 */
int calculate_movement_path(s32 start_x, s32 start_y, s32 dest_x, s32 dest_y);

/* Get next step in movement path */
int movement_path_get_next(s16* direction, s16* x, s16* y);

/* Clear movement path */
void movement_path_clear(void);

/* Check if movement path is active */
int movement_path_is_active(void);

/* Initialize movement path system */
int movement_path_init(void);

#endif /* PATHFIND_H */
