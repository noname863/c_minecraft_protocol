cmake_minimum_required(VERSION 3.21)
# insert project name here
project(sample_project)

# here declared global conan dependencies, or dependencies related only for this project
# if some dependencies are needed for libraries, that can be considered standalone,
# it is better to wrap that standalone library in conan, and request that library with conan

# also here configured global project parameters, like cmake module directory, global compile flags,
# actions to do with assets on build, e.t.c

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# maybe add visual studio specific check for debug build
# if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#     set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}"-fsanitize=address")
# endif()

file(GLOB_RECURSE CONAN_RECIPES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/conan_modules/*/conanfile.py)
foreach(RECIPE ${CONAN_RECIPES})
    execute_process(COMMAND conan export ${RECIPE})
endforeach()

# to have full c++20 with msvc, it is needed to specify c++-latest as standard.
# in order to specify c++-latest to cmake, it is needed to set CMAKE_CXX_STANDARD to 23
# info in issue
# https://github.com/microsoft/STL/issues/1814#issuecomment-845572895
if (MSVC)
    set(CMAKE_CXX_STANDARD 23)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# download conan.cmake if it is not exists
# need to check for new versions sometimes
set(HARDCODED_CONAN_DOT_CMAKE_HASH 3BEF79DA16C2E031DC429E1DAC87A08B9226418B300CE004CC125A82687BAEEF)
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/cmake/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.17.0/conan.cmake"
                "${CMAKE_SOURCE_DIR}/cmake/conan.cmake"
                EXPECTED_HASH SHA256=${HARDCODED_CONAN_DOT_CMAKE_HASH}
                TLS_VERIFY ON)
endif()

# check for updates, by downloading master and comparing hash with current conan.cmake hash
file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
            "${CMAKE_SOURCE_DIR}/cmake/temp_conan_master.cmake")
execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files 
    "${CMAKE_SOURCE_DIR}/cmake/temp_conan_master.cmake"
    "${CMAKE_SOURCE_DIR}/cmake/conan.cmake"
    RESULT_VARIABLE COMPARE_RESULT)

if (compare_result EQUAL 1)
    message(WARNING "New version of conan.cmake available. Consider updating")
endif()

file(REMOVE "${CMAKE_SOURCE_DIR}/cmake/temp_conan_master.cmake")

include(conan)
include(utilities)

# conan_cmake_configure(
#     REQUIRES 
#     GENERATORS
#     cmake_find_package
#     cmake_paths
# )
# 
# conan_cmake_autodetect(settings)
# 
# conan_cmake_install(
#     PATH_OR_REFERENCE ${CMAKE_BINARY_DIR}/.
#     BUILD missing
#     REMOTE conancenter
#     SETTINGS ${settings}
# )

# include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

# find_packages of dependencies here

# TODO: change this include_directories to include, only what linked
include_directories(src)

add_subdirectory(src)
