#ifndef ALPHARAD_TEST_DATA_EXTRACTORS_H
#define ALPHARAD_TEST_DATA_EXTRACTORS_H

#include <stdlib.h>

void data_extractors_test_init(void);

void data_extractors_test_teardown(void);

void test_data_default(void);

#if HAVE_OPENSSL

void test_data_sha256_non_blank_frames(void);

void test_data_sha256_all_frames(void);

#endif //HAVE_OPENSSL

void test_data_bit_accumulator(void);

#endif //ALPHARAD_TEST_DATA_EXTRACTORS_H
