# add current directory to the compiler included directories when compiling the given target.
target_include_directories(${CURRENT_EXE_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

# load and run the CMake code from subdirectories for current target.
include(${CMAKE_CURRENT_LIST_DIR}/backoffAlgorithm/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/Fleet-Provisioning-for-AWS-IoT-embedded-sdk/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/Fleet_Provisioning_With_CSR_Demo/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/Mqtt_Demo_Helpers/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/tinycbor/CMakeLists.txt)
