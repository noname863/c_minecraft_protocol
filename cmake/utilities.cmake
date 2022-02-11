
# function signature:
#       add_target(name LIB|EXE|EXE_CONSOLE)

# there is no linking in interface for now
# TODO: add linking in interface, to
#     1. configure includes
#     2. provide unified ways to explicitly add another library to includes, but not link that library
#        it is needed to allow calls to virtual functions of interfaces, defined in another library.
#     3. possibly provide more fancy ways to link


function(add_target TARGET_NAME TARGET_TYPE)
    file(GLOB_RECURSE TARGET_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*")
    list(FIND TARGET_SOURCES "CMakeLists.txt" EXCLUDE_POS)
    if (NOT EXCLUDE_POS EQUAL -1)
        list(REMOVE_AT TARGET_SOURCES ${EXCLUDE_POS})
    endif()
    if (TARGET_TYPE STREQUAL "LIB")
        add_library(${TARGET_NAME} ${TARGET_SOURCES} STATIC)
    elseif (TARGET_TYPE STREQUAL "EXE")
        if (WIN32)
            add_executable(${TARGET_NAME} ${TARGET_SOURCES} WIN32)
        else()
            add_executable(${TARGET_NAME} ${TARGET_SOURCES})
        endif()
    elseif(TARGET_TYPE STREQUAL "EXE_CONSOLE")
        add_executable(${TARGET_NAME} ${TARGET_SOURCES})
    endif()
endfunction()
