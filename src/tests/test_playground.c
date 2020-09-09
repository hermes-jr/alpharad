#include <CUnit/Basic.h>
#include "test_playground.h"
#include <stdint.h>

void test_playground_overflow_rotation(void) {
    uint8_t counter = 0u;
    for (uint16_t iter = 0; iter < 514u; iter++, counter++) {
//        printf("%3d | %3d | %d | %d | %2d\n", iter, counter, counter % 7, counter % 8, counter % 16);
        if (iter == 0u) {
            CU_ASSERT_EQUAL(counter % 8, 0u);
            CU_ASSERT_EQUAL(counter % 16, 0u);
        }
        if (iter == 255u) {
            CU_ASSERT_EQUAL(counter % 8, 7u);
            CU_ASSERT_EQUAL(counter % 16, 15u);
        }
        if (iter == 256u) {
            CU_ASSERT_EQUAL(counter % 8, 0u);
            CU_ASSERT_EQUAL(counter % 16, 0u);
            CU_ASSERT_EQUAL(counter, 0u);
        }
    }
}
