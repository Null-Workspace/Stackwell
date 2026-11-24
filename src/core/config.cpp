#include <recore/core/config.hpp>

#include <fstream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

namespace recore
{
    namespace
    {
        using json = nlohmann::json;

        void validate_section(
            const json& root,
            const char* section_name
        )
        {
            if (!root.contains(section_name))
            {
                return;
            }

            if (!root.at(section_name).is_object())
            {
                throw std::runtime_error(
                    std::string("Config section must be an object: ") +
                    section_name
                );
            }
        }

        std::filesystem::path resolve_path(
            const std::filesystem::path& base,
            const std::filesystem::path& value
        )
        {
            if (value.is_absolute())
            {
                return value.lexically_normal();
            }

            return (base / value).lexically_normal();
        }
    }

    Config Config::load(
        const std::filesystem::path& config_path,
        const std::filesystem::path& base_directory
    )
    {
        std::ifstream file(config_path);

        if (!file.is_open())
        {
            throw std::runtime_error(
                "Unable to open config file: " +
                config_path.string()
            );
        }

        json root;

        try
        {
            file >> root;
        }
        catch (const json::exception& error)
        {
            throw std::runtime_error(
                std::string("Invalid config JSON: ") + error.what()
            );
        }

        if (!root.is_object())
        {
            throw std::runtime_error(
                "Config root must be a JSON object"
            );
        }

        validate_section(root, "application");
        validate_section(root, "logging");
        validate_section(root, "plugins");
        validate_section(root, "storage");

        Config config;

        if (root.contains("application"))
        {
            const auto& section = root.at("application");

            config.application.name =
                section.value("name", config.application.name);

            config.application.version =
                section.value("version", config.application.version);
        }

        if (root.contains("logging"))
        {
            const auto& section = root.at("logging");

            config.logging.level =
                section.value("level", config.logging.level);

            config.logging.file =
                section.value("file", config.logging.file.string());

            config.logging.max_size_mb =
                section.value("max_size_mb", config.logging.max_size_mb);

            config.logging.max_files =
                section.value("max_files", config.logging.max_files);
        }

        if (root.contains("plugins"))
        {
            const auto& section = root.at("plugins");

            config.plugins.directory =
                section.value(
                    "directory",
                    config.plugins.directory.string()
                );
        }

        if (root.contains("storage"))
        {
            const auto& section = root.at("storage");

            config.storage.directory =
                section.value(
                    "directory",
                    config.storage.directory.string()
                );
        }

        if (config.application.name.empty())
        {
            throw std::runtime_error(
                "Application name cannot be empty"
            );
        }

        if (config.application.version.empty())
        {
            throw std::runtime_error(
                "Application version cannot be empty"
            );
        }

        if (config.logging.max_size_mb == 0 ||
            config.logging.max_size_mb > 1024)
        {
            throw std::runtime_error(
                "logging.max_size_mb must be between 1 and 1024"
            );
        }

        if (config.logging.max_files == 0 ||
            config.logging.max_files > 100)
        {
            throw std::runtime_error(
                "logging.max_files must be between 1 and 100"
            );
        }

        if (config.logging.level != "trace" &&
            config.logging.level != "debug" &&
            config.logging.level != "info" &&
            config.logging.level != "warn" &&
            config.logging.level != "error" &&
            config.logging.level != "critical" &&
            config.logging.level != "off")
        {
            throw std::runtime_error(
                "Invalid logging.level: " + config.logging.level
            );
        }

        config.logging.file = resolve_path(
            base_directory,
            config.logging.file
        );

        config.plugins.directory = resolve_path(
            base_directory,
            config.plugins.directory
        );

        config.storage.directory = resolve_path(
            base_directory,
            config.storage.directory
        );

        return config;
    }
}