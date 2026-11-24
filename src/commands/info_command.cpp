
#include <recore/commands/info_command.hpp>

#include <spdlog/spdlog.h>

#include <iostream>

namespace recore
{
    void register_info_command(CLI::App& app)
    {
        auto* info = app.add_subcommand(
            "info",
            "Show information about ReCore"
        );

        info->callback([]()
        {
            spdlog::info("Executing command: info");

            std::cout << "\n";
            std::cout << "ReCore System Administration Toolkit\n";
            std::cout << "-------------------------------------\n";
            std::cout << "Version: 0.0.01\n";
            std::cout << "Architecture: Modular Monolith\n";
            std::cout << "Language: C++20\n";
            std::cout << "\n";

            spdlog::info("Command info completed");
        });
    }
}
