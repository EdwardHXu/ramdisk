# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.8

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug

# Utility rule file for mytest.

# Include the progress variables for this target.
include CMakeFiles/mytest.dir/progress.make

CMakeFiles/mytest:
	make -C /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk CLION_EXE_DIR=/Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug

mytest: CMakeFiles/mytest
mytest: CMakeFiles/mytest.dir/build.make

.PHONY : mytest

# Rule to build all files generated by this target.
CMakeFiles/mytest.dir/build: mytest

.PHONY : CMakeFiles/mytest.dir/build

CMakeFiles/mytest.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/mytest.dir/cmake_clean.cmake
.PHONY : CMakeFiles/mytest.dir/clean

CMakeFiles/mytest.dir/depend:
	cd /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug/CMakeFiles/mytest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/mytest.dir/depend

