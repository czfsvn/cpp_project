#pragma once

#include <iostream>
#include <map>

namespace cncpp
{
    template <typename Func>
    class EventCallBack
    {
    public:
        EventCallBack()  = default;
        ~EventCallBack() = default;

        EventCallBack(const EventCallBack&) = delete;
        EventCallBack& operator=(const EventCallBack&) = delete;

        uint32_t reg(Func&& f)
        {
            return assign(f);
        }

        uint32_t reg(const Func& f)
        {
            return assign(f);
        }

        void unreg(uint32_t key)
        {
            cb_map_.erase(key);
        }

        template <typename... Args>
        void call(Args&&... args)
        {
            for (auto& item : cb_map_)
            {
                if (item.second)
                    item.second(std::forward<Args>(args)...);
            }
        }

    private:
        template <typename F>
        uint32_t assign(F&& f)
        {
            cb_map_.emplace(++index_, std::forward<F>(f));
            return index_;
        }

    private:
        uint32_t                 index_  = 0;
        std::map<uint32_t, Func> cb_map_ = {};
    };
}  // namespace cncpp