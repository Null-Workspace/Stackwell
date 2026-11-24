#include <CLI/CLI.hpp>
#include <cstdint>
#include <stackwell/core/application.hpp>
#include <stackwell/core/logger.hpp>
#include <stackwell/commands/command_registry.hpp>
#include <string>
#include <exception>
#include <iostream>
#include <filesystem>
#include <vector>

#ifdef _WIN32
    #define NOMINMAX
    #include <Windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#else
    #include <unistd.h>
#endif

namespace
{
    std::filesystem::path get_executable_path(
        [[maybe_unused]] const char* argv0
    )
    {
#ifdef _WIN32
        std::vector<wchar_t> buffer(32768);

        const DWORD length = GetModuleFileNameW(
            nullptr,
            buffer.data(),
            static_cast<DWORD>(buffer.size())
        );

        if (length == 0 || length >= buffer.size())
        {
            throw std::runtime_error(
                "Unable to determine executable path"
            );
        }

        return std::filesystem::path(
            std::wstring(buffer.data(), length)
        );

#elif defined(__APPLE__)
        std::uint32_t size = 0;

        _NSGetExecutablePath(nullptr, &size);

        std::vector<char> buffer(size);

        if (_NSGetExecutablePath(buffer.data(), &size) != 0)
        {
            throw std::runtime_error(
                "Unable to determine executable path"
            );
        }

        return std::filesystem::weakly_canonical(buffer.data());

#elif defined(__linux__)
        std::vector<char> buffer(4096);

        const auto length = readlink(
            "/proc/self/exe",
            buffer.data(),
            buffer.size() - 1
        );

        if (length < 0)
        {
            throw std::runtime_error(
                "Unable to determine executable path"
            );
        }

        buffer[static_cast<std::size_t>(length)] = '\0';

        return std::filesystem::path(buffer.data());

#else
        return std::filesystem::absolute(argv0);
#endif
    }
}

int main(int argc, char* argv[])
{
    stackwell::Application application;

    try
    {
        const auto executable_path =
            get_executable_path(argv[0]);

        application.initialize(executable_path);

        CLI::App cli{
            "Stackwell - Extensible System Administration Platform"
        };

        stackwell::register_commands(
    cli,
    application.get_plugin_manager()
);
        try
        {
            cli.parse(argc, argv);
        }
        catch (const CLI::ParseError& error)
        {
            const int exit_code = cli.exit(error);

            application.shutdown();

            return exit_code;
        }

        application.shutdown();

        return 0;
    }
    catch (const std::exception& error)
    {
        if (auto logger = stackwell::Logger::get())
        {
            logger->critical(
                "Unhandled exception: {}",
                error.what()
            );
        }
        else
        {
            std::cerr << "Fatal error: "
                      << error.what()
                      << '\n';
        }

        application.shutdown();

        return 1;
    }
}