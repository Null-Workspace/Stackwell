#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <recore/plugin/plugin_info.hpp>

namespace recore
{
    class PluginManager
    {
    public:
        PluginManager();

        ~PluginManager();

        PluginManager(const PluginManager&) = delete;
        PluginManager& operator=(const PluginManager&) = delete;

        PluginManager(PluginManager&&) = delete;
        PluginManager& operator=(PluginManager&&) = delete;

        void load_all(
            const std::filesystem::path& directory,
            const std::string& host_version
        );

        void unload_all();

        const std::vector<PluginInfo>& get_plugins() const;

        std::size_t get_loaded_count() const;

    private:
        struct LoadedPlugin;

        bool load_plugin(
            const std::filesystem::path& path,
            const std::string& host_version
        );

        std::vector<PluginInfo> plugins_;

        std::vector<std::unique_ptr<LoadedPlugin>> loaded_plugins_;

        bool shutting_down_ = false;
    };
}