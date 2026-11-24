
cmake_minimum_required(VERSION 3.25)

project(
    Stackwell
    VERSION 0.0.1
    DESCRIPTION "Extensible System Administration Platform"
    LANGUAGES CXX
)

# --------------------------------------------------
# C++ Standard
# --------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --------------------------------------------------
# Build Options
# --------------------------------------------------

option(STACKWELL_BUILD_PLUGINS "Build Stackwell plugins" ON)
option(STACKWELL_BUILD_TESTS "Build unit tests" OFF)

# --------------------------------------------------
# Output Directories
# --------------------------------------------------

set(STACKWELL_OUTPUT_DIR "${CMAKE_BINARY_DIR}/dist")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${STACKWELL_OUTPUT_DIR}/bin"
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    "${STACKWELL_OUTPUT_DIR}/plugins"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/lib"
)

foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG}
        "${STACKWELL_OUTPUT_DIR}/bin"
    )

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG}
        "${STACKWELL_OUTPUT_DIR}/plugins"
    )
endforeach()

# Copy application resources into the distribution directory.
file(MAKE_DIRECTORY
    "${STACKWELL_OUTPUT_DIR}/resources/config"
)

configure_file(
    "${PROJECT_SOURCE_DIR}/resources/config/config.json"
    "${STACKWELL_OUTPUT_DIR}/resources/config/config.json"
    COPYONLY
)

# --------------------------------------------------
# Dependencies
# --------------------------------------------------

find_package(CLI11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

# --------------------------------------------------
# Compiler Warnings
# --------------------------------------------------

include(cmake/CompilerWarnings.cmake)

# --------------------------------------------------
# Core Library
# --------------------------------------------------

add_library(stackwell_core STATIC
    src/core/application.cpp
    src/core/config.cpp
    src/core/logger.cpp
    src/plugin/plugin_manager.cpp
)

target_include_directories(stackwell_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(stackwell_core
    PUBLIC
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        unofficial::sqlite3::sqlite3
        ${CMAKE_DL_LIBS}
)

stackwell_set_warnings(stackwell_core)

# --------------------------------------------------
# Main Executable
# --------------------------------------------------

add_executable(stackwell
    app/main.cpp

    src/commands/command_registry.cpp
    src/commands/info_command.cpp
    src/commands/plugins_command.cpp
)

target_link_libraries(stackwell
    PRIVATE
        stackwell_core
        CLI11::CLI11
)

stackwell_set_warnings(stackwell)

# --------------------------------------------------
# Plugins
# --------------------------------------------------

if(STACKWELL_BUILD_PLUGINS)
    add_subdirectory(plugins/diagnostics)

    install(
        TARGETS stackwell_diagnostics
        RUNTIME DESTINATION plugins
        LIBRARY DESTINATION plugins
    )
endif()

# --------------------------------------------------
# Installation
# --------------------------------------------------

install(
    TARGETS stackwell
    RUNTIME DESTINATION bin
)

# --------------------------------------------------
# Tests
# --------------------------------------------------

if(STACKWELL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# --------------------------------------------------
# Configuration Summary
# --------------------------------------------------

message(STATUS "--------------------------------")
message(STATUS "Stackwell ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "--------------------------------")
