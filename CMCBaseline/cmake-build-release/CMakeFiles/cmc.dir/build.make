# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zju/junnan/tmp/cmc

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zju/junnan/tmp/cmc/cmake-build-release

# Include any dependencies generated for this target.
include CMakeFiles/cmc.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/cmc.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/cmc.dir/flags.make

CMakeFiles/cmc.dir/main.cpp.o: CMakeFiles/cmc.dir/flags.make
CMakeFiles/cmc.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zju/junnan/tmp/cmc/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/cmc.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/cmc.dir/main.cpp.o -c /home/zju/junnan/tmp/cmc/main.cpp

CMakeFiles/cmc.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cmc.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zju/junnan/tmp/cmc/main.cpp > CMakeFiles/cmc.dir/main.cpp.i

CMakeFiles/cmc.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cmc.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zju/junnan/tmp/cmc/main.cpp -o CMakeFiles/cmc.dir/main.cpp.s

# Object files for target cmc
cmc_OBJECTS = \
"CMakeFiles/cmc.dir/main.cpp.o"

# External object files for target cmc
cmc_EXTERNAL_OBJECTS =

cmc: CMakeFiles/cmc.dir/main.cpp.o
cmc: CMakeFiles/cmc.dir/build.make
cmc: CMakeFiles/cmc.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zju/junnan/tmp/cmc/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable cmc"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cmc.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/cmc.dir/build: cmc

.PHONY : CMakeFiles/cmc.dir/build

CMakeFiles/cmc.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/cmc.dir/cmake_clean.cmake
.PHONY : CMakeFiles/cmc.dir/clean

CMakeFiles/cmc.dir/depend:
	cd /home/zju/junnan/tmp/cmc/cmake-build-release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zju/junnan/tmp/cmc /home/zju/junnan/tmp/cmc /home/zju/junnan/tmp/cmc/cmake-build-release /home/zju/junnan/tmp/cmc/cmake-build-release /home/zju/junnan/tmp/cmc/cmake-build-release/CMakeFiles/cmc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/cmc.dir/depend
