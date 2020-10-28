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

    /* Reset counter predictable effects */
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
    const size_t limit = 1024;
    char mock_buf[limit];
    FILE *mock_out = fmemopen(mock_buf, limit, "w+");
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