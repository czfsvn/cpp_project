#ifndef __GLOBAL_20220116_H__
#define __GLOBAL_20220116_H__

#include "Random.h"

namespace cncpp
{
    extern uint32_t random();
    extern Random   rnd; //(0xdeadbeef);

    // extern std::shared_ptr<spdlog::logger> dailylogger;
    //= nullptr;
    // spdlog::daily_logger_mt("test", "logs/daily.txt", 2, 30);

    // extern void globle_init(const std::string& logwords);
    /*
    {
        dailylogger = spdlog::daily_logger_mt(logwords, "logs/daily.txt", 2, 30);
        dailylogger->set_level(spdlog::level::trace);
        dailylogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    }
    */

} // namespace cncpp
#endif