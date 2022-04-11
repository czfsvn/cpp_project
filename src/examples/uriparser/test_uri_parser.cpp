#include "URIParser.h"

namespace ns_test
{
    /*
    mysql://root:password@127.0.0.1

    mysql://@test.mysql.com:3306/db1?character_set=utf8&character_set_results=utf8

    mysqls://localhost/db1?character_set=big5

    // redis://:password@host:port/db_num
    // url = "redis://:admin@192.168.1.101:6001/3"
    // url = "redis://127.0.0.1:6379"
    */
    void test(const std::string& url)
    {
        ParsedURI uri;

        URIParser::parse(url, uri);

        return;
    }
    void main()
    {

        test("mysql://root:password@127.0.0.1");
        test("mysql://@test.mysql.com:3306/db1?character_set=utf8&character_set_results=utf8");
        test("mysqls://localhost/db1?character_set=big5");
        test("https://github.com/sogou/workflow/blob/master/src/include/workflow/MySQLMessage.inl");

        test("redis://:password@host:port/5");
        test("redis://:admin@192.168.1.101:6001/3");
        test("redis://127.0.0.1:6379");
    }
}  // namespace ns_test
namespace ns_uri
{
    void main()
    {
        ns_test::main();
    }
}  // namespace ns_uri