/*
 * Stone Age Client - Logger
 */

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "types.h"
#include "logger.h"

static FILE* log_file = NULL;
static const char* level_names[] = {"DEBUG", "INFO", "WARN", "ERROR"};

void logger_init(void) {
    log_file = fopen("sa_client.log", "a");
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "\n=== Stone Age Client Started at %s ===\n", ctime(&now));
        fflush(log_file);
    }
}

void logger_shutdown(void) {
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "=== Stone Age Client Stopped at %s ===\n", ctime(&now));
        fclose(log_file);
        log_file = NULL;
    }
}

void logger_log(LogLevel level, const char* format, ...) {
    va_list args;
    char buffer[4096];
    time_t now;
    struct tm* tm_info;

    if (level < LOG_DEBUG || level > LOG_ERROR) {
        return;
    }

    time(&now);
    tm_info = localtime(&now);

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    /* Output to file */
    if (log_file) {
        fprintf(log_file, "[%02d:%02d:%02d] [%s] %s\n",
                tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
                level_names[level], buffer);
        fflush(log_file);
    }

    /* Output to debug console */
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
}
