#ifndef __LISTENER_H__
#define __LISTENER_H__

#include "Session.h"
#include <boost/asio.hpp>

#include "IoServicePool.h"

using SOCKET_PTR = std::shared_ptr<boost::asio::ip::tcp::socket>;

namespace cncpp
{
    class Listener
    {
    public:
        Listener(IoServicePool &pool, uint16_t port);
        virtual ~Listener() {}

        void startAccept();

        virtual void onAccept(
            const IO_Context_Ptr &ios, const SOCKET_PTR sock, boost::system::error_code ec) = 0;

        void handleAcpErr(SOCKET_PTR sock, const boost::system::error_code &err);
        void stopAccept();

    private:
        boost::asio::ip::tcp::acceptor acceptor_;
        IoServicePool &                ios_pool;
    };
} // namespace cncpp

#endif
