# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/red/clion/bin/cmake/linux/x64/bin/cmake

# The command to remove a file.
RM = /home/red/clion/bin/cmake/linux/x64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/red/open/github/simple_server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/red/open/github/simple_server/build

# Include any dependencies generated for this target.
include CMakeFiles/simple_server.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/simple_server.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/simple_server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/simple_server.dir/flags.make

CMakeFiles/simple_server.dir/src/main.cpp.o: CMakeFiles/simple_server.dir/flags.make
CMakeFiles/simple_server.dir/src/main.cpp.o: /home/red/open/github/simple_server/src/main.cpp
CMakeFiles/simple_server.dir/src/main.cpp.o: CMakeFiles/simple_server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/red/open/github/simple_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/simple_server.dir/src/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/simple_server.dir/src/main.cpp.o -MF CMakeFiles/simple_server.dir/src/main.cpp.o.d -o CMakeFiles/simple_server.dir/src/main.cpp.o -c /home/red/open/github/simple_server/src/main.cpp

CMakeFiles/simple_server.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simple_server.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/red/open/github/simple_server/src/main.cpp > CMakeFiles/simple_server.dir/src/main.cpp.i

CMakeFiles/simple_server.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simple_server.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/red/open/github/simple_server/src/main.cpp -o CMakeFiles/simple_server.dir/src/main.cpp.s

CMakeFiles/simple_server.dir/src/make_tools.cpp.o: CMakeFiles/simple_server.dir/flags.make
CMakeFiles/simple_server.dir/src/make_tools.cpp.o: /home/red/open/github/simple_server/src/make_tools.cpp
CMakeFiles/simple_server.dir/src/make_tools.cpp.o: CMakeFiles/simple_server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/red/open/github/simple_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/simple_server.dir/src/make_tools.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/simple_server.dir/src/make_tools.cpp.o -MF CMakeFiles/simple_server.dir/src/make_tools.cpp.o.d -o CMakeFiles/simple_server.dir/src/make_tools.cpp.o -c /home/red/open/github/simple_server/src/make_tools.cpp

CMakeFiles/simple_server.dir/src/make_tools.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simple_server.dir/src/make_tools.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/red/open/github/simple_server/src/make_tools.cpp > CMakeFiles/simple_server.dir/src/make_tools.cpp.i

CMakeFiles/simple_server.dir/src/make_tools.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simple_server.dir/src/make_tools.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/red/open/github/simple_server/src/make_tools.cpp -o CMakeFiles/simple_server.dir/src/make_tools.cpp.s

CMakeFiles/simple_server.dir/src/server_task.cpp.o: CMakeFiles/simple_server.dir/flags.make
CMakeFiles/simple_server.dir/src/server_task.cpp.o: /home/red/open/github/simple_server/src/server_task.cpp
CMakeFiles/simple_server.dir/src/server_task.cpp.o: CMakeFiles/simple_server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/red/open/github/simple_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/simple_server.dir/src/server_task.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/simple_server.dir/src/server_task.cpp.o -MF CMakeFiles/simple_server.dir/src/server_task.cpp.o.d -o CMakeFiles/simple_server.dir/src/server_task.cpp.o -c /home/red/open/github/simple_server/src/server_task.cpp

CMakeFiles/simple_server.dir/src/server_task.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simple_server.dir/src/server_task.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/red/open/github/simple_server/src/server_task.cpp > CMakeFiles/simple_server.dir/src/server_task.cpp.i

CMakeFiles/simple_server.dir/src/server_task.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simple_server.dir/src/server_task.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/red/open/github/simple_server/src/server_task.cpp -o CMakeFiles/simple_server.dir/src/server_task.cpp.s

CMakeFiles/simple_server.dir/src/sqlite3.c.o: CMakeFiles/simple_server.dir/flags.make
CMakeFiles/simple_server.dir/src/sqlite3.c.o: /home/red/open/github/simple_server/src/sqlite3.c
CMakeFiles/simple_server.dir/src/sqlite3.c.o: CMakeFiles/simple_server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/red/open/github/simple_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/simple_server.dir/src/sqlite3.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/simple_server.dir/src/sqlite3.c.o -MF CMakeFiles/simple_server.dir/src/sqlite3.c.o.d -o CMakeFiles/simple_server.dir/src/sqlite3.c.o -c /home/red/open/github/simple_server/src/sqlite3.c

CMakeFiles/simple_server.dir/src/sqlite3.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/simple_server.dir/src/sqlite3.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/red/open/github/simple_server/src/sqlite3.c > CMakeFiles/simple_server.dir/src/sqlite3.c.i

CMakeFiles/simple_server.dir/src/sqlite3.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/simple_server.dir/src/sqlite3.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/red/open/github/simple_server/src/sqlite3.c -o CMakeFiles/simple_server.dir/src/sqlite3.c.s

CMakeFiles/simple_server.dir/src/sqlite_op.cpp.o: CMakeFiles/simple_server.dir/flags.make
CMakeFiles/simple_server.dir/src/sqlite_op.cpp.o: /home/red/open/github/simple_server/src/sqlite_op.cpp
CMakeFiles/simple_server.dir/src/sqlite_op.cpp.o: CMakeFiles/simple_server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/red/open/github/simple_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/simple_server.dir/src/sqlite_op.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/simple_server.dir/src/sqlite_op.cpp.o -MF CMakeFiles/simple_server.dir/src/sqlite_op.cpp.o.d -o CMakeFiles/simple_server.dir/src/sqlite_op.cpp.o -c /home/red/open/github/simple_server/src/sqlite_op.cpp

CMakeFiles/simple_server.dir/src/sqlite_op.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simple_server.dir/src/sqlite_op.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/red/open/github/simple_server/src/sqlite_op.cpp > CMakeFiles/simple_server.dir/src/sqlite_op.cpp.i

CMakeFiles/simple_server.dir/src/sqlite_op.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simple_server.dir/src/sqlite_op.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/red/open/github/simple_server/src/sqlite_op.cpp -o CMakeFiles/simple_server.dir/src/sqlite_op.cpp.s

# Object files for target simple_server
simple_server_OBJECTS = \
"CMakeFiles/simple_server.dir/src/main.cpp.o" \
"CMakeFiles/simple_server.dir/src/make_tools.cpp.o" \
"CMakeFiles/simple_server.dir/src/server_task.cpp.o" \
"CMakeFiles/simple_server.dir/src/sqlite3.c.o" \
"CMakeFiles/simple_server.dir/src/sqlite_op.cpp.o"

# External object files for target simple_server
simple_server_EXTERNAL_OBJECTS =

simple_server: CMakeFiles/simple_server.dir/src/main.cpp.o
simple_server: CMakeFiles/simple_server.dir/src/make_tools.cpp.o
simple_server: CMakeFiles/simple_server.dir/src/server_task.cpp.o
simple_server: CMakeFiles/simple_server.dir/src/sqlite3.c.o
simple_server: CMakeFiles/simple_server.dir/src/sqlite_op.cpp.o
simple_server: CMakeFiles/simple_server.dir/build.make
simple_server: /home/red/open/github/simple_server/lib/libhv.so
simple_server: CMakeFiles/simple_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/red/open/github/simple_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable simple_server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simple_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/simple_server.dir/build: simple_server
.PHONY : CMakeFiles/simple_server.dir/build

CMakeFiles/simple_server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/simple_server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/simple_server.dir/clean

CMakeFiles/simple_server.dir/depend:
	cd /home/red/open/github/simple_server/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/red/open/github/simple_server /home/red/open/github/simple_server /home/red/open/github/simple_server/build /home/red/open/github/simple_server/build /home/red/open/github/simple_server/build/CMakeFiles/simple_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/simple_server.dir/depend
