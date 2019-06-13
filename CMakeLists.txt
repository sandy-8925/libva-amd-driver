cmake_minimum_required(VERSION 3.5)

set(LIB_NAME "amd_drv_video")
set(LIB_VERSION_MAJOR 0)
set(LIB_VERSION_MINOR 1)

project(libva-amd-driver VERSION "${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}" LANGUAGES CXX)

configure_file("src/base.h.in" "${PROJECT_BINARY_DIR}/base.h")

add_library(${LIB_NAME} SHARED)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_14)
target_include_directories(${LIB_NAME}
                           PRIVATE
                                ${PROJECT_BINARY_DIR}
                                ${CMAKE_CURRENT_SOURCE_DIR}/src
                          )
target_sources(${LIB_NAME}
                PRIVATE
                    "src/DataTables.cpp"
                    "src/main.cpp"
              )
