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

function(myra_cna_finalize_parent_dependencies)
    # This driver is included immediately after CNA's project() call, whereas
    # CNA adds sharp-runtime later in its root CMakeLists.txt. Enabling Xml
    # here would therefore fail before the modular component is registered.
    # Defer it to the end of the parent directory so Myra keeps its explicit
    # narrow dependency instead of forcing sharp-runtime's All configuration.
    if(NOT TARGET SharpRuntime::Xml AND COMMAND sharp_runtime_enable_component)
        # sharp-runtime normally keeps this directory-local value while it is
        # configuring its own subdirectory. Its component-enabling helper is
        # intentionally global, so restore the documented source root for the
        # deferred invocation from CNA's directory scope.
        set(SHARP_RUNTIME_ROOT "${MYRA_CNA_SHARP_RUNTIME_DIR}")
        sharp_runtime_enable_component(Xml)
    endif()

    if(TARGET SharpRuntime::Xml)
        target_link_libraries(MYRA_CNA PUBLIC SharpRuntime::Xml)
    elseif(TARGET SHARP_RUNTIME)
        # Pre-modular sharp-runtime exposes only the compatibility target.
        target_link_libraries(MYRA_CNA PUBLIC SHARP_RUNTIME)
    else()
        message(FATAL_ERROR
            "myra-cna: the parent CNA build did not provide SharpRuntime::Xml "
            "or the legacy SHARP_RUNTIME target required by Myra MML.")
    endif()
endfunction()

# The project include runs immediately after CNA's project() command. CMake
# permits a plain target link name to be resolved by a target defined later in
# the same configure pass, so Myra-CNA can declare its linked surface now and
# CNA's root build supplies the umbrella target before generation completes.
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.."
    "${CMAKE_BINARY_DIR}/_myra_cna")

cmake_language(DEFER CALL myra_cna_finalize_parent_dependencies)
