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

#include <CUnit/Basic.h>
#include <stdbool.h>
#include "../settings.h"
#include "test_image_processing.h"

extern struct settings settings;

uint coord1;
uint coord2;
uint8_t *mock_frame;
uint screen_buffer_size;

/* Imitate fake camera with 16:8 resolution and predictable pixels (per test) being turned on */
void image_processing_test_init(void) {
    settings.width = 16;
    settings.height = 8;

    /* Create empty canvas, each test will fill in the needed data */
    screen_buffer_size = settings.width * settings.height * 2;
    mock_frame = calloc(screen_buffer_size, sizeof(uint8_t));

    /* Common test points */
    coord1 = cartesian_to_yuv((coordinate) {1, 1});
    coord2 = cartesian_to_yuv((coordinate) {3, 1});
}

/* Free fake frame buffer */
void image_processing_test_teardown(void) {
    free(mock_frame);
}

void test_image_return_single(void) {
    mock_frame[coord1] = mock_frame[coord2] = 0xFF;

    points_detected result = get_all_flashes(mock_frame, screen_buffer_size, FIRST_ONLY);
    CU_ASSERT_EQUAL_FATAL(result.len, 1)
    CU_ASSERT_PTR_NOT_EQUAL_FATAL(result.arr, NULL)
    CU_ASSERT_EQUAL(cartesian_to_yuv(result.arr[0]), coord1)
    free(result.arr);
}

void test_image_return_multiple(void) {
    mock_frame[coord1] = mock_frame[coord2] = 0xFF;

    points_detected result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    CU_ASSERT_EQUAL(result.len, 2)
    CU_ASSERT_PTR_NOT_EQUAL(result.arr, NULL)
    CU_ASSERT_EQUAL(cartesian_to_yuv(result.arr[0]), coord1)
    CU_ASSERT_EQUAL(cartesian_to_yuv(result.arr[1]), coord2)
    free(result.arr);
}

void test_image_wrapper(void) {
    /* All zeros */
    bool flashes_in_blank = has_flashes(mock_frame, screen_buffer_size);
    CU_ASSERT_FALSE(flashes_in_blank)

    uint full_line_width = settings.width * 2;
    mock_frame[full_line_width + 4] = 0xFF;
    /* Single flash inside the frame */
    bool flashes_in_single = has_flashes(mock_frame, screen_buffer_size);
    CU_ASSERT_TRUE(flashes_in_single)

    memset(mock_frame + full_line_width + 5, 0xFF, 9);
    /* Multiple flashes in a row */
    bool flashes_in_multi = has_flashes(mock_frame, screen_buffer_size);
    CU_ASSERT_TRUE(flashes_in_multi)
}

void test_image_empty(void) {
    points_detected result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    CU_ASSERT_EQUAL(result.len, 0)
    CU_ASSERT_PTR_EQUAL(result.arr, NULL)
}

/**
 * Testing the following configuration:
 * ##
 * ##  #
 *
 * CCL should detect a square and a standalone point and return them as two different entities.
 */
void test_image_ccl(void) {
    uint p1 = cartesian_to_yuv((coordinate) {1, 2});
    uint p2 = cartesian_to_yuv((coordinate) {2, 2});
    uint p3 = cartesian_to_yuv((coordinate) {1, 3});
    uint p4 = cartesian_to_yuv((coordinate) {2, 3});
    uint p5 = cartesian_to_yuv((coordinate) {5, 3});
    mock_frame[p1] = mock_frame[p2] = mock_frame[p3] = mock_frame[p4] = 0xFF;
    mock_frame[p5] = 0xFF;

    points_detected result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    CU_ASSERT_EQUAL(result.len, 2)
    CU_ASSERT_PTR_NOT_EQUAL(result.arr, NULL)
    CU_ASSERT_EQUAL(cartesian_to_yuv(result.arr[1]), p5)
    uint square_representative = cartesian_to_yuv(result.arr[0]);
    CU_ASSERT_TRUE(square_representative == p1 || square_representative == p2 || square_representative == p3 ||
                   square_representative == p4)
    free(result.arr);
}

/**
 * Testing the following configuration:
 * ##
 * ##
 *
 * CCL should detect the square shape and yield a new representative for it
 * in round-robin fashion each time to avoid bias.
 *
 * In this case pixels would be discovered in reading order:
 * 12
 * 34
 */
void test_image_ccl_rr(void) {
    uint p1 = cartesian_to_yuv((coordinate) {1, 2});
    uint p2 = cartesian_to_yuv((coordinate) {2, 2});
    uint p3 = cartesian_to_yuv((coordinate) {1, 3});
    uint p4 = cartesian_to_yuv((coordinate) {2, 3});
    mock_frame[p1] = mock_frame[p2] = mock_frame[p3] = mock_frame[p4] = 0xFF;

    /* Reset counter for predictable effects */
    rr = 0;

    points_detected result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    CU_ASSERT_EQUAL(result.len, 1)
    CU_ASSERT_PTR_NOT_EQUAL(result.arr, NULL)
    CU_ASSERT_EQUAL(cartesian_to_yuv(result.arr[0]), p1)
    free(result.arr);

    /* rr should be 1 by now */
    CU_ASSERT_EQUAL(rr, 1)

    /* Expect round-robin to pick another representative next time */
    result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    CU_ASSERT_EQUAL(result.len, 1)
    CU_ASSERT_EQUAL(cartesian_to_yuv(result.arr[0]), p2)
    free(result.arr);
    CU_ASSERT_EQUAL(rr, 2)

    result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    CU_ASSERT_EQUAL(cartesian_to_yuv(result.arr[0]), p3)
    free(result.arr);
    CU_ASSERT_EQUAL(rr, 3)

    result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    CU_ASSERT_EQUAL(cartesian_to_yuv(result.arr[0]), p4)
    free(result.arr);

    /* And back to the origin */
    result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    CU_ASSERT_EQUAL_FATAL(result.len, 1)
    CU_ASSERT_EQUAL(cartesian_to_yuv(result.arr[0]), p1)
    free(result.arr);

    CU_ASSERT_EQUAL(rr, 5)
}

void test_image_logging(void) {
    char mock_buf[BUFSIZ];
    FILE *mock_out = fmemopen(mock_buf, BUFSIZ, "w+");
    settings.file_hits = mock_out;

    mock_frame[coord1] = 0xFF; // 1:1 should be logged

    points_detected result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    fflush(settings.file_hits);
    CU_ASSERT_EQUAL(result.len, 1)
    free(result.arr);

    CU_ASSERT_NSTRING_EQUAL(mock_buf, "1:1", 3)

    settings.file_hits = NULL;
    fclose(mock_out);
}

void test_image_border_crop(void) {
    uint *counter_xs = calloc(settings.width, sizeof(counter_xs));
    uint *counter_ys = calloc(settings.height, sizeof(counter_ys));

    uint rounds = 0;
    uint flashes_returned = 0;

    const uint border_size = 2;
    settings.crop = border_size;
    /* Hit each pixel exactly once. 2 pixels thick border should be ignored */
    for (uint i = 0; i < screen_buffer_size; i += 2, rounds++) {
        const uint yuv_coord = i % screen_buffer_size;
        mock_frame[yuv_coord] = 0xFF;
        points_detected result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
        flashes_returned += result.len;
        if (result.len > 0) {
            counter_xs[result.arr->x]++;
            counter_ys[result.arr->y]++;
        }
        free(result.arr);
        mock_frame[yuv_coord] = 0x00;
    }

    const uint dbs = 2 * border_size;

    /* Only hits within cropped area should be registered, everything else should be equal to 0 */
    CU_ASSERT_EQUAL(flashes_returned, (settings.width - dbs) * (settings.height - dbs))

    CU_ASSERT_EQUAL(counter_xs[border_size], settings.height - dbs)
    CU_ASSERT_EQUAL(counter_xs[0], 0)
    CU_ASSERT_EQUAL(counter_xs[settings.width - 1], 0)

    CU_ASSERT_EQUAL(counter_ys[border_size], settings.width - dbs)
    CU_ASSERT_EQUAL(counter_ys[0], 0)
    CU_ASSERT_EQUAL(counter_ys[settings.height - 1], 0)

    free(counter_xs);
    free(counter_ys);
    settings.crop = 0;
}

void test_image_rounding_errors(void) {
    uint *counter_xs = calloc(settings.width, sizeof(counter_xs));
    uint *counter_ys = calloc(settings.height, sizeof(counter_ys));

    uint rounds = 0;
    uint flashes_returned = 0;

    /* Hit each pixel exactly once. All X coordinates should be visited settings.height times and vice versa */
    for (uint i = 0; i < screen_buffer_size; i += 2, rounds++) {
        const uint yuv_coord = i % screen_buffer_size;
        mock_frame[yuv_coord] = 0xFF;
        points_detected result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
        flashes_returned += result.len;
        if (result.len > 0) {
            counter_xs[result.arr->x]++;
            counter_ys[result.arr->y]++;
        }
        free(result.arr);
        mock_frame[yuv_coord] = 0x00;
    }

    /* Every hit should've been registered, no extra points expected either */
    CU_ASSERT_EQUAL(flashes_returned, rounds)

    uint reference = counter_xs[0];
    CU_ASSERT_EQUAL(reference, settings.height)

    for (uint i = 0; i < settings.width; i++) {
        if (counter_xs[i] != reference) {
            CU_FAIL("Flashes were registered incorrectly: X")
            break;
        }
    }

    reference = counter_ys[0];
    CU_ASSERT_EQUAL(reference, settings.width)
    for (uint i = 0; i < settings.height; i++) {
        if (counter_ys[i] != reference) {
            CU_FAIL("Flashes were registered incorrectly: Y")
            break;
        }
    }

    free(counter_xs);
    free(counter_ys);
}

void test_image_ll(void) {
    node_t *queue = NULL;
    push_item(&queue, 3u);
    CU_ASSERT_PTR_NOT_NULL(queue)

    push_item(&queue, 5u);
    push_item(&queue, 1u);

    D(dump_list(queue));

    CU_ASSERT_EQUAL(get_item(queue, 0), 3u)
    CU_ASSERT_EQUAL(get_item(queue, 1), 5u)
    CU_ASSERT_EQUAL(get_item(queue, 2), 1u)

    int items_count = 0;
    uint last_item;
    do {
        last_item = pop_item(&queue);
        items_count++;
    } while (queue != NULL);
    CU_ASSERT_EQUAL(items_count, 3)
    CU_ASSERT_EQUAL(last_item, 1u)
    CU_ASSERT_PTR_NULL(queue)

    push_item(&queue, 100u);
    push_item(&queue, 200u);
    push_item(&queue, 300u);
    CU_ASSERT_PTR_NOT_NULL(queue)
    D(dump_list(queue));
    delete_list(&queue);
    CU_ASSERT_PTR_NULL(queue)
}

uint cartesian_to_yuv(coordinate c) {
    return (settings.width * c.y + c.x) * 2;
}

void dump_list(node_t *head) {
    printf("\n");
    while (head != NULL) {
        printf("%d, ", head->v);
        head = head->next;
    }
}