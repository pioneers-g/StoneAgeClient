/*
 * Stone Age Client - Ranking System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "ranking.h"
#include "network.h"
#include "network_protocol.h"
#include "logger.h"

/* Global ranking context */
RankingContext g_ranking = {0};

/*
 * Initialize ranking system
 */
int ranking_init(void) {
    memset(&g_ranking, 0, sizeof(RankingContext));

    g_ranking.current_type = RANK_LEVEL;
    g_ranking.current_period = RANK_PERIOD_ALLTIME;
    g_ranking.update_interval = 60000;  /* 1 minute */

    LOG_INFO("Ranking system initialized");
    return 1;
}

/*
 * Shutdown ranking system
 */
void ranking_shutdown(void) {
    memset(&g_ranking, 0, sizeof(RankingContext));
    LOG_INFO("Ranking system shutdown");
}

/*
 * Request rankings from server
 */
void ranking_request(RankType type, RankPeriod period) {
    g_ranking.current_type = type;
    g_ranking.current_period = period;
    g_ranking.request_pending = 1;

    /* Send request packet: RANK|type|period */
    char packet[32];
    _snprintf(packet, sizeof(packet), "RANK|%d|%d", type, period);
    send_queue_add(packet, 0, 0);

    LOG_DEBUG("Requesting rankings: type=%d period=%d", type, period);
}

/*
 * Refresh current rankings
 */
void ranking_refresh(void) {
    ranking_request(g_ranking.current_type, g_ranking.current_period);
}

/*
 * Handle ranking list packet
 */
void ranking_handle_list(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int i;
    RankEntry* entry;

    g_ranking.entry_count = 0;

    u8 type = *ptr++;
    u8 period = *ptr++;
    u16 count = *(u16*)ptr; ptr += 2;

    g_ranking.current_type = type;
    g_ranking.current_period = period;

    for (i = 0; i < count && i < MAX_RANK_ENTRIES && ptr < (u8*)data + size; i++) {
        entry = &g_ranking.entries[i];

        entry->rank = *(u32*)ptr; ptr += 4;
        entry->id = *(u32*)ptr; ptr += 4;

        strncpy(entry->name, (char*)ptr, MAX_RANK_NAME - 1);
        ptr += strlen(entry->name) + 1;

        entry->value = *(u32*)ptr; ptr += 4;
        entry->value2 = *(u32*)ptr; ptr += 4;
        entry->job = *(u8*)ptr; ptr += 1;
        entry->level = *(u8*)ptr; ptr += 1;
        entry->guild_id = *(u16*)ptr; ptr += 2;

        if (entry->guild_id != 0) {
            strncpy(entry->guild_name, (char*)ptr, MAX_RANK_NAME - 1);
            ptr += strlen(entry->guild_name) + 1;
        }

        g_ranking.entry_count++;
    }

    g_ranking.request_pending = 0;
    g_ranking.last_update = timeGetTime();

    LOG_INFO("Received %d ranking entries", g_ranking.entry_count);
}

/*
 * Handle own ranking info packet
 */
void ranking_handle_self(void* data, u32 size) {
    u8* ptr = (u8*)data;

    g_ranking.self_info.rank = *(u32*)ptr; ptr += 4;
    g_ranking.self_info.value = *(u32*)ptr; ptr += 4;
    g_ranking.self_info.total_players = *(u32*)ptr;

    LOG_DEBUG("Own rank: %u / %u (value: %u)",
              g_ranking.self_info.rank,
              g_ranking.self_info.total_players,
              g_ranking.self_info.value);
}

/*
 * Get entry by index
 */
RankEntry* ranking_get_entry(int index) {
    if (index < 0 || index >= g_ranking.entry_count) {
        return NULL;
    }
    return &g_ranking.entries[index];
}

/*
 * Get entry count
 */
int ranking_get_count(void) {
    return g_ranking.entry_count;
}

/*
 * Get current type
 */
RankType ranking_get_current_type(void) {
    return g_ranking.current_type;
}

/*
 * Get own rank
 */
u32 ranking_get_own_rank(void) {
    return g_ranking.self_info.rank;
}

/*
 * Get own value
 */
u32 ranking_get_own_value(void) {
    return g_ranking.self_info.value;
}

/*
 * Find player in rankings
 */
int ranking_find_player(const char* name) {
    int i;

    for (i = 0; i < g_ranking.entry_count; i++) {
        if (strcmp(g_ranking.entries[i].name, name) == 0) {
            return i;
        }
    }

    return -1;
}

/*
 * Get player rank entry
 */
RankEntry* ranking_get_player_rank(const char* name) {
    int index = ranking_find_player(name);
    return index >= 0 ? &g_ranking.entries[index] : NULL;
}

/*
 * Set ranking type
 */
void ranking_set_type(RankType type) {
    if (type != g_ranking.current_type) {
        g_ranking.current_type = type;
        ranking_request(type, g_ranking.current_period);
    }
}

/*
 * Set ranking period
 */
void ranking_set_period(RankPeriod period) {
    if (period != g_ranking.current_period) {
        g_ranking.current_period = period;
        ranking_request(g_ranking.current_type, period);
    }
}

/*
 * Select entry
 */
void ranking_select(int index) {
    if (index >= 0 && index < g_ranking.entry_count) {
        g_ranking.selected_index = index;
    }
}

/*
 * Get selected entry
 */
RankEntry* ranking_get_selected(void) {
    return ranking_get_entry(g_ranking.selected_index);
}

/*
 * Rank type to string
 */
const char* ranking_type_to_string(RankType type) {
    static const char* type_names[] = {
        "Level", "Gold", "PK", "Guild", "Pet", "Achievement", "PvP", "Craft"
    };

    if (type < sizeof(type_names) / sizeof(type_names[0])) {
        return type_names[type];
    }
    return "Unknown";
}

/*
 * Rank period to string
 */
const char* ranking_period_to_string(RankPeriod period) {
    static const char* period_names[] = {
        "Daily", "Weekly", "Monthly", "All Time"
    };

    if (period < sizeof(period_names) / sizeof(period_names[0])) {
        return period_names[period];
    }
    return "Unknown";
}
