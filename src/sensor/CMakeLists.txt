# add source and header files when compiling the given target.
# although it is not necessary to add header files, we also do that for easing the management of IDE.
target_sources(${CURRENT_EXE_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/icm.h
        ${CMAKE_CURRENT_LIST_DIR}/ICM_20948.c
        ${CMAKE_CURRENT_LIST_DIR}/ICM_20948.h
        ${CMAKE_CURRENT_LIST_DIR}/icp.h
        ${CMAKE_CURRENT_LIST_DIR}/ICP_10101.c
        ${CMAKE_CURRENT_LIST_DIR}/ICP_10101.h
        ${CMAKE_CURRENT_LIST_DIR}/RA_HS3001.c
        ${CMAKE_CURRENT_LIST_DIR}/RA_HS3001.h
        ${CMAKE_CURRENT_LIST_DIR}/RA_ICM20948.c
        ${CMAKE_CURRENT_LIST_DIR}/RA_ICP10101.c
        ${CMAKE_CURRENT_LIST_DIR}/RmcI2C.c
        ${CMAKE_CURRENT_LIST_DIR}/RmcI2C.h
        ${CMAKE_CURRENT_LIST_DIR}/user_choice.c
        ${CMAKE_CURRENT_LIST_DIR}/user_choice.h
        ${CMAKE_CURRENT_LIST_DIR}/usr_data.h
)

# add current directory to the compiler included directories when compiling the given target.
target_include_directories(${CURRENT_EXE_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

# load and run the CMake code from subdirectories for current target.
include(${CMAKE_CURRENT_LIST_DIR}/oximeter/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/zmod/CMakeLists.txt)
