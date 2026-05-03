/*
 * Stone Age Client - Network Send Module
 * Reverse engineered from sa_9061.exe
 *
 * Network send functions:
 * - Data sending
 * - Packet building
 * - Utility functions
 */

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "network.h"
#include "protocol.h"
#include "logger.h"

/* External network context */
extern NetworkContext g_net;

/* Send function pointer type */
typedef int (WSAAPI* SendFunc)(SOCKET, const char*, int, int);

/* Accessor function from network_core.c */
extern SendFunc network_get_send_func(void);

/*
 * Send data - append to send buffer
 */
int network_send(const void* data, size_t len) {
    if (!g_net.initialized || g_net.socket == INVALID_SOCKET) {
        return -1;
    }

    if (g_net.send_len + len > g_net.send_capacity) {
        LOG_WARN("Send buffer overflow");
        return -1;
    }

    memcpy(g_net.send_buffer + g_net.send_len, data, len);
    g_net.send_len += len;

    return (int)len;
}

/*
 * Send data directly (bypass buffer)
 */
int network_send_raw(const void* data, size_t len) {
    int result;
    SendFunc p_send = network_get_send_func();

    if (!g_net.initialized || g_net.socket == INVALID_SOCKET) {
        return -1;
    }

    result = p_send(g_net.socket, (const char*)data, len, 0);
    if (result > 0) {
        g_net.last_send_time = timeGetTime();
    }

    return result;
}

/*
 * Set protocol mode
 */
void network_set_protocol_mode(ProtocolMode mode) {
    g_net.protocol_mode = mode;
    LOG_INFO("Protocol mode changed to %s", mode == PROTOCOL_MODE_TEXT ? "text" : "binary");
}

/*
 * Send movement packet
 */
int network_send_move(int x, int y, int is_running) {
    char packet[64];

    if (g_net.protocol_mode == PROTOCOL_MODE_TEXT) {
        /* Text protocol movement */
        snprintf(packet, sizeof(packet), "M|%d|%d|%d\n", x, y, is_running);
        network_send(packet, strlen(packet));
    }
    else {
        /* Binary protocol movement */
        /* TODO: Implement binary format */
        snprintf(packet, sizeof(packet), "M|%d|%d|%d\n", x, y, is_running);
        network_send(packet, strlen(packet));
    }

    LOG_DEBUG("Sent move: (%d,%d) running=%d", x, y, is_running);
    return 0;
}

/*
 * Send pet action packet
 */
int network_send_pet_action(void) {
    char packet[32];

    if (g_net.protocol_mode == PROTOCOL_MODE_TEXT) {
        /* Text protocol pet action */
        snprintf(packet, sizeof(packet), "P|0\n");
        network_send(packet, strlen(packet));
    }
    else {
        /* Binary protocol pet action */
        /* TODO: Implement binary format */
        snprintf(packet, sizeof(packet), "P|0\n");
        network_send(packet, strlen(packet));
    }

    LOG_DEBUG("Sent pet action");
    return 0;
}

/*
 * Send pong response to server
 * Called from PKT_SV_PING (0x88)
 */
int network_send_pong(u32 timestamp) {
    char packet[16];
    u16 size = 10;

    /* Build pong packet */
    *(u16*)(packet) = size;
    *(u16*)(packet + 2) = PKT_CLIENT_PONG;
    *(u16*)(packet + 4) = 0;  /* checksum */
    *(u32*)(packet + 6) = timestamp;

    /* Send to server */
    network_send(packet, size);

    LOG_DEBUG("Sent pong: %u", timestamp);
    return 1;
}

/*
 * Update ping time from pong response
 * Called from PKT_SV_PONG (0x89)
 */
void network_update_ping_time(u32 latency_ms) {
    g_net.last_ping_time = timeGetTime();
    g_net.latency = latency_ms;

    LOG_DEBUG("Latency: %u ms", latency_ms);
}
