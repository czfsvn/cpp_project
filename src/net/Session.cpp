#include "Session.h"
//#include "Message.h"

//#include "MyLogging.h"
#include <iostream>

// Session::Session(const SOCKET_PTR &sock_pt, const uint32_t &idx) : socket_(sock_pt),
// connect_index_(idx), state(SessionState::eState_verify)
//{
//}
namespace cncpp
{

    Session::Session(IO_SERVICE &ios, const SOCKET_PTR &sock /*, const NetAnalyzePtr &net_analyze*/)
        : strand_(ios) // strand_(sock->get_executor().context())
                       //,strand1_(sock->get_executor())
        , socket_(sock)
        , state(SessionState::eState_verify)
    //, net_analyze_(net_analyze)
    {}

    std::string Session::getRemoteAddr() const
    {
        if (!socket_)
            return "InValid";

        return socket_->remote_endpoint().address().to_string();
    }

    uint16_t Session::getRemotePort() const
    {
        if (!socket_)
            return 0;

        return socket_->remote_endpoint().port();
    }

    void Session::start()
    {
        auto          self(shared_from_this());
        BaseBufferPtr buffer_ptr = receive_list.allocBuff();

        socket_->async_receive(boost::asio::buffer(buffer_ptr->begin(), buffer_ptr->getCapacity()),
            strand_.wrap([this, buffer_ptr, self](
                             const boost::system::error_code &ec, const std::size_t &size) {
                if (!ec && size)
                {
                    buffer_ptr->consume_wr(size);
                    onReceive(buffer_ptr);
                    // net_analyze_->recordRecvd(getConnIdx(), size);
                    start();
                }
                else
                {
                    if (ec.value() == boost::asio::error::eof)
                    {
                        // stop();
                        // LOG_INFO << "session closed";
                        if (disc_cb_)
                            disc_cb_(self);
                    }
                }
            }));
    }

    void Session::sendMsg(Message &&msg)
    {
#ifdef _MULTI_THREADS
        send_list.emplace_back(std::move(msg));
#else
        sendMsgDirect(msg);
#endif
    }

    void Session::onReceive(const BaseBufferPtr &buf)
    {
        // need lock if multithread
        receive_list.emplace_back(buf);
        parseMessage();
    }

    void Session::onReceive(const char *buf, const uint32_t &size)
    {
        receive_list.writeData(buf, size);
        parseMessage();
    }

    void Session::processMsg()
    {
        sendMsgs();

        parseRecivedMsg();
    }

    void Session::parseRecivedMsg()
    {
        while (!recive_list.empty())
        {
            const auto &msg = recive_list.try_pop();
            if (!msg)
                break;

            doMsg(*msg);
        }
    }

    void Session::sendMsgs()
    {
        while (!send_list.empty())
        {
            const auto &msg = recive_list.try_pop();
            if (!msg)
                break;

            sendMsgDirect(*msg);
        }
    }

    void Session::sendMsgDirect(const Message &msg)
    {
        auto self(shared_from_this());
        socket_->async_send(boost::asio::buffer(msg.begin(), msg.size()),
            strand_.wrap([this, self](const boost::system::error_code &ec, std::size_t size) {
                if (!ec && size)
                {
                    // net_analyze_->recordSend(getConnIdx(), size);
                }
                else
                {
                    std::cout << "[sendMsgs()] faild\n";
                }
            }));
    }

    void Session::parseMessage()
    {
        while (1)
        {
            if (!head_ok_)
            {
                // read head
                const uint32_t canReadSize = receive_list.canReadSize();
                if (canReadSize < HEADER_LEN)
                    break;

                uint32_t readed = receive_list.read(&cur_header_, HEADER_LEN);
                if (readed < HEADER_LEN)
                    break;

                head_ok_ = true;
            }

            if (!head_ok_)
                break;

            // read body
            const uint32_t canReadSize = receive_list.canReadSize();
            if (canReadSize < cur_header_.msg_len)
                break;

            Message msg(cur_header_.msg_len);
            if (cur_header_.msg_len != receive_list.read(msg, cur_header_.msg_len))
                break;

            msg.setHeader(cur_header_);
            head_ok_ = false;

#ifdef _MULTI_THREADS
            recive_list.emplace_back(std::move(msg));
#else
            doMsg(msg);
#endif
        }
    }

    void Session::doMsg(const Message &msg)
    {
        doMsg(std::move(msg));
    }

    void Session::doMsg(Message &&msg)
    {
        /*
        if (msg_cb_)
        {
            msg_cb_(msg);
        }
        else
        {
        }

        if (msg.getMsgCode() == 100)
        {
            msg::boost::Person p1(123456789012, 1, 123456789, "person1");

            const std::string &p2_str = msg.readBody();
            msg::boost::Person p2     = msg::deserialize<msg::boost::Person>(p2_str);

            assert(p2 == p1);
            // std::cout << " p1 vs p2 " << (p2 == p1 ? "same\n" : "different\n");
        }
        else
        {
            assert(false);
        }
        */
    }
} // namespace cncpp
