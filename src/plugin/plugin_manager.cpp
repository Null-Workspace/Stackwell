
#include <stackwell/plugin/plugin_manager.hpp>
#include <stackwell/plugin/plugin_api.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef _WIN32
    #define NOMINMAX
    #include <Windows.h>
#else
    #include <dlfcn.h>
#endif

namespace stackwell
{
    namespace
    {
        constexpr std::size_t MAX_PLUGIN_NAME_LENGTH = 128;
        constexpr std::size_t MAX_PLUGIN_VERSION_LENGTH = 64;
        constexpr std::size_t MAX_PLUGIN_AUTHOR_LENGTH = 128;
        constexpr std::size_t MAX_PLUGIN_DESCRIPTION_LENGTH = 512;

        struct SemanticVersion
        {
            std::uint32_t major = 0;
            std::uint32_t minor = 0;
            std::uint32_t patch = 0;
        };

        std::optional<SemanticVersion> parse_version(
            const std::string& value
        )
        {
            SemanticVersion result;

            std::array<std::uint32_t*, 3> components{
                &result.major,
                &result.minor,
                &result.patch
            };

            std::size_t start = 0;

            for (std::size_t i = 0; i < components.size(); ++i)
            {
                const std::size_t separator = value.find('.', start);

                const std::size_t end =
                    separator == std::string::npos
                        ? value.size()
                        : separator;

                if (end == start)
                {
                    return std::nullopt;
                }

                const char* first = value.data() + start;
                const char* last = value.data() + end;

                const auto conversion = std::from_chars(
                    first,
                    last,
                    *components[i]
                );

                if (conversion.ec != std::errc{} ||
                    conversion.ptr != last)
                {
                    return std::nullopt;
                }

                if (i < components.size() - 1)
                {
                    if (separator == std::string::npos)
                    {
                        return std::nullopt;
                    }

                    start = separator + 1;
                }
                else if (separator != std::string::npos)
                {
                    return std::nullopt;
                }
            }

            return result;
        }

        bool is_version_less(
            const SemanticVersion& left,
            const SemanticVersion& right
        )
        {
            if (left.major != right.major)
            {
                return left.major < right.major;
            }

            if (left.minor != right.minor)
            {
                return left.minor < right.minor;
            }

            return left.patch < right.patch;
        }

        bool read_bounded_string(
            const char* source,
            std::size_t max_length,
            std::string& destination
        )
        {
            destination.clear();

            if (source == nullptr)
            {
                return false;
            }

            for (std::size_t i = 0; i <= max_length; ++i)
            {
                if (source[i] == '\0')
                {
                    destination.assign(source, i);
                    return true;
                }
            }

            return false;
        }

        bool is_valid_plugin_name(const std::string& name)
        {
            if (name.empty() ||
                name.size() > MAX_PLUGIN_NAME_LENGTH)
            {
                return false;
            }

            return std::all_of(
                name.begin(),
                name.end(),
                [](unsigned char c)
                {
                    return std::isalnum(c) ||
                        c == '_' ||
                        c == '-' ||
                        c == '.';
                }
            );
        }

        bool is_valid_plugin_version(const std::string& version)
        {
            if (version.empty() ||
                version.size() > MAX_PLUGIN_VERSION_LENGTH)
            {
                return false;
            }

            return std::all_of(
                version.begin(),
                version.end(),
                [](unsigned char c)
                {
                    return std::isprint(c) && !std::isspace(c);
                }
            );
        }

        bool is_valid_metadata_string(const std::string& value)
        {
            return std::all_of(
                value.begin(),
                value.end(),
                [](unsigned char c)
                {
                    return c == '\t' ||
                        (c >= 0x20 && c != 0x7F);
                }
            );
        }

        std::string normalize_plugin_name(std::string name)
        {
            std::transform(
                name.begin(),
                name.end(),
                name.begin(),
                [](unsigned char c)
                {
                    return static_cast<char>(std::tolower(c));
                }
            );

            return name;
        }

        bool has_duplicate_name(
            const std::vector<PluginInfo>& plugins,
            const std::string& name
        )
        {
            const auto normalized = normalize_plugin_name(name);

            return std::any_of(
                plugins.begin(),
                plugins.end(),
                [&normalized](const PluginInfo& plugin)
                {
                    return !plugin.name.empty() &&
                        normalize_plugin_name(plugin.name) == normalized;
                }
            );
        }

        bool descriptor_has_field(
            const StackwellPluginDescriptor* descriptor,
            std::size_t field_offset,
            std::size_t field_size
        )
        {
            return static_cast<std::size_t>(descriptor->struct_size) >=
                field_offset + field_size;
        }

        void STACKWELL_PLUGIN_CALL host_log_info(const char* message)
        {
            spdlog::info(
                "[Plugin] {}",
                message != nullptr ? message : "(null)"
            );
        }

        void STACKWELL_PLUGIN_CALL host_log_error(const char* message)
        {
            spdlog::error(
                "[Plugin] {}",
                message != nullptr ? message : "(null)"
            );
        }

        const StackwellHostApi HOST_API{
            STACKWELL_PLUGIN_API_VERSION,
            static_cast<std::uint32_t>(sizeof(StackwellHostApi)),
            host_log_info,
            host_log_error
        };

        std::string get_platform_error()
        {
#ifdef _WIN32
            return "Windows error code: " +
                std::to_string(GetLastError());
#else
            const char* error = dlerror();

            return error != nullptr
                ? error
                : "Unknown dynamic loader error";
#endif
        }

        bool is_plugin_file(const std::filesystem::path& path)
        {
#ifdef _WIN32
            std::string extension = path.extension().string();

            std::transform(
                extension.begin(),
                extension.end(),
                extension.begin(),
                [](unsigned char c)
                {
                    return static_cast<char>(std::tolower(c));
                }
            );

            return extension == ".dll";
#elif defined(__APPLE__)
            return path.extension() == ".dylib";
#else
            return path.extension() == ".so";
#endif
        }

        std::filesystem::path normalize_path(
            const std::filesystem::path& path
        )
        {
            std::error_code error;

            auto result = std::filesystem::weakly_canonical(
                path,
                error
            );

            if (!error)
            {
                return result;
            }

            error.clear();

            result = std::filesystem::absolute(path, error);

            if (!error)
            {
                return result.lexically_normal();
            }

            return path.lexically_normal();
        }
    }

    struct PluginManager::LoadedPlugin
    {
#ifdef _WIN32
        HMODULE handle = nullptr;
#else
        void* handle = nullptr;
#endif

        const StackwellPluginDescriptor* descriptor = nullptr;

        PluginInfo info;

        bool initialized = false;

        ~LoadedPlugin()
        {
            if (handle == nullptr)
            {
                return;
            }

#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
        }
    };

    PluginManager::PluginManager() = default;

    PluginManager::~PluginManager()
    {
        unload_all();
    }

    bool PluginManager::load_plugin(
        const std::filesystem::path& path,
        const std::string& host_version
    )
    {
        auto plugin = std::make_unique<LoadedPlugin>();

        plugin->info.path = normalize_path(path);
        plugin->info.state = PluginState::Discovered;

        const auto fail = [this, &plugin](const std::string& message)
        {
            plugin->info.state = PluginState::Failed;
            plugin->info.error = message;

            spdlog::error(
                "Plugin load failed: {}: {}",
                plugin->info.path.string(),
                message
            );

            plugins_.push_back(plugin->info);

            return false;
        };

#ifdef _WIN32
        plugin->handle = LoadLibraryW(
            plugin->info.path.c_str()
        );

        if (plugin->handle == nullptr)
        {
            return fail(get_platform_error());
        }

        auto query = reinterpret_cast<StackwellPluginQueryFn>(
            GetProcAddress(
                plugin->handle,
                "stackwell_plugin_query"
            )
        );

        if (query == nullptr)
        {
            return fail("Missing stackwell_plugin_query export");
        }
#else
        dlerror();

        plugin->handle = dlopen(
            plugin->info.path.c_str(),
            RTLD_NOW | RTLD_LOCAL
        );

        if (plugin->handle == nullptr)
        {
            return fail(get_platform_error());
        }

        dlerror();

        auto query = reinterpret_cast<StackwellPluginQueryFn>(
            dlsym(
                plugin->handle,
                "stackwell_plugin_query"
            )
        );

        if (const char* error = dlerror(); error != nullptr)
        {
            return fail(error);
        }

        if (query == nullptr)
        {
            return fail("Missing stackwell_plugin_query export");
        }
#endif

        try
        {
            plugin->descriptor = query();
        }
        catch (...)
        {
            return fail("Plugin query raised an exception");
        }

        if (plugin->descriptor == nullptr)
        {
            return fail("Plugin returned an empty descriptor");
        }

        const auto* descriptor = plugin->descriptor;

        constexpr std::size_t REQUIRED_DESCRIPTOR_SIZE =
            offsetof(StackwellPluginDescriptor, shutdown) +
            sizeof(StackwellPluginShutdownFn);

        if (static_cast<std::size_t>(descriptor->struct_size) <
            REQUIRED_DESCRIPTOR_SIZE)
        {
            return fail("Plugin descriptor is too small");
        }

        if (descriptor->api_version != STACKWELL_PLUGIN_API_VERSION)
        {
            return fail("Incompatible plugin API version");
        }

        if (descriptor->name == nullptr ||
            descriptor->version == nullptr ||
            descriptor->initialize == nullptr ||
            descriptor->shutdown == nullptr)
        {
            return fail("Plugin descriptor contains missing fields");
        }

        if (!read_bounded_string(
                descriptor->name,
                MAX_PLUGIN_NAME_LENGTH,
                plugin->info.name))
        {
            return fail("Plugin name is missing or too long");
        }

        if (!read_bounded_string(
                descriptor->version,
                MAX_PLUGIN_VERSION_LENGTH,
                plugin->info.version))
        {
            return fail("Plugin version is missing or too long");
        }

        plugin->info.api_version = descriptor->api_version;

        if (!is_valid_plugin_name(plugin->info.name))
        {
            return fail("Plugin name is invalid");
        }

        if (!is_valid_plugin_version(plugin->info.version))
        {
            return fail("Plugin version is invalid");
        }

        if (has_duplicate_name(plugins_, plugin->info.name))
        {
            return fail(
                "Another plugin with this name is already registered"
            );
        }

        if (descriptor_has_field(
                descriptor,
                offsetof(StackwellPluginDescriptor, author),
                sizeof(descriptor->author)))
        {
            if (descriptor->author != nullptr)
            {
                if (!read_bounded_string(
                        descriptor->author,
                        MAX_PLUGIN_AUTHOR_LENGTH,
                        plugin->info.author))
                {
                    return fail("Plugin author metadata is too long");
                }

                if (!is_valid_metadata_string(plugin->info.author))
                {
                    return fail("Plugin author metadata is invalid");
                }
            }
        }

        if (descriptor_has_field(
                descriptor,
                offsetof(StackwellPluginDescriptor, description),
                sizeof(descriptor->description)))
        {
            if (descriptor->description != nullptr)
            {
                if (!read_bounded_string(
                        descriptor->description,
                        MAX_PLUGIN_DESCRIPTION_LENGTH,
                        plugin->info.description))
                {
                    return fail("Plugin description metadata is too long");
                }

                if (!is_valid_metadata_string(plugin->info.description))
                {
                    return fail("Plugin description metadata is invalid");
                }
            }
        }

        if (descriptor_has_field(
                descriptor,
                offsetof(StackwellPluginDescriptor, min_host_version),
                sizeof(descriptor->min_host_version)))
        {
            if (descriptor->min_host_version != nullptr)
            {
                if (!read_bounded_string(
                        descriptor->min_host_version,
                        MAX_PLUGIN_VERSION_LENGTH,
                        plugin->info.min_host_version))
                {
                    return fail("Minimum host version is too long");
                }
            }
        }

        if (!plugin->info.min_host_version.empty())
        {
            const auto minimum_version = parse_version(
                plugin->info.min_host_version
            );

            if (!minimum_version.has_value())
            {
                return fail(
                    "Plugin minimum host version must use major.minor.patch"
                );
            }

            const auto current_version = parse_version(host_version);

            if (!current_version.has_value())
            {
                return fail("Host version is invalid");
            }

            if (is_version_less(
                    *current_version,
                    *minimum_version))
            {
                return fail(
                    "Host version " + host_version +
                    " is lower than required version " +
                    plugin->info.min_host_version
                );
            }
        }

        plugin->info.state = PluginState::Loaded;

        spdlog::info(
            "Plugin loaded: {} v{} (API v{})",
            plugin->info.name,
            plugin->info.version,
            plugin->info.api_version
        );

        try
        {
            const int result = descriptor->initialize(&HOST_API);

            if (result != 0)
            {
                return fail(
                    "Initialization returned code " +
                    std::to_string(result)
                );
            }
        }
        catch (...)
        {
            return fail("Plugin initialization raised an exception");
        }

        plugin->initialized = true;
        plugin->info.state = PluginState::Initialized;

        spdlog::info(
            "Plugin initialized: {}",
            plugin->info.name
        );

        plugins_.push_back(plugin->info);
        loaded_plugins_.push_back(std::move(plugin));

        return true;
    }

    void PluginManager::load_all(
        const std::filesystem::path& directory,
        const std::string& host_version
    )
    {
        if (shutting_down_)
        {
            spdlog::warn("Plugin manager is shutting down");
            return;
        }

        if (!parse_version(host_version).has_value())
        {
            spdlog::error(
                "Cannot load plugins: invalid host version '{}'",
                host_version
            );

            return;
        }

        std::error_code error;

        const auto normalized_directory =
            normalize_path(directory);

        if (!std::filesystem::exists(
                normalized_directory,
                error))
        {
            if (error)
            {
                spdlog::error(
                    "Unable to inspect plugin directory: {}",
                    error.message()
                );
            }
            else
            {
                spdlog::warn(
                    "Plugin directory does not exist: {}",
                    normalized_directory.string()
                );
            }

            return;
        }

        if (!std::filesystem::is_directory(
                normalized_directory,
                error))
        {
            spdlog::error(
                "Plugin path is not a directory: {}",
                normalized_directory.string()
            );

            return;
        }

        std::unordered_set<std::filesystem::path> known_paths;

        for (const auto& plugin : plugins_)
        {
            known_paths.insert(normalize_path(plugin.path));
        }

        std::vector<std::filesystem::path> candidates;

        std::filesystem::directory_iterator iterator(
            normalized_directory,
            error
        );

        const std::filesystem::directory_iterator end;

        if (error)
        {
            spdlog::error(
                "Unable to scan plugin directory: {}",
                error.message()
            );

            return;
        }

        while (iterator != end)
        {
            const auto path = iterator->path();

            std::error_code entry_error;

            const bool regular_file =
                iterator->is_regular_file(entry_error);

            if (entry_error)
            {
                spdlog::warn(
                    "Unable to inspect plugin candidate {}: {}",
                    path.string(),
                    entry_error.message()
                );
            }
            else if (regular_file && is_plugin_file(path))
            {
                candidates.push_back(normalize_path(path));
            }

            iterator.increment(error);

            if (error)
            {
                spdlog::error(
                    "Plugin directory scan interrupted: {}",
                    error.message()
                );

                return;
            }
        }

        std::sort(
            candidates.begin(),
            candidates.end()
        );

        for (const auto& path : candidates)
        {
            if (!known_paths.insert(path).second)
            {
                spdlog::debug(
                    "Skipping already processed plugin: {}",
                    path.string()
                );

                continue;
            }

            load_plugin(path, host_version);
        }

        spdlog::info(
            "Plugin scan completed. Initialized: {}",
            get_loaded_count()
        );
    }

    void PluginManager::unload_all()
    {
        if (shutting_down_)
        {
            return;
        }

        shutting_down_ = true;

        for (
            auto iterator = loaded_plugins_.rbegin();
            iterator != loaded_plugins_.rend();
            ++iterator
        )
        {
            auto& plugin = **iterator;

            if (!plugin.initialized ||
                plugin.descriptor == nullptr)
            {
                continue;
            }

            spdlog::info(
                "Stopping plugin: {}",
                plugin.info.name
            );

            try
            {
                plugin.descriptor->shutdown();
            }
            catch (...)
            {
                spdlog::error(
                    "Plugin shutdown raised an exception: {}",
                    plugin.info.name
                );
            }

            plugin.initialized = false;
        }

        loaded_plugins_.clear();

        spdlog::info("All plugins unloaded");
    }

    const std::vector<PluginInfo>&
    PluginManager::get_plugins() const
    {
        return plugins_;
    }

    std::size_t PluginManager::get_loaded_count() const
    {
        return loaded_plugins_.size();
    }
}
