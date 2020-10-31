#include<stdarg.h>
#include "settings.h"
#include "logger.h"

extern struct settings settings;

void log_p(log_level level, const char *fmt, ...) {
    if (settings.verbose >= level) {
        va_list args;
        va_start (args, fmt);
        vfprintf(stdout, fmt, args);
        va_end (args);
    }
}

void log_fp(log_level level, FILE *ofp, const char *fmt, ...) {
    if (settings.verbose >= level) {
        va_list args;
        va_start (args, fmt);
        vfprintf(ofp, fmt, args);
        va_end (args);
    }
}