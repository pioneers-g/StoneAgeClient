/*
 * Stone Age Client - Minimap/Radar System Implementation
 * Reverse engineered from sa_9061.exe
 *
 * Displays a small map showing:
 * - Player position (center)
 * - NPCs, monsters, other players as colored dots
 * - Party members in blue
 * - Click-to-navigate functionality
 */

#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "minimap.h"
#include "map.h"
#include "map_npc.h"
#include "character.h"
#include "party.h"
#include "pathfind.h"
#include "sprite.h"
#include "directx.h"
#include "render.h"
#include "logger.h"

/* Forward declarations for static functions */
static void minimap_add_warps(void);
static void minimap_add_items(void);
static void minimap_blit_to_screen(void* screen_surface);

/* Global minimap context */
MinimapContext g_minimap = {0};

/* 16-bit color definitions (565 format) - RGB565 */
static const u16 s_dot_colors[] = {
    0x0000,  /* DOT_NONE - Black/transparent */
    0x07E0,  /* DOT_PLAYER - Green (0, 255, 0) */
    0xFFE0,  /* DOT_NPC - Yellow (255, 255, 0) */
    0xF800,  /* DOT_MONSTER - Red (255, 0, 0) */
    0x001F,  /* DOT_PARTY_MEMBER - Blue (0, 0, 255) */
    0xF81F,  /* DOT_WARP - Magenta (255, 0, 255) */
    0xFD20,  /* DOT_ITEM - Orange (255, 165, 0) */
    0xFFFF,  /* DOT_PLAYER_OTHER - White */
    0x8410,  /* DOT_BACKGROUND - Gray background */
    0x0421   /* DOT_BORDER - Dark gray border */
};

/* Terrain colors for map background */
static const u16 s_terrain_colors[] = {
    0x07E0,  /* Grass - Green */
    0x001F,  /* Water - Blue */
    0xFFE0,  /* Sand - Yellow */
    0x8410,  /* Stone - Gray */
    0x07E0,  /* Forest - Dark green */
    0x0000   /* Void - Black */
};

/*
 * Initialize minimap system
 */
int minimap_init(void) {
    memset(&g_minimap, 0, sizeof(MinimapContext));

    /* Default position in top-right corner */
    g_minimap.screen_x = 10;
    g_minimap.screen_y = 10;
    g_minimap.width = MINIMAP_WIDTH;
    g_minimap.height = MINIMAP_HEIGHT;
    g_minimap.scale = MINIMAP_SCALE;
    g_minimap.visible = 1;
    g_minimap.alpha = 128;
    g_minimap.background_enabled = 1;

    /* Allocate surface for minimap rendering */
    g_minimap.surface_size = g_minimap.width * g_minimap.height * 2;
    g_minimap.surface = malloc(g_minimap.surface_size);

    if (!g_minimap.surface) {
        LOG_ERROR("Failed to allocate minimap surface");
        return 0;
    }

    memset(g_minimap.surface, 0, g_minimap.surface_size);

    LOG_INFO("Minimap system initialized (%dx%d)", g_minimap.width, g_minimap.height);
    return 1;
}

/*
 * Shutdown minimap system
 */
void minimap_shutdown(void) {
    if (g_minimap.surface) {
        free(g_minimap.surface);
        g_minimap.surface = NULL;
    }

    memset(&g_minimap, 0, sizeof(MinimapContext));
    LOG_INFO("Minimap system shutdown");
}

/*
 * Render minimap to screen
 * Uses DirectX back buffer for rendering
 */
void minimap_render(void* surface) {
    u16* pixels;
    int i;

    if (!g_minimap.visible) {
        return;
    }

    /* Update minimap data if dirty */
    if (g_minimap.dirty) {
        minimap_update();
    }

    /* Clear minimap surface */
    pixels = (u16*)g_minimap.surface;
    if (!pixels) return;

    /* Fill with semi-transparent background */
    for (i = 0; i < g_minimap.width * g_minimap.height; i++) {
        pixels[i] = s_dot_colors[DOT_BACKGROUND];
    }

    /* Draw map background (terrain) */
    if (g_minimap.background_enabled) {
        minimap_draw_background(pixels);
    }

    /* Draw entity dots */
    minimap_draw_dots(pixels);

    /* Draw player indicator (center) */
    minimap_draw_player(pixels);

    /* Draw border */
    minimap_draw_border(pixels);

    /* Blit to screen surface */
    minimap_blit_to_screen(surface);
}

/*
 * Blit minimap surface to screen
 */
void minimap_blit_to_screen(void* screen_surface) {
    u16* src = (u16*)g_minimap.surface;
    u16* dest = (u16*)screen_surface;
    int x, y;
    int src_idx, dest_idx;
    u16 pixel;

    if (!src || !dest) return;

    /* Copy minimap to screen position */
    for (y = 0; y < g_minimap.height; y++) {
        for (x = 0; x < g_minimap.width; x++) {
            src_idx = y * g_minimap.width + x;
            pixel = src[src_idx];

            /* Skip transparent pixels */
            if (pixel == 0) continue;

            /* Calculate destination position */
            int screen_x = g_minimap.screen_x + x;
            int screen_y = g_minimap.screen_y + y;

            /* Bounds check for 640x480 screen */
            if (screen_x < 0 || screen_x >= 640) continue;
            if (screen_y < 0 || screen_y >= 480) continue;

            dest_idx = screen_y * 640 + screen_x;
            dest[dest_idx] = pixel;
        }
    }
}

/*
 * Draw minimap background (terrain types)
 */
void minimap_draw_background(void* surface) {
    u16* pixels = (u16*)surface;
    int x, y;
    int map_x, map_y;
    int tile_x, tile_y;
    u16 tile_color;
    int idx;

    if (!g_minimap.background_enabled || !pixels) {
        return;
    }

    /* Draw terrain background */
    for (y = 0; y < g_minimap.height; y++) {
        for (x = 0; x < g_minimap.width; x++) {
            /* Calculate world coordinates centered on player */
            map_x = g_minimap.center_x + (x - g_minimap.width / 2) * g_minimap.scale;
            map_y = g_minimap.center_y + (y - g_minimap.height / 2) * g_minimap.scale;

            /* Convert to tile coordinates */
            tile_x = map_x / 32;
            tile_y = map_y / 32;

            /* Get tile type from map */
            int tile_type = map_get_tile_type(tile_x, tile_y);

            /* Get color for terrain type */
            if (tile_type >= 0 && tile_type < 6) {
                tile_color = s_terrain_colors[tile_type];
            } else {
                tile_color = s_terrain_colors[0];  /* Default to grass */
            }

            /* Apply to surface */
            idx = y * g_minimap.width + x;
            pixels[idx] = tile_color;
        }
    }
}

/*
 * Draw dots representing entities on the minimap
 */
void minimap_draw_dots(void* surface) {
    u16* pixels = (u16*)surface;
    int i, dx, dy;
    MinimapDot* dot;
    int minimap_x, minimap_y;
    u16 color;
    int idx;

    if (!pixels) return;

    for (i = 0; i < g_minimap.dot_count; i++) {
        dot = &g_minimap.dots[i];

        /* Convert world to minimap coordinates */
        minimap_x = (dot->x - g_minimap.center_x) / g_minimap.scale + g_minimap.width / 2;
        minimap_y = (dot->y - g_minimap.center_y) / g_minimap.scale + g_minimap.height / 2;

        /* Check bounds */
        if (minimap_x < 0 || minimap_x >= g_minimap.width - 1) continue;
        if (minimap_y < 0 || minimap_y >= g_minimap.height - 1) continue;

        /* Get color for dot type */
        if (dot->type < sizeof(s_dot_colors) / sizeof(s_dot_colors[0])) {
            color = s_dot_colors[dot->type];
        } else {
            color = 0xFFFF;  /* White for unknown types */
        }

        /* Draw dot (2x2 pixels for visibility) */
        for (dy = 0; dy < MINIMAP_DOT_SIZE; dy++) {
            for (dx = 0; dx < MINIMAP_DOT_SIZE; dx++) {
                int px = minimap_x + dx;
                int py = minimap_y + dy;

                if (px >= 0 && px < g_minimap.width &&
                    py >= 0 && py < g_minimap.height) {
                    idx = py * g_minimap.width + px;
                    pixels[idx] = color;
                }
            }
        }
    }
}

/*
 * Draw player indicator at center of minimap
 * Arrow shape pointing in player direction
 */
void minimap_draw_player(void* surface) {
    u16* pixels = (u16*)surface;
    int center_x, center_y;
    int dx, dy;
    int idx;

    if (!pixels) return;

    /* Player is always at center */
    center_x = g_minimap.width / 2;
    center_y = g_minimap.height / 2;

    /* Draw a 5x5 cross pattern for player */
    for (dy = -2; dy <= 2; dy++) {
        for (dx = -2; dx <= 2; dx++) {
            int px = center_x + dx;
            int py = center_y + dy;

            if (px < 0 || px >= g_minimap.width) continue;
            if (py < 0 || py >= g_minimap.height) continue;

            idx = py * g_minimap.width + px;

            /* Create a cross/arrow pattern */
            if (abs(dx) + abs(dy) <= 2) {
                if (dx == 0 && dy == 0) {
                    /* Center - bright green */
                    pixels[idx] = 0x07E0;
                } else if (abs(dx) <= 1 && abs(dy) <= 1) {
                    /* Inner square - green */
                    pixels[idx] = 0x05E0;
                } else {
                    /* Outer ring - dark green */
                    pixels[idx] = 0x0340;
                }
            }
        }
    }
}

/*
 * Draw minimap border
 */
void minimap_draw_border(void* surface) {
    u16* pixels = (u16*)surface;
    int x, y;
    int idx;

    if (!pixels) return;

    /* Draw border lines */
    for (x = 0; x < g_minimap.width; x++) {
        /* Top border */
        idx = 0 * g_minimap.width + x;
        pixels[idx] = s_dot_colors[DOT_BORDER];

        /* Bottom border */
        idx = (g_minimap.height - 1) * g_minimap.width + x;
        pixels[idx] = s_dot_colors[DOT_BORDER];
    }

    for (y = 0; y < g_minimap.height; y++) {
        /* Left border */
        idx = y * g_minimap.width + 0;
        pixels[idx] = s_dot_colors[DOT_BORDER];

        /* Right border */
        idx = y * g_minimap.width + (g_minimap.width - 1);
        pixels[idx] = s_dot_colors[DOT_BORDER];
    }
}

/*
 * Update minimap data from game entities
 * Called periodically to refresh entity positions
 */
void minimap_update(void) {
    u32 i;

    /* Clear old dots */
    minimap_clear_dots();

    /* Add NPCs from map object list */
    for (i = 0; i < g_map_npcs.count && g_minimap.dot_count < MAX_MINIMAP_DOTS; i++) {
        MapNPC* npc = &g_map_npcs.objects[i];
        if (npc->id == 0) continue;

        /* Determine dot type based on NPC type */
        MinimapDotType dot_type;
        if (npc->type == OBJ_TYPE_NPC) {
            dot_type = DOT_NPC;
        } else if (npc->type == OBJ_TYPE_ENEMY) {
            dot_type = DOT_MONSTER;
        } else if (npc->type == OBJ_TYPE_PLAYER) {
            dot_type = DOT_PLAYER_OTHER;
        } else {
            continue;  /* Skip other types */
        }

        minimap_add_dot((u16)npc->x, (u16)npc->y, dot_type, npc->id);
    }

    /* Add NPCs from legacy character NPC list */
    for (i = 0; i < g_chars.npc_count && g_minimap.dot_count < MAX_MINIMAP_DOTS; i++) {
        NPC* npc = &g_chars.npcs[i];
        if (npc->id == 0) continue;

        minimap_add_dot(npc->x, npc->y, DOT_NPC, npc->id);
    }

    /* Add other characters on screen */
    for (i = 0; i < g_chars.character_count && g_minimap.dot_count < MAX_MINIMAP_DOTS; i++) {
        Character* ch = &g_chars.characters[i];
        if (!ch->visible || ch->id == g_chars.player.id) continue;

        /* Check if in party - party members shown in blue */
        MinimapDotType dot_type = party_find_member(ch->id) ? DOT_PARTY_MEMBER : DOT_PLAYER_OTHER;
        minimap_add_dot(ch->x, ch->y, dot_type, ch->id);
    }

    /* Add party members (even if off-screen, show with relative position) */
    for (i = 0; i < MAX_PARTY_MEMBERS && g_minimap.dot_count < MAX_MINIMAP_DOTS; i++) {
        PartyMember* member = &g_party.members[i];
        if (member->id == 0) continue;
        if (member->id == g_chars.player.id) continue;  /* Skip self */
        if (!(member->status & PARTY_STATUS_ONLINE)) continue;

        minimap_add_dot(member->x, member->y, DOT_PARTY_MEMBER, member->id);
    }

    /* Add warps/teleports if available */
    minimap_add_warps();

    /* Add items on ground if available */
    minimap_add_items();

    g_minimap.dirty = 0;
}

/*
 * Add warp points to minimap
 */
static void minimap_add_warps(void) {
    /* Check for warps in map data */
    int i;
    for (i = 0; i < g_minimap.dot_count && g_minimap.dot_count < MAX_MINIMAP_DOTS; i++) {
        /* Would need map warp data - placeholder */
    }
}

/*
 * Add ground items to minimap
 */
static void minimap_add_items(void) {
    /* Check for items on ground */
    int i;
    for (i = 0; i < g_minimap.dot_count && g_minimap.dot_count < MAX_MINIMAP_DOTS; i++) {
        /* Would need ground item data - placeholder */
    }
}

/*
 * Set minimap center (player position)
 */
void minimap_set_center(u16 x, u16 y) {
    g_minimap.center_x = x;
    g_minimap.center_y = y;
    g_minimap.dirty = 1;
}

/*
 * Set current map info
 */
void minimap_set_map(u32 map_id, u16 width, u16 height) {
    g_minimap.current_map_id = map_id;
    g_minimap.map_width = width;
    g_minimap.map_height = height;
    g_minimap.dirty = 1;

    LOG_DEBUG("Minimap set to map %u (%ux%u)", map_id, width, height);
}

/*
 * Refresh minimap
 */
void minimap_refresh(void) {
    g_minimap.dirty = 1;
    minimap_update();
}

/*
 * Clear all dots
 */
void minimap_clear_dots(void) {
    g_minimap.dot_count = 0;
    memset(g_minimap.dots, 0, sizeof(g_minimap.dots));
}

/*
 * Add dot to minimap
 */
void minimap_add_dot(u16 x, u16 y, MinimapDotType type, u32 id) {
    MinimapDot* dot;

    if (g_minimap.dot_count >= 255) {
        return;
    }

    dot = &g_minimap.dots[g_minimap.dot_count];
    dot->x = x;
    dot->y = y;
    dot->type = type;
    dot->id = id;

    g_minimap.dot_count++;
}

/*
 * Remove dot by ID
 */
void minimap_remove_dot(u32 id) {
    int i;

    for (i = 0; i < g_minimap.dot_count; i++) {
        if (g_minimap.dots[i].id == id) {
            /* Shift remaining dots */
            memmove(&g_minimap.dots[i], &g_minimap.dots[i + 1],
                    (g_minimap.dot_count - i - 1) * sizeof(MinimapDot));
            g_minimap.dot_count--;
            return;
        }
    }
}

/*
 * Update dot position
 */
void minimap_update_dot(u32 id, u16 x, u16 y) {
    int i;

    for (i = 0; i < g_minimap.dot_count; i++) {
        if (g_minimap.dots[i].id == id) {
            g_minimap.dots[i].x = x;
            g_minimap.dots[i].y = y;
            return;
        }
    }
}

/*
 * Convert world coordinates to minimap coordinates
 */
void minimap_world_to_minimap(u16 world_x, u16 world_y, u16* minimap_x, u16* minimap_y) {
    int dx = (int)world_x - g_minimap.center_x;
    int dy = (int)world_y - g_minimap.center_y;

    *minimap_x = (u16)(g_minimap.width / 2 + dx / g_minimap.scale);
    *minimap_y = (u16)(g_minimap.height / 2 + dy / g_minimap.scale);
}

/*
 * Convert minimap coordinates to world coordinates
 */
void minimap_minimap_to_world(u16 minimap_x, u16 minimap_y, u16* world_x, u16* world_y) {
    int dx = (int)minimap_x - g_minimap.width / 2;
    int dy = (int)minimap_y - g_minimap.height / 2;

    *world_x = (u16)(g_minimap.center_x + dx * g_minimap.scale);
    *world_y = (u16)(g_minimap.center_y + dy * g_minimap.scale);
}

/*
 * Show minimap
 */
void minimap_show(void) {
    g_minimap.visible = 1;
}

/*
 * Hide minimap
 */
void minimap_hide(void) {
    g_minimap.visible = 0;
}

/*
 * Toggle minimap visibility
 */
void minimap_toggle(void) {
    g_minimap.visible = !g_minimap.visible;
    LOG_DEBUG("Minimap %s", g_minimap.visible ? "shown" : "hidden");
}

/*
 * Check if minimap is visible
 */
int minimap_is_visible(void) {
    return g_minimap.visible;
}

/*
 * Set minimap position on screen
 */
void minimap_set_position(u16 x, u16 y) {
    g_minimap.screen_x = x;
    g_minimap.screen_y = y;
}

/*
 * Get minimap position on screen
 */
void minimap_get_position(u16* x, u16* y) {
    *x = g_minimap.screen_x;
    *y = g_minimap.screen_y;
}

/*
 * Set minimap scale
 */
void minimap_set_scale(u8 scale) {
    if (scale < 1) scale = 1;
    if (scale > 16) scale = 16;

    g_minimap.scale = scale;
    g_minimap.dirty = 1;
}

/*
 * Get minimap scale
 */
u8 minimap_get_scale(void) {
    return g_minimap.scale;
}

/*
 * Handle click on minimap
 */
int minimap_handle_click(u16 screen_x, u16 screen_y) {
    u16 minimap_x, minimap_y;
    u16 world_x, world_y;
    Character* player;

    /* Check if click is within minimap bounds */
    if (screen_x < g_minimap.screen_x ||
        screen_x >= g_minimap.screen_x + g_minimap.width ||
        screen_y < g_minimap.screen_y ||
        screen_y >= g_minimap.screen_y + g_minimap.height) {
        return 0;
    }

    /* Convert to minimap coordinates */
    minimap_x = screen_x - g_minimap.screen_x;
    minimap_y = screen_y - g_minimap.screen_y;

    /* Convert to world coordinates */
    minimap_minimap_to_world(minimap_x, minimap_y, &world_x, &world_y);

    /* Check if destination is walkable */
    if (!map_is_walkable(world_x, world_y)) {
        LOG_DEBUG("Minimap click on non-walkable tile (%u, %u)", world_x, world_y);
        return 1;  /* Handled but no movement */
    }

    /* Get player and initiate pathfinding */
    player = character_get_player();
    if (player) {
        /* Use pathfinding system to navigate to clicked position */
        if (pathfind_to(world_x, world_y, g_minimap.current_map_id)) {
            LOG_DEBUG("Minimap click: pathfinding to (%u, %u)", world_x, world_y);
        } else {
            /* Pathfinding failed, try direct movement */
            character_move_to(world_x, world_y);
            LOG_DEBUG("Minimap click: direct move to (%u, %u)", world_x, world_y);
        }
    }

    return 1;
}
