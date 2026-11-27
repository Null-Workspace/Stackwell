#include <stackwell/commands/plugins_command.hpp>
#include <stackwell/plugin/plugin_info.hpp>
#include <iostream>

namespace stackwell
{
    void register_plugins_command(CLI::App& app, const PluginManager& manager)
    {
        auto* command = app.add_subcommand("plugins", "List discovered plugins");
        command->callback([&manager]()
        {
            const auto& plugins = manager.get_plugins();
            std::cout << "Stackwell plugins\n\n";
            if (plugins.empty())
            {
                std::cout << "No plugins found.\n";
                return;
            }

            for (const auto& plugin : plugins)
            {
                std::cout << plugin.name << " v" << plugin.version << '\n'
                          << "  State: " << plugin_state_to_string(plugin.state) << '\n'
                          << "  API: " << plugin.api_version << '\n'
                          << "  File: " << plugin.path.string() << '\n';
                if (!plugin.description.empty())
                    std::cout << "  " << plugin.description << '\n';
                if (!plugin.error.empty())
                    std::cout << "  Error: " << plugin.error << '\n';
                std::cout << '\n';
            }
        });
    }
}
