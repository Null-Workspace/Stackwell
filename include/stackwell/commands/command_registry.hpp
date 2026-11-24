#pragma once

#include <CLI/CLI.hpp>

#include <stackwell/plugin/plugin_manager.hpp>

namespace stackwell
{
    void register_commands(
        CLI::App& app,
        const PluginManager& manager
    );

    void register_info_command(CLI::App& app);
}