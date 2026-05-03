/*
 * Stone Age Client - Map Battle Module
 * Reverse engineered from sa_9061.exe (FUN_00404850)
 *
 * Battle map loading and sprite selection
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "map.h"
#include "render.h"
#include "logger.h"

/* Battle field data - from FUN_00404850 */
static BattleField s_battle_field;
static u32 s_current_battle_id = 0xFFFFFFFF;
static u16 s_battle_sprite_id = 0;  /* DAT_004d7f74 */

/* LCG random state - DAT_004d7160 */
static u32 s_lcg_state = 1;

/* Battle sprite tables from DAT_0049ea64, DAT_0049ea70, DAT_0049ea7c, DAT_0049ea88 */
static const u16 s_battle_sprites_7546[4] = { 0x7184, 0x7185, 0x7186, 0x7182 };
static const u16 s_battle_sprites_7547[4] = { 0x7182, 0x7183, 0x718a, 0x7187 };
static const u16 s_battle_sprites_7548[4] = { 0x7187, 0x7188, 0x7189, 0x7181 };
static const u16 s_battle_sprites_7549[4] = { 0x7181, 0x718b, 0x718c, 0x0000 };

/* External reference to current map ID */
extern u32 map_get_current_map_id(void);

/*
 * LCG random number generator - FUN_00492403
 * Uses Microsoft C rand() algorithm: state = state * 0x343fd + 0x269ec3
 */
static u16 map_lcg_rand(void) {
    s_lcg_state = s_lcg_state * 0x343fd + 0x269ec3;
    return (u16)((s_lcg_state >> 16) & 0x7fff);
}

/*
 * Initialize battle LCG with seed
 */
void map_battle_lcg_seed(u32 seed) {
    s_lcg_state = seed;
}

/*
 * Select battle sprite based on map ID - from FUN_00404850 sprite selection logic
 */
u16 map_select_battle_sprite(u32 map_id) {
    u16 terrain_type;
    u16 variant;

    /* Special maps: 0x331, 0x1f47, 0x1fa5, 0x1fa4, 0x1f5b, 0x1f5c, 0x1f5d, 0x1f4f */
    /* Also 0x97, 0xa0, 0xa1, 0x7546-0x7549, 0x4e-0x63, 0x1fab-0x1fb0, 0x1fb4-0x1fb8, 0x1fbc-0x1fbe */
    /* And 0x1d4c, 0x1d6a, 0x1db0 */

    if (map_id == 0x331 || map_id == 0x1f47 || map_id == 0x1fa5 || map_id == 0x1fa4 ||
        map_id == 0x1f5b || map_id == 0x1f5c || map_id == 0x1f5d || map_id == 0x1f4f ||
        map_id == 0x97 || map_id == 0xa0 || map_id == 0xa1 ||
        (map_id >= 0x7546 && map_id <= 0x7549) ||
        (map_id >= 0x4e && map_id <= 0x63) ||
        (map_id >= 0x1fab && map_id <= 0x1fb0) ||
        (map_id >= 0x1fb4 && map_id <= 0x1fb8) ||
        (map_id >= 0x1fbc && map_id <= 0x1fbe) ||
        map_id == 0x1d4c || map_id == 0x1d6a || map_id == 0x1db0) {

        if (s_battle_sprite_id != 0) {
            return s_battle_sprite_id;
        }

        if (map_id == 0x7546) {
            terrain_type = map_lcg_rand();
            return s_battle_sprites_7546[terrain_type & 3];
        }
        else if (map_id == 0x7547) {
            terrain_type = map_lcg_rand();
            return s_battle_sprites_7547[terrain_type & 3];
        }
        else if (map_id == 0x7548) {
            terrain_type = map_lcg_rand();
            return s_battle_sprites_7548[terrain_type & 3];
        }
        else if (map_id == 0x7549) {
            terrain_type = map_lcg_rand();
            return s_battle_sprites_7549[terrain_type & 3];
        }

        if ((map_id >= 0x4e && map_id <= 0x63) || map_id == 0x1fab) {
            terrain_type = map_lcg_rand();
            return 0x7192 - (terrain_type & 3);
        }

        if ((map_id >= 0x1fac && map_id <= 0x1fb0) ||
            (map_id >= 0x1fb4 && map_id <= 0x1fb8) ||
            (map_id >= 0x1fbc && map_id <= 0x1fbe) ||
            map_id == 0x1d4c || map_id == 0x1d6a || map_id == 0x1db0) {
            terrain_type = map_lcg_rand();
            return 0x718e - (terrain_type & 3);
        }

        return s_battle_sprite_id != 0 ? s_battle_sprite_id : BATTLE_SPRITE_DEFAULT;
    }

    /* Normal map: use terrain-based random selection */
    terrain_type = map_lcg_rand();

    switch (terrain_type % 5) {
        case 0: s_battle_sprite_id = 0x7149; break;  /* Grass */
        case 1: s_battle_sprite_id = 0x715c; break;  /* Desert */
        case 2: s_battle_sprite_id = 0x715b; break;  /* Snow */
        case 3: s_battle_sprite_id = 0x7160; break;  /* Cave */
        case 4: s_battle_sprite_id = 0x7161; break;  /* Indoor */
    }

    variant = map_lcg_rand();
    s_battle_sprite_id = 0x7177 - (variant & 3);

    if (map_id == 0x2147) {
        s_battle_sprite_id = 0x718d;
    }

    return s_battle_sprite_id;
}

/*
 * Load battle map - FUN_00404850
 */
int map_load_battle(u32 battle_id) {
    char path[260];
    FILE* fp;
    int i;
    u8 low_byte, high_byte;

    if (battle_id > MAX_BATTLE_FIELDS - 1) {
        battle_id = 0;
    }

    s_current_battle_id = battle_id;

    snprintf(path, sizeof(path), "data/battleMap/battle%03d.sab", battle_id);

    LOG_INFO("Loading battle field: %s", path);

    fp = fopen(path, "rb");
    if (fp == NULL) {
        LOG_ERROR("Failed to open battle file: %s", path);
        return 0;
    }

    fseek(fp, 4, SEEK_SET);

    for (i = 0; i < BATTLE_TILE_COUNT; i++) {
        low_byte = (u8)fgetc(fp);
        high_byte = (u8)fgetc(fp);
        s_battle_field.tiles[i] = (u16)low_byte << 8 | (u16)high_byte;
    }

    fclose(fp);

    s_battle_sprite_id = map_select_battle_sprite(map_get_current_map_id());

    LOG_INFO("Battle field %d loaded, sprite 0x%04x", battle_id, s_battle_sprite_id);
    return 1;
}

/*
 * Render battle field - FUN_00404850
 */
void map_render_battle(void) {
    int row, col;
    int screen_x, screen_y;
    int base_x, base_y;
    u16 tile_id;
    u16* tile_ptr;

    base_x = -288;
    base_y = 240;

    tile_ptr = s_battle_field.tiles;

    for (row = 0; row < BATTLE_FIELD_SIZE; row++) {
        int row_x = base_x;
        int row_y = base_y;

        for (col = 0; col < BATTLE_FIELD_SIZE; col++) {
            tile_id = *tile_ptr++;

            if (tile_id != 0) {
                render_queue_add_sprite(row_x, row_y, 1, tile_id, 0);
            }

            row_x += 32;
            row_y -= 24;
        }

        base_x += 32;
        base_y += 24;
    }
}

/*
 * Get battle map path
 */
const char* map_get_battle_path(u32 battle_id) {
    static char path[260];
    snprintf(path, sizeof(path), "data/battleMap/battle%03d.sab", battle_id);
    return path;
}

/*
 * Get current battle sprite ID
 */
u16 map_get_battle_sprite(void) {
    return s_battle_sprite_id;
}

/*
 * Set battle sprite ID
 */
void map_set_battle_sprite(u16 sprite_id) {
    s_battle_sprite_id = sprite_id;
}

/*
 * Get battle field tiles
 */
u16* map_get_battle_tiles(void) {
    return s_battle_field.tiles;
}

/*
 * Get current battle ID
 */
u32 map_get_battle_id(void) {
    return s_current_battle_id;
}
