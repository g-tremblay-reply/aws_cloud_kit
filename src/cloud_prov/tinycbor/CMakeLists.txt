# add current directory to the compiler included directories when compiling the given target.
target_include_directories(${CURRENT_EXE_NAME} PUBLIC  "${CMAKE_CURRENT_LIST_DIR}")

# load and run the CMake code from subdirectories for current target.
include(${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt)
