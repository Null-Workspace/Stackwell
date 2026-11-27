#include <stackwell/core/application.hpp>
#include <stackwell/core/logger.hpp>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace stackwell
{
    void Application::initialize(const std::filesystem::path& executable_path)
    {
        if (initialized_) return;

        const auto absolute_executable = std::filesystem::absolute(executable_path);
        base_directory_ = absolute_executable.parent_path().parent_path();

        const auto config_path = base_directory_ / "resources" / "config" / "config.json";
        config_ = Config::load(config_path, base_directory_);

        std::filesystem::create_directories(config_.plugins.directory);
        std::filesystem::create_directories(config_.storage.directory);
        Logger::initialize(config_.logging);

        spdlog::info("Starting Stackwell {}", config_.application.version);
        spdlog::info("Distribution directory: {}", base_directory_.string());
        spdlog::info("Plugin directory: {}", config_.plugins.directory.string());

        initialized_ = true;
        plugin_manager_.load_all(config_.plugins.directory, config_.application.version);
    }

    void Application::shutdown()
    {
        if (!initialized_) return;
        spdlog::info("Stackwell shutting down");
        plugin_manager_.unload_all();
        Logger::shutdown();
        initialized_ = false;
    }
}
