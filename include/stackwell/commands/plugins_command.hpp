#pragma once

#include <CLI/CLI.hpp>

#include <stackwell/plugin/plugin_manager.hpp>

namespace stackwell
{
    void register_plugins_command(
        CLI::App& app,
        const PluginManager& manager
    );
}