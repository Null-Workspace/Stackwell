#include <stackwell/commands/command_registry.hpp>
#include <stackwell/commands/info_command.hpp>
#include <stackwell/commands/plugins_command.hpp>
#include <stackwell/commands/access_recovery_command.hpp>

namespace stackwell
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

        register_access_recovery_command(app);
    }
}