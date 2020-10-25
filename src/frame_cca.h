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

#endif //ALPHARAD_FRAME_CCA_H
