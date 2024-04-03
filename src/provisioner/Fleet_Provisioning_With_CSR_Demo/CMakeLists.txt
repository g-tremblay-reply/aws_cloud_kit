# add source and header files when compiling the given target.
# although it is not necessary to add header files, we also do that for easing the management of IDE.
target_sources(${CURRENT_EXE_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/core_mqtt_config.h
        ${CMAKE_CURRENT_LIST_DIR}/FleetProvisioningDemoExample.c
        ${CMAKE_CURRENT_LIST_DIR}/fleet_provisioning_config.h
        ${CMAKE_CURRENT_LIST_DIR}/mbedtls_rng_pkcs11.c
        ${CMAKE_CURRENT_LIST_DIR}/pkcs11_operations.c
        ${CMAKE_CURRENT_LIST_DIR}/pkcs11_operations.h
        ${CMAKE_CURRENT_LIST_DIR}/tinycbor_serializer.c
        ${CMAKE_CURRENT_LIST_DIR}/tinycbor_serializer.h
)

# add current directory to the compiler included directories when compiling the given target.
target_include_directories(${CURRENT_EXE_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

# load and run the CMake code from subdirectories for current target.
include(${CMAKE_CURRENT_LIST_DIR}/DemoSetup/CMakeLists.txt)
