#include "EventCallback.h"
#include "Properties.h"
#include "StringUtil.h"
#include "TypeDefine.h"
#include <iostream>
#include <list>
#include <vector>

namespace ns_test1
{
    void test1()
    {
        std::string str1 = "123";
        std::string str2 = "-123";
        std::string str3 = "1111111111123";
        std::string str4 = "-11111111123";
        std::string str5 = "1.23";
        std::string str6 = "-1.23";
        std::string str7 = "hello,world";

        const uint8_t  ui_8   = cncpp::cast<uint8_t>(str1);
        const int8_t   i_8    = cncpp::cast<int8_t>(str1);
        const uint16_t ui_16  = cncpp::cast<uint16_t>(str2);
        const int16_t  i_16   = cncpp::cast<int16_t>(str2);
        const uint16_t ui16_2 = cncpp::cast<uint16_t>(str3);
        const uint32_t ui32_1 = cncpp::cast<uint32_t>(str3);
        const uint64_t ui64_1 = cncpp::cast<uint64_t>(str3);

        const int16_t ui16_3 = cncpp::cast<int16_t>(str3);
        const int32_t ui32_3 = cncpp::cast<int32_t>(str3);
        const int64_t ui64_3 = cncpp::cast<int64_t>(str3);

        const float f1 = cncpp::cast<float>(str5);
        const float f2 = cncpp::cast<float>(str6);

        const double d1 = cncpp::cast<double>(str5);
        const double d2 = cncpp::cast<double>(str6);

        auto v1 = cncpp::cast<int64_t>(str7);
        auto v2 = cncpp::cast<int32_t>(str7);
        auto v3 = cncpp::cast<int16_t>(str7);
        auto v4 = cncpp::cast<int8_t>(str7);

        auto v11 = cncpp::cast<uint64_t>(str7);
        auto v12 = cncpp::cast<uint32_t>(str7);
        auto v13 = cncpp::cast<uint16_t>(str7);
        auto v14 = cncpp::cast<uint8_t>(str7);

        auto fs1 = cncpp::cast<float>(str7);
        auto ds1 = cncpp::cast<double>(str7);

        return;
    }

    void test2()
    {
        cncpp::Properties prop;
        // const std::striing line = "jump op=abd act=cddd value=100";
        prop.parseCmdLine("jump op=abd act=cddd value=100");
        std::cout << "jump=" << prop["jump"] << std::endl;
        std::cout << "op=" << prop["op"] << std::endl;
        std::cout << "act=" << prop["act"] << std::endl;
        std::cout << "value=" << prop["value"] << std::endl;
        std::cout << "cddd=" << prop["cddd"] << std::endl;
        return;
    }

    void test_transform()
    {
        std::string hell = "Hello, WorldGame";
        cncpp::to_upper(hell);
        std::cout << "toupper: " << hell << std::endl;

        cncpp::to_lower(hell);
        std::cout << "to_lower: " << hell << std::endl;

        std::string res = cncpp::replace_all(hell, "l", "TTT");
        std::cout << "replace_all: " << res << std::endl;
    }
}  // namespace ns_test1

namespace test_template
{
    template <typename T, template <typename U> typename Cont>
    class TestTemplate
    {
    public:
        using DATE_TYPE = T;
        Cont<DATE_TYPE> cont;
    };

    template <typename T>
    class Test
    {
    private:
        T t;
    };

    template <typename T>
    using ListT = std::list<T, std::allocator<T>>;

    template <typename T>
    using VectorT = std::vector<T, std::allocator<T>>;

    void test()
    {
        TestTemplate<std::string, Test> aabb;

        // TestTemplate<std::string, std::list> aabbcc;
        TestTemplate<std::string, ListT>   aabbccdd;
        TestTemplate<std::string, VectorT> aabbccee;
    }
}  // namespace test_template
namespace ns_typeconvert
{
    void main()
    {
        ns_test1::test1();
        ns_test1::test2();
        ns_test1::test_transform();
    }
}  // namespace ns_typeconvert