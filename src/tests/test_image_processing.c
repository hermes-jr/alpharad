#include <CUnit/Basic.h>
#include <stdint.h>
#include <stdbool.h>
#include "../frame_cca.h"
#include "../settings.h"

extern struct settings settings;

uint coord1;
uint coord2;
uint8_t *mock_frame;
uint screen_buffer_size;

int image_processing_test_init(void) {
    settings.width = 16;
    settings.height = 8;

    /* Create empty canvas, each test will fill in the needed data */
    screen_buffer_size = settings.width * settings.height * 2;
    mock_frame = malloc(screen_buffer_size);
    memset(mock_frame, 0, screen_buffer_size);

    /* Common test points */
    coord1 = settings.width * 2 + 2; // x=1 y=1
    coord2 = settings.width * 2 + 4; // x=2 y=1

    return 0;
}

int image_processing_suite_cleanup(void) {
    free(mock_frame);
    return 0;
}

void test_image_ccl(void) {
    CU_FAIL("TBD")
}

void test_image_return_single(void) {
    mock_frame[coord1] = 0xFF;
    mock_frame[coord2] = 0xFF;

    points_detected result = get_all_flashes(mock_frame, screen_buffer_size, true);
    CU_ASSERT_EQUAL_FATAL(result.len, 1)
    CU_ASSERT_PTR_NOT_EQUAL_FATAL(result.arr, NULL)
    CU_ASSERT_EQUAL(*result.arr, coord1)
    free(result.arr);
}

void test_image_return_multiple(void) {
    memset(mock_frame, 0, screen_buffer_size);
    mock_frame[coord1] = 0xFF;
    mock_frame[coord2] = 0xFF;

    points_detected result = get_all_flashes(mock_frame, screen_buffer_size,
                                             false /* todo: cosmetic FULL_SCAN | FIRST_ONLY */);
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
    points_detected result = get_all_flashes(mock_frame, screen_buffer_size,
                                             false /* todo: cosmetic FULL_SCAN | FIRST_ONLY */);
    CU_ASSERT_EQUAL(result.len, 0)
    CU_ASSERT_PTR_EQUAL(result.arr, NULL)
}