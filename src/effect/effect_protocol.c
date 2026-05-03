/*
 * Stone Age Client - Effect Protocol Handlers
 * Network packet handlers for effect system
 */

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "effect.h"
#include "logger.h"

/*
 * Handle effect add packet
 */
void effect_handle_add(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u16 effect_id;
    u16 level;
    u32 duration;
    u32 source_id;

    effect_id = *(u16*)ptr; ptr += 2;
    level = *(u16*)ptr; ptr += 2;
    duration = *(u32*)ptr; ptr += 4;
    source_id = *(u32*)ptr;

    effect_add(effect_id, level, duration, source_id);
}

/*
 * Handle effect remove packet
 */
void effect_handle_remove(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u16 effect_id;

    effect_id = *(u16*)ptr;
    effect_remove(effect_id);
}

/*
 * Handle effect list packet
 */
void effect_handle_list(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int i, count;

    effect_remove_all();

    count = *(u8*)ptr++;

    for (i = 0; i < count && ptr < (u8*)data + size; i++) {
        u16 effect_id = *(u16*)ptr; ptr += 2;
        u16 level = *(u16*)ptr; ptr += 2;
        u32 duration = *(u32*)ptr; ptr += 4;
        u32 source_id = *(u32*)ptr; ptr += 4;

        effect_add(effect_id, level, duration, source_id);
    }

    LOG_DEBUG("Received %d effects", count);
}

/*
 * Handle pet effect add packet
 */
void effect_handle_pet_add(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 pet_slot;
    u16 effect_id;
    u16 level;
    u32 duration;

    pet_slot = *(u8*)ptr++;
    effect_id = *(u16*)ptr; ptr += 2;
    level = *(u16*)ptr; ptr += 2;
    duration = *(u32*)ptr;

    pet_effect_add(pet_slot, effect_id, level, duration);
}

/*
 * Handle pet effect remove packet
 */
void effect_handle_pet_remove(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 pet_slot;
    u16 effect_id;

    pet_slot = *(u8*)ptr++;
    effect_id = *(u16*)ptr;

    pet_effect_remove(pet_slot, effect_id);
}
