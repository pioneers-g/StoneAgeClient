/*
 * Stone Age Client - Guild Protocol Handlers
 * Split from guild.c
 *
 * Contains protocol handlers for guild messages:
 * - Create/disband results
 * - Info/member list
 * - Invite handling
 * - Rank/notice changes
 * - Chat and contributions
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "guild.h"
#include "logger.h"

/* External guild context */
extern GuildContext g_guild;

/*
 * Handle guild create result
 */
void guild_handle_create_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;

    result = *ptr++;
    g_guild.create_pending = 0;

    if (result == 0) {
        /* Success */
        g_guild.guild.id = *(u32*)ptr; ptr += 4;
        strncpy(g_guild.guild.name, (char*)ptr, MAX_GUILD_NAME - 1);

        g_guild.in_guild = 1;
        g_guild.guild_loaded = 1;

        LOG_INFO("Guild created: %s (ID: %u)", g_guild.guild.name, g_guild.guild.id);
    } else {
        LOG_WARN("Failed to create guild: result %d", result);
    }
}

/*
 * Handle guild disband result
 */
void guild_handle_disband(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;

    result = *ptr++;

    if (result == 0) {
        LOG_INFO("Guild disbanded: %s", g_guild.guild.name);

        memset(&g_guild, 0, sizeof(GuildContext));
    } else {
        LOG_WARN("Failed to disband guild: result %d", result);
    }
}

/*
 * Handle guild info response
 */
void guild_handle_info(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int i;

    g_guild.guild.id = *(u32*)ptr; ptr += 4;
    if (g_guild.guild.id == 0) {
        g_guild.in_guild = 0;
        g_guild.guild_loaded = 1;
        return;
    }

    strncpy(g_guild.guild.name, (char*)ptr, MAX_GUILD_NAME - 1);
    ptr += strlen(g_guild.guild.name) + 1;

    strncpy(g_guild.guild.notice, (char*)ptr, MAX_GUILD_NOTICE - 1);
    ptr += strlen(g_guild.guild.notice) + 1;

    g_guild.guild.leader_id = *(u32*)ptr; ptr += 4;
    g_guild.guild.create_time = *(u32*)ptr; ptr += 4;
    g_guild.guild.member_count = *(u32*)ptr; ptr += 4;
    g_guild.guild.max_members = *(u32*)ptr; ptr += 4;
    g_guild.guild.level = *(u32*)ptr; ptr += 4;
    g_guild.guild.exp = *(u32*)ptr; ptr += 4;
    g_guild.guild.funds = *(u32*)ptr; ptr += 4;

    /* Parse rank info */
    for (i = 0; i < MAX_GUILD_RANK && ptr < (u8*)data + size; i++) {
        strncpy(g_guild.guild.ranks[i].name, (char*)ptr, 15);
        ptr += strlen(g_guild.guild.ranks[i].name) + 1;
        g_guild.guild.ranks[i].permissions = *ptr++;
    }

    g_guild.in_guild = 1;
    g_guild.guild_loaded = 1;

    LOG_INFO("Guild info loaded: %s (%u members)", g_guild.guild.name, g_guild.guild.member_count);
}

/*
 * Handle guild members response
 */
void guild_handle_members(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int i;
    GuildMember* member;

    g_guild.member_count = 0;

    for (i = 0; i < MAX_GUILD_MEMBERS && ptr < (u8*)data + size; i++) {
        member = &g_guild.members[i];

        member->id = *(u32*)ptr; ptr += 4;
        if (member->id == 0) continue;

        strncpy(member->name, (char*)ptr, sizeof(member->name) - 1);
        ptr += strlen(member->name) + 1;

        member->level = *(u16*)ptr; ptr += 2;
        member->job = *(u16*)ptr; ptr += 2;
        member->rank = *(u8*)ptr; ptr += 1;
        member->online = *(u8*)ptr; ptr += 1;
        member->contribution = *(u32*)ptr; ptr += 4;

        g_guild.member_count++;
    }

    g_guild.member_loaded = 1;
    LOG_INFO("Loaded %d guild members", g_guild.member_count);
}

/*
 * Handle guild invite result
 */
void guild_handle_invite(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;
    char name[24];

    result = *ptr++;
    strncpy(name, (char*)ptr, sizeof(name) - 1);

    if (result == 0) {
        LOG_INFO("Invited %s to guild", name);
    } else {
        LOG_WARN("Failed to invite %s: result %d", name, result);
    }
}

/*
 * Handle incoming guild invite
 */
void guild_handle_invite_request(void* data, u32 size) {
    u8* ptr = (u8*)data;

    g_guild.invite_guild_id = *(u32*)ptr; ptr += 4;
    strncpy(g_guild.invite_guild_name, (char*)ptr, MAX_GUILD_NAME - 1);
    ptr += strlen(g_guild.invite_guild_name) + 1;
    strncpy(g_guild.invite_leader_name, (char*)ptr, sizeof(g_guild.invite_leader_name) - 1);

    g_guild.invite_pending = 1;

    LOG_INFO("Guild invite from %s (%s)", g_guild.invite_guild_name, g_guild.invite_leader_name);
}

/*
 * Handle guild leave result
 */
void guild_handle_leave(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;

    result = *ptr++;

    if (result == 0) {
        LOG_INFO("Left guild: %s", g_guild.guild.name);
        memset(&g_guild, 0, sizeof(GuildContext));
    } else {
        LOG_WARN("Failed to leave guild: result %d", result);
    }
}

/*
 * Handle guild kick result
 */
void guild_handle_kick(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;
    char name[24];

    result = *ptr++;
    strncpy(name, (char*)ptr, sizeof(name) - 1);

    if (result == 0) {
        LOG_INFO("Kicked %s from guild", name);
    } else {
        LOG_WARN("Failed to kick %s: result %d", name, result);
    }
}

/*
 * Handle rank change result
 */
void guild_handle_rank_change(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 member_id;
    u8 new_rank;
    GuildMember* member;

    member_id = *(u32*)ptr; ptr += 4;
    new_rank = *ptr;

    member = guild_find_member_by_id(member_id);
    if (member) {
        member->rank = new_rank;
        LOG_DEBUG("Rank changed for %s to %d", member->name, new_rank);
    }
}

/*
 * Handle notice change
 */
void guild_handle_notice_change(void* data, u32 size) {
    u8* ptr = (u8*)data;

    strncpy(g_guild.guild.notice, (char*)ptr, MAX_GUILD_NOTICE - 1);
    LOG_INFO("Guild notice updated");
}

/*
 * Handle guild chat
 */
void guild_handle_chat(void* data, u32 size) {
    u8* ptr = (u8*)data;
    char sender[24];
    char message[256];

    strncpy(sender, (char*)ptr, sizeof(sender) - 1);
    ptr += strlen(sender) + 1;
    strncpy(message, (char*)ptr, sizeof(message) - 1);

    LOG_DEBUG("[Guild] %s: %s", sender, message);
}

/*
 * Handle leadership transfer
 */
void guild_handle_transfer(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 new_leader_id;
    u8 result;

    result = *ptr++;
    new_leader_id = *(u32*)ptr;

    if (result == 0) {
        g_guild.guild.leader_id = new_leader_id;
        LOG_INFO("Guild leadership transferred");
    }
}

/*
 * Handle contribution result
 */
void guild_handle_contribution(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 amount;
    u32 total;

    amount = *(u32*)ptr; ptr += 4;
    total = *(u32*)ptr;

    g_guild.guild.funds = total;
    LOG_INFO("Contributed %u gold, total: %u", amount, total);
}

/*
 * Handle member join notification
 */
void guild_handle_member_join(void* data, u32 size) {
    u8* ptr = (u8*)data;
    GuildMember* member;

    if (g_guild.member_count >= MAX_GUILD_MEMBERS) return;

    member = &g_guild.members[g_guild.member_count];
    member->id = *(u32*)ptr; ptr += 4;
    strncpy(member->name, (char*)ptr, sizeof(member->name) - 1);
    ptr += strlen(member->name) + 1;
    member->level = *(u16*)ptr; ptr += 2;
    member->job = *(u16*)ptr; ptr += 2;
    member->rank = *(u8*)ptr; ptr += 1;
    member->online = *(u8*)ptr;

    g_guild.member_count++;
    g_guild.guild.member_count++;

    LOG_INFO("%s joined the guild", member->name);
}

/*
 * Handle member leave notification
 */
void guild_handle_member_leave(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 member_id;
    int i;

    member_id = *(u32*)ptr;

    for (i = 0; i < g_guild.member_count; i++) {
        if (g_guild.members[i].id == member_id) {
            LOG_INFO("%s left the guild", g_guild.members[i].name);

            /* Remove member by shifting array */
            for (; i < g_guild.member_count - 1; i++) {
                g_guild.members[i] = g_guild.members[i + 1];
            }
            g_guild.member_count--;
            g_guild.guild.member_count--;
            break;
        }
    }
}

/*
 * Handle guild level up
 */
void guild_handle_level_up(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 new_level;

    new_level = *(u32*)ptr;
    g_guild.guild.level = new_level;

    LOG_INFO("Guild leveled up to %u", new_level);
}
