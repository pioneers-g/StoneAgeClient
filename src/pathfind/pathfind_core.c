/*
 * Stone Age Client - Pathfinding Core Module
 * Main pathfinding requests, state management, map links
 * Split from pathfind.c (1361 lines -> ~500 lines)
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "pathfind.h"
#include "pathfind_internal.h"
#include "map.h"
#include "character.h"
#include "party.h"
#include "network_protocol.h"
#include "logger.h"

/* Global pathfind context */
PathfindContext g_pathfind = {0};

/* Direction deltas for A* - WARNING: These are INCORRECT!
 * See pathfind_movement.c for correct values (s_move_dx/s_move_dy)
 * TODO: Fix this table to match binary FUN_00443e80
 */
const s16 s_dir_dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
const s16 s_dir_dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

/* A* node pool */
PathfindNode s_nodes[MAX_PATHFIND_NODES];
int s_node_count;

/* Open list for A* */
int s_open_list[MAX_PATHFIND_NODES];
int s_open_count;

/* Node grid lookup */
int s_node_grid[PATHFIND_GRID_MAX][PATHFIND_GRID_MAX];

/*
 * Initialize pathfinding system
 */
int pathfind_init(void) {
    memset(&g_pathfind, 0, sizeof(PathfindContext));

    g_pathfind.status = PATH_STATUS_IDLE;
    g_pathfind.move_delay = 200;  /* 200ms between moves */

    pathfind_load_map_links();

    LOG_INFO("Pathfinding system initialized");
    return 1;
}

/*
 * Shutdown pathfinding system
 */
void pathfind_shutdown(void) {
    if (g_pathfind.map_links) {
        free(g_pathfind.map_links);
    }

    memset(&g_pathfind, 0, sizeof(PathfindContext));
    LOG_INFO("Pathfinding system shutdown");
}

/*
 * Find path to destination
 */
int pathfind_to(u16 dest_x, u16 dest_y, u32 map_id) {
    u16 start_x, start_y;
    Character* player;

    player = character_get_player();
    if (player) {
        start_x = player->x;
        start_y = player->y;
    } else {
        start_x = 0;
        start_y = 0;
    }

    g_pathfind.start_x = start_x;
    g_pathfind.start_y = start_y;
    g_pathfind.dest_x = dest_x;
    g_pathfind.dest_y = dest_y;
    g_pathfind.dest_map = map_id;
    g_pathfind.current_map = map_id;

    if (start_x == dest_x && start_y == dest_y) {
        g_pathfind.status = PATH_STATUS_COMPLETE;
        return 1;
    }

    if (!pathfind_astar(start_x, start_y, dest_x, dest_y)) {
        g_pathfind.status = PATH_STATUS_FAILED;
        return 0;
    }

    g_pathfind.current_node = 0;
    g_pathfind.status = PATH_STATUS_MOVING;
    g_pathfind.auto_move = 1;
    g_pathfind.last_update = timeGetTime();

    g_pathfind.path_requests++;

    LOG_DEBUG("Path found: (%d,%d) -> (%d,%d), length=%d",
              start_x, start_y, dest_x, dest_y, g_pathfind.path_length);

    return 1;
}

/*
 * Find path to waypoint
 */
int pathfind_to_waypoint(const Waypoint* waypoint) {
    if (!waypoint) return 0;
    return pathfind_to(waypoint->x, waypoint->y, waypoint->map_id);
}

/*
 * Find path to NPC
 */
int pathfind_to_npc(u32 npc_id) {
    MapNPC* npc;
    u16 npc_x, npc_y;

    npc = npc_get_by_id(npc_id);
    if (npc) {
        npc_x = (u16)npc->x;
        npc_y = (u16)npc->y;
        return pathfind_to(npc_x, npc_y, g_pathfind.current_map);
    }

    {
        NPC* legacy_npc = npc_find_by_id(npc_id);
        if (legacy_npc) {
            return pathfind_to(legacy_npc->x, legacy_npc->y, g_pathfind.current_map);
        }
    }

    LOG_WARN("NPC %u not found for pathfinding", npc_id);
    return 0;
}

/*
 * Find path to player
 */
int pathfind_to_player(const char* name) {
    PartyMember* member;
    Character* ch;
    u32 i;

    if (!name || !name[0]) return 0;

    member = party_find_member_by_name(name);
    if (member) {
        return pathfind_to(member->x, member->y, g_pathfind.current_map);
    }

    for (i = 0; i < g_chars.character_count; i++) {
        ch = &g_chars.characters[i];
        if (ch->visible && _stricmp(ch->name, name) == 0) {
            return pathfind_to(ch->x, ch->y, g_pathfind.current_map);
        }
    }

    LOG_WARN("Player '%s' not found for pathfinding", name);
    return 0;
}

/*
 * Parse coordinate string and find path
 */
int pathfind_to_coord(const char* coord_str) {
    u32 map_id;
    u16 x, y;

    if (!coord_str) return 0;

    if (sscanf(coord_str, "%u,%hu,%hu", &map_id, &x, &y) == 3) {
        return pathfind_to(x, y, map_id);
    }

    return 0;
}

/*
 * Cross-map pathfinding
 */
int pathfind_cross_map(u32 from_map, u32 to_map) {
    MapLink* link;

    if (from_map == to_map) {
        return 1;
    }

    link = pathfind_find_link(from_map, to_map);
    if (!link) {
        LOG_WARN("No path from map %u to %u", from_map, to_map);
        return 0;
    }

    pathfind_add_waypoint(link->to_x, link->to_y, link->to_map, 0);

    return pathfind_to(link->from_x, link->from_y, from_map);
}

/*
 * Add waypoint
 */
int pathfind_add_waypoint(u16 x, u16 y, u32 map_id, u8 type) {
    if (g_pathfind.waypoint_count >= MAX_WAYPOINTS) {
        return 0;
    }

    g_pathfind.waypoints[g_pathfind.waypoint_count].x = x;
    g_pathfind.waypoints[g_pathfind.waypoint_count].y = y;
    g_pathfind.waypoints[g_pathfind.waypoint_count].map_id = map_id;
    g_pathfind.waypoints[g_pathfind.waypoint_count].type = type;

    g_pathfind.waypoint_count++;
    return 1;
}

/*
 * Clear waypoints
 */
void pathfind_clear_waypoints(void) {
    g_pathfind.waypoint_count = 0;
    g_pathfind.current_waypoint = 0;
}

/*
 * Update pathfinding
 */
void pathfind_update(u16 current_x, u16 current_y) {
    u32 current_time;
    PathNode* node;

    if (g_pathfind.status != PATH_STATUS_MOVING) {
        return;
    }

    current_time = timeGetTime();

    if (current_time - g_pathfind.last_update < g_pathfind.move_delay) {
        return;
    }

    node = &g_pathfind.path[g_pathfind.current_node];

    if (current_x == node->x && current_y == node->y) {
        g_pathfind.current_node++;

        if (g_pathfind.current_node >= g_pathfind.path_length) {
            g_pathfind.status = PATH_STATUS_COMPLETE;
            LOG_DEBUG("Path complete");
            return;
        }
    }

    if (g_pathfind.auto_move) {
        int dir = pathfind_get_next_direction();
        if (dir >= 0) {
            char move_cmd[32];
            _snprintf(move_cmd, sizeof(move_cmd), "MV|%d", dir);
            send_queue_add(move_cmd, 0, 0);
            character_move(dir);
            g_pathfind.moves_sent++;
        }
    }

    g_pathfind.last_update = current_time;
}

/*
 * Get pathfinding status
 */
PathStatus pathfind_get_status(void) {
    return g_pathfind.status;
}

/*
 * Check if has active path
 */
int pathfind_has_path(void) {
    return g_pathfind.path_length > 0 &&
           g_pathfind.status == PATH_STATUS_MOVING;
}

/*
 * Get next direction to move
 */
int pathfind_get_next_direction(void) {
    PathNode* current;
    PathNode* next;
    s16 dx, dy;
    int i;

    if (g_pathfind.current_node >= g_pathfind.path_length - 1) {
        return -1;
    }

    current = &g_pathfind.path[g_pathfind.current_node];
    next = &g_pathfind.path[g_pathfind.current_node + 1];

    dx = (s16)next->x - (s16)current->x;
    dy = (s16)next->y - (s16)current->y;

    for (i = 0; i < 8; i++) {
        if (s_dir_dx[i] == dx && s_dir_dy[i] == dy) {
            return i;
        }
    }

    return -1;
}

/*
 * Get next node
 */
int pathfind_get_next_node(PathNode* node) {
    if (g_pathfind.current_node >= g_pathfind.path_length) {
        return 0;
    }

    *node = g_pathfind.path[g_pathfind.current_node];
    return 1;
}

/*
 * Advance to next node
 */
void pathfind_advance_node(void) {
    if (g_pathfind.current_node < g_pathfind.path_length - 1) {
        g_pathfind.current_node++;
    }
}

/*
 * Get remaining distance
 */
int pathfind_get_remaining_distance(void) {
    int distance = 0;
    int i;

    for (i = g_pathfind.current_node; i < g_pathfind.path_length - 1; i++) {
        distance += pathfind_distance(
            g_pathfind.path[i].x, g_pathfind.path[i].y,
            g_pathfind.path[i + 1].x, g_pathfind.path[i + 1].y);
    }

    return distance;
}

/*
 * Stop pathfinding
 */
void pathfind_stop(void) {
    g_pathfind.status = PATH_STATUS_IDLE;
    g_pathfind.auto_move = 0;
}

/*
 * Cancel pathfinding
 */
void pathfind_cancel(void) {
    pathfind_stop();
    g_pathfind.path_length = 0;
    g_pathfind.current_node = 0;
    pathfind_clear_waypoints();
}

/*
 * Get path
 */
int pathfind_get_path(PathNode* path, int max_length) {
    int i;
    int copy_count = g_pathfind.path_length < max_length ?
                     g_pathfind.path_length : max_length;

    for (i = 0; i < copy_count; i++) {
        path[i] = g_pathfind.path[i];
    }

    return copy_count;
}

/*
 * Get path length
 */
int pathfind_get_length(void) {
    return g_pathfind.path_length;
}

/*
 * Get current index
 */
int pathfind_get_current_index(void) {
    return g_pathfind.current_node;
}

/*
 * Cache current path
 */
void pathfind_cache_path(void) {
    g_pathfind.cached_map = g_pathfind.current_map;
    g_pathfind.cached_dest_x = g_pathfind.dest_x;
    g_pathfind.cached_dest_y = g_pathfind.dest_y;
    g_pathfind.cached_path_length = g_pathfind.path_length;

    memcpy(g_pathfind.cached_path, g_pathfind.path,
           g_pathfind.path_length * sizeof(PathNode));
}

/*
 * Use cached path
 */
int pathfind_use_cached(u16 dest_x, u16 dest_y, u32 map_id) {
    if (g_pathfind.cached_map != map_id) return 0;
    if (g_pathfind.cached_dest_x != dest_x) return 0;
    if (g_pathfind.cached_dest_y != dest_y) return 0;

    memcpy(g_pathfind.path, g_pathfind.cached_path,
           g_pathfind.cached_path_length * sizeof(PathNode));
    g_pathfind.path_length = g_pathfind.cached_path_length;

    return 1;
}

/*
 * Load map links from data file
 */
int pathfind_load_map_links(void) {
    FILE* fp;
    char line[256];
    char path[MAX_PATH];
    int count = 0;

    GetModuleFileNameA(NULL, path, MAX_PATH);
    {
        char* last_slash = strrchr(path, '\\');
        if (last_slash) {
            strcpy(last_slash + 1, "data\\maplinks.txt");
        } else {
            strcpy(path, "data\\maplinks.txt");
        }
    }

    fp = fopen(path, "r");
    if (!fp) {
        LOG_DEBUG("Map links file not found, using empty");
        g_pathfind.link_count = 0;
        return 1;
    }

    g_pathfind.map_links = (MapLink*)malloc(sizeof(MapLink) * 100);
    if (!g_pathfind.map_links) {
        fclose(fp);
        return 0;
    }

    while (fgets(line, sizeof(line), fp) && count < 100) {
        MapLink* link = &g_pathfind.map_links[count];
        int from_map, to_map;
        int from_x, from_y, to_x, to_y;

        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        if (sscanf(line, "%d,%d,%d,%d,%d,%d",
                   &from_map, &from_x, &from_y,
                   &to_map, &to_x, &to_y) == 6) {
            link->from_map = (u32)from_map;
            link->from_x = (u16)from_x;
            link->from_y = (u16)from_y;
            link->to_map = (u32)to_map;
            link->to_x = (u16)to_x;
            link->to_y = (u16)to_y;
            count++;
        }
    }

    fclose(fp);
    g_pathfind.link_count = count;

    LOG_INFO("Loaded %d map links", count);
    return 1;
}

/*
 * Find map link
 */
MapLink* pathfind_find_link(u32 from_map, u32 to_map) {
    int i;

    for (i = 0; i < g_pathfind.link_count; i++) {
        if (g_pathfind.map_links[i].from_map == from_map &&
            g_pathfind.map_links[i].to_map == to_map) {
            return &g_pathfind.map_links[i];
        }
    }

    return NULL;
}

/*
 * Find nearest map link
 */
MapLink* pathfind_find_nearest_link(u32 map_id, u16 x, u16 y, u32 to_map) {
    MapLink* nearest = NULL;
    u32 min_dist = 0xFFFFFFFF;
    int i;

    for (i = 0; i < g_pathfind.link_count; i++) {
        if (g_pathfind.map_links[i].from_map == map_id &&
            g_pathfind.map_links[i].to_map == to_map) {

            u32 dist = pathfind_distance(x, y,
                g_pathfind.map_links[i].from_x,
                g_pathfind.map_links[i].from_y);

            if (dist < min_dist) {
                min_dist = dist;
                nearest = &g_pathfind.map_links[i];
            }
        }
    }

    return nearest;
}

/*
 * Check if position is valid
 */
int pathfind_is_valid_pos(u16 x, u16 y, u32 map_id) {
    if (map_id != g_map.current_map_id) {
        return 1;
    }

    if (g_map.width > 0 && g_map.height > 0) {
        if (x >= g_map.width || y >= g_map.height) {
            return 0;
        }
    }

    return 1;
}

/*
 * Check if position is walkable
 */
int pathfind_is_walkable(u16 x, u16 y) {
    MapTile* tile = map_get_tile(x, y);
    if (!tile) {
        return 0;
    }

    return (tile->flags & 0x01) == 0;
}

/*
 * Calculate distance between two points
 */
u16 pathfind_distance(u16 x1, u16 y1, u16 x2, u16 y2) {
    s32 dx = abs((s32)x2 - (s32)x1);
    s32 dy = abs((s32)y2 - (s32)y1);

    return (u16)(dx > dy ? dx : dy);
}

/*
 * Estimate travel time
 */
u32 pathfind_estimate_time(u16 distance) {
    return distance * 200;
}

/*
 * Get destination X
 */
u16 pathfind_get_dest_x(void) {
    return g_pathfind.dest_x;
}

/*
 * Get destination Y
 */
u16 pathfind_get_dest_y(void) {
    return g_pathfind.dest_y;
}

/*
 * Check if currently moving
 */
int pathfind_is_moving(void) {
    return g_pathfind.status == PATH_STATUS_MOVING;
}
