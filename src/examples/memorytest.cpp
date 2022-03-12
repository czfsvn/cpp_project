#include "MemoryPool.h"
#include "SimpleMemoryPool.h"
#include "TimerUsage.h"
#include "cmdline.h"
#include "logger.h"
#include <iostream>

#include <boost/pool/object_pool.hpp>

using namespace std;

namespace Test
{
    class TestBase1
    {
    public:
        TestBase1()
        {
            DEBUG("TestBase1 default");
        }

        TestBase1(uint32_t i) : i32_value(i), i64_value(i)
        {
            DEBUG("TestBase1 construct: i32_value={}, this={}", i32_value, ADDR_TO_UINT64(this));
        }
        ~TestBase1()
        {
            DEBUG("TestBase1 destruct: i32_value={}, this={}", i32_value, ADDR_TO_UINT64(this));
        }

        void Print() const
        {
            DEBUG(
                "TestBase1 print: i32_value={}, i64_value={}, this={}", i32_value, i64_value,
                ADDR_TO_UINT64(this));
        }

    private:
        uint32_t i32_value = 100;
        char     c_value   = '3';
        uint64_t i64_value = 101;
        // std::string str_valu1 = "abdafdfasdfasdfasdfasdfsdfsdfasdfasdfas";
        // std::string str_valu2 = "abdafdfasdfasdfasdfasdfsdfsdfasdfasdfas";
    };

    void test_index()
    {
        for (uint32_t idx = 0; idx < 200; idx++)
        {
            std::cout << "idx :" << idx << "--" << cncpp::Index(idx) << std::endl;
            DEBUG("test_index idx={}, --- cncpp::Index({})={}", idx, idx, cncpp::Index(idx));
        }
    }

    void test_round()
    {
        std::map<uint32_t, std::vector<uint32_t>> tmpmap;
        for (uint32_t idx = 0; idx < 3000; idx++)
        {
            tmpmap[cncpp::Roundup(idx)].emplace_back(idx);
        }

        for (const auto& item : tmpmap)
        {
            std::stringstream oss;
            oss << "(";
            for (const auto& v : item.second)
            {
                oss << v << ",";
            }
            oss << ")";

            DEBUG("test_round key={}, oss={}", item.first, oss.str());
        }
    }

    void test_toAddr()
    {
        uint32_t* value1 = new uint32_t(100);
        uint32_t* value2 = new uint32_t(1200);

        std::cout << "value1: " << value1 << ", addr: " << ADDR_TO_UINT64(value1)
                  << ", uint: " << uint64_t(value1) << std::endl;
        std::cout << "value2: " << value2 << ", addr: " << ADDR_TO_UINT64(value2)
                  << ", uint: " << uint64_t(value2) << std::endl;
    }

    void test_charAddr()
    {
        const uint16_t arr_size = 20;
        char*          addr     = new char[arr_size];
        for (uint16_t i = 0; i < arr_size; i++)
        {
            addr[i] = 'c';
        }
    }

    void test_one_alloc()
    {
        cncpp::MyMemPool pool;
        TestBase1*       base = new (pool.myalloc(sizeof(TestBase1))) TestBase1();
        if (!base)
        {
            ERR("test_one_alloc, malloc failed");
        }
        else
        {
            pool.myfree(base);
        }
        std::cout << "test\n";
    }

    void test_some_alloc()
    {
        cncpp::MyMemPool      pool;
        std::list<TestBase1*> test_list = {};
        const uint32_t        size      = 10;
        for (uint32_t i = 0; i < size; i++)
        {
            test_list.emplace_back(new (pool.myalloc(sizeof(TestBase1))) TestBase1(i));
        }

        INFO("test_some_alloc , first allocte");
        std::for_each(
            test_list.begin(), test_list.end(),
            [](TestBase1* base)
            {
                if (!base)
                    return;

                base->Print();
            });

        while (test_list.size() >= 6)
        {
            TestBase1* base = test_list.front();
            test_list.pop_front();
            pool.myfree(base);
        }

        INFO("test_some_alloc , after erase");
        std::for_each(
            test_list.begin(), test_list.end(),
            [](TestBase1* base)
            {
                if (!base)
                    return;

                base->Print();
            });

        for (uint32_t i = 0; i < size; i++)
        {
            test_list.emplace_back(new (pool.myalloc(sizeof(TestBase1))) TestBase1(i + 1000));
        }

        INFO("test_some_alloc , after alloc");
        std::for_each(
            test_list.begin(), test_list.end(),
            [](TestBase1* base)
            {
                if (!base)
                    return;

                base->Print();
            });

        while (test_list.size() >= 6)
        {
            TestBase1* base = test_list.front();
            test_list.pop_front();
            pool.myfree(base);
        }

        INFO("test_some_alloc , after erase");
        std::for_each(
            test_list.begin(), test_list.end(),
            [](TestBase1* base)
            {
                if (!base)
                    return;

                base->Print();
            });
    }

    /*----------------------------------------------------------------
    内存池测试相关
    1、相同大小单个对象指针的分配和释放
    2、多个对象指针的分配和释放
    3、内存池从重复利用
    4、page 中剩余内存的split 到内存池
    5、不同大小对象的混合存储释放
    6、数组分配释放

    MemPoolWrap
    1、智能指针的存储和释放
    2、智能指针数组的存储和释放
    */

    void main()
    {
        std::cout << "test\n";
        DEBUG("test main");
        // test_alloc();
        test_some_alloc();
        // test_round();
        // test_toAddr();
        // test_charAddr();
    }

}  // namespace Test

namespace ns_misc
{
    struct Base1
    {
        uint32_t i32_value = 100;
        char     c_value   = '3';
        uint64_t i64_value = 101;
        uint16_t u16_value = 150;
    };

    struct Base2
    {
        Base1 base11;
        Base1 base12;
    };

    struct Base3
    {
        Base1 base11;
        Base1 base12;

        Base2 base21;
        Base2 base22;
    };

    struct Base4
    {
        Base1 base11;
        Base1 base12;

        Base2 base21;
        Base2 base22;

        Base3 base31;
        Base3 base32;
    };

    cncpp::MemPoolWrap poolwrap;  //; = new cncpp::MemPoolWrap();
    uint16_t           std_flag = 0;
    uint16_t           recycle  = 0;

    boost::object_pool<Base1> base1_pool;
    boost::object_pool<Base2> base2_pool;
    boost::object_pool<Base3> base3_pool;
    boost::object_pool<Base4> base4_pool;

    template <typename T>
    std::shared_ptr<T> test_uniq(uint32_t size)
    {
        if (std_flag == 0)
            return poolwrap.make_shared<T>();
        else if (std_flag == 1)
            return std::make_shared<T>();
        else if (std_flag == 2)
            return cncpp::BlockPoolManager::getMe().make_shared<T>();

        return std::shared_ptr<T>();
    }

    template <typename T>
    std::shared_ptr<T> boost_uniq(boost::object_pool<T>& pool)
    {
        T* obj = pool.construct();
        return std::shared_ptr<T>(
            obj,
            [&pool](T* data)
            {
                pool.free(data);
            });
    }

    void test1(uint32_t size)
    {
        BLOCK_COST;
        std::list<std::shared_ptr<Base1>> base1_list = {};
        std::list<std::shared_ptr<Base2>> base2_list = {};
        std::list<std::shared_ptr<Base3>> base3_list = {};
        std::list<std::shared_ptr<Base4>> base4_list = {};

        for (uint32_t len = 0; len < size; len++)
        {
            if (std_flag == 3)
            {
                // std::shared_ptr<Base1> base(base1_pool.construct(),
                //                          [&base1_pool](Base1* data)
                //                          {
                //                              base1_pool.free(data);
                //                          });
                // base1_list.emplace_back(std::shared_ptr<Base1>(base1_pool.construct()));
                base1_list.emplace_back(boost_uniq<Base1>(base1_pool));
            }
            else
            {
                base1_list.emplace_back(test_uniq<Base1>(1));
            }

            if (recycle && len % 10 == 0)
            {
                base1_list.clear();
            }
        }
        for (uint32_t len = 0; len < size; len++)
        {
            if (std_flag == 3)
            {
                // base2_list.emplace_back(std::shared_ptr<Base2(base2_pool.construct()));
                base2_list.emplace_back(boost_uniq<Base2>(base2_pool));
            }
            else
            {
                base2_list.emplace_back(test_uniq<Base2>(1));
            }

            if (recycle && len % 10 == 0)
            {
                base2_list.clear();
            }
        }
        for (uint32_t len = 0; len < size; len++)
        {
            if (std_flag == 3)
            {
                // base3_list.emplace_back(std::shared_ptr<Base3>(base3_pool.construct()));
                base3_list.emplace_back(boost_uniq<Base3>(base3_pool));
            }
            else
            {
                base3_list.emplace_back(test_uniq<Base3>(1));
            }

            if (recycle && len % 10 == 0)
            {
                base3_list.clear();
            }
        }
        for (uint32_t len = 0; len < size; len++)
        {
            if (std_flag == 3)
            {
                // base4_list.emplace_back(std::shared_ptr<Base4>(base4_pool.construct()));
                base4_list.emplace_back(boost_uniq<Base4>(base4_pool));
            }
            else
            {
                base4_list.emplace_back(test_uniq<Base4>(1));
            }

            if (recycle && len % 10 == 0)
            {
                base4_list.clear();
            }
        }
    }

    void test2(uint32_t size)
    {
        BLOCK_COST;
        std::list<std::shared_ptr<Base1>> base1_list = {};
        std::list<std::shared_ptr<Base2>> base2_list = {};
        std::list<std::shared_ptr<Base3>> base3_list = {};
        std::list<std::shared_ptr<Base4>> base4_list = {};

        for (uint32_t len = 0; len < size; len++)
        {
            if (std_flag == 3)
            {
                base1_list.emplace_back(boost_uniq<Base1>(base1_pool));
                base2_list.emplace_back(boost_uniq<Base2>(base2_pool));
                base3_list.emplace_back(boost_uniq<Base3>(base3_pool));
                base4_list.emplace_back(boost_uniq<Base4>(base4_pool));
            }
            else
            {
                base1_list.emplace_back(test_uniq<Base1>(1));
                base2_list.emplace_back(test_uniq<Base2>(1));
                base3_list.emplace_back(test_uniq<Base3>(1));
                base4_list.emplace_back(test_uniq<Base4>(1));
            }

            if (recycle && len % 10 == 0)
            {
                base1_list.clear();
                base2_list.clear();
                base3_list.clear();
                base4_list.clear();
            }
        }
    }

    void main(int argc, char* argv[])
    {
        cmdline::parser opt;
        opt.add<uint32_t>(
            "count", 'c', "allocate times", false, 10000, cmdline::range(1, 65535000));
        opt.add<uint32_t>("std", 's', "using std", false, 10000, cmdline::range(0, 3));
        opt.add<uint32_t>("recycle", 'r', "using recycle", false, 10000, cmdline::range(0, 1));

        opt.parse_check(argc, argv);

        std_flag             = opt.get<uint32_t>("std");
        recycle              = opt.get<uint32_t>("recycle");
        const uint32_t count = static_cast<uint32_t>(opt.get<uint32_t>("count"));
        if (std_flag == 2)
            cncpp::BlockPoolManager::getMe().init();

        // std::cout << "std_flag: " << std_flag << std::endl;

        BLOCK_COST;
        // cncpp::BlockCost(__FUNCTION__, __LINE__);
        test1(count);
        test2(count);
    }

}  // namespace ns_misc

namespace ns_pool
{

};  // namespace ns_pool

// ./examples/bin/testmemory --count=1000 --std=2

namespace ns_memory
{
    void main(int argc, char** argv)
    {
        ns_misc::main(argc, argv);
    }
}  // namespace ns_memory