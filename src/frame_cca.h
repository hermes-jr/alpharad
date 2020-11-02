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
#ifndef ALPHARAD_FRAME_CCA_H
#define ALPHARAD_FRAME_CCA_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dto.h"

typedef enum {
    FULL_SCAN,
    FIRST_ONLY
} scan_mode;

typedef struct node_t {
    uint v;
    struct node_t *next;
    struct node_t *tail;
} node_t;

#define DEBUG_OUT_TRIM_LIMIT 10

/* Make it global for unit tests, improve if possible */
uint rr;

bool is_pixel_lit(const uint8_t *p, uint idx);

points_detected get_all_flashes(const uint8_t *p, uint size, scan_mode mode);

bool has_flashes(const uint8_t *p, uint size);

void push_item(node_t **head, uint v);

uint pop_item(node_t **head);

void delete_list(node_t **head);

uint get_item(node_t *head, uint n);

uint check_visited(uint_fast16_t const *visited, uint idx);

void mark_visited(uint_fast16_t *visited, uint idx);

void enqueue_neighbors(const uint_fast16_t *visited, node_t **queue, uint inner_idx, uint cx, uint cy);

void log_flash_at_coordinates(coordinate *c);

#endif //ALPHARAD_FRAME_CCA_H
