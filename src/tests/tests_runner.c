#include <CUnit/Basic.h>
#include "test_settings.h"

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

    // Suites
    CU_SuiteInfo suites[] = {
            {"Settings suite", NULL, NULL, NULL, NULL, settings_suite_tests},
//            { "suitename2", suite2_init-func, suite2_cleanup_func, test_array2 },
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