cmake_minimum_required(VERSION 2.6)

project(libva-amd-driver)
set(LIB_VERSION_MAJOR 0)
set(LIB_VERSION_MINOR 1)

configure_file("base.h.in" "base.h")
add_library(libva_driver base.h)
