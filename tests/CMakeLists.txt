# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
# See NOTICE.md and THIRD_PARTY_NOTICES.md.
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_executable(myra_cna_unit_tests
    EventHandlingStrategyTests.cpp
    EventsTests.cpp
    InputEventsManagerTests.cpp
    MathematicsTests.cpp
    ThicknessTests.cpp
    ThicknessRectangleTests.cpp
    VersionTests.cpp
)

# Rectangle construction belongs to CNA. Compile its small, dependency-free
# implementation only into the test executable so the public headers-only
# mode still exercises `Rectangle - Thickness` without building all of CNA.
set(_myra_cna_rectangle_source
    "${MYRA_CNA_CNA_DIR}/src/Microsoft/Xna/Framework/Rectangle.cpp")
set(_myra_cna_point_source
    "${MYRA_CNA_CNA_DIR}/src/Microsoft/Xna/Framework/Point.cpp")
if(NOT EXISTS "${_myra_cna_rectangle_source}" OR NOT EXISTS "${_myra_cna_point_source}")
    message(FATAL_ERROR
        "myra-cna: tests require CNA Point.cpp and Rectangle.cpp under '${MYRA_CNA_CNA_DIR}'.")
endif()
target_sources(myra_cna_unit_tests PRIVATE
    "${_myra_cna_point_source}"
    "${_myra_cna_rectangle_source}"
)
target_link_libraries(myra_cna_unit_tests PRIVATE Myra::CNA GTest::gtest_main)

if(MSVC)
    target_compile_options(myra_cna_unit_tests PRIVATE /W4 /WX)
else()
    target_compile_options(myra_cna_unit_tests PRIVATE -Wall -Wextra -Werror)
endif()

include(GoogleTest)
gtest_discover_tests(myra_cna_unit_tests)

add_test(
    NAME myra_cna_provenance
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/check_provenance.py"
            --root "${PROJECT_SOURCE_DIR}"
)
