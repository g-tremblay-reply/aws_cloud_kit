# add source and header files when compiling the given target.
# although it is not necessary to add header files, we also do that for easing the management of IDE.
target_sources(${CURRENT_EXE_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/console.c
        ${CMAKE_CURRENT_LIST_DIR}/console.h
        ${CMAKE_CURRENT_LIST_DIR}/menu_flash.c
        ${CMAKE_CURRENT_LIST_DIR}/menu_flash.h
        ${CMAKE_CURRENT_LIST_DIR}/menu_kis.c
        ${CMAKE_CURRENT_LIST_DIR}/menu_kis.h
        ${CMAKE_CURRENT_LIST_DIR}/menu_main.c
        ${CMAKE_CURRENT_LIST_DIR}/menu_main.h
)

# add current directory to the compiler included directories when compiling the given target.
target_include_directories(${CURRENT_EXE_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

