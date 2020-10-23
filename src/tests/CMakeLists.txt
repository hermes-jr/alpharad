enable_testing()

set(test_files
        ../settings.c
        ../frame_cca.c
        ../data_extractors.c
        test_settings.c
        test_image_processing.c
        test_playground.c
        tests_runner.c
        )

add_executable(tests_runner ${test_files})
target_link_libraries(tests_runner PRIVATE m)

if (OPENSSL_FOUND)
    message("Compiling with OpenSSL crypto support")
    add_compile_definitions(HAVE_OPENSSL=1)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    target_link_libraries(tests_runner PRIVATE crypto)
endif ()
target_link_libraries(tests_runner PRIVATE cunit)

add_test(tests_runner COMMAND ${EXECUTABLE_OUTPUT_PATH}/tests_runner WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})