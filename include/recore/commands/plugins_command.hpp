#pragma once

#include <CLI/CLI.hpp>

#include <recore/plugin/plugin_manager.hpp>

namespace recore
{
    void register_plugins_command(
        CLI::App& app,
        const PluginManager& manager
    );
}