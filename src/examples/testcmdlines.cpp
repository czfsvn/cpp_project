#include "boost_asio.h"
#include "cmdline.h"
#include <iostream>

using namespace std;

cmdline::parser a;

namespace test_cmdline
{
    int cmdline_main(int argc, char* argv[])
    {
        a.add<string>("host", 'h', "host name", true, "");
        a.add<int>("port", 'p', "port number", false, 80, cmdline::range(1, 65535));
        a.add<string>("type", 't', "protocol type", false, "http",
                      cmdline::oneof<string>("http", "https", "ssh", "ftp"));
        a.add("gzip", '\0', "gzip when transfer");

        a.parse_check(argc, argv);

        cout << a.get<string>("type") << "://" << a.get<string>("host") << ":" << a.get<int>("port") << endl;

        if (a.exist("gzip"))
            cout << "gzip" << endl;
        return 0;
    }
}  // namespace test_cmdline

namespace ns_testcmdline
{
    void main(int argc, char** argv)
    {
        test_cmdline::cmdline_main(argc, argv);
    }
}  // namespace ns_testcmdline