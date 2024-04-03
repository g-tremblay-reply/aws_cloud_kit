# add source and header files when compiling the given target.
# although it is not necessary to add header files, we also do that for easing the management of IDE.
target_sources(${CURRENT_EXE_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/ICM_20948.c
        ${CMAKE_CURRENT_LIST_DIR}/ICM_20948.h
        ${CMAKE_CURRENT_LIST_DIR}/sensor_icp10101.c
        ${CMAKE_CURRENT_LIST_DIR}/sensor_icp10101.h
        ${CMAKE_CURRENT_LIST_DIR}/sensor_hs3001.c
        ${CMAKE_CURRENT_LIST_DIR}/sensor_hs3001.h
        ${CMAKE_CURRENT_LIST_DIR}/usr_data.h
        ${CMAKE_CURRENT_LIST_DIR}/sensor_ob1203.c
        ${CMAKE_CURRENT_LIST_DIR}/sensor_ob1203.h
        ${CMAKE_CURRENT_LIST_DIR}/sensor_iaq.c
        ${CMAKE_CURRENT_LIST_DIR}/sensor_iaq.h
        ${CMAKE_CURRENT_LIST_DIR}/sensor_oaq.c
        ${CMAKE_CURRENT_LIST_DIR}/sensor_oaq.h
)

# add current directory to the compiler included directories when compiling the given target.
target_include_directories(${CURRENT_EXE_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

# load and run the CMake code from subdirectories for current target.
include(${CMAKE_CURRENT_LIST_DIR}/ob1203_bio/CMakeLists.txt)
