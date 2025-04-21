#ifndef _UTIL_LOG_H_
#define _UTIL_LOG_H_

#include "spdlog/spdlog.h"

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"

inline void init_log() {
    // default just console logger

    spdlog::set_level(spdlog::level::info);
    // spdlog::set_pattern("[%H:%M:%s] [%n] [thread %t] %v");
}

inline void init_console_log() {

    // logger name "console" will be write into log line like '[console]'
    auto console = spdlog::stdout_logger_mt("console");
    // console->set_pattern("%^[%T] %n: %v%$");
    console->set_level(spdlog::level::info);
    spdlog::set_default_logger(console);
}

inline void init_rotating_file_log() {

    auto max_size = 1048576 * 5;
    auto max_files = 3;
    auto logger = spdlog::rotating_logger_mt("some_logger_name", "logs/rotating.txt", max_size, max_files);
    spdlog::flush_every(std::chrono::seconds(1));
    spdlog::set_default_logger(logger);
}

inline void init_daily_file_log() {

    auto logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
    spdlog::flush_every(std::chrono::seconds(1));
    spdlog::set_default_logger(logger);
}

template <typename... Args>
inline void info_log(const char* format, Args &&...args) {
    spdlog::info(format, args...);
}

template <typename... Args>
inline void err_log(const char* format, Args &&...args) {
    spdlog::error(format, args...);
}

#endif