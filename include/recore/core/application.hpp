#pragma once

#include <filesystem>

#include <recore/core/config.hpp>
#include <recore/plugin/plugin_manager.hpp>

namespace recore
{
    class Application
    {
    public:
        Application() = default;
        ~Application() = default;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void initialize(
            const std::filesystem::path& executable_path
        );

        void shutdown();

        const Config& get_config() const
        {
            return config_;
        }

        const std::filesystem::path& get_base_directory() const
        {
            return base_directory_;
        }

        PluginManager& get_plugin_manager()
        {
            return plugin_manager_;
        }

        const PluginManager& get_plugin_manager() const
        {
            return plugin_manager_;
        }

    private:
        Config config_;

        std::filesystem::path base_directory_;

        PluginManager plugin_manager_;

        bool initialized_ = false;
    };
}