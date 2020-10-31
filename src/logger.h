#ifndef ALPHARAD_LOGGER_H
#define ALPHARAD_LOGGER_H

typedef enum {
    LOG_FATAL,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
    LOG_TRACE
} log_level;

void log_p(log_level, const char *fmt, ...);

void log_fp(log_level level, FILE *ofp, const char *fmt, ...);

#endif //ALPHARAD_LOGGER_H
