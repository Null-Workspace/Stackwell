add_library(stackwell_diagnostics MODULE
    diagnostics.cpp
)

target_include_directories(stackwell_diagnostics
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_definitions(stackwell_diagnostics
    PRIVATE
        STACKWELL_PLUGIN_BUILD
)

set_target_properties(stackwell_diagnostics PROPERTIES
    PREFIX ""
    OUTPUT_NAME "diagnostics"
)

stackwell_set_warnings(stackwell_diagnostics)