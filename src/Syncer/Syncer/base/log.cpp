#include "log.h"

#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Syncer {

Logger::Logger() {};

Logger::~Logger() {
    if (core_logger){
        LOG_ERROR("不要忘了在退出时关闭日志");
        assert(false);//需要先手动调用drop释放logger
    }
};

void Logger::inititalize() {

    auto file_detail_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log_detail.txt");
    file_detail_sink->set_level(spdlog::level::level_enum::trace);
    file_detail_sink->set_pattern("%^[%T][%n][%l] %v%$");

    auto file_warn_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log_warn.txt");
    file_warn_sink->set_level(spdlog::level::level_enum::warn);
    file_warn_sink->set_pattern("%^[%T][%n][%l] %v%$");

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
#ifdef NDEBUG
    console_sink->set_level(spdlog::level::level_enum::info);
#else
    console_sink->set_level(spdlog::level::level_enum::trace);
#endif
    console_sink->set_pattern("%^[%T][%n][%l] %v%$");

    spdlog::init_thread_pool(10000, 1);

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_detail_sink, file_warn_sink};
    core_logger = std::make_shared<spdlog::async_logger>("core", sinks.begin(), sinks.end(),
                                                         spdlog::thread_pool());

    core_logger->set_level(spdlog::level::level_enum::trace);

    spdlog::register_logger(core_logger);
};

void Logger::drop(){
    core_logger->flush();
    core_logger.reset();
    spdlog::drop_all(); 
}

//全局logger
Logger logger;
} // namespace BoxShooting