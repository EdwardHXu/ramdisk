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

# Include any dependencies generated for this target.
include CMakeFiles/RAMDISK.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/RAMDISK.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/RAMDISK.dir/flags.make

CMakeFiles/RAMDISK.dir/ramdisk_module.c.o: CMakeFiles/RAMDISK.dir/flags.make
CMakeFiles/RAMDISK.dir/ramdisk_module.c.o: ../ramdisk_module.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/RAMDISK.dir/ramdisk_module.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/RAMDISK.dir/ramdisk_module.c.o   -c /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/ramdisk_module.c

CMakeFiles/RAMDISK.dir/ramdisk_module.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/RAMDISK.dir/ramdisk_module.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/ramdisk_module.c > CMakeFiles/RAMDISK.dir/ramdisk_module.c.i

CMakeFiles/RAMDISK.dir/ramdisk_module.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/RAMDISK.dir/ramdisk_module.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/ramdisk_module.c -o CMakeFiles/RAMDISK.dir/ramdisk_module.c.s

CMakeFiles/RAMDISK.dir/ramdisk_module.c.o.requires:

.PHONY : CMakeFiles/RAMDISK.dir/ramdisk_module.c.o.requires

CMakeFiles/RAMDISK.dir/ramdisk_module.c.o.provides: CMakeFiles/RAMDISK.dir/ramdisk_module.c.o.requires
	$(MAKE) -f CMakeFiles/RAMDISK.dir/build.make CMakeFiles/RAMDISK.dir/ramdisk_module.c.o.provides.build
.PHONY : CMakeFiles/RAMDISK.dir/ramdisk_module.c.o.provides

CMakeFiles/RAMDISK.dir/ramdisk_module.c.o.provides.build: CMakeFiles/RAMDISK.dir/ramdisk_module.c.o


CMakeFiles/RAMDISK.dir/our_tests.c.o: CMakeFiles/RAMDISK.dir/flags.make
CMakeFiles/RAMDISK.dir/our_tests.c.o: ../our_tests.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/RAMDISK.dir/our_tests.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/RAMDISK.dir/our_tests.c.o   -c /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/our_tests.c

CMakeFiles/RAMDISK.dir/our_tests.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/RAMDISK.dir/our_tests.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/our_tests.c > CMakeFiles/RAMDISK.dir/our_tests.c.i

CMakeFiles/RAMDISK.dir/our_tests.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/RAMDISK.dir/our_tests.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/our_tests.c -o CMakeFiles/RAMDISK.dir/our_tests.c.s

CMakeFiles/RAMDISK.dir/our_tests.c.o.requires:

.PHONY : CMakeFiles/RAMDISK.dir/our_tests.c.o.requires

CMakeFiles/RAMDISK.dir/our_tests.c.o.provides: CMakeFiles/RAMDISK.dir/our_tests.c.o.requires
	$(MAKE) -f CMakeFiles/RAMDISK.dir/build.make CMakeFiles/RAMDISK.dir/our_tests.c.o.provides.build
.PHONY : CMakeFiles/RAMDISK.dir/our_tests.c.o.provides

CMakeFiles/RAMDISK.dir/our_tests.c.o.provides.build: CMakeFiles/RAMDISK.dir/our_tests.c.o


CMakeFiles/RAMDISK.dir/prof_tests.c.o: CMakeFiles/RAMDISK.dir/flags.make
CMakeFiles/RAMDISK.dir/prof_tests.c.o: ../prof_tests.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/RAMDISK.dir/prof_tests.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/RAMDISK.dir/prof_tests.c.o   -c /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/prof_tests.c

CMakeFiles/RAMDISK.dir/prof_tests.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/RAMDISK.dir/prof_tests.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/prof_tests.c > CMakeFiles/RAMDISK.dir/prof_tests.c.i

CMakeFiles/RAMDISK.dir/prof_tests.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/RAMDISK.dir/prof_tests.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/prof_tests.c -o CMakeFiles/RAMDISK.dir/prof_tests.c.s

CMakeFiles/RAMDISK.dir/prof_tests.c.o.requires:

.PHONY : CMakeFiles/RAMDISK.dir/prof_tests.c.o.requires

CMakeFiles/RAMDISK.dir/prof_tests.c.o.provides: CMakeFiles/RAMDISK.dir/prof_tests.c.o.requires
	$(MAKE) -f CMakeFiles/RAMDISK.dir/build.make CMakeFiles/RAMDISK.dir/prof_tests.c.o.provides.build
.PHONY : CMakeFiles/RAMDISK.dir/prof_tests.c.o.provides

CMakeFiles/RAMDISK.dir/prof_tests.c.o.provides.build: CMakeFiles/RAMDISK.dir/prof_tests.c.o


CMakeFiles/RAMDISK.dir/real_tests.c.o: CMakeFiles/RAMDISK.dir/flags.make
CMakeFiles/RAMDISK.dir/real_tests.c.o: ../real_tests.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/RAMDISK.dir/real_tests.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/RAMDISK.dir/real_tests.c.o   -c /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/real_tests.c

CMakeFiles/RAMDISK.dir/real_tests.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/RAMDISK.dir/real_tests.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/real_tests.c > CMakeFiles/RAMDISK.dir/real_tests.c.i

CMakeFiles/RAMDISK.dir/real_tests.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/RAMDISK.dir/real_tests.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/real_tests.c -o CMakeFiles/RAMDISK.dir/real_tests.c.s

CMakeFiles/RAMDISK.dir/real_tests.c.o.requires:

.PHONY : CMakeFiles/RAMDISK.dir/real_tests.c.o.requires

CMakeFiles/RAMDISK.dir/real_tests.c.o.provides: CMakeFiles/RAMDISK.dir/real_tests.c.o.requires
	$(MAKE) -f CMakeFiles/RAMDISK.dir/build.make CMakeFiles/RAMDISK.dir/real_tests.c.o.provides.build
.PHONY : CMakeFiles/RAMDISK.dir/real_tests.c.o.provides

CMakeFiles/RAMDISK.dir/real_tests.c.o.provides.build: CMakeFiles/RAMDISK.dir/real_tests.c.o


CMakeFiles/RAMDISK.dir/ramdisk.c.o: CMakeFiles/RAMDISK.dir/flags.make
CMakeFiles/RAMDISK.dir/ramdisk.c.o: ../ramdisk.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/RAMDISK.dir/ramdisk.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/RAMDISK.dir/ramdisk.c.o   -c /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/ramdisk.c

CMakeFiles/RAMDISK.dir/ramdisk.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/RAMDISK.dir/ramdisk.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/ramdisk.c > CMakeFiles/RAMDISK.dir/ramdisk.c.i

CMakeFiles/RAMDISK.dir/ramdisk.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/RAMDISK.dir/ramdisk.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/ramdisk.c -o CMakeFiles/RAMDISK.dir/ramdisk.c.s

CMakeFiles/RAMDISK.dir/ramdisk.c.o.requires:

.PHONY : CMakeFiles/RAMDISK.dir/ramdisk.c.o.requires

CMakeFiles/RAMDISK.dir/ramdisk.c.o.provides: CMakeFiles/RAMDISK.dir/ramdisk.c.o.requires
	$(MAKE) -f CMakeFiles/RAMDISK.dir/build.make CMakeFiles/RAMDISK.dir/ramdisk.c.o.provides.build
.PHONY : CMakeFiles/RAMDISK.dir/ramdisk.c.o.provides

CMakeFiles/RAMDISK.dir/ramdisk.c.o.provides.build: CMakeFiles/RAMDISK.dir/ramdisk.c.o


# Object files for target RAMDISK
RAMDISK_OBJECTS = \
"CMakeFiles/RAMDISK.dir/ramdisk_module.c.o" \
"CMakeFiles/RAMDISK.dir/our_tests.c.o" \
"CMakeFiles/RAMDISK.dir/prof_tests.c.o" \
"CMakeFiles/RAMDISK.dir/real_tests.c.o" \
"CMakeFiles/RAMDISK.dir/ramdisk.c.o"

# External object files for target RAMDISK
RAMDISK_EXTERNAL_OBJECTS =

RAMDISK: CMakeFiles/RAMDISK.dir/ramdisk_module.c.o
RAMDISK: CMakeFiles/RAMDISK.dir/our_tests.c.o
RAMDISK: CMakeFiles/RAMDISK.dir/prof_tests.c.o
RAMDISK: CMakeFiles/RAMDISK.dir/real_tests.c.o
RAMDISK: CMakeFiles/RAMDISK.dir/ramdisk.c.o
RAMDISK: CMakeFiles/RAMDISK.dir/build.make
RAMDISK: CMakeFiles/RAMDISK.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking C executable RAMDISK"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/RAMDISK.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/RAMDISK.dir/build: RAMDISK

.PHONY : CMakeFiles/RAMDISK.dir/build

CMakeFiles/RAMDISK.dir/requires: CMakeFiles/RAMDISK.dir/ramdisk_module.c.o.requires
CMakeFiles/RAMDISK.dir/requires: CMakeFiles/RAMDISK.dir/our_tests.c.o.requires
CMakeFiles/RAMDISK.dir/requires: CMakeFiles/RAMDISK.dir/prof_tests.c.o.requires
CMakeFiles/RAMDISK.dir/requires: CMakeFiles/RAMDISK.dir/real_tests.c.o.requires
CMakeFiles/RAMDISK.dir/requires: CMakeFiles/RAMDISK.dir/ramdisk.c.o.requires

.PHONY : CMakeFiles/RAMDISK.dir/requires

CMakeFiles/RAMDISK.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/RAMDISK.dir/cmake_clean.cmake
.PHONY : CMakeFiles/RAMDISK.dir/clean

CMakeFiles/RAMDISK.dir/depend:
	cd /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug /Users/edwardxu/Files/CBP/workbench/BU/CS552/ramdisk/cmake-build-debug/CMakeFiles/RAMDISK.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/RAMDISK.dir/depend
