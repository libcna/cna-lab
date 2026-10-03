# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
# See NOTICE.md and THIRD_PARTY_NOTICES.md.
cmake_minimum_required(VERSION 3.20)

project(myra_cna_parent_consumer LANGUAGES CXX)

foreach(_required_path IN ITEMS MYRA_CNA_SOURCE_DIR CNA_SOURCE_DIR SHARP_RUNTIME_SOURCE_DIR)
    if(NOT DEFINED ${_required_path} OR NOT IS_DIRECTORY "${${_required_path}}")
        message(FATAL_ERROR "${_required_path} must name an existing source directory")
    endif()
endforeach()

file(GLOB _cna_include_dirs LIST_DIRECTORIES TRUE
    "${CNA_SOURCE_DIR}/modules/*/include")
file(GLOB _sharp_runtime_include_dirs LIST_DIRECTORIES TRUE
    "${SHARP_RUNTIME_SOURCE_DIR}/modules/*/include")
list(SORT _cna_include_dirs)
list(SORT _sharp_runtime_include_dirs)

# This fixture verifies Myra-CNA's ordinary parent-target contract without
# recursively configuring the modular CNA project, which must remain top-level.
# The repository's main linked matrix separately supplies the real libraries.
add_library(sharp_runtime_xml_contract INTERFACE)
add_library(SharpRuntime::Xml ALIAS sharp_runtime_xml_contract)
target_include_directories(sharp_runtime_xml_contract INTERFACE
    ${_sharp_runtime_include_dirs})
if(IS_DIRECTORY "${SHARP_RUNTIME_SOURCE_DIR}/vendor")
    target_include_directories(sharp_runtime_xml_contract SYSTEM INTERFACE
        "${SHARP_RUNTIME_SOURCE_DIR}/vendor")
endif()

add_library(CNA INTERFACE)
target_include_directories(CNA INTERFACE ${_cna_include_dirs})
target_compile_definitions(CNA INTERFACE CNA_RENDERER_SOFTWARE)
target_link_libraries(CNA INTERFACE SharpRuntime::Xml)

set(MYRA_CNA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MYRA_CNA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(MYRA_CNA_ENABLE_FORMAT_TARGETS OFF CACHE BOOL "" FORCE)
set(MYRA_CNA_CNA_DIR "${CNA_SOURCE_DIR}" CACHE PATH "" FORCE)
set(MYRA_CNA_SHARP_RUNTIME_DIR "${SHARP_RUNTIME_SOURCE_DIR}" CACHE PATH "" FORCE)
add_subdirectory("${MYRA_CNA_SOURCE_DIR}" myra-cna)

if(NOT TARGET Myra::CNA)
    message(FATAL_ERROR "Myra-CNA did not provide its documented Myra::CNA alias")
endif()
get_target_property(_myra_cna_definitions MYRA_CNA INTERFACE_COMPILE_DEFINITIONS)
if(NOT "MYRA_CNA_HAS_CNA_TARGET=1" IN_LIST _myra_cna_definitions)
    message(FATAL_ERROR "Myra-CNA did not select its parent-provided CNA path")
endif()

add_executable(myra_cna_parent_consumer Main.cpp)
target_link_libraries(myra_cna_parent_consumer PRIVATE Myra::CNA)
