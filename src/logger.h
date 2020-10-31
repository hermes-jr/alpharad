#ifndef ALPHARAD_LOGGER_H
#define ALPHARAD_LOGGER_H

typedef enum {
    ERROR,
    INFO,
    TRACE
} log_level;

void log_p(log_level, char *fmt, char *msg);

#endif //ALPHARAD_LOGGER_H
