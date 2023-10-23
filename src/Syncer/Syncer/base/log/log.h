#pragma once
#include <memory>

#include <spdlog/spdlog.h>

namespace Syncer {

class Logger {
public:
    Logger();
    ~Logger();

    void inititalize();
    void drop();

    std::shared_ptr<spdlog::logger> core_logger;
};

extern Logger logger;

} // namespace BoxShooting

#define LOG_TRACE(...) ::Syncer::logger.core_logger->trace(__VA_ARGS__)
#define LOG_DEBUG(...) ::Syncer::logger.core_logger->debug(__VA_ARGS__)
#define LOG_INFO(...) ::Syncer::logger.core_logger->info(__VA_ARGS__)
#define LOG_WARN(...) ::Syncer::logger.core_logger->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::Syncer::logger.core_logger->error(__VA_ARGS__)