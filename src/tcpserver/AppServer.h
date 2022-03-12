#ifndef __APP_SERVER_20220116_H__
#define __APP_SERVER_20220116_H__

#include "Singleton.h"

#include "TcpServer.h"

class AppServer : public cncpp::Singleton<AppServer>
{
public:
    AppServer();
    ~AppServer();

    void init();

private:
    cncpp::TcpServer* server;
};

#endif