/*
 * Stone Age Client - Field Update Handler Header
 * Reverse engineered from sa_9061.exe (FUN_00418370)
 */

#ifndef FIELD_UPDATE_H
#define FIELD_UPDATE_H

#include "types.h"

/* Action codes are defined in input.h, use those */

/* Menu states */
typedef enum {
    MENU_STATE_CLOSED = 0,
    MENU_STATE_OPENING = 1,
    MENU_STATE_ACTIVE = 2,
    MENU_STATE_CLOSING = 3
} MenuState;

/* Encounter states */
typedef enum {
    ENCOUNTER_NONE = 0,
    ENCOUNTER_TRIGGERED = 1,
    ENCOUNTER_TRANSITION = 2
} EncounterState;

/*
 * Initialize field update state
 */
void field_update_init(void);

/*
 * Shutdown field update
 */
void field_update_shutdown(void);

/*
 * Main field update - FUN_00418370
 * Called every frame during gameplay
 */
void field_update(void);

/*
 * Get current action menu state
 */
int field_get_menu_state(void);

/*
 * Set encounter state (for testing or scripted encounters)
 */
void field_set_encounter(int state);

#endif /* FIELD_UPDATE_H */
