# add source and header files when compiling the given target.
# although it is not necessary to add header files, we also do that for easing the management of IDE.
target_sources(${CURRENT_EXE_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/hal_entry.c
        ${CMAKE_CURRENT_LIST_DIR}/r_typedefs.h
        ${CMAKE_CURRENT_LIST_DIR}/usr_hal.c
        ${CMAKE_CURRENT_LIST_DIR}/usr_hal.h
        ${CMAKE_CURRENT_LIST_DIR}/console_thread_entry.c
        ${CMAKE_CURRENT_LIST_DIR}/sensor_thread_entry.c
        ${CMAKE_CURRENT_LIST_DIR}/oximeter_thread_entry.c
        ${CMAKE_CURRENT_LIST_DIR}/zmod_thread_entry.c
        ${CMAKE_CURRENT_LIST_DIR}/cloud_app_thread_entry.c
)

# add current directory to the compiler included directories when compiling the given target.
target_include_directories(${CURRENT_EXE_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

# load and run the CMake code from subdirectories for current target.
include(${CMAKE_CURRENT_LIST_DIR}/console_menu/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/sensor/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/cloud_app/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/cloud_prov/CMakeLists.txt)
