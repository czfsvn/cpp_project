/*
 * @Author: czf
 * @Date: 2022-01-29 10:22:18
 * @LastEditors: czf
 * @LastEditTime: 2022-02-18 19:24:13
 * @FilePath: \cpp_project2022\src\base\logger.h
 * @Description:
 *
 * Copyright (c) 2022 by 用户/公司名, All Rights Reserved.
 */
#ifndef __LOGGER_20220128_H__
#define __LOGGER_20220128_H__

#include "Singleton.h"

#include "spdlog/cfg/env.h"   // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h"  // support for user defined types
#include "spdlog/logger.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/hourly_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
/*
#define TRACE    cncpp::dailylogger->trace    // 0
#define DEBUG    cncpp::dailylogger->debug    // 1
#define INFO     cncpp::dailylogger->info     // 2
#define WARN     cncpp::dailylogger->warn     // 3
#define ERR      cncpp::dailylogger->error    // 4
#define CRITICAL cncpp::dailylogger->critical // 5
*/

#ifdef SPDLOG

#define TRACE    cncpp::TinyLogger::getMe().log()->trace     // 0
#define DEBUG    cncpp::TinyLogger::getMe().log()->debug     // 1
#define INFO     cncpp::TinyLogger::getMe().log()->info      // 2
#define WARN     cncpp::TinyLogger::getMe().log()->warn      // 3
#define ERR      cncpp::TinyLogger::getMe().log()->error     // 4
#define CRITICAL cncpp::TinyLogger::getMe().log()->critical  // 5

#else

#define TRACE(fmt, ...)
#define DEBUG(fmt, ...)
#define INFO(fmt, ...)
#define WARN(fmt, ...)
#define ERR(fmt, ...)
#define CRITICAL(fmt, ...)

#endif

namespace cncpp
{
    class TinyLogger : public Singleton<TinyLogger>
    {
    public:
        TinyLogger()
        {
            init();
        }

        ~TinyLogger() {}

        void init()
        {
            // daily_logger = spdlog::daily_logger_mt("tinylogger", "logs/daily.txt", 2, 30);
            // console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            hour_logger = spdlog::hourly_logger_mt("tinylogger", "logs/hour.log");
            hour_logger->set_level(spdlog::level::trace);
            hour_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
#ifdef SPDDEBUG
            hour_logger->flush_on(spdlog::level::trace);
#endif

            // tinylogger = std::make_shared<spdlog::logger>("multi_sink", hour_logger);
        }

        std::shared_ptr<spdlog::logger> log()
        {
            // return tinylogger;
            return hour_logger;
        }

    private:
        std::shared_ptr<spdlog::logger> hour_logger = nullptr;
        // std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink = nullptr;
        std::shared_ptr<spdlog::logger> tinylogger = nullptr;
    };
}  // namespace cncpp

#endif