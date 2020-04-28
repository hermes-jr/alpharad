cmake_minimum_required(VERSION 3.13)
project(alpharad C)

set(CMAKE_C_STANDARD 11)

add_executable(alpharad main.c)

target_link_libraries(${PROJECT_NAME} crypto)