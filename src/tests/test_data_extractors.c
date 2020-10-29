#include <CUnit/Basic.h>
#include "../settings.h"
#include "../data_extractors.h"
#include "test_data_extractors.h"

#if HAVE_OPENSSL

#include <openssl/sha.h>

#endif //HAVE_OPENSSL

extern struct settings settings;

uint coord1;
uint coord2;
uint8_t *mock_frame;
uint screen_buffer_size;

/* Imitate fake camera with 640:480 resolution and predictable pixels (per test) being turned on */
void data_extractors_test_init(void) {
    settings.width = 640;
    settings.height = 480;

    /* Create empty canvas, each test will fill in the needed data */
    screen_buffer_size = settings.width * settings.height * 2;
    mock_frame = calloc(screen_buffer_size, sizeof(uint8_t));

    /* Common test points */
    coord1 = settings.width * 2 + 2; // x=1 y=1
    coord2 = settings.width * 2 + 6; // x=3 y=1
}

/* Free fake frame buffer */
void data_extractors_test_teardown(void) {
    free(mock_frame);
}

void test_data_default(void) {
    /* Nothing should come from blank frames */
    bytes_spawned result = process_image_default(mock_frame, screen_buffer_size);
    CU_ASSERT_EQUAL(result.len, 0)

    /* Each flash should spawn one byte */
    mock_frame[0] = mock_frame[screen_buffer_size - 4] = 0xFF;

    result = process_image_default(mock_frame, screen_buffer_size);
    CU_ASSERT_EQUAL_FATAL(result.len, 2)
    CU_ASSERT_PTR_NOT_EQUAL_FATAL(result.arr, NULL)
    CU_ASSERT_EQUAL(result.arr[0], 0)
    CU_ASSERT_EQUAL(result.arr[1], 255)

    free(result.arr);
}

#if HAVE_OPENSSL

void test_data_sha256_non_blank_frames(void) {
    /* Nothing should come from blank frames */
    bytes_spawned result = process_image_sha256_non_blank_frames(mock_frame, screen_buffer_size);
    CU_ASSERT_EQUAL(result.len, 0)

    /* Any number of flashes should spawn a hash */
    mock_frame[coord1] = 0xFF;

    result = process_image_sha256_non_blank_frames(mock_frame, screen_buffer_size);
    CU_ASSERT_EQUAL_FATAL(result.len, SHA256_DIGEST_LENGTH)
    CU_ASSERT_PTR_NOT_EQUAL_FATAL(result.arr, NULL)

    uint8_t digest[] = {
            0x62, 0x53, 0x42, 0x85, 0x09, 0x7b, 0x4d, 0x84, 0x52, 0xb2, 0xab,
            0x6d, 0x5c, 0x94, 0x79, 0xb6, 0xe8, 0x18, 0xe0, 0x47, 0xdc, 0x16,
            0x66, 0xf2, 0xdd, 0x8f, 0x8e, 0x15, 0xa3, 0x7d, 0xf1, 0x91
    };
    CU_ASSERT_NSTRING_EQUAL(result.arr, digest, SHA256_DIGEST_LENGTH)

    free(result.arr);
}

void test_data_sha256_all_frames(void) {
    /* In this mode every frame should be hashed */
    bytes_spawned result = process_image_sha256_all_frames(mock_frame, screen_buffer_size);
    CU_ASSERT_EQUAL(result.len, SHA256_DIGEST_LENGTH)
    free(result.arr);

    mock_frame[coord1] = 0xFF;

    result = process_image_sha256_all_frames(mock_frame, screen_buffer_size);
    CU_ASSERT_EQUAL_FATAL(result.len, SHA256_DIGEST_LENGTH)
    CU_ASSERT_PTR_NOT_EQUAL_FATAL(result.arr, NULL)

    free(result.arr);
}

#endif //HAVE_OPENSSL

void test_data_bit_accumulator(void) {
    uint8_t result = 0u;
    bool full_byte = false;

    for (int i = 0; i < 7; i++) {
        full_byte = bit_accumulator(false, &result);
    }
    CU_ASSERT_FALSE(full_byte)

    full_byte = bit_accumulator(true, &result);
    CU_ASSERT_TRUE(full_byte)
    CU_ASSERT_EQUAL(result, 1)

    bool v[] = {true, false, true, false, false, false, false, true};
    for (int i = 0; i < 8; i++) {
        full_byte = bit_accumulator(v[i], &result);
    }

    CU_ASSERT_TRUE(full_byte)
    /* 10100001 */
    CU_ASSERT_EQUAL(result, 161)
}