add_library(recore_diagnostics MODULE
    diagnostics.cpp
)

target_include_directories(recore_diagnostics
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_definitions(recore_diagnostics
    PRIVATE
        RECORE_PLUGIN_BUILD
)

set_target_properties(recore_diagnostics PROPERTIES
    PREFIX ""
    OUTPUT_NAME "diagnostics"
)

recore_set_warnings(recore_diagnostics)