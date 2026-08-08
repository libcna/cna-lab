# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
# See NOTICE.md and THIRD_PARTY_NOTICES.md.
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_executable(myra_cna_unit_tests
    BaseObjectTests.cpp
    EventHandlingStrategyTests.cpp
    EventsTests.cpp
    InputEventsManagerTests.cpp
    LayoutUtilsTests.cpp
    MathematicsTests.cpp
    StringUtilsTests.cpp
    ThicknessTests.cpp
    VersionTests.cpp
    WidgetContractsTests.cpp
)

# In the default headers-only configuration, compile CNA's small Point and
# Rectangle implementations into the test executable to exercise `Rectangle -
# Thickness` without building all of CNA. A linked CNA target already supplies
# those symbols, and additionally enables matrix/vector transform tests.
if(MYRA_CNA_CNA_LINKED)
    target_sources(myra_cna_unit_tests PRIVATE
        ColorHSVTests.cpp
        ThicknessRectangleTests.cpp
        TransformableContractTests.cpp
        TransformTests.cpp
    )
else()
    set(_myra_cna_rectangle_source
        "${MYRA_CNA_CNA_DIR}/src/Microsoft/Xna/Framework/Rectangle.cpp")
    set(_myra_cna_point_source
        "${MYRA_CNA_CNA_DIR}/src/Microsoft/Xna/Framework/Point.cpp")
    if(NOT EXISTS "${_myra_cna_rectangle_source}" OR NOT EXISTS "${_myra_cna_point_source}")
        message(FATAL_ERROR
            "myra-cna: tests require CNA Point.cpp and Rectangle.cpp under '${MYRA_CNA_CNA_DIR}'.")
    endif()
    target_sources(myra_cna_unit_tests PRIVATE
        ThicknessRectangleTests.cpp
        "${_myra_cna_point_source}"
        "${_myra_cna_rectangle_source}"
    )
endif()
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
