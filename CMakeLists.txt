cmake_minimum_required(VERSION 3.13)

project(matrix-client LANGUAGES C VERSION 1.0.0)

set (CMAKE_C_STANDARD 23)

set(MATRIXC_SRCS
        main.c
        utils.c
        utils.h
        matrix/matrix-client.h
        matrix/login.c
        matrix/login.h
        matrix/matrix-utils.c
        matrix/matrix-utils.h
)

find_package(PkgConfig)
pkg_check_modules(curl REQUIRED libcurl)
pkg_check_modules(jansson REQUIRED jansson)

add_executable(${PROJECT_NAME} ${MATRIXC_SRCS})

target_compile_options(${PROJECT_NAME} PRIVATE
        -g -O2 -Wall
        ${curl_CFLAGS_OTHER}
        ${jansson_CFLAGS_OTHER}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        include
        ${curl_INCLUDE_DIRS}
        ${jansson_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
        ${curl_LIBRARIES}
        ${jansson_LIBRARIES}
)

ctr_generate_smdh(${PROJECT_NAME}.smdh
        NAME "${PROJECT_NAME}"
        DESCRIPTION "v${PROJECT_VERSION}"
        AUTHOR "A Sickly Silver Moon"
)

ctr_create_3dsx(${PROJECT_NAME}
        SMDH ${PROJECT_NAME}.smdh
)
