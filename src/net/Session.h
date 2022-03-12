#ifndef __SESSION_H__
#define __SESSION_H__

#include "Buffer.h"
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

//#include "NetAnalyze.h"
#include "SafeContainer.h"
#include <boost/asio/strand.hpp>

#include <list>

namespace cncpp
{
    class Session;

    using SOCKET_PTR  = std::shared_ptr<boost::asio::ip::tcp::socket>;
    using SESSION_PTR = std::shared_ptr<Session>;
    using IO_SERVICE  = boost::asio::io_context;

    enum class SessionState : uint16_t
    {
        eState_inValid = 0, // 无效
        eState_verify  = 1, // 待验证连接
        eState_ok      = 2, // 验证Ok
    };

    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        Session(IO_SERVICE &ios, const SOCKET_PTR &socket /*, const NetAnalyzePtr &net_analyze*/);
        // Session(const SOCKET_PTR &sock_ptr, const uint32_t &idx);
        ~Session() {}

        void setMsgCallBack(const std::function<void(const Message &)> cb)
        {
            msg_cb_ = cb;
        }

        void setDisconnectCB(std::function<void(const SESSION_PTR &)> cb)
        {
            disc_cb_ = cb;
        }

        void start();
        void processMsg();
        void onReceive(const BaseBufferPtr &buf);
        void onReceive(const char *buf, const uint32_t &size);
        void sendMsg(Message &&msg);
        void sendMsgDirect(const Message &msg);

        void setConnIdx(const uint32_t idx)
        {
            connect_index_ = idx;
        }
        uint32_t getConnIdx()
        {
            return connect_index_;
        }

        bool isStateOk()
        {
            return state == SessionState::eState_ok;
        }

        SOCKET_PTR getSocket()
        {
            return socket_;
        }

        std::string getRemoteAddr() const;
        uint16_t    getRemotePort() const;

    private:
        void parseRecivedMsg();
        void parseMessage();
        void doMsg(const Message &msg);
        void doMsg(Message &&msg);
        void sendMsgs();

    private:
        SOCKET_PTR                      socket_;
        uint32_t                        connect_index_ = 0;
        SessionState                    state          = SessionState::eState_inValid;
        boost::asio::io_context::strand strand_;
        // boost::asio::strand<boost::asio::io_context::executor_type> strand1_;

        MsgHeader cur_header_ = {};
        bool      head_ok_    = false;

        RingBuffer receive_list;

        SafeQueue<Message> send_list   = {};
        SafeQueue<Message> recive_list = {};

        std::function<void(const Message &)>     msg_cb_  = {};
        std::function<void(const SESSION_PTR &)> disc_cb_ = {};

        // 统计流量
        // NetAnalyzePtr net_analyze_ = nullptr;
    };
} // namespace cncpp

#endif