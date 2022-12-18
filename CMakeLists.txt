cmake_minimum_required(VERSION 3.21)
# insert project name here
project(sample_project)

# here declared global conan dependencies, or dependencies related only for this project
# if some dependencies are needed for libraries, that can be considered standalone,
# it is better to wrap that standalone library in conan, and request that library with conan

# also here configured global project parameters, like cmake module directory, global compile flags,
# actions to do with assets on build, e.t.c

# How to start downloading dependencies with conan
# 0. install python3 and conan using pip install conan
# 1. uncomment calls to functions conan_cmake_configure,
#       conan_cmake_autodetect, conan_cmake_install. Also
#       uncomment include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
# 2. add your conan dependencies in REQUIRES in call to
#       conan_cmake_configure, same way as in conanfile.txt.
# 3. after include conan_paths add corresponding find_packages
#       most often name of cmake package is same as name of conan
#       package, but sometimes it is different. only way to know
#       names for sure, is read conanfile.py of conan package
# 4. link dependencies to according libraries. to know how to
#       refer to downloaded libraries read conanfile of package
# as example, protobuf added as dependency and linked to executable


set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# maybe add visual studio specific check for debug build
# if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#     set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}"-fsanitize=address")
# endif()

set(ENV{CONAN_USER_HOME} ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE CONAN_RECIPES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/conan_modules/*/conanfile.py)
foreach(RECIPE ${CONAN_RECIPES})
    execute_process(COMMAND conan export ${RECIPE})
endforeach()

# download conan.cmake if it is not exists
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/cmake/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake"
                "${CMAKE_SOURCE_DIR}/cmake/conan.cmake"
                TLS_VERIFY ON)
endif()

# check for updates, by downloading master and comparing hash with current conan.cmake hash
file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake"
            "${CMAKE_SOURCE_DIR}/cmake/temp_conan_master.cmake")
execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files 
    "${CMAKE_SOURCE_DIR}/cmake/temp_conan_master.cmake"
    "${CMAKE_SOURCE_DIR}/cmake/conan.cmake"
    RESULT_VARIABLE COMPARE_RESULT)

if (COMPARE_RESULT EQUAL 1)
    message(WARNING "New version of conan.cmake available. Consider updating")
    message(WARNING "To update remove cmake/conan.cmake. It may be required to also update conan")
endif()

file(REMOVE "${CMAKE_SOURCE_DIR}/cmake/temp_conan_master.cmake")

include(conan)
include(utilities)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# How to start downloading dependencies with conan
# 0. install python3 and conan using pip install conan
# 1. uncomment calls to functions conan_cmake_configure,
#       conan_cmake_autodetect, conan_cmake_install. Also
#       uncomment include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
# 2. add your conan dependencies in REQUIRES in call to
#       conan_cmake_configure, same way as in conanfile.txt.
# 3. after include conan_paths add corresponding find_packages
#       most often name of cmake package is same as name of conan
#       package, but sometimes it is different. only way to know
#       names for sure, is read conanfile.py of conan package
# 4. link dependencies to according libraries. to know how to
#       refer to downloaded libraries read conanfile of package
# as example, protobuf added as dependency and linked to executable

conan_cmake_configure(
    REQUIRES 
    protobuf/3.21.9
    GENERATORS
    cmake_find_package
    cmake_paths
)

conan_cmake_autodetect(settings)

conan_cmake_install(
    PATH_OR_REFERENCE ${CMAKE_BINARY_DIR}/.
    BUILD missing
    REMOTE conancenter
    SETTINGS ${settings}
)

# to have full c++20 with msvc, it is needed to specify c++-latest as standard.
# in order to specify c++-latest to cmake, it is needed to set CMAKE_CXX_STANDARD to 23
# info in issue
# https://github.com/microsoft/STL/issues/1814#issuecomment-845572895
# also hoping to not break abi, since cxx20 was passed to cmake_install
# TODO: fix different versions in cmake and conan, eather by switching to VS2022, or
# waiting for fix from conan side
if (MSVC)
    set(CMAKE_CXX_STANDARD 23)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

# find_packages of dependencies here
find_package(Protobuf)

# TODO: change this include_directories to include, only what linked
include_directories(src)

add_subdirectory(src)
