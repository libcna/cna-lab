# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
# See NOTICE.md and THIRD_PARTY_NOTICES.md.

foreach(_required_value IN ITEMS
        MYRA_CNA_SOURCE_DIR CNA_SOURCE_DIR SHARP_RUNTIME_SOURCE_DIR BINARY_DIR)
    if(NOT DEFINED ${_required_value} OR "${${_required_value}}" STREQUAL "")
        message(FATAL_ERROR "${_required_value} is required")
    endif()
endforeach()

execute_process(
    COMMAND "${CMAKE_COMMAND}"
        -S "${MYRA_CNA_SOURCE_DIR}/tests/consumer-parent"
        -B "${BINARY_DIR}"
        "-DMYRA_CNA_SOURCE_DIR=${MYRA_CNA_SOURCE_DIR}"
        "-DCNA_SOURCE_DIR=${CNA_SOURCE_DIR}"
        "-DSHARP_RUNTIME_SOURCE_DIR=${SHARP_RUNTIME_SOURCE_DIR}"
    RESULT_VARIABLE _configure_result
)
if(NOT _configure_result EQUAL 0)
    message(FATAL_ERROR "Parent-consumer configure failed: ${_configure_result}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${BINARY_DIR}"
        --target myra_cna_parent_consumer --parallel 3
    RESULT_VARIABLE _build_result
)
if(NOT _build_result EQUAL 0)
    message(FATAL_ERROR "Parent-consumer build failed: ${_build_result}")
endif()

execute_process(
    COMMAND "${BINARY_DIR}/myra_cna_parent_consumer"
    RESULT_VARIABLE _run_result
    OUTPUT_VARIABLE _run_output
    ERROR_VARIABLE _run_error
)
if(NOT _run_result EQUAL 0)
    message(FATAL_ERROR
        "Parent consumer failed: ${_run_result}\n${_run_output}${_run_error}")
endif()
if(NOT _run_output MATCHES "parent target -> Myra::CNA -> 0\\.1\\.0-dev")
    message(FATAL_ERROR "Unexpected parent-consumer output: ${_run_output}")
endif()
