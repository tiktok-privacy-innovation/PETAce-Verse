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

project(VERSEExamples VERSION 0.2.0 LANGUAGES CXX)

# If not called from root CMakeLists.txt
if(NOT DEFINED VERSE_BUILD_EXAMPLE)
    set(VERSE_BUILD_EXAMPLE ON)

    # Import Microsoft VERSE
    find_package(PETAce-Verse 0.2.0 EXACT REQUIRED)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif()

if(VERSE_BUILD_EXAMPLE)
    # Add source files to example
    set(PETACE_VERSE_EXAMPLE_FILES "")
    # Test files in this directory
    set(PETACE_VERSE_EXAMPLE_FILES ${PETACE_VERSE_EXAMPLE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/example.cpp
        ${CMAKE_CURRENT_LIST_DIR}/iknp_ot_example.cpp
        ${CMAKE_CURRENT_LIST_DIR}/kkrt_ot_example.cpp
        ${CMAKE_CURRENT_LIST_DIR}/np_ot_example.cpp
    )

    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl -lrt")
    add_executable(verse_example ${PETACE_VERSE_EXAMPLE_FILES})

    if(TARGET PETAce-Verse::verse)
        target_link_libraries(verse_example PRIVATE PETAce-Verse::verse)
    elseif(TARGET PETAce-Verse::verse_shared)
        target_link_libraries(verse_example PRIVATE PETAce-Verse::verse_shared)
    else()
        message(FATAL_ERROR "Cannot find target PETAce::verse or PETAce::verse_shared")
    endif()
endif()
