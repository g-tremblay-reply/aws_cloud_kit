# add source and header files when compiling the given target.
# although it is not necessary to add header files, we also do that for easing the management of IDE.
target_sources(${CURRENT_EXE_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/cbor.h
        ${CMAKE_CURRENT_LIST_DIR}/cborencoder.c
        ${CMAKE_CURRENT_LIST_DIR}/cborencoder_close_container_checked.c
        ${CMAKE_CURRENT_LIST_DIR}/cborencoder_float.c
        ${CMAKE_CURRENT_LIST_DIR}/cborerrorstrings.c
        ${CMAKE_CURRENT_LIST_DIR}/cborinternal_p.h
        ${CMAKE_CURRENT_LIST_DIR}/cborjson.h
        ${CMAKE_CURRENT_LIST_DIR}/cborparser.c
        ${CMAKE_CURRENT_LIST_DIR}/cborparser_dup_string.c
        ${CMAKE_CURRENT_LIST_DIR}/cborparser_float.c
        ${CMAKE_CURRENT_LIST_DIR}/cborpretty.c
        ${CMAKE_CURRENT_LIST_DIR}/cborpretty_stdio.c
        ${CMAKE_CURRENT_LIST_DIR}/cbortojson.c
        ${CMAKE_CURRENT_LIST_DIR}/cborvalidation.c
        ${CMAKE_CURRENT_LIST_DIR}/compilersupport_p.h
        ${CMAKE_CURRENT_LIST_DIR}/open_memstream.c
        ${CMAKE_CURRENT_LIST_DIR}/tinycbor-version.h
        ${CMAKE_CURRENT_LIST_DIR}/utf8_p.h
)

# add current directory to the compiler included directories when compiling the given target.
target_include_directories(${CURRENT_EXE_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

