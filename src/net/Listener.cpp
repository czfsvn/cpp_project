#include "Listener.h"

#include <boost/bind/bind.hpp>
#include <iostream>

namespace cncpp
{

    Listener::Listener(IoServicePool &pool, uint16_t port)
        : ios_pool(pool)
        , acceptor_(
              *pool.getIos(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    {
        // startAccept();
    }

    void Listener::startAccept()
    {
        std::cout << "server start listen on port: 9999\n";
#if 0
    acceptor_.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket sock) {
        this->onAccept(std::move(sock), ec);
        this->startAccept();
    });
#endif

        const auto ios  = ios_pool.getIos();
        SOCKET_PTR sock = std::make_shared<boost::asio::ip::tcp::socket>(*ios);
        acceptor_.async_accept(*sock, [this, sock, ios](const boost::system::error_code &err) {
            // boost::asio::io_context
            onAccept(ios, sock, err);
            startAccept();
        });
    }

    void Listener::handleAcpErr(SOCKET_PTR sock, const boost::system::error_code &err)
    {
        boost::system::error_code ec;
        sock->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        sock->close(ec);
        stopAccept();
    }

    void Listener::stopAccept()
    {
        boost::system::error_code err;
        acceptor_.cancel(err);
        acceptor_.close(err);
    }
} // namespace cncpp
