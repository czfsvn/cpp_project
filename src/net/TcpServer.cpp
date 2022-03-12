#include "TcpServer.h"
#include <iostream>
#include <thread>

namespace cncpp
{

    TcpServer::TcpServer(IoServicePool &ios_pool, uint16_t port)
        : Listener(ios_pool, port)
    {
        // net_analyze = std::make_shared<NetAnalyze>("tcp server");
    }
    uint32_t TcpServer::generateConnIdx()
    {
        return ++session_idx_;
    }

    void TcpServer::onAccept(
        const IO_Context_Ptr &ios, const SOCKET_PTR sock, boost::system::error_code ec)
    {
        if (ec)
        {
            // LOG_ERROR << "TcpServer onAccept failed ec:" << ec << ", msg:" << ec.message()
            //          << ", thread_id : " << std::this_thread::get_id();
            return;
        }

        // LOG_INFO << "new conection, thread_id : " << std::this_thread::get_id()
        //         << " remote: " << sock->remote_endpoint().address() << ":"
        //         << sock->remote_endpoint().port();
        SESSION_PTR session = std::make_shared<Session>(*ios, sock /*, net_analyze*/);
        if (!session)
            return;

        session->setConnIdx(generateConnIdx());
        session->start();

        session->setDisconnectCB(std::bind(&TcpServer::onDisconnect, this, std::placeholders::_1));

        // LOG_INFO << "new conection: " << session->getConnIdx()
        //        << ", thread_id : " << std::this_thread::get_id();

        session_map_.emplace(session->getConnIdx(), session);
    }

    void TcpServer::processMsg()
    {
#ifdef _MULTI_THREADS
        session_map_.for_each([](const SafeMap<uint32_t, SESSION_PTR>::value_type &item) {
            if (item.second)
                item.second->processMsg();
        });
#else
        for (auto &item : session_map_)
        {
            if (item.second)
                item.second->processMsg();
        }
#endif

        // net_analyze->onTimer();
    }

    void TcpServer::onDisconnect(const SESSION_PTR &ptr)
    {
        if (!ptr)
            return;

        // LOG_INFO << "losed conection: " << ptr->getConnIdx()
        //         << ", thread_id : " << std::this_thread::get_id();
        session_map_.erase(ptr->getConnIdx());
    }

} // namespace cncpp
