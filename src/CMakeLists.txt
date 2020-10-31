add_executable(${PROJECT_NAME} alpharad.c)

set(LIBALPHARAD_FILES
        v4l2_util.c settings.c logger.c settings.c
        dto.h
        frame_cca.c frame_cca.h data_extractors.c
        )
add_library(libalpharad ${LIBALPHARAD_FILES})

# Math functions (round, ceil)
target_link_libraries(libalpharad PRIVATE m)

# Enable hash-based data generation if OpenSSL is available
find_package(OpenSSL)
if (OPENSSL_FOUND)
    message("Compiling with OpenSSL crypto support")
    add_definitions(-DHAVE_OPENSSL=1)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    target_link_libraries(libalpharad PRIVATE crypto)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE libalpharad)

add_subdirectory(tests)