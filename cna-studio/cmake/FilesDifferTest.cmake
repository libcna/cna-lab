# SPDX-License-Identifier: MS-PL
#
# Fails when two files are byte-identical.
#
# `plan.md` CORE-03. `cmake -E compare_files` answers the opposite question, and the question worth
# asking about a pair of captures is whether the state they differ in reached the screen at all: two
# identical PNGs from a selected and an unselected shell mean the selection is invisible, which is
# exactly the defect the pair exists to catch.

foreach(_required FIRST SECOND)
    if(NOT DEFINED ${_required})
        message(FATAL_ERROR "FilesDifferTest.cmake needs -D${_required}=<path>")
    endif()
    if(NOT EXISTS "${${_required}}")
        message(FATAL_ERROR "${_required} '${${_required}}' was not written; the capture that "
                            "produces it must run before this test")
    endif()
endforeach()

execute_process(COMMAND "${CMAKE_COMMAND}" -E compare_files "${FIRST}" "${SECOND}"
                RESULT_VARIABLE _identical)

if(_identical EQUAL 0)
    message(FATAL_ERROR
        "'${FIRST}' and '${SECOND}' are byte-identical. They are captures of two states that must "
        "look different -- a selection that changes nothing on screen is a selection a user cannot "
        "see (plan.md CORE-03).")
endif()

message(STATUS "the two captures differ, as they must")
