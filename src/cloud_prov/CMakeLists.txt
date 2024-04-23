# add current directory to the compiler included directories when compiling the given target.
target_include_directories(${CURRENT_EXE_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

# load and run the CMake code from subdirectories for current target.
# add source and header files when compiling the given target.
# although it is not necessary to add header files, we also do that for easing the management of IDE.
target_sources(${CURRENT_EXE_NAME}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/cloud_prov.c
        ${CMAKE_CURRENT_LIST_DIR}/cloud_prov.h
        ${CMAKE_CURRENT_LIST_DIR}/cloud_prov_config.h
        ${CMAKE_CURRENT_LIST_DIR}/cloud_prov_serializer.h
        ${CMAKE_CURRENT_LIST_DIR}/cloud_prov_serializer.c
        ${CMAKE_CURRENT_LIST_DIR}/cloud_prov_pkcs11.h
        ${CMAKE_CURRENT_LIST_DIR}/cloud_prov_pkcs11.c
        ${CMAKE_CURRENT_LIST_DIR}/cloud_prov_network.c
)

include(${CMAKE_CURRENT_LIST_DIR}/tinycbor/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/fleet_provisioning/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/backoffAlgorithm/CMakeLists.txt)

