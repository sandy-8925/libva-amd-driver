cmake_minimum_required(VERSION 2.6)

project(libva-amd-driver)

set(LIB_VERSION_MAJOR 0)
set(LIB_VERSION_MINOR 1)
set(LIB_NAME "amd_drv_video")

configure_file("base.h.in" "${PROJECT_BINARY_DIR}/base.h")

include_directories("${PROJECT_BINARY_DIR}")

set(SOURCE_FILES
    "main.cpp"
    )

add_library(${LIB_NAME} SHARED ${SOURCE_FILES})
