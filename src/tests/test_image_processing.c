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
    coord1 = settings.width * 2 + 2; // x=1 y=1
    coord2 = settings.width * 2 + 6; // x=3 y=1
}

/* Free fake frame buffer */
int image_processing_suite_cleanup(void) {
    free(mock_frame);
    return 0;
}

void test_image_return_single(void) {
    mock_frame[coord1] = 0xFF;
    mock_frame[coord2] = 0xFF;

    points_detected result = get_all_flashes(mock_frame, screen_buffer_size, FIRST_ONLY);
    CU_ASSERT_EQUAL_FATAL(result.len, 1)
    CU_ASSERT_PTR_NOT_EQUAL_FATAL(result.arr, NULL)
    CU_ASSERT_EQUAL(*result.arr, coord1)
    free(result.arr);
}

void test_image_return_multiple(void) {
    mock_frame[coord1] = 0xFF;
    mock_frame[coord2] = 0xFF;

    points_detected result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    CU_ASSERT_EQUAL(result.len, 2)
    CU_ASSERT_PTR_NOT_EQUAL(result.arr, NULL)
    CU_ASSERT_EQUAL(result.arr[0], coord1)
    CU_ASSERT_EQUAL(result.arr[1], coord2)
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

void test_image_ccl(void) {
    uint p1 = xy_to_yuv(1u, 2u);
    uint p2 = xy_to_yuv(1u, 3u);
    uint p3 = xy_to_yuv(2u, 2u);
    uint p4 = xy_to_yuv(2u, 3u);
    mock_frame[p1] = 0xFF;
    mock_frame[p2] = 0xFF;
    mock_frame[p3] = 0xFF;
    mock_frame[p4] = 0xFF;

    points_detected result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    CU_ASSERT_EQUAL(result.len, 1)
    CU_ASSERT_PTR_NOT_EQUAL(result.arr, NULL)
    CU_ASSERT_EQUAL(result.arr[0], p1)
    free(result.arr);

    /* Expect round-robin to change the representative next time */
    result = get_all_flashes(mock_frame, screen_buffer_size, FULL_SCAN);
    CU_ASSERT_EQUAL(result.arr[0], p2)
    free(result.arr);
}

void test_image_ll(void) {
    node_t *queue = NULL;
    push_item(&queue, 3u);
    CU_ASSERT_PTR_NOT_NULL(queue)

    push_item(&queue, 5u);
    push_item(&queue, 1u);

    D(dump_list(queue));

    CU_ASSERT_EQUAL(get_item(queue, 0), 3u);
    CU_ASSERT_EQUAL(get_item(queue, 1), 5u);
    CU_ASSERT_EQUAL(get_item(queue, 2), 1u);

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
    CU_ASSERT_PTR_NOT_NULL(queue)
    D(dump_list(queue));
    delete_list(&queue);
    CU_ASSERT_PTR_NULL(queue)
}

uint xy_to_yuv(uint x, uint y) {
    return (settings.width * y + x) * 2;
}

void dump_list(node_t *head) {
    printf("\n");
    while (head != NULL) {
        printf("%d, ", head->v);
        head = head->next;
    }
}