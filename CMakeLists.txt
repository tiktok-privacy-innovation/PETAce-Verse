# Copyright 2023 TikTok Pte. Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.14)

####################################################
# Project VERSE includes the following components: #
#   1. VERSE C++ library                           #
#   2. VERSE C++ test                              #
#   3. VERSE C++ example                           #
####################################################

# [OPTION] CMAKE_BUILD_TYPE (DEFAULT: "Release")
# Select from Release, Debug, MiniSizeRel, or RelWithDebInfo.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
        STRINGS "Release" "Debug" "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "Build type (CMAKE_BUILD_TYPE): ${CMAKE_BUILD_TYPE}")

project(VERSE VERSION 0.2.0 LANGUAGES CXX C)

########################
# Global configuration #
########################

# CMake modules
include(CMakeDependentOption)
include(CMakePushCheckState)
include(CheckIncludeFiles)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)

# Custom modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(VerseCustomMacros)

# In Debug mode, define VERSE_DEBUG.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(VERSE_DEBUG ON)
else()
    set(VERSE_DEBUG OFF)
endif()
message(STATUS "VERSE debug mode: ${VERSE_DEBUG}")

# In Debug mode, enable extra compiler flags.
include(EnableDebugFlags)

# Always build position-independent-code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# [OPTION] VERSE_USE_CXX17 (default: ON)
# Use C++17, use C++14 otherwise.
set(VERSE_USE_CXX17_OPTION_STR "Use C++17")
option(VERSE_USE_CXX17 ${VERSE_USE_CXX17_OPTION_STR} OFF)
message(STATUS "VERSE_USE_CXX17: ${VERSE_USE_CXX17}")
# Enable features from C++17 if available, disable features if set to OFF.
include(EnableCXX17)

# Add default files and directories.
include(GNUInstallDirs)

# Runtime path
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Source Tree
set(VERSE_INCLUDES_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(VERSE_CONFIG_IN_FILENAME ${CMAKE_CURRENT_LIST_DIR}/cmake/PETAce-VerseConfig.cmake.in)
set(VERSE_CONFIG_H_IN_FILENAME ${VERSE_INCLUDES_DIR}/verse/util/config.h.in)

# Build tree
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(VERSE_CONFIG_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/PETAce-VerseConfig.cmake)
set(VERSE_TARGETS_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/PETAce-VerseTargets.cmake)
set(VERSE_CONFIG_VERSION_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/PETAce-VerseConfigVersion.cmake)
set(VERSE_CONFIG_H_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/src/verse/util/config.h)
set(VERSE_THIRDPARTY_DIR ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)

# Installation tree
set(VERSE_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/PETAce-Verse-${VERSE_VERSION_MAJOR}.${VERSE_VERSION_MINOR})
set(VERSE_INCLUDES_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/PETAce-${VERSE_VERSION_MAJOR}.${VERSE_VERSION_MINOR})
set(VERSE_THIRDPARTY_INCLUDES_INSTALL_DIR ${VERSE_INCLUDES_INSTALL_DIR}/thirdparty)

# Make the install target depend on the all target.
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY OFF)

# Supported target operating systems are Linux and macOS.
if (NOT DEFINED LINUX)
    if (UNIX AND NOT APPLE AND NOT CYGWIN AND NOT MINGW)
        set(LINUX ON)
    endif()
endif()
if (UNIX AND APPLE)
    set(MACOS ON)
endif()
if (NOT LINUX AND NOT MACOS)
    message(FATAL_ERROR "Supported target operating systems are Linux and macOS")
endif()

# Only support x86_64 and arm64
set(CMAKE_REQUIRED_QUIET_OLD ${CMAKE_REQUIRED_QUIET})
set(CMAKE_REQUIRED_QUIET ON)
check_cxx_source_runs("
    #if defined(__aarch64__)
        int main() {
            return 0;
        }
    #else
        #error
    #endif
    "
    VERSE_ARM64
)
check_cxx_source_runs("
    #if defined(__amd64)
        int main() {
            return 0;
        }
    #else
        #error
    #endif
    "
    VERSE_AMD64
)
set(CMAKE_REQUIRED_QUIET ${CMAKE_REQUIRED_QUIET_OLD})
if (NOT VERSE_AMD64 AND NOT VERSE_ARM64)
    message(FATAL_ERROR "Supported target architectures are x86_64 and arm64")
endif()

add_compile_options(-msse4.2 -Wno-ignored-attributes)

set(VERSE_ENABLE_GCOV_STR "Enable gcov")
option(VERSE_ENABLE_GCOV ${VERSE_ENABLE_GCOV_STR} OFF)
message(STATUS "VERSE_ENABLE_GCOV: ${VERSE_ENABLE_GCOV_STR}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND VERSE_ENABLE_GCOV)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -fprofile-arcs -ftest-coverage -lgcov")
endif()

# CMAKE_CXX_LINK_EXECUTABLE
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl -lrt")

#########################
# External dependencies #
#########################

# [OPTION] VERSE_BUILD_DEPS (DEFAULT: ON)
# Download and build dependencies if set to ON.
# Look for dependencies using find_package, otherwise.
set(VERSE_BUILD_DEPS_OPTION_STR "Automatically download and build unmet dependencies")
option(VERSE_BUILD_DEPS ${VERSE_BUILD_DEPS_OPTION_STR} ON)
message(STATUS "VERSE_BUILD_DEPS: ${VERSE_BUILD_DEPS}")

if(VERSE_BUILD_DEPS)
    include(FetchContent)
    mark_as_advanced(FETCHCONTENT_BASE_DIR)
    mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
    mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED)
    mark_as_advanced(FETCHCONTENT_QUIET)
endif()

# [OPTION] VERSE_BUILD_SHARED_LIBS (DEFAULT: OFF)
# Build a shared library if set to ON.
set(VERSE_BUILD_SHARED_LIBS_STR "Build shared library")
option(VERSE_BUILD_SHARED_LIBS ${VERSE_BUILD_SHARED_LIBS_STR} OFF)
message(STATUS "VERSE_BUILD_SHARED_LIBS: ${VERSE_BUILD_SHARED_LIBS}")

# Require Threads::Threads
if(NOT TARGET Threads::Threads)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    if(NOT Threads_FOUND)
        message(FATAL_ERROR "Threads: not found, please download and install manually")
    else()
        message(STATUS "Threads: found")
    endif()
endif()

# PETAce-Network::network
find_package(PETAce-Network 0.2 QUIET CONFIG)
if(PETAce-Network_FOUND)
    message(STATUS "PETAce-Network: found")
    if(PETAce-Network_STATIC_FOUND)
        set(network "PETAce-Network::network")
    else()
        set(network "PETAce-Network::network_shared")
    endif()
else()
    if(VERSE_BUILD_DEPS)
        message(STATUS "PETAce-Network: download ...")
        verse_fetch_thirdparty_content(PETAce-Network)
        if(TARGET network)
            set(network "network")
        else()
            set(network "network_shared")
        endif()
        set(VERSE_BUILD_NETWORK TRUE CACHE BOOL "" FORCE)
    else()
        message(FATAL_ERROR "PETAce-Network:: not found, please download and install manually")
    endif()
endif()

# PETAce-Solo::solo
find_package(PETAce-Solo 0.2 QUIET CONFIG)
if(PETAce-Solo_FOUND)
    message(STATUS "PETAce-Solo: found")
    if(PETAce-Solo_STATIC_FOUND)
        set(solo "PETAce-Solo::solo")
    else()
        set(solo "PETAce-Solo::solo_shared")
    endif()
else()
    if(VERSE_BUILD_DEPS)
        message(STATUS "PETAce-Solo: download ...")
        verse_fetch_thirdparty_content(PETAce-Solo)
        if(TARGET solo)
            set(solo "solo")
        else()
            set(solo "solo_shared")
        endif()
        set(VERSE_BUILD_SOLO TRUE CACHE BOOL "" FORCE)
    else()
        message(FATAL_ERROR "PETAce-Solo: not found, please download and install manually")
    endif()
endif()

#####################
# VERSE C++ library #
#####################

# Add source files to library and header files to install
set(VERSE_SOURCE_FILES "")
add_subdirectory(src/verse)

# Create the config file
configure_file(${VERSE_CONFIG_H_IN_FILENAME} ${VERSE_CONFIG_H_FILENAME})
install(
    FILES ${VERSE_CONFIG_H_FILENAME}
    DESTINATION ${VERSE_INCLUDES_INSTALL_DIR}/verse/util)

# Build only a static library
if(NOT VERSE_BUILD_SHARED_LIBS)
    add_library(verse STATIC ${VERSE_SOURCE_FILES})
    if(VERSE_USE_CXX17)
        target_compile_features(verse PUBLIC cxx_std_17)
    else()
        target_compile_features(verse PUBLIC cxx_std_14)
    endif()
    target_include_directories(verse PUBLIC
        $<BUILD_INTERFACE:${VERSE_INCLUDES_DIR}>
        $<INSTALL_INTERFACE:${VERSE_INCLUDES_INSTALL_DIR}>)
    target_include_directories(verse PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src/>)
    set_target_properties(verse PROPERTIES VERSION ${VERSE_VERSION})
    set_target_properties(verse PROPERTIES OUTPUT_NAME petace_verse-${VERSE_VERSION_MAJOR}.${VERSE_VERSION_MINOR})

    if(VERSE_BUILD_NETWORK)
        add_dependencies(verse ${network})
        target_include_directories(verse PUBLIC
            $<BUILD_INTERFACE:$<TARGET_PROPERTY:${network},INTERFACE_INCLUDE_DIRECTORIES>>
            $<INSTALL_INTERFACE:${VERSE_INCLUDES_INSTALL_DIR}>)
        verse_combine_archives(verse ${network})
        set(VERSE_CARRY_NETWORK TRUE)
    else()
        target_link_libraries(verse PUBLIC ${network})
        set(VERSE_CARRY_NETWORK FALSE)
    endif()

    if(VERSE_BUILD_SOLO)
        add_dependencies(verse ${solo})
        target_include_directories(verse PUBLIC
            $<BUILD_INTERFACE:$<TARGET_PROPERTY:${solo},INTERFACE_INCLUDE_DIRECTORIES>>
            $<INSTALL_INTERFACE:${VERSE_INCLUDES_INSTALL_DIR}>)
    endif()
    target_link_libraries(verse PUBLIC ${solo})
    set(VERSE_CARRY_SOLO FALSE)

    target_link_libraries(verse PUBLIC Threads::Threads)

    install(TARGETS verse EXPORT PETAce-VerseTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Build only a shared library
else()
    add_library(verse_shared SHARED ${VERSE_SOURCE_FILES})
    if(VERSE_USE_CXX17)
        target_compile_features(verse_shared PUBLIC cxx_std_17)
    else()
        target_compile_features(verse_shared PUBLIC cxx_std_14)
    endif()
    target_include_directories(verse_shared PUBLIC
        $<BUILD_INTERFACE:${VERSE_INCLUDES_DIR}>
        $<INSTALL_INTERFACE:${VERSE_INCLUDES_INSTALL_DIR}>)
    target_include_directories(verse_shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src/>)
    set_target_properties(verse_shared PROPERTIES VERSION ${VERSE_VERSION})
    set_target_properties(verse_shared PROPERTIES SOVERSION ${VERSE_VERSION_MAJOR}.${VERSE_VERSION_MINOR})
    set_target_properties(verse_shared PROPERTIES OUTPUT_NAME petace_verse)

    if(VERSE_BUILD_NETWORK)
        target_include_directories(verse_shared PUBLIC
            $<BUILD_INTERFACE:$<TARGET_PROPERTY:${network},INTERFACE_INCLUDE_DIRECTORIES>>
            $<INSTALL_INTERFACE:${VERSE_INCLUDES_INSTALL_DIR}>)
    endif()
    target_link_libraries(verse_shared PUBLIC ${network})
    set(VERSE_CARRY_NETWORK FALSE)

    if(VERSE_BUILD_SOLO)
        target_include_directories(verse_shared PUBLIC
            $<BUILD_INTERFACE:$<TARGET_PROPERTY:${solo},INTERFACE_INCLUDE_DIRECTORIES>>
            $<INSTALL_INTERFACE:${VERSE_INCLUDES_INSTALL_DIR}>)
    endif()
    target_link_libraries(verse_shared PUBLIC ${solo})
    set(VERSE_CARRY_SOLO FALSE)

    target_link_libraries(verse_shared PUBLIC Threads::Threads)

    install(TARGETS verse_shared EXPORT PETAce-VerseTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# Add standard alias targets for PETAce-Verse::verse and PETAce-Verse::verse_shared
if(TARGET verse)
    add_library(PETAce-Verse::verse ALIAS verse)
endif()
if(TARGET verse_shared)
    add_library(PETAce-Verse::verse_shared ALIAS verse_shared)
endif()

#################################
# Installation and CMake config #
#################################

# Create the CMake config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${VERSE_CONFIG_IN_FILENAME} ${VERSE_CONFIG_FILENAME}
    INSTALL_DESTINATION ${VERSE_CONFIG_INSTALL_DIR})

# Install the export
install(
    EXPORT PETAce-VerseTargets
    NAMESPACE PETAce-Verse::
    DESTINATION ${VERSE_CONFIG_INSTALL_DIR})

# Version file; we require exact version match for downstream
write_basic_package_version_file(
    ${VERSE_CONFIG_VERSION_FILENAME}
    VERSION ${VERSE_VERSION}
    COMPATIBILITY SameMinorVersion)

# Install config and module files
install(
    FILES
        ${VERSE_CONFIG_FILENAME}
        ${VERSE_CONFIG_VERSION_FILENAME}
    DESTINATION ${VERSE_CONFIG_INSTALL_DIR})

# We export PETAce-VerseTargets from the build tree so it can be used by other projects
# without requiring an install.
export(
    EXPORT PETAce-VerseTargets
    NAMESPACE PETAce-Verse::
    FILE ${VERSE_TARGETS_FILENAME})

# Install header files of dependencies if VERSE_BUILD_DEPS is ON
if(VERSE_BUILD_DEPS)
    # Insert dependencies here
    if(VERSE_BUILD_SOLO)
        install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build ${solo_BINARY_DIR} -t install)")
    endif()
    if(VERSE_BUILD_NETWORK)
        install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build ${network_BINARY_DIR} -t install)")
    endif()
endif()

#####################
# VERSE C++ example #
#####################

# [option] VERSE_BUILD_EXAMPLE
set(VERSE_BUILD_EXAMPLE_OPTION_STR "Build C++ example for VERSE")
option(VERSE_BUILD_EXAMPLE ${VERSE_BUILD_EXAMPLE_OPTION_STR} ON)
message(STATUS "VERSE_BUILD_EXAMPLE: ${VERSE_BUILD_EXAMPLE}")

if(VERSE_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

##################
# VERSE C++ test #
##################

# [option] VERSE_BUILD_TEST
set(VERSE_BUILD_TEST_OPTION_STR "Build C++ test for VERSE")
option(VERSE_BUILD_TEST ${VERSE_BUILD_TEST_OPTION_STR} ON)
message(STATUS "VERSE_BUILD_TEST: ${VERSE_BUILD_TEST}")

if(VERSE_BUILD_TEST)
    add_subdirectory(test)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND VERSE_ENABLE_GCOV)
        add_custom_target(test_coverage
            COMMAND gcovr -r ${CMAKE_CURRENT_LIST_DIR} -f \"src\" -e \".+\(test\\.cpp\)\" --xml-pretty -o "${CMAKE_CURRENT_BINARY_DIR}/report/coverage.xml"
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    endif()
endif()
