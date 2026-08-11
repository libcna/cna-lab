# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
# See NOTICE.md and THIRD_PARTY_NOTICES.md.

include_guard(GLOBAL)

if(NOT PROJECT_NAME STREQUAL "CNA")
    message(FATAL_ERROR
        "AddMyraCnaToCnaBuild.cmake must be supplied through "
        "CMAKE_PROJECT_CNA_INCLUDE while configuring the CNA source tree.")
endif()

set(MYRA_CNA_CNA_DIR "${CMAKE_SOURCE_DIR}" CACHE PATH
    "Path to the CNA checkout supplied by the parent build" FORCE)
set(MYRA_CNA_SHARP_RUNTIME_DIR "${CMAKE_SOURCE_DIR}/../sharp-runtime" CACHE PATH
    "Path to the sharp-runtime checkout consumed by Myra-CNA" FORCE)
set(MYRA_CNA_EXPECT_PARENT_CNA ON CACHE INTERNAL
    "The CNA top-level build will define its umbrella target after Myra-CNA is added" FORCE)

# The project include runs immediately after CNA's project() command. CMake
# permits a plain target link name to be resolved by a target defined later in
# the same configure pass, so Myra-CNA can declare its linked surface now and
# CNA's root build supplies the umbrella target before generation completes.
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.."
    "${CMAKE_BINARY_DIR}/_myra_cna")
