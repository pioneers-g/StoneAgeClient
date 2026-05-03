#ifndef LOGGER_H
#define LOGGER_H

#include "types.h"

/* Logging levels */
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
} LogLevel;

/* Logger functions */
void logger_init(void);
void logger_shutdown(void);
void logger_log(LogLevel level, const char* format, ...);

#define LOG_DEBUG(fmt, ...)  logger_log(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)   logger_log(LOG_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)   logger_log(LOG_WARNING, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)  logger_log(LOG_ERROR, fmt, ##__VA_ARGS__)

#endif /* LOGGER_H */
