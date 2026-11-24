#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

namespace recore
{
    struct ApplicationConfig
    {
        std::string name = "ReCore";
        std::string version = "0.0.01";
    };

    struct LoggingConfig
    {
        std::string level = "info";
        std::filesystem::path file = "logs/recore.log";

        std::size_t max_size_mb = 5;
        std::size_t max_files = 3;
    };

    struct PluginConfig
    {
        std::filesystem::path directory = "plugins";
    };

    struct StorageConfig
    {
        std::filesystem::path directory = "data";
    };

    class Config
    {
    public:
        static Config load(
            const std::filesystem::path& config_path,
            const std::filesystem::path& base_directory
        );

        ApplicationConfig application;
        LoggingConfig logging;
        PluginConfig plugins;
        StorageConfig storage;
    };
}