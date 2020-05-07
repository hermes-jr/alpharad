add_executable(alpharad alpharad.c)

add_library(v4l2_util v4l2_util.c)
target_link_libraries(alpharad PRIVATE v4l2_util)

find_package(OpenSSL)

if (OPENSSL_FOUND)
    message("Compiling with OpenSSL crypto support")
    add_compile_definitions(HAVE_OPENSSL=1)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE crypto)
endif ()