/*
 * Stone Age Client - Ranking System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef RANKING_H
#define RANKING_H

#include "types.h"

/* Constants */
#define MAX_RANK_ENTRIES    100
#define MAX_RANK_NAME       24

/* Ranking types */
typedef enum {
    RANK_LEVEL = 0,         /* By level */
    RANK_GOLD = 1,          /* By gold */
    RANK_PK = 2,            /* By PK points */
    RANK_GUILD = 3,         /* Guild rankings */
    RANK_PET = 4,           /* Pet rankings */
    RANK_ACHIEVEMENT = 5,   /* Achievement points */
    RANK_PVP = 6,           /* PvP wins */
    RANK_CRAFT = 7          /* Crafting skill */
} RankType;

/* Ranking period */
typedef enum {
    RANK_PERIOD_DAILY = 0,
    RANK_PERIOD_WEEKLY = 1,
    RANK_PERIOD_MONTHLY = 2,
    RANK_PERIOD_ALLTIME = 3
} RankPeriod;

/* Rank entry */
typedef struct {
    u32 rank;
    u32 id;
    char name[MAX_RANK_NAME];
    u32 value;
    u32 value2;         /* Secondary stat */
    u8 job;
    u8 level;
    u16 guild_id;
    char guild_name[MAX_RANK_NAME];
} RankEntry;

/* Own ranking info */
typedef struct {
    u32 rank;
    u32 value;
    u32 total_players;
} RankSelfInfo;

/* Ranking context */
typedef struct {
    /* Current rankings */
    RankEntry entries[MAX_RANK_ENTRIES];
    int entry_count;
    RankType current_type;
    RankPeriod current_period;

    /* Own rank */
    RankSelfInfo self_info;

    /* UI state */
    int selected_index;
    int scroll_offset;
    u8 show_guild;      /* Show guild name in list */
    u8 request_pending;

    /* Timestamps */
    u32 last_update;
    u32 update_interval;

} RankingContext;

/* Global ranking context */
extern RankingContext g_ranking;

/* Initialization */
int ranking_init(void);
void ranking_shutdown(void);

/* Request rankings */
void ranking_request(RankType type, RankPeriod period);
void ranking_refresh(void);

/* Handle packets */
void ranking_handle_list(void* data, u32 size);
void ranking_handle_self(void* data, u32 size);

/* Query */
RankEntry* ranking_get_entry(int index);
int ranking_get_count(void);
RankType ranking_get_current_type(void);
u32 ranking_get_own_rank(void);
u32 ranking_get_own_value(void);

/* Find player */
int ranking_find_player(const char* name);
RankEntry* ranking_get_player_rank(const char* name);

/* UI */
void ranking_set_type(RankType type);
void ranking_set_period(RankPeriod period);
void ranking_select(int index);
RankEntry* ranking_get_selected(void);

/* Utility */
const char* ranking_type_to_string(RankType type);
const char* ranking_period_to_string(RankPeriod period);

#endif /* RANKING_H */
