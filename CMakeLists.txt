cmake_minimum_required(VERSION 3.20)

project(CnaKiller LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# cna-killer is a deliberately malicious CNA game: it drives ../cnanext (the
# CNA runtime) through the sibling ../sharp-runtimenext checkout, exactly like
# cna-samples does, so a single sibling checkout is fixed up by every consumer
# in the same session.
# ---------------------------------------------------------------------------

if(EMSCRIPTEN)
    set(_cna_killer_reference_renderer "WEBGL2")
else()
    set(_cna_killer_reference_renderer "OPENGLES3")
endif()
set(CNA_GRAPHICS_RENDERER "${_cna_killer_reference_renderer}" CACHE STRING
    "CNA Killer reference renderer")

set(CNA_SHARP_RUNTIME_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../sharp-runtimenext"
    CACHE PATH "sharp-runtimenext checkout used by cna-killer")

set(CNA_BUILD_TESTS    OFF CACHE BOOL "Build CNA tests")
set(CNA_BUILD_EXAMPLES OFF CACHE BOOL "Build CNA examples")
set(SHARP_RUNTIME_BUILD_TESTS OFF CACHE BOOL "Build sharp-runtime tests")

add_subdirectory(../cnanext CNA_BUILD)

add_executable(cna-killer
    src/Program.cpp
    src/CliOptions.cpp
    src/ChaosLog.cpp
    src/ChaosEngine.cpp
    src/CnaKillerGame.cpp
)

target_include_directories(cna-killer PRIVATE src)

target_link_libraries(cna-killer PRIVATE CNA)

if(EMSCRIPTEN)
    set_target_properties(cna-killer PROPERTIES SUFFIX ".html")
    # See cna-samples/CMakeLists.txt: Game::Run() is CNA's blocking XNA loop contract, so the
    # final web artifact needs the matching Asyncify-based browser-loop suspension policy.
    target_link_libraries(cna-killer PRIVATE CNA::EmscriptenAsyncify)
    target_link_options(cna-killer PRIVATE
        -sALLOW_MEMORY_GROWTH=1
        "-sMIN_WEBGL_VERSION=2"
        "-sMAX_WEBGL_VERSION=2"
    )
endif()

if(TARGET SDL3::SDL3main)
    target_link_libraries(cna-killer PRIVATE SDL3::SDL3main)
endif()

if(WIN32)
    set_target_properties(cna-killer PROPERTIES WIN32_EXECUTABLE TRUE)
    if(COMMAND cna_copy_sdl_runtime)
        cna_copy_sdl_runtime(cna-killer)
    endif()
endif()
