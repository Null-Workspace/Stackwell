
#include <stackwell/commands/plugins_command.hpp>

#include <stackwell/plugin/plugin_info.hpp>

#include <iostream>

namespace stackwell
{
    void register_plugins_command(
        CLI::App& app,
        const PluginManager& manager
    )
    {
        auto* command = app.add_subcommand(
            "plugins",
            "List discovered and loaded plugins"
        );

        command->callback([&manager]()
        {
            const auto& plugins = manager.get_plugins();

            std::cout << '\n';
            std::cout << "Stackwell Plugin Manager\n";
            std::cout << "=====================\n\n";

            if (plugins.empty())
            {
                std::cout << "No plugins discovered.\n\n";
                return;
            }

            for (const auto& plugin : plugins)
            {
                std::cout
                    << (plugin.name.empty()
                        ? "(unknown)"
                        : plugin.name);

                std::cout << '\n';

                std::cout
                    << "  Version:       "
                    << (plugin.version.empty()
                        ? "Unknown"
                        : plugin.version)
                    << '\n';

                std::cout
                    << "  API version:   ";

                if (plugin.api_version == 0)
                {
                    std::cout << "Unknown\n";
                }
                else
                {
                    std::cout << plugin.api_version << '\n';
                }

                std::cout
                    << "  Author:        "
                    << (plugin.author.empty()
                        ? "Unknown"
                        : plugin.author)
                    << '\n';

                std::cout
                    << "  Description:   "
                    << (plugin.description.empty()
                        ? "No description"
                        : plugin.description)
                    << '\n';

                std::cout
                    << "  Min host:      "
                    << (plugin.min_host_version.empty()
                        ? "Not specified"
                        : plugin.min_host_version)
                    << '\n';

                std::cout
                    << "  State:         "
                    << static_cast<int>(plugin.state)
                    << '\n';

                std::cout
                    << "  Path:          "
                    << plugin.path
                    << '\n';

                if (!plugin.error.empty())
                {
                    std::cout
                        << "  Error:         "
                        << plugin.error
                        << '\n';
                }

                std::cout << '\n';
            }
        });
    }
}
