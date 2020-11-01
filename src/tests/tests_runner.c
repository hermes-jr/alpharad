#include <CUnit/Basic.h>
#include "test_settings.h"
#include "test_image_processing.h"
#include "test_data_extractors.h"
#include "test_logger.h"
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
            {"Incorrect dimensions should be reported", test_settings_incorrect_width},
            {"Incorrect dimensions should be reported", test_settings_incorrect_height},
            {"Outfile should be set", test_settings_population_outfile},
            {"Logfile should be set", test_settings_population_logfile},
            {"Verbosity level should be set", test_settings_population_verbose},
            {"Border cropping should be set", test_settings_population_crop},
            {"Mode should be set", test_settings_population_mode},
            {"Incorrect mode should be reported", test_settings_population_mode_unrecognized},
            {"Mode list should be printed", test_settings_population_print_modes},
            {"Help option behavior", test_settings_population_help},
            {"Help should be printed if unrecognized option passed", test_settings_unrecognized_option},
            CU_TEST_INFO_NULL
    };

    CU_TestInfo image_processing_suite_tests[] = {
            {"Image processor should return empty result with blank frame", test_image_empty},
            {"Image processor should break early returning a single point if requested", test_image_return_single},
            {"Image processor return multiple points if requested", test_image_return_multiple},
            {"Image processor wrapper function test", test_image_wrapper},
            {"Image processor should detect discrete groups of pixels", test_image_ccl},
            {"Image processor should use round-robin to pick a different representative each time", test_image_ccl_rr},
            {"Detected points should be logged", test_image_logging},
            {"Check if N-pixels thick border pixels are actually ignored", test_image_border_crop},
            {"Check for rounding errors in ideal conditions", test_image_rounding_errors},
            {"Duh, linked list, queue, whatever this is. Inventing the wheel", test_image_ll},
            CU_TEST_INFO_NULL
    };

    CU_TestInfo data_extraction_suite_tests[] = {
            {"Data default", test_data_default},
#if HAVE_OPENSSL
            {"Data SHA256 single frame", test_data_sha256_non_blank_frames},
            {"Data SHA256 every frame", test_data_sha256_all_frames},
#endif //HAVE_OPENSSL
            {"Bit shift", test_data_bit_accumulator},
            CU_TEST_INFO_NULL
    };

    CU_TestInfo logger_suite_tests[] = {
            {"Log level", test_logger_levels},
            CU_TEST_INFO_NULL
    };

    CU_TestInfo playground_suite_tests[] = {
            {"Int overflow rotation", test_playground_overflow_rotation},
            {"Duct tape bitmap", test_playground_bitmap},
            CU_TEST_INFO_NULL
    };

    // Suites
    CU_SuiteInfo suites[] = {
            {"Settings suite", settings_suite_init, NULL, settings_test_init, settings_test_teardown,
             settings_suite_tests},
            {"Image processing suite", NULL, NULL, image_processing_test_init, image_processing_test_teardown,
             image_processing_suite_tests},
            {"Data extraction suite", NULL, NULL, data_extractors_test_init, data_extractors_test_teardown,
             data_extraction_suite_tests},
            {"Logger suite", NULL, NULL, NULL, NULL, logger_suite_tests},
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