#ifndef __TIME_USEAGE_20220116_H__
#define __TIME_USEAGE_20220116_H__

#include <iostream>
#include <chrono>

namespace cncpp
{
    using namespace std::chrono;
    class TimeUseage
    {
    public:
        TimeUseage()
            : begin(high_resolution_clock::now())
        {}

        void reset()
        {
            begin = high_resolution_clock::now();
        }

        // Ĭ���������
        template<typename Duartion = milliseconds>
        int64_t elapsed() const
        {
            return duration_cast<Duartion>(high_resolution_clock::now() - begin).count();
        }

        // �������
        int64_t elapsed_nano() const
        {
            return elapsed<nanoseconds>();
        }
        // ���΢��
        int64_t elapsed_micro() const
        {
            return elapsed<microseconds>();
        }

        // �����
        int64_t elapsed_seconds() const
        {
            return elapsed<seconds>();
        }

        // �����
        int64_t elapsed_minutes() const
        {
            return elapsed<minutes>();
        }

        // ���ʱ
        int64_t elapsed_hours() const
        {
            return elapsed<hours>();
        }

    private:
        time_point<high_resolution_clock> begin = {};
    };

    class BlockCost
    {
    public:
        BlockCost(const char *func, uint32_t lines)
            : func_name(func)
            , line(lines)
        {}

        ~BlockCost()
        {
            std::cout << func_name << ":" << line << ", cost: [" << timer.elapsed_nano()
                      << "] nano sec" << std::endl;
        }

    private:
        std::string func_name = {};
        uint32_t    line      = 0;
        TimeUseage  timer     = {};
    };
} // namespace cncpp

#endif