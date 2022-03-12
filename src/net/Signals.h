#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <boost/asio.hpp>
#include <signal.h>

namespace cncpp
{
    class Signals
    {
        void init(boost::asio::io_context &ioc) {}

    private:
        boost::asio::signal_set *signals;
    };

} // namespace cncpp

#endif