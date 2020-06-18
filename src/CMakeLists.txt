add_executable(${PROJECT_NAME} alpharad.c settings.c)

add_library(v4l2_util v4l2_util.c settings.c)
target_link_libraries(${PROJECT_NAME} PRIVATE v4l2_util m)

find_package(OpenSSL)

if (OPENSSL_FOUND)
    message("Compiling with OpenSSL crypto support")
    add_compile_definitions(HAVE_OPENSSL=1)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE crypto)
endif ()

enable_testing()

add_executable(test_lib tests/experiment.c settings.c)

target_link_libraries(test_lib PRIVATE cunit)

add_test(test_lib COMMAND ${EXECUTABLE_OUTPUT_PATH}/test_lib WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})