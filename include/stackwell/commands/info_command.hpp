#pragma once
#include <CLI/CLI.hpp>
#include <string>
namespace stackwell {
void register_info_command(CLI::App& app, const std::string& version);
}
