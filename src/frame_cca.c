#include "frame_cca.h"
#include "settings.h"
#include <stdio.h>

extern struct settings settings;

/* Add item to the end of linked list */
void push_item(node_t **head, uint v) {
    node_t *new_node = calloc(1, sizeof(node_t));
    if (!new_node) {
        return;
    }
    new_node->v = v;
    if (*head == NULL) {
        /* First element */
        *head = new_node;
    } else {
        /* Add to the end */
        node_t **tmp = &((*head)->tail->next);
        (*head)->tail->tail = NULL;
        *tmp = new_node;
    }
    (*head)->tail = new_node;
}

/* Get first item and remove it from list */
uint pop_item(node_t **head) {
    uint result = (*head)->v;
    node_t *tmp = *head;
    *head = (*head)->next;
    if (*head != NULL) {
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
    points_detected result = {0, NULL};

    const uint dw = settings.width * 2;

    /* As we don't care about color values, only half the space is enough */
    uint_fast16_t *visited = calloc((size / 2 + 15) / 16, sizeof(uint_fast16_t)); // ceil(half_size/16.0)

    for (uint idx = 0; idx < size; idx += 2) {
        if (!is_pixel_lit(p, idx)) {
            /* We're on black, skip */
            mark_visited(visited, idx);
            continue;
        }

        if (check_visited(visited, idx)) { continue; }

        /* By this point we are at bright and non-visited pixel */
        node_t *queue = NULL;
        points_detected current_batch = {0, NULL};

        push_item(&queue, idx);

        D(printf("\nAnalyzing neighbors of %d:%d: { ", (idx % dw) / 2, idx / dw));
        do {
            uint inner_idx = pop_item(&queue);

            if (check_visited(visited, inner_idx)) { continue; }
            mark_visited(visited, inner_idx);
            if (!is_pixel_lit(p, inner_idx)) { continue; }

            uint cx = (inner_idx % dw) / 2;
            uint cy = inner_idx / dw;
            D(printf("%d:%d, ", cx, cy));

            /* Skip borders (n pixels thick), they behave weirdly in my particular camera */
            uint n_crop = settings.crop;
            if (n_crop > 0 && (cx <= n_crop || cy <= n_crop || cx + n_crop + 1 == settings.width ||
                               cy + n_crop + 1 == settings.height)) {
                continue;
            }

            /* Register this single point and return ASAP */
            if (mode == FIRST_ONLY) {
                result.len = 1;
                result.arr = malloc(sizeof(coordinate));
                result.arr[0] = (coordinate) {cx, cy};
                free(visited);
                return result;
            }

            current_batch.len++;
            /* 99.99% of the time there will be no more than 4-6 reallocations per frame. Should not be a problem */
            current_batch.arr = realloc(current_batch.arr, current_batch.len * sizeof(coordinate));
            current_batch.arr[current_batch.len - 1] = (coordinate) {cx, cy};

            enqueue_neighbors(visited, &queue, inner_idx, cx, cy);

        } while (queue != NULL);
        D(printf("}\n"));

        if (current_batch.len == 0) {
            continue;
        }

        /* We have a region now, select a representative */
        result.len++;
        result.arr = realloc(result.arr, result.len * sizeof(coordinate));
        result.arr[result.len - 1] = current_batch.arr[rr++ % current_batch.len];
        free(current_batch.arr);

        log_flash_at_coordinates(result.arr + result.len - 1);
    }

    free(visited);
    return result;
}

/* Add neighbors to the queue. 8-connectivity, in reading order */
inline void enqueue_neighbors(const uint_fast16_t *visited, node_t **queue, uint inner_idx, uint cx, uint cy) {
    /* Too lazy to optimize this crap. Maybe later */
    const uint dw = settings.width * 2;
    bool up = cy > 0;
    bool down = cy < settings.height - 1;
    bool left = cx > 0;
    bool right = cx < settings.width - 1;
    /* In reading order for no particular reason other than to simplify testing */
    if (up && left && !check_visited(visited, inner_idx - dw - 2)) {
        push_item(queue, inner_idx - dw - 2); // NW
    }
    if (up && !check_visited(visited, inner_idx - dw)) {
        push_item(queue, inner_idx - dw); // N
    }
    if (up && right && !check_visited(visited, inner_idx - dw + 2)) {
        push_item(queue, inner_idx - dw + 2); // NE
    }
    if (left && !check_visited(visited, inner_idx - 2)) {
        push_item(queue, inner_idx - 2); // W
    }
    if (right && !check_visited(visited, inner_idx + 2)) {
        push_item(queue, inner_idx + 2); // E
    }
    if (down && left && !check_visited(visited, inner_idx + dw - 2)) {
        push_item(queue, inner_idx + dw - 2); // SW
    }
    if (down && !check_visited(visited, inner_idx + dw)) {
        push_item(queue, inner_idx + dw); // S
    }
    if (down && right && !check_visited(visited, inner_idx + dw + 2)) {
        push_item(queue, inner_idx + dw + 2); // SE
    }
}

inline void mark_visited(uint_fast16_t *visited, uint idx) {
    idx /= 2; // We never visit Cb and Cr bytes in YUV
    visited[idx / 16] |= 1u << (idx % 16);
}

inline uint check_visited(uint_fast16_t const *visited, uint idx) {
    idx /= 2; // We never visit Cb and Cr bytes in YUV
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

void log_flash_at_coordinates(coordinate *c) {
    if (settings.file_hits == NULL) {
        return;
    }
    char buf[32];
    snprintf(buf, 32, "%d:%d\n", c->x, c->y);
    fputs(buf, settings.file_hits);
    fflush(settings.file_hits);
}


/* Return true if requested byte value is greater than settings.threshold */
bool is_pixel_lit(const uint8_t *p, uint idx) { return p[idx] > settings.threshold; }
