# add source and header files when compiling the given target.
# although it is not necessary to add header files, we also do that for easing the management of IDE.
target_sources(${CURRENT_EXE_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/console.c
        ${CMAKE_CURRENT_LIST_DIR}/console.h
        ${CMAKE_CURRENT_LIST_DIR}/console_flash.c
        ${CMAKE_CURRENT_LIST_DIR}/console_flash.h
)

# add current directory to the compiler included directories when compiling the given target.
target_include_directories(${CURRENT_EXE_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

# load and run the CMake code from subdirectories for current target.
include(${CMAKE_CURRENT_LIST_DIR}/SEGGER_RTT/CMakeLists.txt)
