/*
Copyright (C) 2020 Mikhail Antonov <hermes@cyllene.net>

This file is part of alpharad project.

alpharad is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published bythe Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

$project.name is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with $project.name.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef ALPHARAD_SETTINGS_H
#define ALPHARAD_SETTINGS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "logger.h"
#include "dto.h"

#ifdef DEBUG
#  define D(x) (x)
#else
#  define D(x) do{}while(0)
#endif //DEBUG

typedef bytes_spawned (*frame_processor_func)(const uint8_t *p, uint size);

typedef struct {
    const char *code;
    const char *description;
    frame_processor_func execute;
} frame_processor_t;

frame_processor_t *registered_processors;

#define FRAME_PROCESSOR_NULL {NULL, NULL, NULL}

#define S_DEFAULT_DEV_NAME "/dev/video0"
#define S_DEFAULT_FILE_OUT_NAME "out.dat"
#define S_DEFAULT_FILE_OUT NULL
#define S_DEFAULT_FILE_HITS_NAME "points.log"
#define S_DEFAULT_FILE_HITS NULL
#define S_DEFAULT_FRAME_PROCESSOR FRAME_PROCESSOR_NULL
#define S_DEFAULT_WIDTH 640
#define S_DEFAULT_HEIGHT 480
#define S_DEFAULT_CROP 0
#define S_DEFAULT_THRESHOLD 8u
#define S_DEFAULT_VERBOSE LOG_FATAL

struct settings {
    uint width;
    uint height;
    uint crop;
    uint8_t threshold;
    uint8_t verbose;
    frame_processor_t frame_processor;
    char *dev_name;
    char *file_hits_name;
    char *file_out_name;
    FILE *file_hits;
    FILE *file_out;
};

int populate_settings(FILE *ofp, char **argv, int argc);

void print_usage(FILE *ofp, char *self_name);

void print_supported_modes(FILE *ofp);

int validated_long_parse(FILE *ofp, uint *target, char *input, const char *err_msg);

bool is_processor_null(frame_processor_t *p);

#endif //ALPHARAD_SETTINGS_H
