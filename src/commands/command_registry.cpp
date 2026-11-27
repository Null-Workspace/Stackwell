#include <stackwell/commands/command_registry.hpp>
#include <stackwell/commands/info_command.hpp>
#include <stackwell/commands/plugins_command.hpp>
namespace stackwell {
void register_commands(CLI::App& app, const PluginManager& manager, const std::string& version) {
 register_info_command(app, version);
 register_plugins_command(app, manager);
}
}
