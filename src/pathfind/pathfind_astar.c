/*
 * Stone Age Client - A* Pathfinding Algorithm
 * Implementation of A* pathfinding with open/closed lists
 * Split from pathfind.c (1361 lines -> ~250 lines)
 */

#include <windows.h>
#include <string.h>
#include "types.h"
#include "pathfind.h"
#include "pathfind_internal.h"
#include "logger.h"

/*
 * Reset node pool
 */
void pathfind_reset_nodes(void) {
    memset(s_nodes, 0, sizeof(s_nodes));
    memset(s_node_grid, -1, sizeof(s_node_grid));
    s_node_count = 0;
    s_open_count = 0;
}

/*
 * Get node index at position, create if needed
 */
int pathfind_get_node(u16 x, u16 y) {
    int idx;

    if (x >= PATHFIND_GRID_MAX || y >= PATHFIND_GRID_MAX) {
        return -1;
    }

    if (s_node_grid[y][x] >= 0) {
        return s_node_grid[y][x];
    }

    if (s_node_count >= MAX_PATHFIND_NODES) {
        return -1;
    }

    idx = s_node_count++;
    s_nodes[idx].x = x;
    s_nodes[idx].y = y;
    s_nodes[idx].g_cost = 0xFFFFFFFF;
    s_nodes[idx].h_cost = 0;
    s_nodes[idx].f_cost = 0xFFFFFFFF;
    s_nodes[idx].closed = 0;
    s_nodes[idx].in_open = 0;

    s_node_grid[y][x] = idx;
    return idx;
}

/*
 * Calculate heuristic (Chebyshev distance for 8-directional)
 */
u32 pathfind_heuristic(u16 x1, u16 y1, u16 x2, u16 y2) {
    s32 dx = abs((s32)x2 - (s32)x1);
    s32 dy = abs((s32)y2 - (s32)y1);
    return (u32)(dx > dy ? dx : dy);
}

/*
 * Add node to open list
 */
void pathfind_add_to_open(int idx) {
    if (s_nodes[idx].in_open) return;

    s_nodes[idx].in_open = 1;
    s_open_list[s_open_count++] = idx;
}

/*
 * Get node with lowest F cost from open list
 */
int pathfind_get_lowest_f(void) {
    u32 min_f = 0xFFFFFFFF;
    int min_idx = -1;
    int i, idx;

    for (i = 0; i < s_open_count; i++) {
        idx = s_open_list[i];
        if (!s_nodes[idx].closed && s_nodes[idx].f_cost < min_f) {
            min_f = s_nodes[idx].f_cost;
            min_idx = i;
        }
    }

    if (min_idx < 0) return -1;

    idx = s_open_list[min_idx];
    s_open_list[min_idx] = s_open_list[--s_open_count];
    return idx;
}

/*
 * Reconstruct path from destination
 */
int pathfind_reconstruct_path(int dest_idx) {
    PathfindNode* node = &s_nodes[dest_idx];
    int path[MAX_PATH_LENGTH];
    int path_len = 0;
    int idx;
    u16 cur_x, cur_y;

    cur_x = node->x;
    cur_y = node->y;

    while (path_len < MAX_PATH_LENGTH) {
        if (cur_x == g_pathfind.start_x && cur_y == g_pathfind.start_y) {
            break;
        }

        path[path_len++] = (cur_y << 16) | cur_x;

        idx = s_node_grid[cur_y][cur_x];
        if (idx < 0) break;

        cur_x = s_nodes[idx].parent_x;
        cur_y = s_nodes[idx].parent_y;
    }

    g_pathfind.path_length = path_len + 1;
    g_pathfind.path[0].x = g_pathfind.start_x;
    g_pathfind.path[0].y = g_pathfind.start_y;

    for (idx = 0; idx < path_len; idx++) {
        g_pathfind.path[path_len - idx].x = (path[idx] & 0xFFFF);
        g_pathfind.path[path_len - idx].y = (path[idx] >> 16);
    }

    return 1;
}

/*
 * Main A* implementation
 *
 * TODO: This uses s_dir_dx/s_dir_dy which has INCORRECT values!
 * Should use s_move_dx/s_move_dy from pathfind_movement.c
 */
int pathfind_astar(u16 start_x, u16 start_y, u16 dest_x, u16 dest_y) {
    int start_idx, current_idx, neighbor_idx;
    int dir;
    u16 nx, ny;
    u32 new_g, movement_cost;
    PathfindNode* current;
    PathfindNode* neighbor;

    pathfind_reset_nodes();

    g_pathfind.start_x = start_x;
    g_pathfind.start_y = start_y;
    g_pathfind.dest_x = dest_x;
    g_pathfind.dest_y = dest_y;

    if (!pathfind_is_walkable(dest_x, dest_y)) {
        LOG_DEBUG("Destination (%d,%d) is not walkable", dest_x, dest_y);
        return 0;
    }

    if (start_x == dest_x && start_y == dest_y) {
        g_pathfind.path[0].x = start_x;
        g_pathfind.path[0].y = start_y;
        g_pathfind.path_length = 1;
        return 1;
    }

    start_idx = pathfind_get_node(start_x, start_y);
    if (start_idx < 0) return 0;

    s_nodes[start_idx].g_cost = 0;
    s_nodes[start_idx].h_cost = pathfind_heuristic(start_x, start_y, dest_x, dest_y);
    s_nodes[start_idx].f_cost = s_nodes[start_idx].h_cost;
    pathfind_add_to_open(start_idx);

    while (s_open_count > 0) {
        current_idx = pathfind_get_lowest_f();
        if (current_idx < 0) break;

        current = &s_nodes[current_idx];
        current->closed = 1;

        if (current->x == dest_x && current->y == dest_y) {
            return pathfind_reconstruct_path(current_idx);
        }

        for (dir = 0; dir < 8; dir++) {
            nx = current->x + s_dir_dx[dir];
            ny = current->y + s_dir_dy[dir];

            if (nx >= PATHFIND_GRID_MAX || ny >= PATHFIND_GRID_MAX) continue;

            if (!pathfind_is_walkable(nx, ny)) continue;

            neighbor_idx = pathfind_get_node(nx, ny);
            if (neighbor_idx < 0) continue;

            neighbor = &s_nodes[neighbor_idx];

            if (neighbor->closed) continue;

            movement_cost = (dir % 2 == 0) ? 10 : 14;
            new_g = current->g_cost + movement_cost;

            if (new_g < neighbor->g_cost) {
                neighbor->parent_x = current->x;
                neighbor->parent_y = current->y;
                neighbor->g_cost = new_g;
                neighbor->h_cost = pathfind_heuristic(nx, ny, dest_x, dest_y);
                neighbor->f_cost = neighbor->g_cost + neighbor->h_cost;

                if (!neighbor->in_open) {
                    pathfind_add_to_open(neighbor_idx);
                }
            }
        }
    }

    LOG_DEBUG("No path found from (%d,%d) to (%d,%d)", start_x, start_y, dest_x, dest_y);
    return 0;
}
