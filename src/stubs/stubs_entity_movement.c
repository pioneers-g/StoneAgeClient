/*
 * Stone Age Client - Entity Movement Stub Functions
 * Split from stubs_entity.c to reduce file size
 */

#include <windows.h>
#include <math.h>
#include "types.h"

/* Entity structure offsets from binary analysis */
#define ENTITY_OFFSET_X_QUEUE      0xc0
#define ENTITY_OFFSET_Y_QUEUE      0xe8
#define ENTITY_OFFSET_QUEUE_COUNT  0x110
#define ENTITY_OFFSET_CURRENT_X    0xb8
#define ENTITY_OFFSET_CURRENT_Y    0xbc
#define ENTITY_OFFSET_FLOAT_X      0x114
#define ENTITY_OFFSET_FLOAT_Y      0x118
#define ENTITY_OFFSET_VEL_X        0x11c
#define ENTITY_OFFSET_VEL_Y        0x120
#define ENTITY_OFFSET_DIRECTION    0x150
#define ENTITY_OFFSET_MOVE_FLAG    0x112

/* External speed constants */
extern float _DAT_0049c32c;  /* Default speed 1.0f */
extern float _DAT_0049c308;  /* Speed for queue < 6: 1.5f */
extern float _DAT_0049c330;  /* Speed for queue < 4: 2.0f */
extern float _DAT_0049c30c;  /* Speed for queue 2-3: 1.2f */
extern float _DAT_0049c318;  /* Minimum distance threshold: 0.001f */
extern float _DAT_0049c328;  /* Speed multiplier: 1.0f */
extern float _DAT_0049c324;  /* Direction offset: 0.0f */

/*
 * FUN_0040b6e0 - Entity Movement Queue Add
 *
 * Binary analysis:
 * - Adds a movement target to entity's path queue
 * - param_1: entity pointer
 * - param_2: target X coordinate
 * - param_3: target Y coordinate
 * - Queue stored at offset 0xc0 (X) and 0xe8 (Y), max 10 entries
 * - Queue count at offset 0x110
 * - If queue is full (10 entries), resets count to 0 and calls FUN_0040bfc0
 * - This allows for path queueing up to 10 waypoints
 */
void FUN_0040b6e0(void* entity, int target_x, int target_y) {
    unsigned short* queue_count;
    int* x_queue;
    int* y_queue;

    if (entity == NULL) return;

    /* Queue count at offset 0x110 */
    queue_count = (unsigned short*)((char*)entity + ENTITY_OFFSET_QUEUE_COUNT);

    /* Check if queue has space (< 10 entries) */
    if (*queue_count < 10) {
        /* X queue at offset 0xc0, Y queue at offset 0xe8 */
        x_queue = (int*)((char*)entity + ENTITY_OFFSET_X_QUEUE);
        y_queue = (int*)((char*)entity + ENTITY_OFFSET_Y_QUEUE);

        /* Add to queue */
        x_queue[*queue_count] = target_x;
        y_queue[*queue_count] = target_y;

        /* Increment count */
        (*queue_count)++;
    } else {
        /* Queue full - reset and call immediate movement */
        *queue_count = 0;
        /* FUN_0040bfc0(entity, target_x, target_y); - immediate move */
    }
}

/*
 * FUN_0040b740 - Entity Movement with Interpolation
 *
 * Binary analysis:
 * - Adds smooth path interpolation to entity movement
 * - Calculates intermediate waypoints for diagonal movement
 * - param_1: entity pointer
 * - param_2: target X coordinate
 * - param_3: target Y coordinate
 * - Uses current position from entity at offsets 0xb8 (X) and 0xbc (Y)
 *   or last queued position if queue not empty
 * - For diagonal moves (delta = 2 in both axes), adds midpoint
 * - Handles path queue overflow (> 10 entries)
 */
void FUN_0040b740(void* entity, int target_x, int target_y) {
    short* queue_count;
    int* x_queue;
    int* y_queue;
    int current_x, current_y;
    int delta_x, delta_y;
    int abs_dx, abs_dy;
    int waypoints[3][2];  /* Max 3 waypoints: midpoint, target */
    int num_waypoints = 1;
    int i;

    if (entity == NULL) return;

    queue_count = (short*)((char*)entity + ENTITY_OFFSET_QUEUE_COUNT);
    x_queue = (int*)((char*)entity + ENTITY_OFFSET_X_QUEUE);
    y_queue = (int*)((char*)entity + ENTITY_OFFSET_Y_QUEUE);

    /* Get current position or last queued position */
    if (*queue_count < 1) {
        current_x = *(int*)((char*)entity + ENTITY_OFFSET_CURRENT_X);
        current_y = *(int*)((char*)entity + ENTITY_OFFSET_CURRENT_Y);
    } else {
        current_x = x_queue[*queue_count - 1];
        current_y = y_queue[*queue_count - 1];
    }

    /* Calculate delta */
    delta_x = target_x - current_x;
    delta_y = target_y - current_y;
    abs_dx = (delta_x < 0) ? -delta_x : delta_x;
    abs_dy = (delta_y < 0) ? -delta_y : delta_y;

    /* Check for diagonal movement (delta = 2 in both axes) */
    if (abs_dx == 2 && abs_dy == 2) {
        /* Add midpoint for smooth diagonal */
        waypoints[0][0] = current_y + delta_y / 2;  /* Y */
        waypoints[0][1] = current_x + delta_x / 2;  /* X */
        num_waypoints = 2;
    } else if (abs_dx == 2) {
        /* Diagonal in X only */
        waypoints[0][0] = target_y;
        waypoints[0][1] = current_x + delta_x / 2;
        num_waypoints = 2;
    } else if (abs_dy == 2) {
        /* Diagonal in Y only */
        waypoints[0][0] = current_y + delta_y / 2;
        waypoints[0][1] = target_x;
        num_waypoints = 2;
    }

    /* Add target as final waypoint */
    waypoints[num_waypoints - 1][0] = target_y;
    waypoints[num_waypoints - 1][1] = target_x;

    /* Check if we have room in queue */
    if ((unsigned int)(*queue_count + num_waypoints) < 11) {
        for (i = 0; i < num_waypoints; i++) {
            x_queue[*queue_count] = waypoints[i][1];
            y_queue[*queue_count] = waypoints[i][0];
            (*queue_count)++;
        }
    } else {
        /* Queue would overflow - reset and use immediate move */
        *queue_count = 0;
        /* FUN_0040bfc0(entity, target_x, target_y); */
    }
}

/*
 * FUN_0040bb00 - Entity Queue Shift
 *
 * Binary analysis:
 * - Shifts entity movement queue left by one position
 * - Decrements queue count
 * - param_1: entity pointer
 * - Called when entity reaches a waypoint
 */
void FUN_0040bb00(void* entity) {
    short* queue_count;
    int* x_queue;
    int* y_queue;
    int i;

    if (entity == NULL) return;

    queue_count = (short*)((char*)entity + ENTITY_OFFSET_QUEUE_COUNT);
    if (*queue_count <= 0) return;

    /* Decrement count */
    (*queue_count)--;

    /* Shift queue left */
    if (*queue_count > 0) {
        x_queue = (int*)((char*)entity + ENTITY_OFFSET_X_QUEUE);
        y_queue = (int*)((char*)entity + ENTITY_OFFSET_Y_QUEUE);

        for (i = 0; i < *queue_count; i++) {
            x_queue[i] = x_queue[i + 1];
            y_queue[i] = y_queue[i + 1];
        }
    }
}

/*
 * FUN_0040b880 - Entity Set Target Position with Speed
 *
 * Binary analysis:
 * - Sets entity movement target with speed calculation
 * - param_1: entity pointer
 * - param_2: target X (world coordinates)
 * - param_3: target Y (world coordinates)
 * - Calculates velocity based on distance and speed constants
 * - Uses SQRT for distance calculation
 * - Sets direction via FUN_00447150
 */
void FUN_0040b880(void* entity, float target_x, float target_y) {
    short queue_count;
    float speed, dx, dy, dist;

    if (entity == NULL) return;

    queue_count = *(short*)((char*)entity + ENTITY_OFFSET_QUEUE_COUNT);

    /* Determine speed based on queue count */
    speed = _DAT_0049c32c;
    if (queue_count < 6) {
        speed = _DAT_0049c308;
        if (queue_count < 4) {
            speed = _DAT_0049c330;
            if (queue_count > 1) {
                speed = _DAT_0049c30c;
            }
        }
    }

    /* Calculate direction vector */
    dx = ((int)target_x << 6) - *(float*)((char*)entity + ENTITY_OFFSET_FLOAT_X);
    dy = ((int)target_y << 6) - *(float*)((char*)entity + ENTITY_OFFSET_FLOAT_Y);

    /* Calculate distance */
    dist = (float)sqrt(dx * dx + dy * dy);

    /* Normalize or zero if too close */
    if (dist <= _DAT_0049c318) {
        dx = 0.0f;
        dy = 0.0f;
    } else {
        dx = dx / dist;
        dy = dy / dist;
    }

    /* Set position and velocity */
    *(int*)((char*)entity + ENTITY_OFFSET_CURRENT_X) = (int)target_x;
    *(float*)((char*)entity + ENTITY_OFFSET_CURRENT_Y) = target_y;
    *(float*)((char*)entity + ENTITY_OFFSET_VEL_X) = dx * speed * _DAT_0049c328;
    *(float*)((char*)entity + ENTITY_OFFSET_VEL_Y) = dy * speed * _DAT_0049c328;

    /* Set direction if moving */
    if (dx != 0.0f || dy != 0.0f) {
        float direction = (float)0;  /* FUN_00447150(dx, dy) + _DAT_0049c324 */
        /* FUN_004470d0(&direction) - normalize direction */
        *(int*)((char*)entity + ENTITY_OFFSET_DIRECTION) = (int)direction;
        *(short*)((char*)entity + ENTITY_OFFSET_MOVE_FLAG) = 1;
    }
}

/*
 * FUN_0040b9e0 - Entity Set Target Position Simple
 *
 * Binary analysis:
 * - Sets entity movement target without speed modification
 * - param_1: entity pointer
 * - param_2: target X (world coordinates)
 * - param_3: target Y (world coordinates)
 * - Simpler version of FUN_0040b880
 */
void FUN_0040b9e0(void* entity, float target_x, float target_y) {
    float dx, dy, dist;

    if (entity == NULL) return;

    /* Calculate direction vector */
    dx = ((int)target_x << 6) - *(float*)((char*)entity + ENTITY_OFFSET_FLOAT_X);
    dy = ((int)target_y << 6) - *(float*)((char*)entity + ENTITY_OFFSET_FLOAT_Y);

    /* Calculate distance */
    dist = (float)sqrt(dx * dx + dy * dy);

    /* Normalize or zero if too close */
    if (dist <= _DAT_0049c318) {
        dx = 0.0f;
        dy = 0.0f;
    } else {
        dx = dx / dist;
        dy = dy / dist;
    }

    /* Set position and velocity */
    *(float*)((char*)entity + ENTITY_OFFSET_CURRENT_X) = target_x;
    *(int*)((char*)entity + ENTITY_OFFSET_CURRENT_Y) = (int)target_y;
    *(float*)((char*)entity + ENTITY_OFFSET_VEL_X) = dx * _DAT_0049c328;
    *(float*)((char*)entity + ENTITY_OFFSET_VEL_Y) = dy * _DAT_0049c328;

    /* Set direction if moving */
    if (dx != 0.0f || dy != 0.0f) {
        float direction = (float)0;  /* FUN_00447150(dx, dy) */
        *(int*)((char*)entity + ENTITY_OFFSET_DIRECTION) = (int)direction;
        *(short*)((char*)entity + ENTITY_OFFSET_MOVE_FLAG) = 1;
    }
}
