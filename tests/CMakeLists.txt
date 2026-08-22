# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
# See NOTICE.md and THIRD_PARTY_NOTICES.md.
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_executable(myra_cna_unit_tests
    AttachedPropertiesRegistryTests.cpp
    AttributeMetadataTests.cpp
    BaseContextTests.cpp
    BaseObjectTests.cpp
    EventHandlingStrategyTests.cpp
    MyraEnvironmentSettingsTests.cpp
    EventsExtensionsTests.cpp
    EventsTests.cpp
    GraphicsContractsTests.cpp
    InputExtensionTests.cpp
    InputEventsManagerTests.cpp
    LayoutUtilsTests.cpp
    MathematicsTests.cpp
    PathUtilsTests.cpp
    RestTests.cpp
    StringUtilsTests.cpp
    TemporaryAssetDirectoryTests.cpp
    ThicknessTests.cpp
    TypeRegistryTests.cpp
    ValueCodecRegistryTests.cpp
    VersionTests.cpp
    WidgetContractsTests.cpp
)

# In the default headers-only configuration, compile CNA's small Point and
# Rectangle implementations into the test executable to exercise `Rectangle -
# Thickness` without building all of CNA. A linked CNA target already supplies
# those symbols, and additionally enables matrix/vector transform tests.
if(MYRA_CNA_CNA_LINKED)
    target_sources(myra_cna_unit_tests PRIVATE
        ButtonTests.cpp
        ButtonBaseTests.cpp
        CheckButtonTests.cpp
        CheckButtonBaseTests.cpp
        ColorHSVTests.cpp
        ColoredRegionTests.cpp
        ComboViewTests.cpp
        ComparisonHelpersTests.cpp
        CrossEngineStuffTests.cpp
        DefaultAssetsTests.cpp
        GridLayoutTests.cpp
        IHasColorTests.cpp
        ImageTests.cpp
        ListViewButtonTests.cpp
        ListViewTests.cpp
        MMLContextTests.cpp
        MenuSeparatorTests.cpp
        MyraEnvironmentTests.cpp
        MyraTypeRegistryTests.cpp
        NinePatchRegionTests.cpp
        PanelTests.cpp
        ProjectTests.cpp
        ProportionTests.cpp
        ProgressBarTests.cpp
        RadioButtonTests.cpp
        RenderContextShapesTests.cpp
        RenderContextTests.cpp
        ScrollViewerTests.cpp
        SeparatorTests.cpp
        SelectorTests.cpp
        SingleItemLayoutTests.cpp
        SolidBrushTests.cpp
        StackPanelTests.cpp
        SliderTests.cpp
        ThicknessRectangleTests.cpp
        TextureRegionTests.cpp
        TextureRegionAtlasTests.cpp
        TintedRegionTests.cpp
        ToggleButtonTests.cpp
        TransformableContractTests.cpp
        TransformTests.cpp
        TypeSerializersTests.cpp
        UIUtilsTests.cpp
        WidgetCloneTests.cpp
        WidgetInputTests.cpp
        WidgetRenderTests.cpp
        WidgetTests.cpp
    )
else()
    if(EXISTS "${MYRA_CNA_CNA_DIR}/modules/math/src/Rectangle.cpp")
        set(_myra_cna_rectangle_source
            "${MYRA_CNA_CNA_DIR}/modules/math/src/Rectangle.cpp")
        set(_myra_cna_point_source
            "${MYRA_CNA_CNA_DIR}/modules/math/src/Point.cpp")
    else()
        set(_myra_cna_rectangle_source
            "${MYRA_CNA_CNA_DIR}/src/Microsoft/Xna/Framework/Rectangle.cpp")
        set(_myra_cna_point_source
            "${MYRA_CNA_CNA_DIR}/src/Microsoft/Xna/Framework/Point.cpp")
    endif()
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
myra_cna_enable_strict_warnings(myra_cna_unit_tests)

include(GoogleTest)
gtest_discover_tests(myra_cna_unit_tests)

add_test(
    NAME myra_cna_provenance
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/check_provenance.py"
            --root "${PROJECT_SOURCE_DIR}"
)

if(MYRA_CNA_CNA_LINKED
   AND MYRA_CNA_CNA_GRAPHICS_BACKEND STREQUAL "SDL_RENDERER")
    add_executable(myra_cna_sdlrenderer_smoke SdlRendererSmoke.cpp)
    target_link_libraries(myra_cna_sdlrenderer_smoke PRIVATE Myra::CNA)
    if(TARGET SDL3::SDL3main)
        target_link_libraries(myra_cna_sdlrenderer_smoke PRIVATE SDL3::SDL3main)
    endif()
    myra_cna_enable_strict_warnings(myra_cna_sdlrenderer_smoke)

    add_test(NAME myra_cna_sdlrenderer_smoke COMMAND myra_cna_sdlrenderer_smoke)
    set_tests_properties(myra_cna_sdlrenderer_smoke PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=x11;DISPLAY=${MYRA_CNA_TEST_DISPLAY}"
        LABELS "GraphicsSmoke;SDLRenderer;RequiresDisplay"
        RESOURCE_LOCK myra_cna_display
        TIMEOUT 30)
endif()
