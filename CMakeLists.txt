cmake_minimum_required(VERSION 3.23)

project(CnaLab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# This laboratory intentionally consumes the active next-generation checkouts.
# Keeping both roots explicit prevents an accidental fallback to the older CNA
# or sharp-runtime siblings when several are available in the workspace.
set(CNA_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cnanext" CACHE PATH
    "Path to the cnanext checkout")
set(CNA_SHARP_RUNTIME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../sharp-runtimenext" CACHE PATH
    "Path to the sharp-runtimenext checkout used by CNA" FORCE)

foreach(_cna_lab_dependency IN ITEMS "${CNA_ROOT_DIR}" "${CNA_SHARP_RUNTIME_ROOT}")
    if(NOT EXISTS "${_cna_lab_dependency}/CMakeLists.txt")
        message(FATAL_ERROR "cna-lab requires a sibling checkout at '${_cna_lab_dependency}'.")
    endif()
endforeach()

# OPENGLES3 is a practical interactive default on Linux.  HEADLESS remains a
# first-class configuration for deterministic CI smoke tests.
if(NOT DEFINED CNA_GRAPHICS_RENDERER)
    set(CNA_GRAPHICS_RENDERER "OPENGLES3" CACHE STRING "CNA graphics renderer")
endif()

set(CNA_BUILD_TESTS OFF CACHE BOOL "Do not build CNA's own test suite" FORCE)
set(CNA_BUILD_EXAMPLES OFF CACHE BOOL "Do not build CNA's own example suite" FORCE)
set(SHARP_RUNTIME_BUILD_TESTS OFF CACHE BOOL "Do not build sharp-runtime's own tests" FORCE)
set(EASYGL_BUILD_TESTS OFF CACHE BOOL "Do not build easy-gl tests" FORCE)
set(EASYGL_BUILD_EXAMPLES OFF CACHE BOOL "Do not build easy-gl examples" FORCE)
set(CNA_ENABLE_NET OFF CACHE BOOL "Networking is outside this graphics/media lab" FORCE)
set(CNA_CNAEXT ON CACHE BOOL "Enable modern CNA graphics extensions" FORCE)
set(CNA_EASYGL_COMPILED_EFFECTS ON CACHE BOOL "Enable compiled effects on EasyGL renderers" FORCE)

# The lab has a real FFmpeg-backed VideoPlayer demo, not just Video metadata.
# Linux development dependencies are part of this workspace and are checked at
# configuration time by CNA with this explicit contract.
set(CNA_ENABLE_VIDEO ON CACHE STRING "Require FFmpeg video playback" FORCE)

add_subdirectory("${CNA_ROOT_DIR}" CNA_BUILD)

if(NOT TARGET CNA)
    message(FATAL_ERROR "cnanext did not define the CNA umbrella target.")
endif()

add_executable(cna_lab
    app/CnaLabGame.cpp
    app/main.cpp
)

target_include_directories(cna_lab PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/app")
target_link_libraries(cna_lab PRIVATE CNA)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cna_lab PRIVATE -Wall -Wextra)
endif()

# CNA's tools turn one compact, deliberately synthetic input model plus raw
# media into every content path the game demonstrates:
#   direct glTF runtime import, generated CNJ, compiled CNB, XNB, CNB sound,
#   and a compiled-CNB Video that VideoPlayer decodes through FFmpeg.
set(CNA_LAB_GLTF_SOURCE "${CNA_ROOT_DIR}/tests/assets/gltf/glb-basic.glb")
set(CNA_LAB_XNB_SOURCE
    "${CNA_ROOT_DIR}/tests/assets/xnb/monogame/windows/uncompressed/white-1.xnb")
set(CNA_LAB_SOUND_SOURCE "${CNA_ROOT_DIR}/modules/audio/examples/demo_sound/Content/sounds/beep.wav")
set(CNA_LAB_VIDEO_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../cna-examples/Content/MediaDemo/test_clip.mp4")

foreach(_cna_lab_asset IN ITEMS
        "${CNA_LAB_GLTF_SOURCE}"
        "${CNA_LAB_XNB_SOURCE}"
        "${CNA_LAB_SOUND_SOURCE}"
        "${CNA_LAB_VIDEO_SOURCE}")
    if(NOT EXISTS "${_cna_lab_asset}")
        message(FATAL_ERROR "Required cna-lab demonstration asset is missing: ${_cna_lab_asset}")
    endif()
endforeach()

foreach(_cna_lab_tool IN ITEMS cna_tool_gltf_to_cnj cna_tool_gltf_to_cnb cna_tool_source_to_cnb)
    if(NOT TARGET ${_cna_lab_tool})
        message(FATAL_ERROR "cnanext did not define required content tool '${_cna_lab_tool}'.")
    endif()
endforeach()
add_dependencies(cna_lab cna_tool_gltf_to_cnj cna_tool_gltf_to_cnb cna_tool_source_to_cnb)

set(_cna_lab_content "$<TARGET_FILE_DIR:cna_lab>/Content")
add_custom_command(TARGET cna_lab POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${_cna_lab_content}/Models"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${_cna_lab_content}/Pipeline"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${_cna_lab_content}/Legacy"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${_cna_lab_content}/Media"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CNA_LAB_GLTF_SOURCE}" "${_cna_lab_content}/Models/triangle.glb"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CNA_LAB_XNB_SOURCE}" "${_cna_lab_content}/Legacy/white-1.xnb"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CNA_LAB_SOUND_SOURCE}" "${_cna_lab_content}/Media/lab-beep.wav"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CNA_LAB_VIDEO_SOURCE}" "${_cna_lab_content}/Media/lab-demo.mp4"
    COMMAND "$<TARGET_FILE:cna_tool_gltf_to_cnj>"
        "${_cna_lab_content}/Models/triangle.glb" "${_cna_lab_content}/Pipeline" triangle
    # CNJ Model sidecars are content-root-relative by CNA's runtime contract.
    # The converter intentionally writes the binaries next to its output, so
    # qualify the generated names when the output lives in Content/Pipeline.
    COMMAND "${CMAKE_COMMAND}"
        "-DINPUT=${_cna_lab_content}/Pipeline/triangle.cnj" "-DPREFIX=Pipeline"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PrefixCnjSidecars.cmake"
    COMMAND "$<TARGET_FILE:cna_tool_gltf_to_cnb>"
        "${_cna_lab_content}/Models/triangle.glb" "${_cna_lab_content}/Pipeline" triangle --quiet
    COMMAND "$<TARGET_FILE:cna_tool_source_to_cnb>"
        "${_cna_lab_content}/Media/lab-beep.wav" "${_cna_lab_content}/Media/lab-beep.cnb" --quiet
    COMMAND "$<TARGET_FILE:cna_tool_source_to_cnb>"
        "${_cna_lab_content}/Media/lab-demo.mp4" "${_cna_lab_content}/Media/lab-video.cnb"
        --as video --stream Media/lab-demo.mp4 --duration-ms 4000 --frame-size 320x240
        --fps 24 --soundtrack 2 --quiet
    COMMENT "Building cna-lab content pipelines (glTF, CNJ, CNB, XNB, sound and video)"
    VERBATIM
)

if(COMMAND cna_copy_renderer_runtime)
    cna_copy_renderer_runtime(cna_lab)
endif()
if(WIN32 AND COMMAND cna_copy_sdl_runtime)
    cna_copy_sdl_runtime(cna_lab)
endif()
if(MINGW AND COMMAND cna_copy_mingw_cxx_runtime)
    cna_copy_mingw_cxx_runtime(cna_lab)
endif()

enable_testing()
add_test(NAME CnaLabMediaSmoke COMMAND cna_lab --smoke-media)
set_tests_properties(CnaLabMediaSmoke PROPERTIES
    LABELS "headless;content;media"
    TIMEOUT 45
    ENVIRONMENT "CNA_HEADLESS_MODE=Fast;SDL_AUDIODRIVER=dummy")
