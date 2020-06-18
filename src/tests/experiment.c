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

void test_settings_population_dimensions(void) {
    extern struct settings settings;
    char **argv = (char *[]) {"prog", "--geometry", "100x200"};
    populate_settings(3, argv);
    CU_ASSERT_EQUAL(settings.width, 100);
    CU_ASSERT_EQUAL(settings.height, 200);
}

int main(int argc, char **argv) {
    // Get rid of 'unused parameter' warning
    (void) argc;
    (void) argv;

    // Suites
    CU_pSuite trivial_suite = NULL,
            settings_suite = NULL,
            v4l2_util_suite = NULL;

    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    trivial_suite = CU_add_suite("Trivial functions", NULL, NULL);
    settings_suite = CU_add_suite("Settings suite", NULL, NULL);
    v4l2_util_suite = CU_add_suite("v4l2 utilities", NULL, NULL);
    if (NULL == trivial_suite
        || NULL == settings_suite
        || NULL == v4l2_util_suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Tests
    if (
            NULL == CU_add_test(trivial_suite, "Simplest test", test_1)
            || NULL == CU_add_test(trivial_suite, "Simplest test 2", test_2)
            || NULL ==
               CU_add_test(settings_suite, "Dimensions should be parsed properly", test_settings_population_dimensions)
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