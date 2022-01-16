#ifndef __MISC_20220116_H__
#define __MISC_20220116_H__

#include <thread>

#define SAFE_SUB(x, y) ((x) > (y) ? (x - y) : 0)

#define SAFE_DELETE(x)                                                                             \
    {                                                                                              \
        if (x)                                                                                     \
        {                                                                                          \
            delete (x);                                                                            \
            (x) = nullptr;                                                                         \
        }                                                                                          \
    }

#define SAFE_DELETE_ARR(x)                                                                         \
    {                                                                                              \
        if (x)                                                                                     \
        {                                                                                          \
            delete[](x);                                                                           \
            (x) = nullptr;                                                                         \
        }                                                                                          \
    }

namespace cncpp
{
    void inline sleepfor_seconds(const uint32_t sec)
    {
        std::this_thread::sleep_for(std::chrono::seconds(sec));
    }

    void inline sleepfor_microseconds(const uint32_t micr_sec)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(micr_sec));
    }

    void inline sleepfor_milliseconds(const uint32_t mill_sec)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(mill_sec));
    }

    const std::thread::id inline getThreadId()
    {
        return std::this_thread::get_id();
    }
    // 支持普通指针
    template<typename T, typename... Args>
    inline typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type make_unique(
        Args &&...args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    // 支持动态数组
    template<class T>
    inline typename std::enable_if<std::is_array<T>::value && std::extent<T>::value == 0,
        std::unique_ptr<T>>::type
    make_unique(size_t size)
    {
        typedef typename std::remove_extent<T>::type U;
        return std::unique_ptr<T>(new U[size]());
    }

    // 过滤掉定长数组的情况
    template<class T, class... Args>
    typename std::enable_if<std::extent<T>::value != 0, void>::type make_unique(
        Args &&...) = delete;
} // namespace cn

#endif
