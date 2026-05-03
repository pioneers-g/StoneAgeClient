/*
 * Stone Age Client - Network Process Module
 * Reverse engineered from sa_9061.exe (FUN_0045e880)
 *
 * Main network processing:
 * - Main I/O loop with select()
 * - Packet processing
 * - Heartbeat handling
 */

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "network.h"
#include "protocol.h"
#include "gamestate.h"
#include "logger.h"

/* Winsock function pointer types */
typedef int (WSAAPI* SelectFunc)(int, fd_set*, fd_set*, fd_set*, const struct timeval*);
typedef int (WSAAPI* RecvFunc)(SOCKET, char*, int, int);
typedef int (WSAAPI* SendFunc)(SOCKET, const char*, int, int);
typedef int (WSAAPI* CloseSocketFunc)(SOCKET);
typedef int (WSAAPI* FDIsSetFunc)(SOCKET, fd_set*);
typedef int (WSAAPI* WSAGetLastErrorFunc)(void);

/* Accessor functions from network_core.c */
extern SelectFunc network_get_select_func(void);
extern RecvFunc network_get_recv_func(void);
extern SendFunc network_get_send_func(void);
extern CloseSocketFunc network_get_closesocket_func(void);
extern FDIsSetFunc network_get_fdisset_func(void);
extern WSAGetLastErrorFunc network_get_wsagetlasterror_func(void);

/* External network context */
extern NetworkContext g_net;

/* External functions from network_core.c */
extern int network_buffer_append(const void* data, u32 len);
extern int network_send_buffer_remove(u32 len);
extern int network_extract_line(char* out, u32 max_len);

/*
 * Extract message from receive buffer - FUN_0045ee40
 * Returns 0 on success, non-zero on failure
 */
int network_extract_message(void) {
    u32 i;

    if (!g_net.initialized || g_net.recv_len == 0) {
        return -1;
    }

    /* Find newline character */
    for (i = 0; i < g_net.recv_len; i++) {
        if (g_net.recv_buffer[i] == '\n') {
            break;
        }
    }

    if (i >= g_net.recv_len) {
        return -1;  /* No complete message */
    }

    return 0;
}

/*
 * Process login packet - FUN_0048a6f0 pattern
 * Handles packets before login is complete
 */
void network_process_login_packet(void) {
    char line[4096];

    /* Extract line from buffer */
    if (network_extract_line(line, sizeof(line)) != 0) {
        return;
    }

    /* Process the line - dispatch to text protocol */
    packet_dispatch_text(g_net.socket, line);
}

/*
 * Send heartbeat text - FUN_0043bea0
 */
void packet_send_heartbeat_text(SOCKET sock) {
    const char* heartbeat = "t\n";
    network_send(heartbeat, 2);
    g_net.last_send_time = timeGetTime();
    LOG_DEBUG("Sent text heartbeat");
}

/*
 * Send heartbeat binary - FUN_004901f0
 */
void packet_send_heartbeat_binary(SOCKET sock) {
    u8 packet[10];

    packet[0] = 0x00;
    packet[1] = 0x0a;  /* Size: 10 bytes */
    packet[2] = 0x00;
    packet[3] = 0x57;  /* Opcode: heartbeat */
    packet[4] = 0x00;
    packet[5] = 0x00;
    packet[6] = 0x00;
    packet[7] = 0x00;
    packet[8] = 0x00;
    packet[9] = 0x00;

    network_send(packet, 10);
    g_net.last_send_time = timeGetTime();
    LOG_DEBUG("Sent binary heartbeat");
}

/*
 * Dispatch binary encrypted packet - FUN_0048d3e0
 */
void packet_dispatch_binary_encrypted(SOCKET sock) {
    /* LSRPC encrypted binary protocol */
    /* TODO: Implement LSRPC decryption */
    packet_dispatch_binary(sock);
}

/*
 * Send heartbeat packet
 */
void packet_send_heartbeat(void) {
    if (g_net.protocol_mode == PROTOCOL_MODE_TEXT) {
        /* Text protocol heartbeat - FUN_0043bea0 pattern */
        network_send("t\n", 2);
    }
    else {
        /* Binary protocol heartbeat - FUN_004901f0 pattern */
        u8 packet[10];
        packet[0] = 0x00;
        packet[1] = 0x0a;  /* Size: 10 */
        packet[2] = 0x00;
        packet[3] = 0x57;  /* Opcode: 0x57 (heartbeat) */
        packet[4] = 0x00;
        packet[5] = 0x00;  /* Checksum placeholder */
        network_send(packet, 10);
    }

    g_net.last_send_time = timeGetTime();
    LOG_DEBUG("Sent heartbeat");
}

/*
 * Process packets from receive buffer
 */
void packet_process(void) {
    char line[4096];

    while (g_net.recv_len > 0) {
        if (g_net.state == NET_STATE_LOGGED_IN) {
            if (g_net.protocol_mode == PROTOCOL_MODE_TEXT) {
                /* Text protocol - extract lines */
                if (network_extract_line(line, sizeof(line)) != 0) {
                    break;
                }
                packet_dispatch_text(g_net.socket, line);
            }
            else {
                /* Binary protocol - parse packets */
                packet_dispatch_binary(g_net.socket);
            }
        }
        else {
            /* Not logged in - use text protocol for login */
            if (network_extract_line(line, sizeof(line)) != 0) {
                break;
            }
            packet_dispatch_text(g_net.socket, line);
        }
    }
}

/*
 * Main network I/O - FUN_0045e880
 * Handles select(), recv(), send(), and heartbeat
 * Uses VirtualProtect for buffer memory protection
 */
void network_process(void) {
    fd_set read_fds, write_fds;
    struct timeval timeout;
    int result;
    DWORD current_time;
    DWORD old_protect;

    /* Cache Winsock function pointers */
    SelectFunc p_select = network_get_select_func();
    RecvFunc p_recv = network_get_recv_func();
    SendFunc p_send = network_get_send_func();
    CloseSocketFunc p_closesocket = network_get_closesocket_func();
    FDIsSetFunc p___WSAFDIsSet = network_get_fdisset_func();
    WSAGetLastErrorFunc p_WSAGetLastError = network_get_wsagetlasterror_func();

    /* Check if network is enabled - DAT_0461b3f8 */
    if (!g_net.initialized) {
        return;
    }

    /* Check connection state flags - DAT_0461b420, DAT_0461b424 */
    if (g_net.connection_flag && !g_net.heartbeat_flag) {
        /* Connection lost - call gamestate_set_state(0xb) - FUN_00479bc0 */
        gamestate_set_state(GAME_STATE_DISCONNECTED, 1);
    }

    g_net.heartbeat_flag = g_net.connection_flag;

    /* Only process if connected - DAT_0461b420 == 0, DAT_0461b3fc != 0 */
    if (!g_net.connection_flag && g_net.socket != INVALID_SOCKET) {
        /* Setup select */
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_SET(g_net.socket, &read_fds);

        if (g_net.send_len > 0) {
            FD_SET(g_net.socket, &write_fds);
        }

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        /* VirtualProtect(recv_buffer, 0x4000, PAGE_READWRITE, &old) */
        VirtualProtect(g_net.recv_buffer, NET_RECV_BUFFER_SIZE, PAGE_READWRITE, &old_protect);

        result = p_select(0, &read_fds, &write_fds, NULL, &timeout);
        if (result > 0) {
            /* Handle receive - check if socket is in read_fds */
            if (p___WSAFDIsSet(g_net.socket, &read_fds)) {
                result = p_recv(g_net.socket, (char*)g_net.temp_buffer,
                               NET_TEMP_BUFFER_SIZE, 0);

                /* Large packet handling - DAT_0461b414 flag */
                if (g_net.large_packet_flag && result > 200 && result < 0x1127) {
                    Sleep(2000);

                    FD_ZERO(&read_fds);
                    FD_SET(g_net.socket, &read_fds);
                    p_select(0, &read_fds, NULL, NULL, &timeout);

                    if (p___WSAFDIsSet(g_net.socket, &read_fds)) {
                        int result2 = p_recv(g_net.socket,
                                            (char*)g_net.temp_buffer + result,
                                            NET_TEMP_BUFFER_SIZE - result, 0);
                        if (result2 > 0) {
                            result += result2;
                        }
                    }
                    g_net.large_packet_flag = 0;
                }

                if (result == SOCKET_ERROR) {
                    int err = p_WSAGetLastError();
                    if (err != WSAEWOULDBLOCK) {
                        LOG_ERROR("Recv error: %d", err);
                        p_closesocket(g_net.socket);
                        g_net.socket = INVALID_SOCKET;
                        g_net.state = NET_STATE_DISCONNECTED;
                        g_net.connection_flag = 1;
                        VirtualProtect(g_net.recv_buffer, NET_RECV_BUFFER_SIZE, PAGE_READONLY, &old_protect);
                        return;
                    }
                }
                else if (result > 0) {
                    /* Append to main buffer - FUN_0045ec80 */
                    network_buffer_append(g_net.temp_buffer, result);

                    /* Process packets while recv_len > 0 */
                    while (g_net.recv_len > 0) {
                        if (g_net.state == NET_STATE_LOGGED_IN) {
                            /* FUN_0045ee40 - extract message */
                            result = network_extract_message();
                            if (result != 0) break;

                            /* Dispatch based on protocol mode */
                            if (g_net.protocol_mode == PROTOCOL_MODE_TEXT) {
                                /* Text protocol - FUN_0043bf90 */
                                packet_dispatch_text(g_net.socket, (char*)g_net.recv_buffer);
                            } else {
                                /* Binary protocol - FUN_0048d3e0 */
                                packet_dispatch_binary_encrypted(g_net.socket);
                            }
                        } else {
                            /* Not logged in - FUN_0048a6f0 pattern */
                            network_process_login_packet();
                        }
                    }
                }
            }
        }

        /* VirtualProtect(recv_buffer, 0x4000, PAGE_READONLY, &old) */
        VirtualProtect(g_net.recv_buffer, NET_RECV_BUFFER_SIZE, PAGE_READONLY, &old_protect);

        /* Initialize heartbeat timer if not set - DAT_0461b40c */
        if ((g_net.timer_flags & 1) == 0) {
            g_net.timer_flags |= 1;
            g_net.last_recv_time = timeGetTime();
        }

        /* Handle send - check if socket is in write_fds */
        if (p___WSAFDIsSet(g_net.socket, &write_fds) && g_net.send_len > 0) {
            /* VirtualProtect(send_buffer, 0x4000, PAGE_READWRITE, &old) */
            VirtualProtect(g_net.send_buffer, NET_SEND_BUFFER_SIZE, PAGE_READWRITE, &old_protect);

            if (g_net.send_len > 0) {
                result = p_send(g_net.socket, (const char*)g_net.send_buffer,
                               g_net.send_len, 0);

                if (result > 0) {
                    g_net.last_send_time = timeGetTime();
                }

                if (result == SOCKET_ERROR) {
                    int err = p_WSAGetLastError();
                    if (err != WSAEWOULDBLOCK) {
                        LOG_ERROR("Send error: %d", err);
                        p_closesocket(g_net.socket);
                        g_net.socket = INVALID_SOCKET;
                        g_net.state = NET_STATE_DISCONNECTED;
                        g_net.connection_flag = 1;
                        VirtualProtect(g_net.send_buffer, NET_SEND_BUFFER_SIZE, PAGE_NOACCESS, &old_protect);
                        return;
                    }
                }
                else if (result > 0) {
                    /* Remove sent data - FUN_0045ede0 */
                    network_send_buffer_remove(result);
                }
            }

            /* VirtualProtect(send_buffer, 0x4000, PAGE_NOACCESS, &old) */
            VirtualProtect(g_net.send_buffer, NET_SEND_BUFFER_SIZE, PAGE_NOACCESS, &old_protect);
        }

        /* Heartbeat check - every 30 seconds if logged in */
        if (g_net.state == NET_STATE_LOGGED_IN) {
            current_time = timeGetTime();
            if (current_time - g_net.last_send_time >= 30000 && g_net.initialized == 1) {
                /* Send heartbeat based on protocol mode */
                if (g_net.protocol_mode == PROTOCOL_MODE_TEXT) {
                    /* FUN_0043bea0 - text heartbeat */
                    packet_send_heartbeat_text(g_net.socket);
                } else {
                    /* FUN_004901f0 - binary heartbeat */
                    packet_send_heartbeat_binary(g_net.socket);
                }
                g_net.heartbeat_count++;  /* DAT_005ab714 */
            }
        }
    }
}
