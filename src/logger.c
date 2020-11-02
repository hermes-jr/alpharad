/*
Copyright (C) 2020 Mikhail Antonov <hermes@cyllene.net>

This file is part of alpharad project.

alpharad is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

alpharad is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with alpharad.  If not, see <https://www.gnu.org/licenses/>.
*/

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