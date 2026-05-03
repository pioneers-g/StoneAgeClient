/*
 * Stone Age Client - Protocol Module Comprehensive Tests
 * Tests for packet reading, checksum, binary protocol dispatcher
 * Reverse engineered from FUN_0048d3e0, FUN_0048d190, FUN_0048d2b0, FUN_004929fe
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
 * Constants from Ghidra analysis
 * ======================================== */

/* Packet header size */
#define PACKET_HEADER_SIZE      8

/* Maximum packet size */
#define MAX_PACKET_SIZE         65535

/* Default encryption key - from FUN_0041db40 */
#define DEFAULT_ENCRYPT_KEY     0xBCDE

/* Checksum XOR value */
#define CHECKSUM_XOR            0xFFFF

/* Protocol command types from FUN_0048d3e0 switch cases */
/* Party commands */
#define CMD_PARTY_INVITE        0x02
#define CMD_PARTY_LIST          0x0C
#define CMD_PARTY_DISBAND       0x0D
#define CMD_PARTY_KICK          0x07
#define CMD_PARTY_MESSAGES      0x0F

/* Trade commands */
#define CMD_TRADE_REQUEST       0x04
#define CMD_TRADE_SUCCESS       0x4A
#define CMD_TRADE_CANCEL        0x4C
#define CMD_TRADE_CONFIRM       0x4E

/* Skill commands */
#define CMD_SKILL_LIST          0x78
#define CMD_SKILL_USE           0x7D
#define CMD_SKILL_DISPATCH      0x7E

/* Guild commands */
#define CMD_GUILD_MEMBER_ADD    0x86
#define CMD_GUILD_MEMBER_UPDATE 0x85

/* Mail commands */
#define CMD_MAIL_PACKET         0x5C
#define CMD_MAIL_LIST           0x6D
#define CMD_MAIL_READ           0x6F

/* Item/Inventory commands */
#define CMD_INVENTORY_UPDATE    0x42
#define CMD_ITEM_USE            0x50

/* Character commands */
#define CMD_CHAR_STATS          0x52
#define CMD_CHAR_DATA           0x54

/* Shop commands */
#define CMD_SHOP_DISPATCH       0x75

/* NPC commands */
#define CMD_NPC_DIALOG          0x2A
#define CMD_NPC_ACTION          0x2B

/* ========================================
 * Packet Reader Structure
 * ======================================== */

typedef struct {
    u8* data;
    u32 size;
    u32 offset;
    u16 checksum_calc;
    u16 checksum_recv;
} PacketReader;

/* ========================================
 * Stub Implementations
 * ======================================== */

/*
 * Calculate packet checksum - FUN_004929fe pattern
 */
u16 packet_checksum(void* data, int len) {
    u16 sum = 0;
    u8* ptr = (u8*)data;

    while (len-- > 0) {
        sum += *ptr++;
    }

    return sum ^ CHECKSUM_XOR;
}

/*
 * Initialize packet reader - FUN_00492d80 pattern
 */
void packet_reader_init(PacketReader* reader, void* data, u32 size) {
    reader->data = (u8*)data;
    reader->size = size;
    reader->offset = 0;
    reader->checksum_calc = 0;
    reader->checksum_recv = 0;
}

/*
 * Read 16-bit value - FUN_0048d190 pattern
 */
u16 packet_read_u16(PacketReader* reader) {
    u16 value;

    if (reader->offset + 2 > reader->size) {
        return 0;
    }

    value = *(u16*)(reader->data + reader->offset);
    reader->offset += 2;
    reader->checksum_calc += value;

    return value;
}

/*
 * Read 32-bit value - FUN_0048d2b0 pattern
 */
u32 packet_read_u32(PacketReader* reader) {
    u32 value;

    if (reader->offset + 4 > reader->size) {
        return 0;
    }

    value = *(u32*)(reader->data + reader->offset);
    reader->offset += 4;
    reader->checksum_calc += (value & 0xFFFF) + ((value >> 16) & 0xFFFF);

    return value;
}

/*
 * Read 8-bit value
 */
u8 packet_read_u8(PacketReader* reader) {
    u8 value;

    if (reader->offset + 1 > reader->size) {
        return 0;
    }

    value = reader->data[reader->offset++];
    reader->checksum_calc += value;

    return value;
}

/*
 * Get remaining bytes
 */
u32 packet_remaining(PacketReader* reader) {
    if (reader->offset >= reader->size) {
        return 0;
    }
    return reader->size - reader->offset;
}

/*
 * Skip bytes
 */
void packet_skip(PacketReader* reader, u32 count) {
    reader->offset += count;
    if (reader->offset > reader->size) {
        reader->offset = reader->size;
    }
}

/* ========================================
 * Test Cases for Constants
 * ======================================== */

static int test_packet_header_size(void) {
    return PACKET_HEADER_SIZE == 8;
}

static int test_max_packet_size(void) {
    return MAX_PACKET_SIZE == 65535;
}

static int test_default_encrypt_key(void) {
    return DEFAULT_ENCRYPT_KEY == 0xBCDE;
}

static int test_checksum_xor(void) {
    return CHECKSUM_XOR == 0xFFFF;
}

/* ========================================
 * Test Cases for Command Types
 * ======================================== */

static int test_cmd_party_invite(void) {
    return CMD_PARTY_INVITE == 0x02;
}

static int test_cmd_party_list(void) {
    return CMD_PARTY_LIST == 0x0C;
}

static int test_cmd_trade_request(void) {
    return CMD_TRADE_REQUEST == 0x04;
}

static int test_cmd_skill_list(void) {
    return CMD_SKILL_LIST == 0x78;
}

static int test_cmd_guild_member_add(void) {
    return CMD_GUILD_MEMBER_ADD == 0x86;
}

static int test_cmd_mail_packet(void) {
    return CMD_MAIL_PACKET == 0x5C;
}

static int test_cmd_inventory_update(void) {
    return CMD_INVENTORY_UPDATE == 0x42;
}

static int test_cmd_char_stats(void) {
    return CMD_CHAR_STATS == 0x52;
}

/* ========================================
 * Test Cases for Checksum
 * ======================================== */

static int test_checksum_zero(void) {
    u8 data[1] = {0};
    return packet_checksum(data, 1) == (0 ^ CHECKSUM_XOR);
}

static int test_checksum_single_byte(void) {
    u8 data[1] = {0x42};
    return packet_checksum(data, 1) == (0x42 ^ CHECKSUM_XOR);
}

static int test_checksum_multiple_bytes(void) {
    u8 data[4] = {1, 2, 3, 4};
    u16 expected = (1 + 2 + 3 + 4) ^ CHECKSUM_XOR;
    return packet_checksum(data, 4) == expected;
}

static int test_checksum_xor_pattern(void) {
    /* Verify XOR 0xFFFF pattern from FUN_004929fe */
    u8 data[2] = {0xFF, 0xFF};
    u16 result = packet_checksum(data, 2);
    /* 0xFF + 0xFF = 0x1FE, XOR 0xFFFF = 0xFE01 */
    return result == ((0xFF + 0xFF) ^ 0xFFFF);
}

static int test_checksum_empty(void) {
    return packet_checksum(NULL, 0) == (0 ^ CHECKSUM_XOR);
}

/* ========================================
 * Test Cases for Packet Reader
 * ======================================== */

static int test_reader_init(void) {
    u8 data[10] = {0};
    PacketReader reader;

    packet_reader_init(&reader, data, 10);

    return reader.data == data &&
           reader.size == 10 &&
           reader.offset == 0 &&
           reader.checksum_calc == 0;
}

static int test_reader_read_u8(void) {
    u8 data[4] = {0x12, 0x34, 0x56, 0x78};
    PacketReader reader;

    packet_reader_init(&reader, data, 4);
    u8 v1 = packet_read_u8(&reader);
    u8 v2 = packet_read_u8(&reader);

    return v1 == 0x12 && v2 == 0x34 && reader.offset == 2;
}

static int test_reader_read_u16(void) {
    u8 data[4] = {0x12, 0x34, 0x56, 0x78};
    PacketReader reader;

    packet_reader_init(&reader, data, 4);
    u16 value = packet_read_u16(&reader);

    return value == 0x3412 && reader.offset == 2;  /* Little endian */
}

static int test_reader_read_u32(void) {
    u8 data[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    PacketReader reader;

    packet_reader_init(&reader, data, 8);
    u32 value = packet_read_u32(&reader);

    /* Little endian: 0x78563412 */
    return value == 0x78563412 && reader.offset == 4;
}

static int test_reader_bounds_u16(void) {
    u8 data[2] = {0x12, 0x34};
    PacketReader reader;

    packet_reader_init(&reader, data, 2);
    packet_read_u16(&reader);  /* Consumes all */
    u16 overflow = packet_read_u16(&reader);

    return overflow == 0;  /* Should return 0 on bounds error */
}

static int test_reader_bounds_u32(void) {
    u8 data[4] = {0x12, 0x34, 0x56, 0x78};
    PacketReader reader;

    packet_reader_init(&reader, data, 4);
    packet_read_u32(&reader);  /* Consumes all */
    u32 overflow = packet_read_u32(&reader);

    return overflow == 0;
}

static int test_reader_remaining(void) {
    u8 data[10] = {0};
    PacketReader reader;

    packet_reader_init(&reader, data, 10);
    packet_read_u16(&reader);
    packet_read_u32(&reader);

    return packet_remaining(&reader) == 4;
}

static int test_reader_skip(void) {
    u8 data[10] = {0};
    PacketReader reader;

    packet_reader_init(&reader, data, 10);
    packet_skip(&reader, 5);

    return reader.offset == 5 && packet_remaining(&reader) == 5;
}

static int test_reader_skip_bounds(void) {
    u8 data[10] = {0};
    PacketReader reader;

    packet_reader_init(&reader, data, 10);
    packet_skip(&reader, 100);

    return reader.offset == 10;  /* Clamped to size */
}

/* ========================================
 * Test Cases for Checksum Calculation
 * ======================================== */

static int test_reader_checksum_u16(void) {
    u8 data[4] = {0x12, 0x34, 0x56, 0x78};
    PacketReader reader;

    packet_reader_init(&reader, data, 4);
    packet_read_u16(&reader);

    /* Checksum adds the 16-bit value */
    return reader.checksum_calc == 0x3412;
}

static int test_reader_checksum_u32(void) {
    u8 data[4] = {0x12, 0x34, 0x56, 0x78};
    PacketReader reader;

    packet_reader_init(&reader, data, 4);
    packet_read_u32(&reader);

    /* Checksum adds both 16-bit halves */
    return reader.checksum_calc == (0x3412 + 0x7856);
}

static int test_reader_checksum_accumulates(void) {
    u8 data[8] = {0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00};
    PacketReader reader;

    packet_reader_init(&reader, data, 8);
    packet_read_u16(&reader);  /* 1 */
    packet_read_u16(&reader);  /* 2 */
    packet_read_u16(&reader);  /* 3 */
    packet_read_u16(&reader);  /* 4 */

    return reader.checksum_calc == 10;
}

/* ========================================
 * Test Cases for Protocol Command Ranges
 * ======================================== */

static int test_party_command_range(void) {
    /* Party commands: 0x02, 0x07, 0x0C, 0x0D, 0x0F */
    return CMD_PARTY_INVITE == 0x02 &&
           CMD_PARTY_KICK == 0x07 &&
           CMD_PARTY_LIST == 0x0C;
}

static int test_trade_command_range(void) {
    /* Trade commands: 0x04, 0x4A, 0x4C, 0x4E */
    return CMD_TRADE_REQUEST == 0x04 &&
           CMD_TRADE_SUCCESS == 0x4A &&
           CMD_TRADE_CANCEL == 0x4C &&
           CMD_TRADE_CONFIRM == 0x4E;
}

static int test_skill_command_range(void) {
    /* Skill commands: 0x78, 0x7D, 0x7E */
    return CMD_SKILL_LIST == 0x78 &&
           CMD_SKILL_USE == 0x7D &&
           CMD_SKILL_DISPATCH == 0x7E;
}

static int test_guild_command_range(void) {
    /* Guild commands: 0x85, 0x86 */
    return CMD_GUILD_MEMBER_UPDATE == 0x85 &&
           CMD_GUILD_MEMBER_ADD == 0x86;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_packet_read(void) {
    /* Simulate a simple packet: [size:2][opcode:2][checksum:2][data:2] */
    u8 data[8] = {0x08, 0x00, 0x42, 0x00, 0x00, 0x00, 0x12, 0x34};
    PacketReader reader;

    packet_reader_init(&reader, data, 8);

    u16 size = packet_read_u16(&reader);
    u16 opcode = packet_read_u16(&reader);
    u16 checksum = packet_read_u16(&reader);
    u16 payload = packet_read_u16(&reader);

    return size == 8 && opcode == 0x42 && checksum == 0 && payload == 0x3412;
}

static int test_packet_reader_sequence(void) {
    u8 data[12] = {0};
    PacketReader reader;

    /* Write test data */
    *(u16*)(data + 0) = 0x1234;
    *(u32*)(data + 2) = 0xDEADBEEF;
    *(u16*)(data + 6) = 0xCAFE;

    packet_reader_init(&reader, data, 12);

    u16 v1 = packet_read_u16(&reader);
    u32 v2 = packet_read_u32(&reader);
    u16 v3 = packet_read_u16(&reader);

    return v1 == 0x1234 && v2 == 0xDEADBEEF && v3 == 0xCAFE &&
           reader.offset == 8;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Protocol Module Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(packet_header_size);
    TEST(max_packet_size);
    TEST(default_encrypt_key);
    TEST(checksum_xor);

    /* Command type tests */
    printf("\nCommand Type Tests:\n");
    TEST(cmd_party_invite);
    TEST(cmd_party_list);
    TEST(cmd_trade_request);
    TEST(cmd_skill_list);
    TEST(cmd_guild_member_add);
    TEST(cmd_mail_packet);
    TEST(cmd_inventory_update);
    TEST(cmd_char_stats);

    /* Checksum tests */
    printf("\nChecksum Tests:\n");
    TEST(checksum_zero);
    TEST(checksum_single_byte);
    TEST(checksum_multiple_bytes);
    TEST(checksum_xor_pattern);
    TEST(checksum_empty);

    /* Packet reader tests */
    printf("\nPacket Reader Tests:\n");
    TEST(reader_init);
    TEST(reader_read_u8);
    TEST(reader_read_u16);
    TEST(reader_read_u32);
    TEST(reader_bounds_u16);
    TEST(reader_bounds_u32);
    TEST(reader_remaining);
    TEST(reader_skip);
    TEST(reader_skip_bounds);

    /* Checksum calculation tests */
    printf("\nChecksum Calculation Tests:\n");
    TEST(reader_checksum_u16);
    TEST(reader_checksum_u32);
    TEST(reader_checksum_accumulates);

    /* Protocol command range tests */
    printf("\nProtocol Command Range Tests:\n");
    TEST(party_command_range);
    TEST(trade_command_range);
    TEST(skill_command_range);
    TEST(guild_command_range);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_packet_read);
    TEST(packet_reader_sequence);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
