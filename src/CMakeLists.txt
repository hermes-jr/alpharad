add_executable(${PROJECT_NAME} alpharad.c settings.c frame_processors.c frame_processors.h)

add_library(v4l2_util v4l2_util.c settings.c)
target_link_libraries(${PROJECT_NAME} PRIVATE v4l2_util m)

find_package(OpenSSL)

if (OPENSSL_FOUND)
    message("Compiling with OpenSSL crypto support")
    add_compile_definitions(HAVE_OPENSSL=1)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE crypto)
endif ()

add_subdirectory(tests)