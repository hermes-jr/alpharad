#ifndef ALPHARAD_TEST_IMAGE_PROCESSING_H
#define ALPHARAD_TEST_IMAGE_PROCESSING_H

#include <stdlib.h>
#include "../frame_cca.h"

void image_processing_test_init(void);

void image_processing_test_teardown(void);

void test_image_ccl(void);

void test_image_ccl_rr(void);

void test_image_return_single(void);

void test_image_return_multiple(void);

void test_image_wrapper(void);

void test_image_empty(void);

void test_image_logging(void);

void test_image_border_crop(void);

void test_image_rounding_errors(void);

void test_image_ll(void);

uint cartesian_to_yuv(coordinate c);

void dump_list(node_t *head);

#endif //ALPHARAD_TEST_IMAGE_PROCESSING_H
