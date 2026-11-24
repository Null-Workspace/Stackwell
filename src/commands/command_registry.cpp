#include <recore/commands/command_registry.hpp>
#include <recore/commands/info_command.hpp>
#include <recore/commands/plugins_command.hpp>

namespace recore
{
    void register_commands(
        CLI::App& app,
        const PluginManager& manager
    )
    {
        register_info_command(app);

        register_plugins_command(
            app,
            manager
        );
    }
}