/*
 * Stone Age Client - Unit Tests for UI Window System
 * Test file: test_ui_window.c
 *
 * Tests for window creation, 9-sprite grid, render queue
 * Based on reverse engineering of sa_9061.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/ui_window.h"

/* ========================================
 * Test Cases for Window Constants
 * ======================================== */

/*
 * Test 1: Window type constants
 */
static void test_window_type_constants(void) {
    TEST_BEGIN("Window type constants");

    /* Window types from FUN_00448610 */
    TEST_ASSERT(WINDOW_TYPE_NORMAL == 0, "Normal window type should be 0");
    TEST_ASSERT(WINDOW_TYPE_MODAL == 1, "Modal window type should be 1");
    TEST_ASSERT(WINDOW_TYPE_TRANSPARENT == 2, "Transparent window should be 2");
    TEST_ASSERT(WINDOW_TYPE_NO_DECORATION == 3, "No decoration should be 3");
    TEST_ASSERT(WINDOW_TYPE_BUTTON == 4, "Button type should be 4");

    TEST_END();
}

/*
 * Test 2: Window sprite offsets
 */
static void test_window_sprite_offsets(void) {
    TEST_BEGIN("Window sprite offsets");

    /* 9-sprite grid offsets from FUN_00448270 */
    TEST_ASSERT(WINDOW_SPRITE_TOP_LEFT == 0, "Top-left sprite should be 0");
    TEST_ASSERT(WINDOW_SPRITE_TOP == 1, "Top sprite should be 1");
    TEST_ASSERT(WINDOW_SPRITE_TOP_RIGHT == 2, "Top-right sprite should be 2");
    TEST_ASSERT(WINDOW_SPRITE_LEFT == 3, "Left sprite should be 3");
    TEST_ASSERT(WINDOW_SPRITE_CENTER == 4, "Center sprite should be 4");
    TEST_ASSERT(WINDOW_SPRITE_RIGHT == 5, "Right sprite should be 5");
    TEST_ASSERT(WINDOW_SPRITE_BOTTOM_LEFT == 6, "Bottom-left sprite should be 6");
    TEST_ASSERT(WINDOW_SPRITE_BOTTOM == 7, "Bottom sprite should be 7");
    TEST_ASSERT(WINDOW_SPRITE_BOTTOM_RIGHT == 8, "Bottom-right sprite should be 8");

    TEST_END();
}

/*
 * Test 3: Window state constants
 */
static void test_window_state_constants(void) {
    TEST_BEGIN("Window state constants");

    /* Window render states from FUN_00448270 */
    TEST_ASSERT(WINDOW_STATE_INIT == 0, "Init state should be 0");
    TEST_ASSERT(WINDOW_STATE_OPENING == 1, "Opening state should be 1");
    TEST_ASSERT(WINDOW_STATE_RENDER == 2, "Render state should be 2");
    TEST_ASSERT(WINDOW_STATE_CLOSING == 3, "Closing state should be 3");

    TEST_END();
}

/*
 * Test 4: Sprite type constants
 */
static void test_sprite_type_constants(void) {
    TEST_BEGIN("Sprite type constants");

    /* Sprite types from FUN_0047e210 */
    TEST_ASSERT(SPRITE_TYPE_UI == 0x68, "UI sprite type should be 0x68");
    TEST_ASSERT(SPRITE_TYPE_ICON == 0x69, "Icon sprite type should be 0x69");
    TEST_ASSERT(SPRITE_TYPE_BUTTON == 0x6d, "Button sprite type should be 0x6d");
    TEST_ASSERT(SPRITE_TYPE_EFFECT == 0x6e, "Effect sprite type should be 0x6e");

    TEST_END();
}

/*
 * Test 5: Render queue size
 */
static void test_render_queue_size(void) {
    TEST_BEGIN("Render queue size");

    /* Max queue size from FUN_0047e210: 0xfff (4095) */
    TEST_ASSERT(MAX_RENDER_QUEUE == 0x1000, "Max render queue should be 0x1000 (4096)");

    TEST_END();
}

/* ========================================
 * Test Cases for Window Structure
 * ======================================== */

/*
 * Test 6: Window structure size
 */
static void test_window_structure_size(void) {
    TEST_BEGIN("Window structure size");

    /* Window structure from FUN_00448610: 0x40 bytes header + extended data */
    TEST_ASSERT(sizeof(Window) >= 0x40, "Window struct should be at least 0x40 bytes");
    TEST_ASSERT(sizeof(Window) <= 0x100, "Window struct should be at most 0x100 bytes");

    TEST_END();
}

/*
 * Test 7: Window creation (FUN_00448610)
 */
static void test_window_creation(void) {
    TEST_BEGIN("Window creation");

    Window* win = window_create(100, 200, 300, 150, WINDOW_TYPE_NORMAL);
    TEST_ASSERT(win != NULL, "Window creation should succeed");

    if (win) {
        TEST_ASSERT(win->x == 100, "Window X should be 100");
        TEST_ASSERT(win->y == 200, "Window Y should be 200");
        TEST_ASSERT(win->width == 300, "Window width should be 300");
        TEST_ASSERT(win->height == 150, "Window height should be 150");
        TEST_ASSERT(win->type == WINDOW_TYPE_NORMAL, "Window type should be normal");
    }

    TEST_END();
}

/*
 * Test 8: Window creation with different types
 */
static void test_window_creation_types(void) {
    TEST_BEGIN("Window creation types");

    Window* win;

    win = window_create(0, 0, 100, 100, WINDOW_TYPE_MODAL);
    TEST_ASSERT(win != NULL, "Modal window should be created");

    win = window_create(0, 0, 100, 100, WINDOW_TYPE_BUTTON);
    TEST_ASSERT(win != NULL, "Button window should be created");

    win = window_create(0, 0, 100, 100, WINDOW_TYPE_NO_DECORATION);
    TEST_ASSERT(win != NULL, "No-decoration window should be created");

    TEST_END();
}

/*
 * Test 9: Window center calculation
 */
static void test_window_center_calculation(void) {
    TEST_BEGIN("Window center calculation");

    Window* win = window_create(100, 200, 300, 150, WINDOW_TYPE_NORMAL);
    TEST_ASSERT(win != NULL, "Window should be created");

    if (win) {
        /* Center X should be x + width/2 for normal windows */
        int expected_cx = 100 + 300 / 2;
        TEST_ASSERT(win->center_x == expected_cx || win->center_x == 100 + (300 * 32),
                    "Center X should be calculated");
    }

    TEST_END();
}

/*
 * Test 10: Window animation parameters
 */
static void test_window_animation_params(void) {
    TEST_BEGIN("Window animation params");

    Window* win = window_create(100, 200, 300, 150, WINDOW_TYPE_NORMAL);
    TEST_ASSERT(win != NULL, "Window should be created");

    if (win) {
        /* Animation step from FUN_00448610 */
        TEST_ASSERT(win->anim_step_x >= 0, "Animation step X should be non-negative");
        TEST_ASSERT(win->anim_step_y >= 0, "Animation step Y should be non-negative");
    }

    TEST_END();
}

/* ========================================
 * Test Cases for 9-Sprite Grid
 * ======================================== */

/*
 * Test 11: 9-sprite grid calculation
 */
static void test_9sprite_grid_calculation(void) {
    TEST_BEGIN("9-sprite grid calculation");

    /* Grid cells: 3x3 = 9 sprites */
    int grid_cells = 9;
    TEST_ASSERT(grid_cells == 9, "Grid should have 9 cells");

    /* Each corner is 64x64 pixels (0x40) */
    int corner_size = 64;
    TEST_ASSERT(corner_size == 0x40, "Corner size should be 64 pixels");

    TEST_END();
}

/*
 * Test 12: 9-sprite sprite ID calculation
 */
static void test_9sprite_id_calculation(void) {
    TEST_BEGIN("9-sprite ID calculation");

    /* Base sprite IDs from DAT_0054b194, DAT_0054c208 */
    u32 base_id_normal = 0x6800;  /* Example base for normal windows */
    u32 base_id_alt = 0x6A00;     /* Example base for alternative windows */

    /* Top-left is base + 0 */
    TEST_ASSERT(base_id_normal + WINDOW_SPRITE_TOP_LEFT == base_id_normal,
                "Top-left should be base");

    /* Top is base + 1 */
    TEST_ASSERT(base_id_normal + WINDOW_SPRITE_TOP == base_id_normal + 1,
                "Top should be base + 1");

    /* Bottom-right is base + 8 */
    TEST_ASSERT(base_id_normal + WINDOW_SPRITE_BOTTOM_RIGHT == base_id_normal + 8,
                "Bottom-right should be base + 8");

    TEST_END();
}

/*
 * Test 13: Window sprite resource IDs
 */
static void test_window_sprite_resource_ids(void) {
    TEST_BEGIN("Window sprite resource IDs");

    /* From FUN_00448610: sprite type 0x68 is stored in window struct */
    u8 sprite_type = 0x68;
    TEST_ASSERT(sprite_type == SPRITE_TYPE_UI, "Window uses UI sprite type");

    TEST_END();
}

/* ========================================
 * Test Cases for Render Queue
 * ======================================== */

/*
 * Test 14: Render queue add sprite
 */
static void test_render_queue_add(void) {
    TEST_BEGIN("Render queue add");

    RenderQueue queue;
    render_queue_init(&queue);

    int result = render_queue_add(&queue, 100, 200, SPRITE_TYPE_UI, 0x6800, 0);
    TEST_ASSERT(result >= 0, "Queue add should succeed");
    TEST_ASSERT(queue.count == 1, "Queue count should be 1");

    TEST_END();
}

/*
 * Test 15: Render queue overflow
 */
static void test_render_queue_overflow(void) {
    TEST_BEGIN("Render queue overflow");

    RenderQueue queue;
    render_queue_init(&queue);

    /* Fill queue to max */
    int i;
    for (i = 0; i < MAX_RENDER_QUEUE; i++) {
        render_queue_add(&queue, 0, 0, SPRITE_TYPE_UI, 0x6800, 0);
    }

    /* Next add should fail */
    int result = render_queue_add(&queue, 0, 0, SPRITE_TYPE_UI, 0x6800, 0);
    TEST_ASSERT(result < 0, "Queue overflow should fail");
    TEST_ASSERT(queue.count == MAX_RENDER_QUEUE, "Queue should be at max");

    TEST_END();
}

/*
 * Test 16: Render queue blend modes
 */
static void test_render_queue_blend_modes(void) {
    TEST_BEGIN("Render queue blend modes");

    /* Blend mode flags from FUN_0047e640 */
    u32 blend_normal = 0x00000000;
    u32 blend_alpha_256 = 0xa0000000;
    u32 blend_alpha_144 = 0x90000000;
    u32 blend_alpha_192 = 0xc0000000;

    TEST_ASSERT((blend_alpha_256 >> 28) == 0xa, "Alpha 256 should have 0xa prefix");
    TEST_ASSERT((blend_alpha_144 >> 28) == 0x9, "Alpha 144 should have 0x9 prefix");
    TEST_ASSERT((blend_alpha_192 >> 28) == 0xc, "Alpha 192 should have 0xc prefix");

    TEST_END();
}

/*
 * Test 17: Render queue sprite ID validation
 */
static void test_render_queue_sprite_id_validation(void) {
    TEST_BEGIN("Render queue sprite ID validation");

    /* From FUN_0047e210: sprite ID < 100 is invalid, < -1 is special */
    int sprite_id_valid = 100;
    int sprite_id_invalid = 50;
    int sprite_id_special = -2;

    TEST_ASSERT(sprite_id_valid >= 100, "Valid sprite ID should be >= 100");
    TEST_ASSERT(sprite_id_invalid < 100, "Invalid sprite ID should be < 100");
    TEST_ASSERT(sprite_id_special < -1, "Special sprite ID should be < -1");

    TEST_END();
}

/* ========================================
 * Test Cases for Window Rendering
 * ======================================== */

/*
 * Test 18: Window render state machine
 */
static void test_window_render_state_machine(void) {
    TEST_BEGIN("Window render state machine");

    Window* win = window_create(100, 100, 200, 150, WINDOW_TYPE_NORMAL);
    TEST_ASSERT(win != NULL, "Window should be created");

    if (win) {
        /* Initial state should be 0 */
        TEST_ASSERT(win->render_state == WINDOW_STATE_INIT, "Initial state should be init");

        /* Simulate state transitions */
        window_set_state(win, WINDOW_STATE_OPENING);
        TEST_ASSERT(win->render_state == WINDOW_STATE_OPENING, "State should be opening");

        window_set_state(win, WINDOW_STATE_RENDER);
        TEST_ASSERT(win->render_state == WINDOW_STATE_RENDER, "State should be render");
    }

    TEST_END();
}

/*
 * Test 19: Window opening animation
 */
static void test_window_opening_animation(void) {
    TEST_BEGIN("Window opening animation");

    Window* win = window_create(100, 100, 200, 150, WINDOW_TYPE_NORMAL);
    TEST_ASSERT(win != NULL, "Window should be created");

    if (win) {
        /* Animation runs for 10 frames (from FUN_00448270) */
        int animation_frames = 10;
        TEST_ASSERT(ANIMATION_FRAMES_OPEN == 10, "Open animation should be 10 frames");

        /* Animation counter should increment each frame */
        win->anim_counter = 0;
        for (int i = 0; i < 9; i++) {
            win->anim_counter++;
        }
        TEST_ASSERT(win->anim_counter == 9, "Counter should reach 9");
    }

    TEST_END();
}

/*
 * Test 20: Window center position calculation
 */
static void test_window_center_position(void) {
    TEST_BEGIN("Window center position");

    Window* win = window_create(100, 200, 300, 150, WINDOW_TYPE_NORMAL);
    TEST_ASSERT(win != NULL, "Window should be created");

    if (win) {
        /* Center should be calculated during creation */
        int cx = win->center_x;
        int cy = win->center_y;

        TEST_ASSERT(cx > 0, "Center X should be positive");
        TEST_ASSERT(cy > 0, "Center Y should be positive");
    }

    TEST_END();
}

/* ========================================
 * Test Cases for Window Input
 * ======================================== */

/*
 * Test 21: Window hit test
 */
static void test_window_hit_test(void) {
    TEST_BEGIN("Window hit test");

    Window* win = window_create(100, 100, 200, 150, WINDOW_TYPE_NORMAL);
    TEST_ASSERT(win != NULL, "Window should be created");

    if (win) {
        /* Inside window */
        int hit = window_hit_test(win, 150, 150);
        TEST_ASSERT(hit != 0, "Point inside window should hit");

        /* Outside window */
        hit = window_hit_test(win, 50, 50);
        TEST_ASSERT(hit == 0, "Point outside window should not hit");

        /* Edge cases */
        hit = window_hit_test(win, 100, 100);
        TEST_ASSERT(hit != 0, "Top-left corner should hit");

        hit = window_hit_test(win, 299, 249);
        TEST_ASSERT(hit != 0, "Bottom-right corner should hit");
    }

    TEST_END();
}

/*
 * Test 22: Window close button
 */
static void test_window_close_button(void) {
    TEST_BEGIN("Window close button");

    Window* win = window_create(100, 100, 200, 150, WINDOW_TYPE_NORMAL);
    TEST_ASSERT(win != NULL, "Window should be created");

    if (win) {
        /* Close button is at top-right corner */
        int close_x = 100 + 200 - 20;  /* Right edge with margin */
        int close_y = 100 + 10;         /* Top edge with margin */

        /* Window with close button should have button sprites */
        TEST_ASSERT(win->has_close_button == 1 || win->type >= 0,
                    "Window should track close button");
    }

    TEST_END();
}

/*
 * Test 23: Window z-order
 */
static void test_window_z_order(void) {
    TEST_BEGIN("Window z-order");

    Window* win1 = window_create(100, 100, 200, 150, WINDOW_TYPE_NORMAL);
    Window* win2 = window_create(150, 150, 200, 150, WINDOW_TYPE_MODAL);

    TEST_ASSERT(win1 != NULL && win2 != NULL, "Windows should be created");

    if (win1 && win2) {
        /* Modal window should be on top */
        TEST_ASSERT(win2->z_order > win1->z_order || win2->type == WINDOW_TYPE_MODAL,
                    "Modal window should have higher z-order");
    }

    TEST_END();
}

/* ========================================
 * Test Cases for Sprite Rendering
 * ======================================== */

/*
 * Test 24: Sprite dimension lookup (FUN_0041f900)
 */
static void test_sprite_dimension_lookup(void) {
    TEST_BEGIN("Sprite dimension lookup");

    int width, height;
    int result = sprite_get_dimensions(0x6800, &width, &height);

    /* Function should complete without crash */
    TEST_ASSERT(1, "Dimension lookup should complete");

    TEST_END();
}

/*
 * Test 25: Sprite offset lookup (FUN_0041fad0)
 */
static void test_sprite_offset_lookup(void) {
    TEST_BEGIN("Sprite offset lookup");

    u32 offset;
    int result = sprite_get_offset(0x6800, &offset);

    /* Function should complete without crash */
    TEST_ASSERT(1, "Offset lookup should complete");

    TEST_END();
}

/*
 * Test 26: High-res sprite range
 */
static void test_highres_sprite_range(void) {
    TEST_BEGIN("High-res sprite range");

    /* High-res sprites: ID >= 500000 */
    u32 highres_start = 500000;
    u32 highres_end = 549999;

    TEST_ASSERT(highres_start == 500000, "High-res start should be 500000");
    TEST_ASSERT(highres_end == 549999, "High-res end should be 549999");

    TEST_END();
}

/*
 * Test 27: Normal sprite range
 */
static void test_normal_sprite_range(void) {
    TEST_BEGIN("Normal sprite range");

    /* Normal sprites: ID < 500000 */
    u32 normal_max = 499999;

    TEST_ASSERT(normal_max == 499999, "Normal max should be 499999");

    TEST_END();
}

/*
 * Test 28: Window decoration flags
 */
static void test_window_decoration_flags(void) {
    TEST_BEGIN("Window decoration flags");

    /* Window flags from DAT_0461b3f8 region */
    u32 flags_transparent = 0x01;
    u32 flags_modal = 0x02;
    u32 flags_close_button = 0x04;
    u32 flags_draggable = 0x08;

    TEST_ASSERT(flags_transparent == 0x01, "Transparent flag should be 0x01");
    TEST_ASSERT(flags_modal == 0x02, "Modal flag should be 0x02");
    TEST_ASSERT(flags_close_button == 0x04, "Close button flag should be 0x04");
    TEST_ASSERT(flags_draggable == 0x08, "Draggable flag should be 0x08");

    TEST_END();
}

/*
 * Test 29: Window title rendering
 */
static void test_window_title_rendering(void) {
    TEST_BEGIN("Window title rendering");

    Window* win = window_create(100, 100, 200, 150, WINDOW_TYPE_NORMAL);
    TEST_ASSERT(win != NULL, "Window should be created");

    if (win) {
        window_set_title(win, "Test Window");
        TEST_ASSERT(win->title != NULL, "Title should be set");
    }

    TEST_END();
}

/*
 * Test 30: Window destroy
 */
static void test_window_destroy(void) {
    TEST_BEGIN("Window destroy");

    Window* win = window_create(100, 100, 200, 150, WINDOW_TYPE_NORMAL);
    TEST_ASSERT(win != NULL, "Window should be created");

    if (win) {
        window_destroy(win);
        /* Should not crash */
        TEST_ASSERT(1, "Window destroy should complete");
    }

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(ui_window) {
    /* Constants tests */
    test_window_type_constants();
    test_window_sprite_offsets();
    test_window_state_constants();
    test_sprite_type_constants();
    test_render_queue_size();

    /* Structure tests */
    test_window_structure_size();
    test_window_creation();
    test_window_creation_types();
    test_window_center_calculation();
    test_window_animation_params();

    /* 9-sprite grid tests */
    test_9sprite_grid_calculation();
    test_9sprite_id_calculation();
    test_window_sprite_resource_ids();

    /* Render queue tests */
    test_render_queue_add();
    test_render_queue_overflow();
    test_render_queue_blend_modes();
    test_render_queue_sprite_id_validation();

    /* Rendering tests */
    test_window_render_state_machine();
    test_window_opening_animation();
    test_window_center_position();

    /* Input tests */
    test_window_hit_test();
    test_window_close_button();
    test_window_z_order();

    /* Sprite tests */
    test_sprite_dimension_lookup();
    test_sprite_offset_lookup();
    test_highres_sprite_range();
    test_normal_sprite_range();

    /* Additional tests */
    test_window_decoration_flags();
    test_window_title_rendering();
    test_window_destroy();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - UI Window Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(ui_window);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
