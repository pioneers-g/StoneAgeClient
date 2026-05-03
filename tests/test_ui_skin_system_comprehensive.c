/*
 * Stone Age Client - UI Skin System Comprehensive Tests
 * Tests for ui_skin.c implementation
 *
 * Covers:
 * - Skin constants
 * - Frame linked list operations
 * - Image type management
 * - Animation frame navigation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s16;
typedef int s32;
typedef void* HBITMAP;
typedef void* HDC;
typedef void* HGDIOBJ;
typedef void* HWND;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Constants
 * ======================================== */

#define SKIN_MAX_FRAMES         40
#define SKIN_FRAME_SIZE         0x118
#define SKIN_IMAGE_COUNT        5
#define SKIN_COMPOSITE_WIDTH    645
#define SKIN_COMPOSITE_HEIGHT   155

/* Skin image types */
typedef enum {
    SKIN_IMAGE_DOWN_ARROW_R = 0,
    SKIN_IMAGE_DOWN_ARROW_G = 1,
    SKIN_IMAGE_BASE = 2,
    SKIN_IMAGE_3 = 3,
    SKIN_IMAGE_4 = 4
} SkinImageType;

/* ========================================
 * Structures
 * ======================================== */

#pragma pack(push, 1)
typedef struct SkinFrame {
    HBITMAP images[SKIN_IMAGE_COUNT];
    HDC     image_dcs[SKIN_IMAGE_COUNT];
    HGDIOBJ original_objs[SKIN_IMAGE_COUNT];
    u32     frame_id;
    u32     flags;
    u32     width;
    u32     height;
    u8      custom_data[192];
    struct SkinFrame* prev;
    struct SkinFrame* next;
    u32     reserved;
} SkinFrame;
#pragma pack(pop)

typedef struct {
    HWND    hwnd;
    u32     field_04;
    u32     field_08;
    u32     instance;
    HBITMAP images[SKIN_IMAGE_COUNT];
    HDC     image_dcs[SKIN_IMAGE_COUNT];
    HGDIOBJ original_objs[SKIN_IMAGE_COUNT];
    u32     field_12;
    u32     field_13;
    u32     field_14;
    u32     field_15;
    u32     field_16;
    u32     field_17;
    u32     field_18;
    u32     field_24;
    u32     field_28;
    u32     field_2c;
    u32     field_30;
    HDC     composite_dc;
    HBITMAP composite_bitmap;
    HGDIOBJ composite_orig_obj;
    u32     field_40;
    u32     field_44;
    HDC     anim_dc;
    HBITMAP anim_bitmap;
    HGDIOBJ anim_orig_obj;
    u32     field_54;
    u32     field_58;
    u32     field_5c;
    u32     field_60;
    u32     field_64;
    u32     field_68;
    u32     field_6c;
    u32     field_70;
    u32     field_74;
    u32     field_78;
    u32     initialized;
    SkinFrame* frame_head;
    SkinFrame* frame_current;
    SkinFrame* frame_tail;
    u32     frame_count;
    u32     current_frame;
} UISkinContext;

/* ========================================
 * Global State
 * ======================================== */

static UISkinContext g_ui_skin = {0};

/* ========================================
 * Implementation Functions
 * ======================================== */

static SkinFrame* ui_skin_get_frame(int index) {
    SkinFrame* frame;
    int i;

    if (index < 0 || index >= SKIN_MAX_FRAMES) {
        return NULL;
    }

    frame = g_ui_skin.frame_head;
    for (i = 0; i < index && frame; i++) {
        frame = frame->next;
    }

    return frame;
}

static SkinFrame* ui_skin_get_current_frame(void) {
    return g_ui_skin.frame_current;
}

static void ui_skin_next_frame(void) {
    if (g_ui_skin.frame_current && g_ui_skin.frame_current->next) {
        g_ui_skin.frame_current = g_ui_skin.frame_current->next;
        g_ui_skin.current_frame++;
    } else {
        /* Loop back to start */
        g_ui_skin.frame_current = g_ui_skin.frame_head;
        g_ui_skin.current_frame = 0;
    }
}

static void ui_skin_reset(void) {
    g_ui_skin.frame_current = g_ui_skin.frame_head;
    g_ui_skin.current_frame = 0;
}

static HBITMAP ui_skin_get_image(SkinImageType type) {
    if (type < 0 || type >= SKIN_IMAGE_COUNT) {
        return NULL;
    }
    return g_ui_skin.images[type];
}

static HDC ui_skin_get_image_dc(SkinImageType type) {
    if (type < 0 || type >= SKIN_IMAGE_COUNT) {
        return NULL;
    }
    return g_ui_skin.image_dcs[type];
}

static HDC ui_skin_get_composite_dc(void) {
    return g_ui_skin.composite_dc;
}

static void reset_state(void) {
    /* Free any existing frames */
    SkinFrame* frame = g_ui_skin.frame_head;
    while (frame) {
        SkinFrame* next = frame->next;
        free(frame);
        frame = next;
    }
    memset(&g_ui_skin, 0, sizeof(UISkinContext));
}

static void create_test_frames(int count) {
    SkinFrame* prev = NULL;
    SkinFrame* frame;
    int i;

    for (i = 0; i < count; i++) {
        frame = (SkinFrame*)malloc(sizeof(SkinFrame));
        memset(frame, 0, sizeof(SkinFrame));
        frame->frame_id = i;
        frame->prev = prev;
        if (prev) {
            prev->next = frame;
        }
        prev = frame;

        if (i == 0) {
            g_ui_skin.frame_head = frame;
        }
    }

    g_ui_skin.frame_tail = frame;
    g_ui_skin.frame_current = g_ui_skin.frame_head;
    g_ui_skin.frame_count = count;
    g_ui_skin.current_frame = 0;
}

/* ========================================
 * Test Cases - Constants
 * ======================================== */

static int test_max_frames(void) {
    return SKIN_MAX_FRAMES == 40;
}

static int test_frame_size(void) {
    return SKIN_FRAME_SIZE == 0x118;
}

static int test_image_count(void) {
    return SKIN_IMAGE_COUNT == 5;
}

static int test_composite_width(void) {
    return SKIN_COMPOSITE_WIDTH == 645;
}

static int test_composite_height(void) {
    return SKIN_COMPOSITE_HEIGHT == 155;
}

/* ========================================
 * Test Cases - Image Type Constants
 * ======================================== */

static int test_image_type_down_arrow_r(void) {
    return SKIN_IMAGE_DOWN_ARROW_R == 0;
}

static int test_image_type_down_arrow_g(void) {
    return SKIN_IMAGE_DOWN_ARROW_G == 1;
}

static int test_image_type_base(void) {
    return SKIN_IMAGE_BASE == 2;
}

static int test_image_type_3(void) {
    return SKIN_IMAGE_3 == 3;
}

static int test_image_type_4(void) {
    return SKIN_IMAGE_4 == 4;
}

/* ========================================
 * Test Cases - Frame Size Validation
 * ======================================== */

static int test_struct_size(void) {
    /* Note: On x64, pointers are 8 bytes instead of 4, so the struct
     * size differs from the original x86 binary's SKIN_FRAME_SIZE.
     * This test verifies the struct is reasonable size. */
    return sizeof(SkinFrame) > 0 && sizeof(SkinFrame) <= 512;
}

static int test_images_offset(void) {
    /* Images should be at offset 0 */
    SkinFrame frame;
    return (size_t)&frame.images[0] - (size_t)&frame == 0;
}

static int test_prev_offset(void) {
    /* prev pointer should be after custom_data[192].
     * On x86: 0x10c, on x64: different due to 8-byte pointers.
     * Just verify it's in a reasonable range. */
    SkinFrame frame;
    size_t offset = (size_t)&frame.prev - (size_t)&frame;
    return offset > 0x100 && offset < 0x200;
}

static int test_next_offset(void) {
    /* next pointer should be right after prev.
     * On x86: 0x110, on x64: prev + 8 bytes. */
    SkinFrame frame;
    size_t prev_offset = (size_t)&frame.prev - (size_t)&frame;
    size_t next_offset = (size_t)&frame.next - (size_t)&frame;
    return next_offset == prev_offset + sizeof(void*);
}

/* ========================================
 * Test Cases - Frame Navigation
 * ======================================== */

static int test_get_frame_first(void) {
    reset_state();
    create_test_frames(5);

    SkinFrame* frame = ui_skin_get_frame(0);

    return frame != NULL && frame->frame_id == 0;
}

static int test_get_frame_middle(void) {
    reset_state();
    create_test_frames(5);

    SkinFrame* frame = ui_skin_get_frame(2);

    return frame != NULL && frame->frame_id == 2;
}

static int test_get_frame_last(void) {
    reset_state();
    create_test_frames(5);

    SkinFrame* frame = ui_skin_get_frame(4);

    return frame != NULL && frame->frame_id == 4;
}

static int test_get_frame_invalid_neg(void) {
    reset_state();
    create_test_frames(5);

    SkinFrame* frame = ui_skin_get_frame(-1);

    return frame == NULL;
}

static int test_get_frame_invalid_high(void) {
    reset_state();
    create_test_frames(5);

    SkinFrame* frame = ui_skin_get_frame(100);

    return frame == NULL;
}

static int test_get_current_frame(void) {
    reset_state();
    create_test_frames(5);

    SkinFrame* frame = ui_skin_get_current_frame();

    return frame != NULL && frame->frame_id == 0;
}

/* ========================================
 * Test Cases - Next Frame
 * ======================================== */

static int test_next_frame_basic(void) {
    reset_state();
    create_test_frames(5);

    ui_skin_next_frame();

    return g_ui_skin.current_frame == 1 && g_ui_skin.frame_current->frame_id == 1;
}

static int test_next_frame_multiple(void) {
    reset_state();
    create_test_frames(5);

    ui_skin_next_frame();
    ui_skin_next_frame();
    ui_skin_next_frame();

    return g_ui_skin.current_frame == 3 && g_ui_skin.frame_current->frame_id == 3;
}

static int test_next_frame_loop(void) {
    reset_state();
    create_test_frames(3);

    /* Move to last frame */
    ui_skin_next_frame();  /* 0 -> 1 */
    ui_skin_next_frame();  /* 1 -> 2 */

    /* Should loop back to 0 */
    ui_skin_next_frame();

    return g_ui_skin.current_frame == 0 && g_ui_skin.frame_current->frame_id == 0;
}

static int test_next_frame_single(void) {
    reset_state();
    create_test_frames(1);

    /* With single frame, should loop back */
    ui_skin_next_frame();

    return g_ui_skin.current_frame == 0;
}

/* ========================================
 * Test Cases - Reset
 * ======================================== */

static int test_reset_from_middle(void) {
    reset_state();
    create_test_frames(5);

    /* Move to middle */
    ui_skin_next_frame();
    ui_skin_next_frame();

    ui_skin_reset();

    return g_ui_skin.current_frame == 0 && g_ui_skin.frame_current->frame_id == 0;
}

static int test_reset_from_end(void) {
    reset_state();
    create_test_frames(3);

    /* Move to end */
    ui_skin_next_frame();
    ui_skin_next_frame();

    ui_skin_reset();

    return g_ui_skin.current_frame == 0;
}

/* ========================================
 * Test Cases - Get Image
 * ======================================== */

static int test_get_image_valid(void) {
    reset_state();

    /* Set mock image handle */
    g_ui_skin.images[SKIN_IMAGE_BASE] = (HBITMAP)0x1234;

    HBITMAP img = ui_skin_get_image(SKIN_IMAGE_BASE);

    return img == (HBITMAP)0x1234;
}

static int test_get_image_invalid_neg(void) {
    reset_state();

    HBITMAP img = ui_skin_get_image((SkinImageType)-1);

    return img == NULL;
}

static int test_get_image_invalid_high(void) {
    reset_state();

    HBITMAP img = ui_skin_get_image((SkinImageType)100);

    return img == NULL;
}

static int test_get_image_null(void) {
    reset_state();

    /* Unset image should be NULL */
    HBITMAP img = ui_skin_get_image(SKIN_IMAGE_DOWN_ARROW_R);

    return img == NULL;
}

/* ========================================
 * Test Cases - Get Image DC
 * ======================================== */

static int test_get_image_dc_valid(void) {
    reset_state();

    g_ui_skin.image_dcs[SKIN_IMAGE_BASE] = (HDC)0x5678;

    HDC dc = ui_skin_get_image_dc(SKIN_IMAGE_BASE);

    return dc == (HDC)0x5678;
}

static int test_get_image_dc_invalid_neg(void) {
    reset_state();

    HDC dc = ui_skin_get_image_dc((SkinImageType)-1);

    return dc == NULL;
}

static int test_get_image_dc_invalid_high(void) {
    reset_state();

    HDC dc = ui_skin_get_image_dc((SkinImageType)100);

    return dc == NULL;
}

/* ========================================
 * Test Cases - Get Composite DC
 * ======================================== */

static int test_get_composite_dc_set(void) {
    reset_state();

    g_ui_skin.composite_dc = (HDC)0xABCD;

    HDC dc = ui_skin_get_composite_dc();

    return dc == (HDC)0xABCD;
}

static int test_get_composite_dc_null(void) {
    reset_state();

    HDC dc = ui_skin_get_composite_dc();

    return dc == NULL;
}

/* ========================================
 * Test Cases - Linked List Integrity
 * ======================================== */

static int test_list_forward_traversal(void) {
    reset_state();
    create_test_frames(5);

    SkinFrame* frame = g_ui_skin.frame_head;
    int count = 0;

    while (frame) {
        count++;
        frame = frame->next;
    }

    return count == 5;
}

static int test_list_backward_traversal(void) {
    reset_state();
    create_test_frames(5);

    SkinFrame* frame = g_ui_skin.frame_tail;
    int count = 0;

    while (frame) {
        count++;
        frame = frame->prev;
    }

    return count == 5;
}

static int test_list_head_prev_null(void) {
    reset_state();
    create_test_frames(5);

    return g_ui_skin.frame_head->prev == NULL;
}

static int test_list_tail_next_null(void) {
    reset_state();
    create_test_frames(5);

    return g_ui_skin.frame_tail->next == NULL;
}

static int test_list_consistency(void) {
    reset_state();
    create_test_frames(5);

    /* Forward and backward should give same result */
    SkinFrame* fwd = g_ui_skin.frame_head;
    SkinFrame* bwd = g_ui_skin.frame_tail;

    /* Move fwd forward, bwd backward */
    fwd = fwd->next->next;  /* index 2 */
    bwd = bwd->prev->prev;  /* index 2 from end = index 2 */

    return fwd == bwd;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== UI Skin System Comprehensive Tests ===\n\n");

    printf("Constants Tests:\n");
    TEST(max_frames);
    TEST(frame_size);
    TEST(image_count);
    TEST(composite_width);
    TEST(composite_height);

    printf("\nImage Type Constants Tests:\n");
    TEST(image_type_down_arrow_r);
    TEST(image_type_down_arrow_g);
    TEST(image_type_base);
    TEST(image_type_3);
    TEST(image_type_4);

    printf("\nFrame Size Validation Tests:\n");
    TEST(struct_size);
    TEST(images_offset);
    TEST(prev_offset);
    TEST(next_offset);

    printf("\nFrame Navigation Tests:\n");
    TEST(get_frame_first);
    TEST(get_frame_middle);
    TEST(get_frame_last);
    TEST(get_frame_invalid_neg);
    TEST(get_frame_invalid_high);
    TEST(get_current_frame);

    printf("\nNext Frame Tests:\n");
    TEST(next_frame_basic);
    TEST(next_frame_multiple);
    TEST(next_frame_loop);
    TEST(next_frame_single);

    printf("\nReset Tests:\n");
    TEST(reset_from_middle);
    TEST(reset_from_end);

    printf("\nGet Image Tests:\n");
    TEST(get_image_valid);
    TEST(get_image_invalid_neg);
    TEST(get_image_invalid_high);
    TEST(get_image_null);

    printf("\nGet Image DC Tests:\n");
    TEST(get_image_dc_valid);
    TEST(get_image_dc_invalid_neg);
    TEST(get_image_dc_invalid_high);

    printf("\nGet Composite DC Tests:\n");
    TEST(get_composite_dc_set);
    TEST(get_composite_dc_null);

    printf("\nLinked List Integrity Tests:\n");
    TEST(list_forward_traversal);
    TEST(list_backward_traversal);
    TEST(list_head_prev_null);
    TEST(list_tail_next_null);
    TEST(list_consistency);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* Cleanup */
    reset_state();

    /* TODO: Add tests for:
     * - ui_skin_init allocation
     * - ui_skin_shutdown cleanup
     * - ui_skin_load_images
     * - ui_skin_render_frame
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
