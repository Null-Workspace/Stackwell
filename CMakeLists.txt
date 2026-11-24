cmake_minimum_required(VERSION 3.25)

project(
    ReCore
    VERSION 0.0.01
    DESCRIPTION "Cross-platform System Administration Toolkit"
    LANGUAGES CXX
)

# --------------------------------------------------
# C++ Configuration
# --------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --------------------------------------------------
# Build Options
# --------------------------------------------------

option(RECORE_BUILD_PLUGINS "Build ReCore plugins" ON)
option(RECORE_BUILD_TESTS "Build unit tests" OFF)

# --------------------------------------------------
# Output Directories
# --------------------------------------------------

set(RECORE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/dist")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${RECORE_OUTPUT_DIR}/bin"
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    "${RECORE_OUTPUT_DIR}/plugins"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/lib"
)

foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG}
        "${RECORE_OUTPUT_DIR}/bin"
    )

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG}
        "${RECORE_OUTPUT_DIR}/plugins"
    )
endforeach()

# Copy application resources into the distribution directory.
file(MAKE_DIRECTORY
    "${RECORE_OUTPUT_DIR}/resources/config"
)

configure_file(
    "${PROJECT_SOURCE_DIR}/resources/config/config.json"
    "${RECORE_OUTPUT_DIR}/resources/config/config.json"
    COPYONLY
)

# --------------------------------------------------
# Dependencies
# --------------------------------------------------

find_package(CLI11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)

# --------------------------------------------------
# Compiler Warnings
# --------------------------------------------------

include(cmake/CompilerWarnings.cmake)


# --------------------------------------------------
# Core Library
# --------------------------------------------------

add_library(recore_core STATIC
    src/core/application.cpp
    src/core/config.cpp
    src/core/logger.cpp

    src/plugin/plugin_manager.cpp
)

target_include_directories(recore_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(recore_core
    PUBLIC
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        SQLite3::SQLite3
        ${CMAKE_DL_LIBS}
)

recore_set_warnings(recore_core)

# --------------------------------------------------
# Main Executable
# --------------------------------------------------

add_executable(recore
    app/main.cpp

    src/commands/command_registry.cpp
    src/commands/info_command.cpp
    src/commands/plugins_command.cpp
)

target_link_libraries(recore
    PRIVATE
        recore_core
        CLI11::CLI11
)

recore_set_warnings(recore)

# --------------------------------------------------
# Plugins
# --------------------------------------------------

if(RECORE_BUILD_PLUGINS)
    add_subdirectory(plugins/diagnostics)

    install(
        TARGETS recore_diagnostics
        RUNTIME DESTINATION plugins
        LIBRARY DESTINATION plugins
    )
endif()

# --------------------------------------------------
# Installation
# --------------------------------------------------

install(
    TARGETS recore
    RUNTIME DESTINATION bin
)

install(
    DIRECTORY resources/config/
    DESTINATION resources/config
)

# --------------------------------------------------
# Tests
# --------------------------------------------------

if(RECORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "--------------------------------")
message(STATUS "ReCore ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "--------------------------------")
