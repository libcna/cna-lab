# CNJ Model sidecars are specified as paths relative to Content's root, while
# cna_tool_gltf_to_cnj emits a compact package with sidecars next to the CNJ.
# This keeps that package under Content/Pipeline without weakening the runtime
# containment rule or duplicating binaries at Content's top level.
if(NOT DEFINED INPUT OR NOT DEFINED PREFIX)
    message(FATAL_ERROR "PrefixCnjSidecars.cmake requires INPUT and PREFIX.")
endif()

file(READ "${INPUT}" _cnj_text)
foreach(_field IN ITEMS vertices indices skeleton clip morphTargets)
    string(REGEX REPLACE
        "\"${_field}\": \"([^\"]+)\""
        "\"${_field}\": \"${PREFIX}/\\1\""
        _cnj_text "${_cnj_text}")
endforeach()
file(WRITE "${INPUT}" "${_cnj_text}")
