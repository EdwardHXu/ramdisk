cmake_minimum_required(VERSION 3.8)
project(ramdisk)

set(CMAKE_CXX_STANDARD 11)


set(SOURCE_FILES
        data_structures.h
        ramdisk_module.h
        ramdisk_module.c
        Makefile
        test_file.c
        ramdisk.h
        ramdisk.c
        )

add_executable(RAMDISK ${SOURCE_FILES})