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
    bool *visited = calloc(size, sizeof(bool));

    for (uint idx = 0; idx < size; idx += 2) {
        if (!is_pixel_lit(p, idx)) {
            /* We're on black, skip */
            visited[idx] = true;
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

        if (visited[idx]) { continue; }

        /* By this point we are at bright and non-visited pixel */
        node_t *queue = NULL;
        push_item(&queue, idx);
        do {
            uint z = pop_item(&queue);
            printf("deq: %d\n", z);
        } while (queue != NULL);

        /* We have a region now, select a representative with round-robin */
        result.len++;
        result.arr = realloc(result.arr, result.len);
        result.arr[result.len - 1] = 1;// region[rr++];
    }
    return result;
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
