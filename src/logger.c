#include "settings.h"
#include "logger.h"

extern struct settings settings;

void log_p(log_level level, char *fmt, char *msg) {
    if (level > 0) {
        printf(fmt, msg);
    }
}
