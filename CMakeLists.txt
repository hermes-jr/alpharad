cmake_minimum_required(VERSION 3.5)
project(alpharad VERSION 0.5 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(alpharad main.c)

#add_library(v4l2-util v4l-util.c)
#target_link_libraries(v4l2-util)

find_package(OpenSSL)

if (OPENSSL_FOUND)
    message("Compiling with OpenSSL crypto support")
    add_compile_definitions(HAVE_OPENSSL)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE crypto)
endif ()
