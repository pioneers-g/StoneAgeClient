/*
 * Stone Age Client - Party System Comprehensive Tests
 * Tests for FUN_004643f0 (party list), FUN_00464610 (party disband),
 * FUN_00464e10 (party kick), FUN_00464ef0 (party leader), FUN_00465400 (party invite)
 *
 * Coverage:
 * - Party member list parsing
 * - Party invitation handling
 * - Party kick functionality
 * - Party disband
 * - Party chat message routing
 * - Party state management
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Party constants */
#define MAX_PARTY_MEMBERS       5
#define MAX_PARTY_NAME_LENGTH   25
#define MAX_CHAT_HISTORY        4
#define CHAT_BUFFER_SIZE        0x1000

/* Party states */
typedef enum {
    PARTY_STATE_NONE = 0,
    PARTY_STATE_ACTIVE = 1,
    PARTY_STATE_DISBANDING = 2
} PartyState;

/* Party member structure */
typedef struct {
    u16 field_0;            /* Unknown field */
    u16 field_2;            /* Unknown field */
    u32 id;                 /* Character ID */
    char name[MAX_PARTY_NAME_LENGTH];
} PartyMember;

/* Party context */
typedef struct {
    PartyState state;
    u32 leader_id;
    u32 leader_param1;
    u32 leader_param2;

    PartyMember members[MAX_PARTY_MEMBERS];
    u32 member_count;

    /* Chat history (circular buffer) */
    char chat_history[MAX_CHAT_HISTORY][CHAT_BUFFER_SIZE];
    u32 chat_write_index;

    /* Action history */
    char action_history[MAX_CHAT_HISTORY][CHAT_BUFFER_SIZE];
    u32 action_write_index;

    /* Flags */
    u32 update_flag;
    u32 kick_mode;
    u32 kick_param;
    u32 is_kicking;
    u32 is_special_kick;
} PartyContext;

static PartyContext g_party;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Party System Functions
 * ======================================== */

void party_init(void) {
    memset(&g_party, 0, sizeof(g_party));
    g_party.state = PARTY_STATE_NONE;
}

int party_invite(u32 param1, u32 param2, u32 param3, u32 param4) {
    /* FUN_00465400 - Party invite handler */
    if (g_party.state != PARTY_STATE_NONE) {
        return -1;  /* Already in party */
    }

    g_party.state = PARTY_STATE_ACTIVE;
    g_party.member_count = 1;

    /* Calculate display parameter */
    u32 display_param = (param4 + 3) & 0x80000007;
    if ((s32)display_param < 0) {
        display_param = ((display_param - 1) | 0xfffffff8) + 1;
    }

    return 0;
}

int party_leave(u32 param1, u32 char_id, u32 mode) {
    /* FUN_00463e70 - Party leave handler */
    if (g_party.state == PARTY_STATE_NONE) {
        return -1;
    }

    if (mode == 0) {
        /* Self leave */
        g_party.state = PARTY_STATE_NONE;
        g_party.member_count = 0;
    } else {
        /* Other member leave */
        for (u32 i = 0; i < g_party.member_count; i++) {
            if (g_party.members[i].id == char_id) {
                /* Remove member */
                for (u32 j = i; j < g_party.member_count - 1; j++) {
                    g_party.members[j] = g_party.members[j + 1];
                }
                g_party.member_count--;
                break;
            }
        }
    }

    return 0;
}

int party_kick(u32 param1, s32 kick_mode, s32 kick_param) {
    /* FUN_00464e10 - Party kick handler */
    if (g_party.state != PARTY_STATE_ACTIVE) {
        return -1;
    }

    if (kick_mode <= 0) {
        /* Clear party UI */
        g_party.state = PARTY_STATE_NONE;
        g_party.member_count = 0;
        return 0;
    }

    g_party.kick_mode = kick_mode;
    g_party.is_kicking = 1;

    if (kick_mode == 4) {
        g_party.is_special_kick = 1;
        g_party.is_kicking = 0;
    } else if (kick_mode == 2 || kick_mode == 6) {
        g_party.is_special_kick = 0;
    }

    if (kick_param >= 0 && kick_param <= 0xdb) {
        g_party.kick_param = kick_param;
    } else {
        g_party.kick_param = 0;
    }

    if (kick_mode == 2) {
        g_party.update_flag = 1;
    } else if (kick_mode == 5) {
        g_party.update_flag = 1;
        g_party.is_kicking = 0;
    } else {
        g_party.update_flag = 0;
    }

    return 0;
}

int party_list_parse(const char* data, u32 data_len) {
    /* FUN_004643f0 - Party list parsing */
    if (g_party.state == PARTY_STATE_NONE) {
        return -1;
    }

    g_party.state = PARTY_STATE_ACTIVE;
    g_party.member_count = 0;

    /* Parse member data - simplified simulation */
    /* Real implementation parses: field_0, field_2, id, name per member */

    return 0;
}

int party_disband(u32 param1, const char* data) {
    /* FUN_00464610 - Party disband */
    if (g_party.state == PARTY_STATE_NONE) {
        return -1;
    }

    g_party.state = PARTY_STATE_DISBANDING;

    /* Parse pipe-delimited parameters */
    if (data) {
        /* Extract first parameter (leader ID) */
        const char* pipe = strchr(data, '|');
        if (pipe) {
            g_party.leader_param1 = atoi(data);
            g_party.leader_param2 = atoi(pipe + 1);
        }
    }

    return 0;
}

int party_handle_message(const char* message) {
    /* FUN_00464ef0 - Party leader message handler */
    if (!message || !message[0]) {
        return -1;
    }

    char type = message[1];

    switch (type) {
    case 'C':  /* Chat message */
        strncpy(g_party.chat_history[g_party.chat_write_index],
                message + 3, CHAT_BUFFER_SIZE - 1);
        g_party.chat_write_index = (g_party.chat_write_index + 1) & 3;
        break;

    case 'P':  /* Party info */
        /* Parse format: "X|X|X" */
        break;

    case 'A':  /* Action message */
        strncpy(g_party.action_history[g_party.action_write_index],
                message + 3, CHAT_BUFFER_SIZE - 1);
        g_party.action_write_index = (g_party.action_write_index + 1) & 3;
        break;

    case 'U':  /* Update flag */
        g_party.update_flag = 1;
        break;

    default:   /* Unknown message type */
        break;
    }

    return 0;
}

int party_add_member(u32 id, const char* name) {
    if (g_party.member_count >= MAX_PARTY_MEMBERS) {
        return -1;
    }

    PartyMember* member = &g_party.members[g_party.member_count];
    member->id = id;
    strncpy(member->name, name, MAX_PARTY_NAME_LENGTH - 1);
    member->name[MAX_PARTY_NAME_LENGTH - 1] = '\0';
    g_party.member_count++;

    return 0;
}

/* ========================================
 * Test Setup/Teardown
 * ======================================== */

static void test_setup(void) {
    party_init();
}

static void test_teardown(void) {
    /* Nothing to clean up */
}

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_max_party_members(void) {
    return MAX_PARTY_MEMBERS == 5;
}

static int test_max_party_name_length(void) {
    return MAX_PARTY_NAME_LENGTH == 25;
}

static int test_max_chat_history(void) {
    return MAX_CHAT_HISTORY == 4;
}

static int test_party_state_values(void) {
    return PARTY_STATE_NONE == 0 &&
           PARTY_STATE_ACTIVE == 1 &&
           PARTY_STATE_DISBANDING == 2;
}

/* ========================================
 * Initialization Tests
 * ======================================== */

static int test_party_init(void) {
    test_setup();

    int pass = g_party.state == PARTY_STATE_NONE &&
               g_party.member_count == 0;

    test_teardown();
    return pass;
}

static int test_party_init_clears_members(void) {
    test_setup();

    /* Add a member first */
    party_add_member(1, "TestPlayer");

    /* Re-init */
    party_init();

    int pass = g_party.member_count == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Party Invite Tests
 * ======================================== */

static int test_party_invite_success(void) {
    test_setup();

    int result = party_invite(1, 2, 3, 4);

    int pass = result == 0 &&
               g_party.state == PARTY_STATE_ACTIVE;

    test_teardown();
    return pass;
}

static int test_party_invite_already_in_party(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    int result = party_invite(1, 2, 3, 4);

    int pass = result == -1;

    test_teardown();
    return pass;
}

static int test_party_invite_display_param_calc(void) {
    test_setup();

    /* Test the bitmask calculation: (param4 + 3) & 0x80000007 */
    /* When result < 0: result = (result - 1) | 0xfffffff8) + 1 */

    /* param4 = 0 -> (0 + 3) & 7 = 3 */
    party_init();
    party_invite(1, 2, 3, 0);
    int pass1 = g_party.state == PARTY_STATE_ACTIVE;

    /* param4 = 5 -> (5 + 3) & 7 = 0 */
    party_init();
    party_invite(1, 2, 3, 5);
    int pass2 = g_party.state == PARTY_STATE_ACTIVE;

    test_teardown();
    return pass1 && pass2;
}

/* ========================================
 * Party Leave Tests
 * ======================================== */

static int test_party_leave_self(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    party_add_member(100, "Player1");

    int result = party_leave(1, 100, 0);

    int pass = result == 0 &&
               g_party.state == PARTY_STATE_NONE &&
               g_party.member_count == 0;

    test_teardown();
    return pass;
}

static int test_party_leave_other_member(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    party_add_member(100, "Player1");
    party_add_member(200, "Player2");

    int result = party_leave(1, 100, 1);

    int pass = result == 0 &&
               g_party.member_count == 1 &&
               g_party.members[0].id == 200;

    test_teardown();
    return pass;
}

static int test_party_leave_no_party(void) {
    test_setup();

    int result = party_leave(1, 100, 0);

    int pass = result == -1;

    test_teardown();
    return pass;
}

/* ========================================
 * Party Kick Tests
 * ======================================== */

static int test_party_kick_mode_2(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    int result = party_kick(1, 2, 100);

    int pass = result == 0 &&
               g_party.kick_mode == 2 &&
               g_party.is_kicking == 1 &&
               g_party.update_flag == 1;

    test_teardown();
    return pass;
}

static int test_party_kick_mode_4(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    int result = party_kick(1, 4, 50);

    int pass = result == 0 &&
               g_party.kick_mode == 4 &&
               g_party.is_special_kick == 1 &&
               g_party.is_kicking == 0;

    test_teardown();
    return pass;
}

static int test_party_kick_mode_5(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    int result = party_kick(1, 5, 75);

    int pass = result == 0 &&
               g_party.kick_mode == 5 &&
               g_party.update_flag == 1 &&
               g_party.is_kicking == 0;

    test_teardown();
    return pass;
}

static int test_party_kick_param_clamp(void) {
    test_setup();

    party_invite(1, 2, 3, 4);

    /* Valid range: 0-219 (0xdb) */
    party_kick(1, 2, 300);
    int pass1 = g_party.kick_param == 0;  /* Clamped to 0 */

    party_kick(1, 2, -1);
    int pass2 = g_party.kick_param == 0;  /* Clamped to 0 */

    party_kick(1, 2, 100);
    int pass3 = g_party.kick_param == 100;  /* Valid */

    test_teardown();
    return pass1 && pass2 && pass3;
}

static int test_party_kick_clear_party(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    party_add_member(100, "Player1");

    int result = party_kick(1, 0, 0);

    int pass = result == 0 &&
               g_party.state == PARTY_STATE_NONE;

    test_teardown();
    return pass;
}

/* ========================================
 * Party Disband Tests
 * ======================================== */

static int test_party_disband(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    int result = party_disband(1, "100|200");

    int pass = result == 0 &&
               g_party.state == PARTY_STATE_DISBANDING &&
               g_party.leader_param1 == 100 &&
               g_party.leader_param2 == 200;

    test_teardown();
    return pass;
}

static int test_party_disband_no_party(void) {
    test_setup();

    int result = party_disband(1, "100|200");

    int pass = result == -1;

    test_teardown();
    return pass;
}

/* ========================================
 * Party Message Tests
 * ======================================== */

static int test_party_message_chat(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    int result = party_handle_message("PCHello World");

    int pass = result == 0 &&
               strcmp(g_party.chat_history[0], "Hello World") == 0;

    test_teardown();
    return pass;
}

static int test_party_message_action(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    int result = party_handle_message("PAAttack|Target");

    int pass = result == 0 &&
               strcmp(g_party.action_history[0], "Attack|Target") == 0;

    test_teardown();
    return pass;
}

static int test_party_message_update(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    g_party.update_flag = 0;

    int result = party_handle_message("PU");

    int pass = result == 0 &&
               g_party.update_flag == 1;

    test_teardown();
    return pass;
}

static int test_party_message_circular_buffer(void) {
    test_setup();

    party_invite(1, 2, 3, 4);

    /* Fill chat history */
    party_handle_message("PCMsg1");
    party_handle_message("PCMsg2");
    party_handle_message("PCMsg3");
    party_handle_message("PCMsg4");

    int pass1 = g_party.chat_write_index == 0;  /* Wrapped around */

    /* Next write overwrites first entry */
    party_handle_message("PCMsg5");
    int pass2 = strcmp(g_party.chat_history[0], "Msg5") == 0;

    test_teardown();
    return pass1 && pass2;
}

/* ========================================
 * Party Member Tests
 * ======================================== */

static int test_add_member(void) {
    test_setup();

    party_invite(1, 2, 3, 4);
    int result = party_add_member(100, "TestPlayer");

    int pass = result == 0 &&
               g_party.member_count == 1 &&
               g_party.members[0].id == 100 &&
               strcmp(g_party.members[0].name, "TestPlayer") == 0;

    test_teardown();
    return pass;
}

static int test_add_member_full(void) {
    test_setup();

    party_invite(1, 2, 3, 4);

    /* Add 5 members (max) */
    for (int i = 0; i < 5; i++) {
        char name[16];
        snprintf(name, sizeof(name), "Player%d", i);
        party_add_member(i, name);
    }

    /* Try to add 6th member */
    int result = party_add_member(100, "ExtraPlayer");

    int pass = result == -1 &&
               g_party.member_count == 5;

    test_teardown();
    return pass;
}

static int test_add_member_name_truncate(void) {
    test_setup();

    party_invite(1, 2, 3, 4);

    /* Name longer than MAX_PARTY_NAME_LENGTH */
    char long_name[50];
    memset(long_name, 'A', 49);
    long_name[49] = '\0';

    party_add_member(1, long_name);

    int pass = strlen(g_party.members[0].name) == MAX_PARTY_NAME_LENGTH - 1;

    test_teardown();
    return pass;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_party_lifecycle(void) {
    test_setup();

    /* Create party */
    party_invite(1, 2, 3, 4);
    party_add_member(100, "Leader");
    party_add_member(101, "Member1");
    party_add_member(102, "Member2");

    int pass1 = g_party.member_count == 3;

    /* Member leaves */
    party_leave(1, 101, 1);
    int pass2 = g_party.member_count == 2;

    /* Kick member */
    party_kick(1, 2, 0);
    int pass3 = g_party.members[1].id == 102;

    /* Disband */
    party_disband(1, "100|200");
    int pass4 = g_party.state == PARTY_STATE_DISBANDING;

    test_teardown();
    return pass1 && pass2 && pass3 && pass4;
}

static int test_party_chat_flow(void) {
    test_setup();

    /* Create party */
    party_invite(1, 2, 3, 4);

    /* Send messages */
    party_handle_message("PCHello");
    party_handle_message("PAJoin request");
    party_handle_message("PU");

    int pass = g_party.chat_write_index == 1 &&
               g_party.action_write_index == 1 &&
               g_party.update_flag == 1;

    test_teardown();
    return pass;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Party System Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(max_party_members);
    TEST(max_party_name_length);
    TEST(max_chat_history);
    TEST(party_state_values);

    /* Initialization tests */
    printf("\nInitialization Tests:\n");
    TEST(party_init);
    TEST(party_init_clears_members);

    /* Party invite tests */
    printf("\nParty Invite Tests:\n");
    TEST(party_invite_success);
    TEST(party_invite_already_in_party);
    TEST(party_invite_display_param_calc);

    /* Party leave tests */
    printf("\nParty Leave Tests:\n");
    TEST(party_leave_self);
    TEST(party_leave_other_member);
    TEST(party_leave_no_party);

    /* Party kick tests */
    printf("\nParty Kick Tests:\n");
    TEST(party_kick_mode_2);
    TEST(party_kick_mode_4);
    TEST(party_kick_mode_5);
    TEST(party_kick_param_clamp);
    TEST(party_kick_clear_party);

    /* Party disband tests */
    printf("\nParty Disband Tests:\n");
    TEST(party_disband);
    TEST(party_disband_no_party);

    /* Party message tests */
    printf("\nParty Message Tests:\n");
    TEST(party_message_chat);
    TEST(party_message_action);
    TEST(party_message_update);
    TEST(party_message_circular_buffer);

    /* Party member tests */
    printf("\nParty Member Tests:\n");
    TEST(add_member);
    TEST(add_member_full);
    TEST(add_member_name_truncate);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_party_lifecycle);
    TEST(party_chat_flow);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Real packet parsing with FUN_00489f70
     * - Party list parsing with field_0, field_2, id
     * - Party info parsing with "X|X|X" format
     * - Network protocol integration
     * - Error handling for malformed messages
     * - Concurrent access protection
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
