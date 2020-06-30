enable_testing()

add_executable(tests_runner tests_runner.c ../settings.c test_settings.c test_settings.h ../frame_processors.c ../frame_processors.h test_playground.c test_playground.h)

target_link_libraries(tests_runner PRIVATE cunit)

add_test(tests_runner COMMAND ${EXECUTABLE_OUTPUT_PATH}/tests_runner WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})