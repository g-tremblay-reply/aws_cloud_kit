cmake_minimum_required(VERSION 3.9)

project(aws_ck_ra6m5_ethernet_app)
enable_language(C CXX ASM)


# export the executable target through a variable to CMakeLists.txt files in subdirectories.
# update the dependent sources.
add_executable(${PROJECT_NAME}
    ""
)

if (LINKER_SCRIPT)
	# if linker script is defined, make the target to depends on it,
	# so the target will be rebuilt when linker script was changed.
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
endif()


# export the name of executable target via a variable to CMakeLists.txt files in subdirectories.
set(CURRENT_EXE_NAME ${PROJECT_NAME})

# Add sources and include directories from generated ra stack and src directory
include(ra_stack.cmake)
add_subdirectory(src)


# if compiler is GNU gcc/g++, then generate *.bin & *.hex files.
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    # generate the hex file from the built target.
    set(HEX_FILE ${PROJECT_NAME}.hex)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${HEX_FILE}
        COMMENT "Building ${HEX_FILE}...")

    # generate the bin file from the built target.
    set(BIN_FILE ${PROJECT_NAME}.bin)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${BIN_FILE}
        COMMENT "Building ${BIN_FILE}...")
endif()