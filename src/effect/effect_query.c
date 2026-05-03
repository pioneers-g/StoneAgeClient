/*
 * Stone Age Client - Effect Query Functions
 * Check character state based on active effects
 */

#include "types.h"
#include "effect.h"

/* External global */
extern EffectContext g_effect;

/*
 * Check if can act
 */
int effect_can_act(void) {
    return !effect_has(EFFECT_STUN) &&
           !effect_has(EFFECT_PARALYZE) &&
           !effect_has(EFFECT_SLEEP) &&
           !effect_has(EFFECT_STONE) &&
           !effect_has(EFFECT_PETRIFY);
}

/*
 * Check if can cast magic
 */
int effect_can_cast(void) {
    return !effect_has(EFFECT_SILENCE) &&
           !effect_has(EFFECT_STUN) &&
           !effect_has(EFFECT_PARALYZE);
}

/*
 * Check if can move
 */
int effect_can_move(void) {
    return !effect_has(EFFECT_STUN) &&
           !effect_has(EFFECT_PARALYZE) &&
           !effect_has(EFFECT_SLEEP) &&
           !effect_has(EFFECT_FREEZE) &&
           !effect_has(EFFECT_STONE);
}

/*
 * Check if stunned
 */
int effect_is_stunned(void) {
    return effect_has(EFFECT_STUN) || effect_has(EFFECT_PARALYZE);
}

/*
 * Check if immobilized
 */
int effect_is_immobilized(void) {
    return effect_has(EFFECT_STUN) ||
           effect_has(EFFECT_PARALYZE) ||
           effect_has(EFFECT_SLEEP) ||
           effect_has(EFFECT_FREEZE) ||
           effect_has(EFFECT_STONE) ||
           effect_has(EFFECT_PETRIFY);
}

/*
 * Check if is silenced
 */
int effect_is_silenced(void) {
    return effect_has(EFFECT_SILENCE);
}

/*
 * Check if is blind
 */
int effect_is_blind(void) {
    return effect_has(EFFECT_BLIND);
}

/*
 * Check if is confused
 */
int effect_is_confused(void) {
    return effect_has(EFFECT_CONFUSE);
}

/*
 * Check if is asleep
 */
int effect_is_asleep(void) {
    return effect_has(EFFECT_SLEEP);
}

/*
 * Check if has immunity
 */
int effect_has_immunity(void) {
    return effect_has(EFFECT_IMMUNITY);
}

/*
 * Check if can be dispelled
 */
int effect_can_dispel(u16 effect_id) {
    EffectData* data = effect_get_data(effect_id);
    return data && (data->flags & EFFECT_FLAG_DISPELLABLE);
}
