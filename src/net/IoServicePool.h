#ifndef __IOS_POOL_H__
#define __IOS_POOL_H__

#include <boost/asio.hpp>
//#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
//#include <boost/serialization/singleton.hpp>
#include <boost/thread.hpp>

using IO_Context_Ptr = std::shared_ptr<boost::asio::io_service>;
using IO_WORK_PTR    = std::shared_ptr<boost::asio::io_service::work>;

namespace cncpp
{

    class IoServicePool // : public boost::serialization::singleton<IoServicePool>  //,
                        // boost::noncopyable
    {
    public:
        IoServicePool() {}
        ~IoServicePool() {}

        bool initPool(uint32_t size = std::thread::hardware_concurrency());

        bool start();
        void stop();

        IO_Context_Ptr &getIos();

    private:
        std::vector<IO_Context_Ptr> ios_pool_ = {};
        std::vector<IO_WORK_PTR>    work_vec_ = {};
        boost::thread_group         threads_;
        uint32_t                    pool_size_ = 0;
        uint32_t                    cur_used_  = 0;
    };
} // namespace cncpp

#endif