#include <CUnit/Basic.h>
#include "test_playground.h"
#include <stdint.h>
#include <stdlib.h>

void test_playground_overflow_rotation(void) {
    uint8_t counter = 0u;
    for (uint16_t iter = 0; iter < 514u; iter++, counter++) {
//        printf("%3d | %3d | %d | %d | %2d\n", iter, counter, counter % 7, counter % 8, counter % 16);
        if (iter == 0u) {
            CU_ASSERT_EQUAL(counter % 8, 0u)
            CU_ASSERT_EQUAL(counter % 16, 0u)
        }
        if (iter == 255u) {
            CU_ASSERT_EQUAL(counter % 8, 7u)
            CU_ASSERT_EQUAL(counter % 16, 15u)
        }
        if (iter == 256u) {
            CU_ASSERT_EQUAL(counter % 8, 0u)
            CU_ASSERT_EQUAL(counter % 16, 0u)
            CU_ASSERT_EQUAL(counter, 0u)
        }
    }
}

/**
 * While scanning our camera buffer we should remember visited pixels.
 * Here is an attempt on using O(1) time with 1 bit per pixel space structure
 * I've been told that compiler automatically does something similar to an array
 * of booleans. But it is Saturday evening and I'm having fun LOL.
 */
void test_playground_bitmap(void) {
    int size = 240 * 120 * 2;

    // Allocate enough memory so that every pixel would have a corresponding bit
    uint_fast16_t *visited = calloc(ceil(size / 16.0), sizeof(uint_fast16_t));
    uint coord_coarse = 9u;
    uint_fast16_t coord_fine = 1u << (coord_coarse % 16);

    // Check if marked
    CU_ASSERT_FALSE((visited[coord_coarse / 16] & coord_fine) > 0)

    // Mark as visited
    visited[coord_coarse / 16] |= coord_fine;
    CU_ASSERT_TRUE((visited[coord_coarse / 16] & coord_fine) > 0)
    // 9 => 9th bit should be set in the zeroth bucket, 2^9
    CU_ASSERT_EQUAL(visited[0], 512u)

    coord_coarse = 101u;
    coord_fine = 1u << (coord_coarse % 16);
    CU_ASSERT_FALSE((visited[coord_coarse / 16] & coord_fine) > 0)

    visited[coord_coarse / 16] |= coord_fine;
    // 101 => 5th bit should be set in the sixth bucket, 2^5
    CU_ASSERT_EQUAL(visited[6], 32u)
}
