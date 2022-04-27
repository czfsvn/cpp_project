#include "Global.h"

namespace cncpp
{
    // std::shared_ptr<spdlog::logger> dailylogger = nullptr;
    // Random rnd(0xdeadbeef);
    Random rnd(time(NULL));

    uint32_t random()
    {
        static Random rnd_(0xdeadbeef);
        return rnd_.Next();
    }
    /*
        void globle_init(const std::string& logwords)
        {

            if (!dailylogger)
            {
                dailylogger = spdlog::daily_logger_mt(logwords, "logs/daily.txt", 2, 30);
                dailylogger->set_level(spdlog::level::trace);
                dailylogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
            }

    }*/
}  // namespace cncpp