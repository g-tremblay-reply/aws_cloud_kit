# add current directory to the compiler included directories when compiling the given target.
target_include_directories(${CURRENT_EXE_NAME} PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}"
        "${CMAKE_CURRENT_LIST_DIR}/subscription_manager/"
)

# load and run the CMake code from subdirectories for current target.
# add source and header files when compiling the given target.
# although it is not necessary to add header files, we also do that for easing the management of IDE.
target_sources(${CURRENT_EXE_NAME}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/cloud_app.h
        ${CMAKE_CURRENT_LIST_DIR}/cloud_app.c
        ${CMAKE_CURRENT_LIST_DIR}/subscription_manager/mqtt_subscription_manager.c
        ${CMAKE_CURRENT_LIST_DIR}/subscription_manager/mqtt_subscription_manager.h
)
