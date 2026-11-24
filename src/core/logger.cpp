#include <recore/core/logger.hpp>

#include <filesystem>
#include <memory>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace recore
{
    namespace
    {
        constexpr const char* LOGGER_NAME = "recore";
        constexpr std::size_t BYTES_IN_MB = 1024 * 1024;
    }

    void Logger::initialize(const LoggingConfig& config)
    {
        const std::filesystem::path log_path = config.file;

        if (!log_path.parent_path().empty())
        {
            std::filesystem::create_directories(
                log_path.parent_path()
            );
        }

        auto console_sink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        auto file_sink =
            std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                log_path.string(),
                config.max_size_mb * BYTES_IN_MB,
                config.max_files
            );

        const auto level = spdlog::level::from_str(config.level);

        console_sink->set_level(level);
        file_sink->set_level(level);

        std::vector<spdlog::sink_ptr> sinks{
            console_sink,
            file_sink
        };

        auto logger = std::make_shared<spdlog::logger>(
            LOGGER_NAME,
            sinks.begin(),
            sinks.end()
        );

        logger->set_level(level);
        logger->flush_on(spdlog::level::warn);

        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);

        logger->info("Logger initialized");
        logger->info("Log file: {}", log_path.string());
        logger->info("Log level: {}", config.level);
    }

    std::shared_ptr<spdlog::logger> Logger::get()
    {
        return spdlog::get(LOGGER_NAME);
    }

    void Logger::shutdown()
    {
        auto logger = get();

        if (logger)
        {
            logger->info("Logger shutting down");
            logger->flush();
        }

        spdlog::shutdown();
    }
}