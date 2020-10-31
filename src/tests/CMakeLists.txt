enable_testing()

set(test_files
        test_settings.c
        test_image_processing.c
        test_data_extractors.c
        test_logger.c
        test_playground.c
        tests_runner.c
        )

add_executable(tests_runner ${test_files})
target_link_libraries(tests_runner PRIVATE libalpharad cunit)

add_test(tests_runner COMMAND ${EXECUTABLE_OUTPUT_PATH}/tests_runner WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})