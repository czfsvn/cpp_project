#include <iostream>

#include "AppServer.h"

int main()
{
    AppServer::getMe().init();
    // AppServer server;
    // server.init();
    std::cout << "tcpserver main\n";
    return 0;
}