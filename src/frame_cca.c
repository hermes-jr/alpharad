#include "frame_cca.h"
#include "settings.h"
#include <stdio.h>

extern struct settings settings;

/* Add item to the end of linked list */
void push_item(node_t **head, uint v) {
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node) {
        return;
    }
    new_node->v = v;
    if (*head == NULL) {
        /* First element */
        *head = new_node;
    } else {
        /* Add to the end */
        (*head)->tail->next = new_node;
    }
    (*head)->tail = new_node;
}

/* Get first item and remove it from list */
uint pop_item(node_t **head) {
    uint result = (*head)->v;
    node_t *tmp = *head;
    *head = (*head)->next;
    if (NULL != *head) {
        (*head)->tail = tmp->tail;
    }
    free(tmp);
    return result;
}

/* Get nth item */
uint get_item(node_t *head, uint n) {
    while (n-- > 0 && head->next != NULL) {
        head = head->next;
    }
    return head->v;
}

/* Free allocated memory, set head to NULL */
void delete_list(node_t **head) {
    node_t *tmp;

    while (NULL != *head) {
        tmp = (*head);
        *head = (*head)->next;
        free(tmp);
    }
}

/**
 * Process frame and detect if there are any bright spots in it.
 * Reduce big spots to a single coordinate.
 * A representative is chosen by the means of round-robin algorithm to provide some uniformity
 *
 * @param p frame raw data buffer
 * @param size  frame buffer length
 * @param mode will return as soon as first flash detected
 * @return linked list of representatives of each group of connected pixels
 */
points_detected get_all_flashes(const uint8_t *p, uint size, scan_mode mode) {
    static uint rr;
    points_detected result = {0, NULL};

    const uint dw = settings.width * 2;
    uint_fast16_t *visited = calloc((size + 15) / 16, sizeof(uint_fast16_t));

    for (uint idx = 0; idx < size; idx += 2) {
        if (!is_pixel_lit(p, idx)) {
            /* We're on black, skip */
            mark_visited(visited, idx);
            continue;
        }

        uint x = (idx % dw) / 2;
        uint y = idx / dw;

        /* Skip borders, they behave weirdly in my particular camera */
        if (x == 0 || y == 0 || x + 1 == settings.width || y + 1 == settings.height) {
            continue;
        }

        /* Register this single point and return ASAP */
        if (mode == FIRST_ONLY) {
            result.len = 1;
            result.arr = malloc(sizeof(idx));
            result.arr[0] = idx;
            return result;
        }

        if (check_visited(visited, idx)) { continue; }

        /* By this point we are at bright and non-visited pixel */
        node_t *queue = NULL;
        node_t *current_batch = NULL;
        push_item(&queue, idx);
        D(printf("\nAnalyzing neighbors of %d:%d: { ", x, y));
        do {
            uint inner_idx = pop_item(&queue);
            uint cx = (inner_idx % dw) / 2;
            uint cy = inner_idx / dw;
            mark_visited(visited, inner_idx);
            if (!is_pixel_lit(p, inner_idx)) { continue; }
            D(printf("%d:%d, ", cx, cy));
            push_item(&current_batch, inner_idx);

            /* Too lazy to optimize this crap. Maybe later */
            bool up = cy > 1;
            bool down = cy < settings.height - 1;
            bool left = cx > 1;
            bool right = cx < settings.width - 1;
            if (down && !check_visited(visited, inner_idx + dw)) {
                push_item(&queue, inner_idx + dw); // S
            }
            if (down && right && !check_visited(visited, inner_idx + dw + 2)) {
                push_item(&queue, inner_idx + dw + 2); // SE
            }
            if (down && left && !check_visited(visited, inner_idx + dw - 2)) {
                push_item(&queue, inner_idx + dw - 2); // SW
            }
            if (up && !check_visited(visited, inner_idx - dw)) {
                push_item(&queue, inner_idx - dw); // N
            }
            if (up && right && !check_visited(visited, inner_idx - dw + 2)) {
                push_item(&queue, inner_idx - dw + 2); // NE
            }
            if (up && left && !check_visited(visited, inner_idx - dw - 2)) {
                push_item(&queue, inner_idx - dw - 2); // NW
            }
            if (left && !check_visited(visited, inner_idx - 2)) {
                push_item(&queue, inner_idx - 2); // W
            }
            if (right && !check_visited(visited, inner_idx + 2)) {
                push_item(&queue, inner_idx + 2); // E
            }

        } while (queue != NULL);
        D(printf("}\n"));

        /* We have a region now, select a representative with round-robin */
        result.len++;
        result.arr = realloc(result.arr, result.len);
        result.arr[result.len - 1] = get_item(current_batch, 0); // FIXME: rr
//        result.arr[result.len - 1] = 1;// region[rr++];
        delete_list(&current_batch);
    }
    return result;
}

inline void mark_visited(uint_fast16_t *visited, uint idx) {
    visited[idx / 16] |= 1u << (idx % 16);
}

inline uint check_visited(uint_fast16_t const *visited, uint idx) {
    return visited[idx / 16] & 1u << (idx % 16);
}

/**
 * Convenience only wrapper method
 * @param p
 * @param size
 * @return true if any pixel is lit
 */
bool has_flashes(const uint8_t *p, uint size) {
    points_detected result = get_all_flashes(p, size, FIRST_ONLY);
    if (result.len > 0) {
        free(result.arr);
        return true;
    } else {
        return false;
    }
}

/* Return true if requested byte value is greater than settings.threshold */
bool is_pixel_lit(const uint8_t *p, uint idx) { return p[idx] > settings.threshold; }
