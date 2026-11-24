#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace stackwell
{
    enum class PluginState
    {
        Discovered,
        Loaded,
        Initialized,
        Failed
    };

    struct PluginInfo
    {
        std::string name;
        std::string version;

        std::string author;
        std::string description;
        std::string min_host_version;

        std::uint32_t api_version = 0;

        std::filesystem::path path;

        PluginState state = PluginState::Discovered;

        std::string error;
    };

    inline const char* plugin_state_to_string(
        PluginState state
    )
    {
        switch (state)
        {
            case PluginState::Discovered:
                return "Discovered";

            case PluginState::Loaded:
                return "Loaded";

            case PluginState::Initialized:
                return "Initialized";

            case PluginState::Failed:
                return "Failed";
        }

        return "Unknown";
    }
}