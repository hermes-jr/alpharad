#include <CUnit/Basic.h>
#include "test_settings.h"
#include "test_image_processing.h"
#include "test_playground.h"

int main(int argc, char **argv) {
    // Get rid of 'unused parameter' warning
    (void) argc;
    (void) argv;

    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    // Tests
    CU_TestInfo settings_suite_tests[] = {
            {"Device should be set", test_settings_population_device},
            {"Dimensions should be parsed properly", test_settings_population_dimensions},
            {"Incorrect dimensions should be reported", test_settings_incorrect_dimensions},
            {"Outfile should be set", test_settings_population_outfile},
            {"Logfile should be set", test_settings_population_logfile},
            {"Verbosity level should be set", test_settings_population_verbose},
            CU_TEST_INFO_NULL
    };

    CU_TestInfo image_processing_suite_tests[] = {
            {"Image processor should return empty result with blank frame", test_image_empty},
            {"Image processor should break early returning a single point if requested", test_image_return_single},
            {"Image processor return multiple points if requested", test_image_return_multiple},
            {"Image processor wrapper function test", test_image_wrapper},
            {"Image processor should yield proper representatives with CCL", test_image_ccl},
            {"Duh, linked list, queue, whatever this is. Inventing the wheel", test_image_ll},
            CU_TEST_INFO_NULL
    };

    CU_TestInfo data_extraction_suite_tests[] = {
//            {"Comparator", test_data_comparator},
            CU_TEST_INFO_NULL
    };

    CU_TestInfo playground_suite_tests[] = {
            {"Int overflow rotation", test_playground_overflow_rotation},
            CU_TEST_INFO_NULL
    };

    // Suites
    CU_SuiteInfo suites[] = {
            {"Settings suite", NULL, NULL, NULL, NULL, settings_suite_tests},
            {"Image processing suite", NULL, image_processing_suite_cleanup, image_processing_test_init, NULL,
             image_processing_suite_tests},
            {"Data extraction suite", NULL, NULL, NULL, NULL, data_extraction_suite_tests},
            {"Playground suite", NULL, NULL, NULL, NULL, playground_suite_tests},
            CU_SUITE_INFO_NULL
    };

    CU_ErrorCode error = CU_register_suites(suites);
    if (CUE_SUCCESS != error) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Run
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    CU_cleanup_registry();
    return CU_get_error();
}