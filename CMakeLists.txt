# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
# See NOTICE.md and THIRD_PARTY_NOTICES.md.
cmake_minimum_required(VERSION 3.20)

project(myra_cna
    VERSION 0.1.0
    DESCRIPTION "Faithful C++23/CNA port of Myra UI"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MYRA_CNA_BUILD_TESTS "Build Myra-CNA tests" ON)
option(MYRA_CNA_BUILD_EXAMPLES "Build Myra-CNA examples" ON)
option(MYRA_CNA_LINK_CNA
       "When no parent supplies CNA, add the sibling CNA project and link it" OFF)

set(MYRA_CNA_CNA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cna"
    CACHE PATH "Path to the sibling CNA repository")
set(MYRA_CNA_SHARP_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sharp-runtime"
    CACHE PATH "Path to the sibling sharp-runtime repository")
set(MYRA_CNA_CNA_GRAPHICS_BACKEND "SOFTWARE"
    CACHE STRING "CNA backend used only when MYRA_CNA_LINK_CNA=ON")
set_property(CACHE MYRA_CNA_CNA_GRAPHICS_BACKEND PROPERTY STRINGS
    SDL_RENDERER EASYGL BGFX VULKAN WEBGPU HEADLESS SOFTWARE CANVAS ASCII DX3 D3D9 D3D11 D3D12 SDL_GPU)

add_library(MYRA_CNA STATIC
    src/Myra/Events/MyraEventArgs.cpp
    src/Myra/Graphics2D/Thickness.cpp
    src/Myra/Graphics2D/UI/InputContext.cpp
    src/Myra/Graphics2D/UI/InputEventsManager.cpp
    src/Myra/MML/BaseObject.cpp
    src/Myra/MyraEnvironment.cpp
    src/Myra/Version.cpp
)
add_library(Myra::CNA ALIAS MYRA_CNA)

target_include_directories(MYRA_CNA
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_compile_features(MYRA_CNA PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(MYRA_CNA PRIVATE /W4 /WX)
else()
    target_compile_options(MYRA_CNA PRIVATE -Wall -Wextra -Werror)
endif()

# CNA is intentionally resolved in the same three modes as cna-extended:
# a target supplied by the parent, an explicit sibling build, or header-only
# bootstrap mode. The bootstrap library does not yet call CNA, so it remains
# useful for documentation and provenance tests without a graphics build.
set(MYRA_CNA_CNA_LINKED OFF)

if(TARGET CNA)
    target_link_libraries(MYRA_CNA PUBLIC CNA)
    target_compile_definitions(MYRA_CNA PUBLIC MYRA_CNA_HAS_CNA_TARGET=1)
    set(MYRA_CNA_CNA_LINKED ON)
    message(STATUS "myra-cna: linking the parent-provided CNA target")
elseif(MYRA_CNA_LINK_CNA)
    if(NOT EXISTS "${MYRA_CNA_CNA_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "myra-cna: MYRA_CNA_LINK_CNA=ON but CNA was not found at "
            "'${MYRA_CNA_CNA_DIR}'. Set MYRA_CNA_CNA_DIR to a CNA checkout.")
    endif()

    set(CNA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(CNA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CNA_GRAPHICS_BACKEND "${MYRA_CNA_CNA_GRAPHICS_BACKEND}" CACHE STRING "" FORCE)
    add_subdirectory("${MYRA_CNA_CNA_DIR}" "${CMAKE_BINARY_DIR}/_cna")
    target_link_libraries(MYRA_CNA PUBLIC CNA)
    target_compile_definitions(MYRA_CNA PUBLIC MYRA_CNA_HAS_CNA_TARGET=1)
    set(MYRA_CNA_CNA_LINKED ON)
    message(STATUS "myra-cna: linked sibling CNA (${MYRA_CNA_CNA_GRAPHICS_BACKEND})")
elseif(EXISTS "${MYRA_CNA_CNA_DIR}/include")
    target_include_directories(MYRA_CNA PUBLIC "${MYRA_CNA_CNA_DIR}/include")
    if(EXISTS "${MYRA_CNA_SHARP_RUNTIME_DIR}/include")
        target_include_directories(MYRA_CNA PUBLIC
            "${MYRA_CNA_SHARP_RUNTIME_DIR}/include")
        # When CNA is not a CMake target, sharp-runtime's transitive vendor
        # include directory is unavailable. Mirror its public target surface.
        if(EXISTS "${MYRA_CNA_SHARP_RUNTIME_DIR}/vendor")
            target_include_directories(MYRA_CNA SYSTEM PUBLIC
                "${MYRA_CNA_SHARP_RUNTIME_DIR}/vendor")
        endif()
    else()
        message(WARNING
            "myra-cna: CNA headers need sharp-runtime headers; set "
            "MYRA_CNA_SHARP_RUNTIME_DIR to a sharp-runtime checkout.")
    endif()
    target_compile_definitions(MYRA_CNA PUBLIC MYRA_CNA_HAS_CNA_HEADERS=1)
    message(STATUS "myra-cna: CNA/sharp-runtime headers available; linkage deferred to consumer")
else()
    message(STATUS "myra-cna: CNA not found; foundation-only bootstrap configuration")
endif()

if(MYRA_CNA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(MYRA_CNA_BUILD_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        if(NOT TARGET GTest::gtest_main)
            if(NOT EXISTS "${MYRA_CNA_CNA_DIR}/vendor/googletest/CMakeLists.txt")
                message(FATAL_ERROR
                    "myra-cna: tests require GoogleTest at "
                    "${MYRA_CNA_CNA_DIR}/vendor/googletest. Set MYRA_CNA_CNA_DIR "
                    "to a CNA checkout or provide GTest::gtest_main from the parent.")
            endif()
            set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            add_subdirectory("${MYRA_CNA_CNA_DIR}/vendor/googletest"
                             "${CMAKE_BINARY_DIR}/_myra_cna_googletest" EXCLUDE_FROM_ALL)
        endif()
        add_subdirectory(tests)
    endif()
endif()
