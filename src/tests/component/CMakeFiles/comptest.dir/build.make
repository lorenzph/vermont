# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canoncical targets will work.
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

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/simon/da/lockfree

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/simon/da/lockfree

# Include any dependencies generated for this target.
include src/tests/component/CMakeFiles/comptest.dir/depend.make

# Include the progress variables for this target.
include src/tests/component/CMakeFiles/comptest.dir/progress.make

# Include the compile flags for this target's objects.
include src/tests/component/CMakeFiles/comptest.dir/flags.make

src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o: src/tests/component/CMakeFiles/comptest.dir/flags.make
src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o: src/tests/component/ComponentTest.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/simon/da/lockfree/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o"
	cd /home/simon/da/lockfree/src/tests/component && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/comptest.dir/ComponentTest.o -c /home/simon/da/lockfree/src/tests/component/ComponentTest.cpp

src/tests/component/CMakeFiles/comptest.dir/ComponentTest.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/comptest.dir/ComponentTest.i"
	cd /home/simon/da/lockfree/src/tests/component && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/simon/da/lockfree/src/tests/component/ComponentTest.cpp > CMakeFiles/comptest.dir/ComponentTest.i

src/tests/component/CMakeFiles/comptest.dir/ComponentTest.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/comptest.dir/ComponentTest.s"
	cd /home/simon/da/lockfree/src/tests/component && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/simon/da/lockfree/src/tests/component/ComponentTest.cpp -o CMakeFiles/comptest.dir/ComponentTest.s

src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o.requires:
.PHONY : src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o.requires

src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o.provides: src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o.requires
	$(MAKE) -f src/tests/component/CMakeFiles/comptest.dir/build.make src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o.provides.build
.PHONY : src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o.provides

src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o.provides.build: src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o
.PHONY : src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o.provides.build

src/tests/component/CMakeFiles/comptest.dir/QueueTest.o: src/tests/component/CMakeFiles/comptest.dir/flags.make
src/tests/component/CMakeFiles/comptest.dir/QueueTest.o: src/tests/component/QueueTest.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/simon/da/lockfree/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/tests/component/CMakeFiles/comptest.dir/QueueTest.o"
	cd /home/simon/da/lockfree/src/tests/component && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/comptest.dir/QueueTest.o -c /home/simon/da/lockfree/src/tests/component/QueueTest.cpp

src/tests/component/CMakeFiles/comptest.dir/QueueTest.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/comptest.dir/QueueTest.i"
	cd /home/simon/da/lockfree/src/tests/component && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/simon/da/lockfree/src/tests/component/QueueTest.cpp > CMakeFiles/comptest.dir/QueueTest.i

src/tests/component/CMakeFiles/comptest.dir/QueueTest.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/comptest.dir/QueueTest.s"
	cd /home/simon/da/lockfree/src/tests/component && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/simon/da/lockfree/src/tests/component/QueueTest.cpp -o CMakeFiles/comptest.dir/QueueTest.s

src/tests/component/CMakeFiles/comptest.dir/QueueTest.o.requires:
.PHONY : src/tests/component/CMakeFiles/comptest.dir/QueueTest.o.requires

src/tests/component/CMakeFiles/comptest.dir/QueueTest.o.provides: src/tests/component/CMakeFiles/comptest.dir/QueueTest.o.requires
	$(MAKE) -f src/tests/component/CMakeFiles/comptest.dir/build.make src/tests/component/CMakeFiles/comptest.dir/QueueTest.o.provides.build
.PHONY : src/tests/component/CMakeFiles/comptest.dir/QueueTest.o.provides

src/tests/component/CMakeFiles/comptest.dir/QueueTest.o.provides.build: src/tests/component/CMakeFiles/comptest.dir/QueueTest.o
.PHONY : src/tests/component/CMakeFiles/comptest.dir/QueueTest.o.provides.build

# Object files for target comptest
comptest_OBJECTS = \
"CMakeFiles/comptest.dir/ComponentTest.o" \
"CMakeFiles/comptest.dir/QueueTest.o"

# External object files for target comptest
comptest_EXTERNAL_OBJECTS =

src/tests/component/comptest: src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o
src/tests/component/comptest: src/tests/component/CMakeFiles/comptest.dir/QueueTest.o
src/tests/component/comptest: src/modules/libmodules.a
src/tests/component/comptest: src/common/ipfixlolib/libipfixlolib.a
src/tests/component/comptest: src/core/libcore.a
src/tests/component/comptest: src/common/libcommon.a
src/tests/component/comptest: src/osdep/linux/libosdep.a
src/tests/component/comptest: /usr/lib/libboost_regex-mt.so
src/tests/component/comptest: /usr/lib/libboost_filesystem-mt.so
src/tests/component/comptest: /usr/lib/libpcap.so
src/tests/component/comptest: /usr/lib/libxml2.so
src/tests/component/comptest: src/tests/component/CMakeFiles/comptest.dir/build.make
src/tests/component/comptest: src/tests/component/CMakeFiles/comptest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable comptest"
	cd /home/simon/da/lockfree/src/tests/component && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/comptest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/tests/component/CMakeFiles/comptest.dir/build: src/tests/component/comptest
.PHONY : src/tests/component/CMakeFiles/comptest.dir/build

src/tests/component/CMakeFiles/comptest.dir/requires: src/tests/component/CMakeFiles/comptest.dir/ComponentTest.o.requires
src/tests/component/CMakeFiles/comptest.dir/requires: src/tests/component/CMakeFiles/comptest.dir/QueueTest.o.requires
.PHONY : src/tests/component/CMakeFiles/comptest.dir/requires

src/tests/component/CMakeFiles/comptest.dir/clean:
	cd /home/simon/da/lockfree/src/tests/component && $(CMAKE_COMMAND) -P CMakeFiles/comptest.dir/cmake_clean.cmake
.PHONY : src/tests/component/CMakeFiles/comptest.dir/clean

src/tests/component/CMakeFiles/comptest.dir/depend:
	cd /home/simon/da/lockfree && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/simon/da/lockfree /home/simon/da/lockfree/src/tests/component /home/simon/da/lockfree /home/simon/da/lockfree/src/tests/component /home/simon/da/lockfree/src/tests/component/CMakeFiles/comptest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/tests/component/CMakeFiles/comptest.dir/depend

