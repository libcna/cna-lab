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
option(MYRA_CNA_WARNINGS_AS_ERRORS
       "Treat warnings from Myra-CNA-owned targets as errors" ON)
option(MYRA_CNA_ENABLE_FORMAT_TARGETS
       "Add myra_cna_format/check_format targets when clang-format is available" ON)
option(MYRA_CNA_ENABLE_IWYU
       "Run include-what-you-use while compiling the MYRA_CNA library" OFF)

function(myra_cna_enable_strict_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(MYRA_CNA_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
        if(MYRA_CNA_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

set(MYRA_CNA_CNA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cna"
    CACHE PATH "Path to the sibling CNA repository")
set(MYRA_CNA_SHARP_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sharp-runtime"
    CACHE PATH "Path to the sibling sharp-runtime repository")
set(MYRA_CNA_CNA_GRAPHICS_BACKEND "SOFTWARE"
    CACHE STRING "CNA backend used only when MYRA_CNA_LINK_CNA=ON")
set(MYRA_CNA_TEST_DISPLAY ":0"
    CACHE STRING "X11 DISPLAY used by Myra-CNA tests labelled RequiresDisplay")
set_property(CACHE MYRA_CNA_CNA_GRAPHICS_BACKEND PROPERTY STRINGS
    SDL_RENDERER EASYGL BGFX VULKAN WEBGPU HEADLESS SOFTWARE CANVAS ASCII DX3 D3D9 D3D11 D3D12 SDL_GPU)

add_library(MYRA_CNA STATIC
    src/Myra/Events/MyraEventArgs.cpp
    src/Myra/Graphics2D/Thickness.cpp
    src/Myra/Graphics2D/UI/ContentControl.cpp
    src/Myra/Graphics2D/UI/Container.cpp
    src/Myra/Graphics2D/UI/Containers/Panel.cpp
    src/Myra/Graphics2D/UI/Containers/Grid.cpp
    src/Myra/Graphics2D/UI/Containers/Proportion.cpp
    src/Myra/Graphics2D/UI/Containers/StackPanel.cpp
    src/Myra/Graphics2D/UI/InputContext.cpp
    src/Myra/Graphics2D/UI/InputEventsManager.cpp
    src/Myra/Graphics2D/UI/Layouts/GridLayout.cpp
    src/Myra/Graphics2D/UI/Layouts/StackPanelLayout.cpp
    src/Myra/Graphics2D/UI/Project.cpp
    src/Myra/Graphics2D/UI/Simple/HorizontalSeparator.cpp
    src/Myra/Graphics2D/UI/Simple/Image.cpp
    src/Myra/Graphics2D/UI/Simple/SeparatorWidget.cpp
    src/Myra/Graphics2D/UI/Simple/VerticalSeparator.cpp
    src/Myra/Graphics2D/UI/Widget.cpp
    src/Myra/MML/AttachedPropertiesRegistry.cpp
    src/Myra/MML/BaseContext.cpp
    src/Myra/MML/BaseObject.cpp
    src/Myra/MML/LoadContext.cpp
    src/Myra/MML/RegisterMyraTypes.cpp
    src/Myra/MML/SaveContext.cpp
    src/Myra/MML/TypeSerializers.cpp
    src/Myra/MML/ValueCodecGeometry.cpp
    src/Myra/MML/ValueCodecRegistry.cpp
    src/Myra/MyraEnvironment.cpp
    src/Myra/Utility/InputExtension.cpp
    src/Myra/Utility/PathUtils.cpp
    src/Myra/Utility/UIUtils.cpp
    src/Myra/Version.cpp
)
add_library(Myra::CNA ALIAS MYRA_CNA)

target_include_directories(MYRA_CNA
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_compile_features(MYRA_CNA PUBLIC cxx_std_23)
myra_cna_enable_strict_warnings(MYRA_CNA)

if(MYRA_CNA_ENABLE_IWYU)
    find_program(MYRA_CNA_IWYU_EXECUTABLE
        NAMES include-what-you-use iwyu
        DOC "Path to include-what-you-use")
    if(NOT MYRA_CNA_IWYU_EXECUTABLE)
        message(FATAL_ERROR
            "myra-cna: MYRA_CNA_ENABLE_IWYU=ON requires include-what-you-use")
    endif()
    set_property(TARGET MYRA_CNA PROPERTY
        CXX_INCLUDE_WHAT_YOU_USE "${MYRA_CNA_IWYU_EXECUTABLE}")
endif()

if(MYRA_CNA_ENABLE_FORMAT_TARGETS)
    find_program(MYRA_CNA_CLANG_FORMAT_EXECUTABLE
        NAMES clang-format clang-format-20 clang-format-19 clang-format-18
              clang-format-17 clang-format-16 clang-format-15 clang-format-14
        DOC "Path to clang-format")
    if(MYRA_CNA_CLANG_FORMAT_EXECUTABLE)
        file(GLOB_RECURSE MYRA_CNA_FORMAT_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.hpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp")
        list(SORT MYRA_CNA_FORMAT_SOURCES)

        add_custom_target(myra_cna_format
            COMMAND "${MYRA_CNA_CLANG_FORMAT_EXECUTABLE}"
                    --style=file -i ${MYRA_CNA_FORMAT_SOURCES}
            COMMENT "Formatting Myra-CNA C++ sources"
            COMMAND_EXPAND_LISTS
            VERBATIM)
        add_custom_target(myra_cna_check_format
            COMMAND "${MYRA_CNA_CLANG_FORMAT_EXECUTABLE}"
                    --style=file --dry-run --Werror ${MYRA_CNA_FORMAT_SOURCES}
            COMMENT "Checking Myra-CNA C++ source formatting"
            COMMAND_EXPAND_LISTS
            VERBATIM)
    else()
        message(STATUS
            "myra-cna: clang-format not found; format targets are unavailable")
    endif()
endif()

# CNA is intentionally resolved in the same three modes as cna-extended:
# a target supplied by the parent, an explicit sibling build, or header-only
# compile-check mode. The latter remains useful for API/provenance validation;
# features that construct CNA value types are exercised only by the linked
# CNA configuration.
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

if(MYRA_CNA_CNA_LINKED)
    target_sources(MYRA_CNA PRIVATE
        src/Myra/DefaultAssets.cpp
        src/Myra/Graphics2D/Brushes/SolidBrush.cpp
        src/Myra/Graphics2D/RenderContext.cpp
        src/Myra/Graphics2D/RenderContextShapes.cpp
        src/Myra/Graphics2D/TextureAtlases/ColoredRegion.cpp
        src/Myra/Graphics2D/TextureAtlases/NinePatchRegion.cpp
        src/Myra/Graphics2D/TextureAtlases/TextureRegion.cpp
        src/Myra/Graphics2D/TextureAtlases/TextureRegionAtlas.cpp
        src/Myra/Graphics2D/TextureAtlases/TintedRegion.cpp
        src/Myra/Graphics2D/UI/WidgetRender.cpp
        src/Myra/MyraEnvironmentCna.cpp
        src/Myra/Utility/CrossEngineStuff.cpp
    )
else()
    target_sources(MYRA_CNA PRIVATE
        src/Myra/Graphics2D/UI/WidgetRenderStub.cpp
    )
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
