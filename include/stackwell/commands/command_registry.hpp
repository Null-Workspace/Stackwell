#pragma once
#include <CLI/CLI.hpp>
#include <stackwell/plugin/plugin_manager.hpp>
#include <string>
namespace stackwell {
void register_commands(CLI::App& app, const PluginManager& manager, const std::string& version);
}
