/*
 * Stone Age Client - Network Core Module
 * Reverse engineered from sa_9061.exe (FUN_0045d890)
 *
 * Core networking functionality:
 * - Winsock initialization
 * - Connection management
 * - Buffer management
 */

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "network.h"
#include "protocol.h"
#include "logger.h"

/* Global network context */
NetworkContext g_net = {0};

/* Winsock function pointers - dynamically loaded */
typedef int (WSAAPI* WSAGetLastErrorFunc)(void);
typedef int (WSAAPI* SelectFunc)(int, fd_set*, fd_set*, fd_set*, const struct timeval*);
typedef int (WSAAPI* RecvFunc)(SOCKET, char*, int, int);
typedef int (WSAAPI* SendFunc)(SOCKET, const char*, int, int);
typedef int (WSAAPI* CloseSocketFunc)(SOCKET);
typedef int (WSAAPI* FDIsSetFunc)(SOCKET, fd_set*);
typedef int (WSAAPI* ConnectFunc)(SOCKET, const struct sockaddr*, int);
typedef SOCKET (WSAAPI* SocketFunc)(int, int, int);
typedef int (WSAAPI* IoctlSocketFunc)(SOCKET, long, u_long*);
typedef struct hostent* (WSAAPI* GetHostByNameFunc)(const char*);
typedef int (WSAAPI* WSAStartupFunc)(WORD, LPWSADATA);
typedef int (WSAAPI* WSACleanupFunc)(void);

static WSAGetLastErrorFunc p_WSAGetLastError = NULL;
static SelectFunc p_select = NULL;
static RecvFunc p_recv = NULL;
static SendFunc p_send = NULL;
static CloseSocketFunc p_closesocket = NULL;
static FDIsSetFunc p___WSAFDIsSet = NULL;
static ConnectFunc p_connect = NULL;
static SocketFunc p_socket = NULL;
static IoctlSocketFunc p_ioctlsocket = NULL;
static GetHostByNameFunc p_gethostbyname = NULL;
static WSAStartupFunc p_WSAStartup = NULL;
static WSACleanupFunc p_WSACleanup = NULL;

static HMODULE g_ws2_module = NULL;
static WSADATA g_wsa_data;

/* Accessor functions for Winsock function pointers - used by network_process.c */
SelectFunc network_get_select_func(void) { return p_select; }
RecvFunc network_get_recv_func(void) { return p_recv; }
SendFunc network_get_send_func(void) { return p_send; }
CloseSocketFunc network_get_closesocket_func(void) { return p_closesocket; }
FDIsSetFunc network_get_fdisset_func(void) { return p___WSAFDIsSet; }
WSAGetLastErrorFunc network_get_wsagetlasterror_func(void) { return p_WSAGetLastError; }

/*
 * Load Winsock functions dynamically - matches original pattern
 */
static int load_winsock_functions(void) {
    g_ws2_module = LoadLibraryA("ws2_32.dll");
    if (!g_ws2_module) {
        LOG_ERROR("Failed to load ws2_32.dll");
        return 0;
    }

#define LOAD_FUNC(name, type) \
    p_##name = (type)GetProcAddress(g_ws2_module, #name); \
    if (!p_##name) { \
        LOG_ERROR("Failed to get " #name); \
        return 0; \
    }

    LOAD_FUNC(WSAStartup, WSAStartupFunc);
    LOAD_FUNC(WSACleanup, WSACleanupFunc);
    LOAD_FUNC(WSAGetLastError, WSAGetLastErrorFunc);
    LOAD_FUNC(socket, SocketFunc);
    LOAD_FUNC(connect, ConnectFunc);
    LOAD_FUNC(select, SelectFunc);
    LOAD_FUNC(recv, RecvFunc);
    LOAD_FUNC(send, SendFunc);
    LOAD_FUNC(closesocket, CloseSocketFunc);
    LOAD_FUNC(ioctlsocket, IoctlSocketFunc);
    LOAD_FUNC(gethostbyname, GetHostByNameFunc);
    LOAD_FUNC(__WSAFDIsSet, FDIsSetFunc);

#undef LOAD_FUNC

    return 1;
}

/*
 * Initialize network module - FUN_0045d890 pattern
 */
int network_init(void) {
    int result;

    LOG_INFO("Initializing network module...");

    memset(&g_net, 0, sizeof(NetworkContext));
    g_net.socket = INVALID_SOCKET;
    g_net.state = NET_STATE_DISCONNECTED;
    g_net.protocol_mode = PROTOCOL_MODE_TEXT;

    if (!load_winsock_functions()) {
        return 0;
    }

    result = p_WSAStartup(MAKEWORD(2, 2), &g_wsa_data);
    if (result != 0) {
        LOG_ERROR("WSAStartup failed: %d", result);
        return 0;
    }

    /* Allocate buffers using VirtualAlloc - matches original */
    g_net.recv_buffer = (u8*)VirtualAlloc(NULL, NET_RECV_BUFFER_SIZE,
                                           MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!g_net.recv_buffer) {
        LOG_ERROR("Failed to allocate receive buffer");
        p_WSACleanup();
        return 0;
    }
    g_net.recv_capacity = NET_RECV_BUFFER_SIZE;

    g_net.send_buffer = (u8*)VirtualAlloc(NULL, NET_SEND_BUFFER_SIZE,
                                           MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_net.send_buffer) {
        LOG_ERROR("Failed to allocate send buffer");
        VirtualFree(g_net.recv_buffer, 0, MEM_RELEASE);
        p_WSACleanup();
        return 0;
    }
    g_net.send_capacity = NET_SEND_BUFFER_SIZE;

    g_net.initialized = 1;

    LOG_INFO("Network module initialized");
    return 1;
}

/*
 * Shutdown network module
 */
void network_shutdown(void) {
    LOG_INFO("Shutting down network module...");

    network_disconnect();

    if (g_net.recv_buffer) {
        VirtualFree(g_net.recv_buffer, 0, MEM_RELEASE);
        g_net.recv_buffer = NULL;
    }

    if (g_net.send_buffer) {
        VirtualFree(g_net.send_buffer, 0, MEM_RELEASE);
        g_net.send_buffer = NULL;
    }

    if (p_WSACleanup) {
        p_WSACleanup();
    }

    if (g_ws2_module) {
        FreeLibrary(g_ws2_module);
        g_ws2_module = NULL;
    }

    memset(&g_net, 0, sizeof(NetworkContext));
    LOG_INFO("Network module shutdown");
}

/*
 * Connect to server
 */
int network_connect(const char* server, uint16_t port) {
    struct hostent* host;
    struct sockaddr_in addr;
    u_long mode = 1;

    if (!g_net.initialized) {
        LOG_ERROR("Network not initialized");
        return 0;
    }

    if (g_net.state != NET_STATE_DISCONNECTED) {
        network_disconnect();
    }

    /* Create socket */
    g_net.socket = p_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_net.socket == INVALID_SOCKET) {
        LOG_ERROR("Failed to create socket");
        return 0;
    }

    /* Resolve host */
    host = p_gethostbyname(server);
    if (!host) {
        LOG_ERROR("Failed to resolve host: %s", server);
        p_closesocket(g_net.socket);
        g_net.socket = INVALID_SOCKET;
        return 0;
    }

    /* Set non-blocking mode */
    p_ioctlsocket(g_net.socket, FIONBIO, &mode);

    /* Setup address */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);

    strncpy(g_net.server_ip, server, sizeof(g_net.server_ip) - 1);
    g_net.server_port = port;

    /* Connect */
    if (p_connect(g_net.socket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        int err = p_WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            LOG_ERROR("Connect failed: %d", err);
            p_closesocket(g_net.socket);
            g_net.socket = INVALID_SOCKET;
            return 0;
        }
    }

    g_net.state = NET_STATE_CONNECTING;
    g_net.last_send_time = timeGetTime();
    g_net.last_recv_time = timeGetTime();
    g_net.recv_len = 0;
    g_net.send_len = 0;
    g_net.connection_flag = 0;
    g_net.heartbeat_flag = 0;

    LOG_INFO("Connecting to %s:%d", server, port);
    return 1;
}

/*
 * Disconnect from server
 */
void network_disconnect(void) {
    if (g_net.socket != INVALID_SOCKET) {
        p_closesocket(g_net.socket);
        g_net.socket = INVALID_SOCKET;
    }

    g_net.state = NET_STATE_DISCONNECTED;
    g_net.recv_len = 0;
    g_net.send_len = 0;
    g_net.connection_flag = 1;

    LOG_INFO("Disconnected from server");
}

/*
 * Append data to receive buffer - FUN_0045ec80 pattern
 */
int network_buffer_append(const void* data, u32 len) {
    const u8* src = (const u8*)data;
    u8* dst;
    u32 i;

    if (!g_net.initialized) {
        return -1;
    }

    /* Check buffer overflow */
    if (g_net.recv_len + len > g_net.recv_capacity) {
        LOG_WARN("Receive buffer overflow");
        return -1;
    }

    /* Append data - optimized dword copy then byte copy */
    dst = g_net.recv_buffer + g_net.recv_len;

    for (i = len >> 2; i != 0; i--) {
        *(u32*)dst = *(const u32*)src;
        src += 4;
        dst += 4;
    }

    for (i = len & 3; i != 0; i--) {
        *dst++ = *src++;
    }

    g_net.recv_len += len;
    return 0;
}

/*
 * Remove bytes from start of receive buffer - FUN_0045ed80 pattern
 */
void network_buffer_remove(u32 len) {
    u32 i;
    u8* buf = g_net.recv_buffer;

    if (len > g_net.recv_len) {
        g_net.recv_len = 0;
        return;
    }

    /* Shift remaining data to front */
    if (len < g_net.recv_len) {
        for (i = len; i < g_net.recv_len; i++) {
            buf[i - len] = buf[i];
        }
    }

    g_net.recv_len -= len;
}

/*
 * Remove bytes from start of send buffer - FUN_0045ede0 pattern
 */
int network_send_buffer_remove(u32 len) {
    u32 i;
    u8* buf = g_net.send_buffer;

    if (len > g_net.send_len) {
        return -1;
    }

    /* Shift remaining data to front */
    if (len < g_net.send_len) {
        for (i = len; i < g_net.send_len; i++) {
            buf[i - len] = buf[i];
        }
    }

    g_net.send_len -= len;
    return 0;
}

/*
 * Extract a line from receive buffer - FUN_0045ee40 pattern
 * Used for text protocol (lines end with \n)
 */
int network_extract_line(char* out, u32 max_len) {
    u32 i;
    u8* buf = g_net.recv_buffer;

    if (!g_net.initialized || g_net.recv_len == 0) {
        return -1;
    }

    /* Find newline */
    for (i = 0; i < g_net.recv_len && i < max_len; i++) {
        if (buf[i] == '\n') {
            break;
        }
    }

    if (i >= max_len || i >= g_net.recv_len) {
        return -1;
    }

    /* Copy line to output */
    memcpy(out, buf, i);
    out[i] = '\0';

    /* Remove trailing \r if present */
    if (i > 0 && out[i - 1] == '\r') {
        out[i - 1] = '\0';
    }

    /* Remove line from buffer (+1 for \n) */
    network_buffer_remove(i + 1);

    return 0;
}

/*
 * Check if connected to server
 */
int network_is_connected(void) {
    return g_net.state >= NET_STATE_CONNECTED && g_net.socket != INVALID_SOCKET;
}

/*
 * Check socket status - used by login_check_connection
 * Returns: 1 = ready, 0 = waiting, negative = error
 */
int network_check_socket(void) {
    fd_set read_fds, write_fds, except_fds;
    struct timeval timeout;
    int result;

    if (g_net.socket == INVALID_SOCKET) {
        return -1;
    }

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);
    FD_SET(g_net.socket, &read_fds);
    FD_SET(g_net.socket, &except_fds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    result = select(0, &read_fds, &write_fds, &except_fds, &timeout);

    if (result < 0) {
        return -1;  /* Socket error */
    }

    if (FD_ISSET(g_net.socket, &except_fds)) {
        return -1;  /* Exception */
    }

    if (FD_ISSET(g_net.socket, &read_fds)) {
        return 1;  /* Data available */
    }

    return 0;  /* No data, but socket is fine */
}
