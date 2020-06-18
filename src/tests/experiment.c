#include <stdio.h>
#include <CUnit/Basic.h>
#include "../settings.h"

void test_1(void) {
    CU_ASSERT(2 == add(1, 1));
}

void test_2(void) {
    CU_ASSERT(4 == add(4, 0));
    CU_ASSERT(4 == add(2, 2));
    CU_ASSERT(4 == add(1, add(1, add(1, 1))));
}

int main(int argc, char **argv) {
    // Get rid of 'unused parameter' warning
    (void) argc;
    (void) argv;

    // Suites
    CU_pSuite s1 = NULL,
            s2 = NULL,
            s3 = NULL;

    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    s1 = CU_add_suite("Utility functions", NULL, NULL);
    if (NULL == s1 /*|| NULL == s2 || NULL == s3*/) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Tests
    if (
            NULL == CU_add_test(s1, "Simplest test", test_1)
            || NULL == CU_add_test(s1, "Simplest test 2", test_2)
            ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Run
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    CU_cleanup_registry();
    return CU_get_error();
}