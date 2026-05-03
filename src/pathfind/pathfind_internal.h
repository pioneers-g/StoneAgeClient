/*
 * Stone Age Client - Pathfinding Internal Header
 * Shared definitions for pathfind module
 */

#ifndef PATHFIND_INTERNAL_H
#define PATHFIND_INTERNAL_H

#include "types.h"

/* A* node pool for pathfinding */
#define MAX_PATHFIND_NODES     4096
#define PATHFIND_GRID_MAX      256

typedef struct PathfindNode {
    u16 x, y;
    u16 parent_x, parent_y;
    u32 g_cost;         /* Cost from start */
    u32 h_cost;         /* Heuristic cost to goal */
    u32 f_cost;         /* Total cost (g + h) */
    u8 closed;
    u8 in_open;
} PathfindNode;

/* Global node pool - shared between core and astar */
extern PathfindNode s_nodes[MAX_PATHFIND_NODES];
extern int s_node_count;

/* Open list for A* */
extern int s_open_list[MAX_PATHFIND_NODES];
extern int s_open_count;

/* Node grid lookup */
extern int s_node_grid[PATHFIND_GRID_MAX][PATHFIND_GRID_MAX];

/* Direction deltas for A* - 8 directional movement
 * WARNING: These values are INCORRECT and do not match binary FUN_00443e80!
 * See pathfind_movement.c for correct direction table (s_move_dx/s_move_dy)
 * TODO: Fix this table to match binary values
 */
extern const s16 s_dir_dx[8];
extern const s16 s_dir_dy[8];

/* A* internal functions */
void pathfind_reset_nodes(void);
int pathfind_get_node(u16 x, u16 y);
u32 pathfind_heuristic(u16 x1, u16 y1, u16 x2, u16 y2);
void pathfind_add_to_open(int idx);
int pathfind_get_lowest_f(void);
int pathfind_reconstruct_path(int dest_idx);

#endif /* PATHFIND_INTERNAL_H */
