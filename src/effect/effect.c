/*
 * Stone Age Client - Effect System Main Module
 * This file provides backward compatibility by including all sub-modules
 *
 * Split into:
 * - effect_core.c: Initialization, database, utility functions
 * - effect_manager.c: Add/remove/query active effects
 * - effect_update.c: Update loop and tick processing
 * - effect_query.c: Character state queries (can_act, can_cast, etc.)
 * - effect_protocol.c: Network packet handlers
 * - pet_effect.c: Pet-specific effect functions
 */

#include "effect.h"

/* All functionality is implemented in the sub-modules above */
