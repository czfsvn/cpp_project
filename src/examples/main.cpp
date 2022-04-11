/*
 * @Author: czf
 * @Date: 2022-01-16 18:25:35
 * @LastEditors: czf
 * @LastEditTime: 2022-04-04 17:13:53
 * @FilePath: \cpp_project2022\src\examples\main.cpp
 * @Description:
 *
 * Copyright (c) 2022 by 用户/公司名, All Rights Reserved.
 */

#include "MySkipList/skiplist.h"
#include "boost_asio.h"

using namespace std;

int32_t main(int argc, char** argv)
{

    // std::cout << "Hello, main\n";
    /*
    ns_boost_thread::main();
    ns_spdlog::main();
    ns_spdlog::examples();
    ns_threadpool::main(argc, argv);
    ns_testcmdline::main(argc, argv);
    ns_memory::main(argc, argv);
    ns_proto::main();
    ns_memset::main(argc, argv);
    ns_skip3::main_skip3(argc, argv);

    */

    // ns_timeutils::main();
    // ns_typeconvert::main();

    // ns_uri::main();

    test_skiplist::main_skiplist(argc, argv);
    return 0;
}
