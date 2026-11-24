#pragma once

#include <memory>

#include <recore/core/config.hpp>

#include <spdlog/logger.h>

namespace recore
{
    class Logger
    {
    public:
        static void initialize(const LoggingConfig& config);
        static void shutdown();

        static std::shared_ptr<spdlog::logger> get();
    };
}