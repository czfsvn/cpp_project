#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include "Listener.h"
#include "Session.h"

namespace cncpp
{
    class TcpServer : public Listener
    {
    public:
        TcpServer(IoServicePool &ios_pool, uint16_t port); // : Listener(ios_pool, port) {}
        virtual ~TcpServer() {}

        virtual void onAccept(const IO_Context_Ptr &ios, const SOCKET_PTR sock,
            boost::system::error_code ec) override;

        uint32_t generateConnIdx();
        void     processMsg();
        void     onDisconnect(const SESSION_PTR &ptr);

    private:
        uint32_t session_idx_ = 0;
#ifdef _MULTI_THREADS
        SafeMap<uint32_t, SESSION_PTR> session_map_ = {};
#else
        std::map<uint32_t, SESSION_PTR> session_map_ = {};
#endif
        // NetAnalyzePtr net_analyze = nullptr;
    };

} // namespace cncpp

#endif