#ifndef ALPHARAD_TEST_IMAGE_PROCESSING_H
#define ALPHARAD_TEST_IMAGE_PROCESSING_H

int image_processing_test_init(void);

int image_processing_suite_cleanup(void);

void test_image_ccl(void);

void test_image_return_single(void);

void test_image_return_multiple(void);

void test_image_wrapper(void);

void test_image_empty(void);

#endif //ALPHARAD_TEST_IMAGE_PROCESSING_H
