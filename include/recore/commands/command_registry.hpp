#pragma once

#include <CLI/CLI.hpp>

#include <recore/plugin/plugin_manager.hpp>

namespace recore
{
    void register_commands(
        CLI::App& app,
        const PluginManager& manager
    );

    void register_info_command(CLI::App& app);
}