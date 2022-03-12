/*
 * @Author: czf
 * @Date: 2022-02-23 14:28:35
 * @LastEditors: czf
 * @LastEditTime: 2022-02-23 16:24:21
 * @FilePath: \cpp_project2022\src\examples\testmemset.cpp
 * @Description: test efficiency of memset
 *
 * Copyright (c) 2022 by 用户/公司名, All Rights Reserved.
 */

#include "TimerUsage.h"
#include "boost_asio.h"
#include "cmdline.h"
#include <iostream>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 65530

char srcbuf[BUF_SIZE] = {};

namespace ns_test1
{
    void memset_inited1(const uint32_t times, const uint32_t bufsize)
    {
        BLOCK_COST;
        for (uint32_t i = 0; i < times; i++)
        {
            char buf[BUF_SIZE] = {};
            strncpy(buf, srcbuf, bufsize);
        }
    }

    void memset_inited2(const uint32_t times, const uint32_t bufsize)
    {
        BLOCK_COST;
        for (uint32_t i = 0; i < times; i++)
        {
            char buf[BUF_SIZE] = { 0 };
            strncpy(buf, srcbuf, bufsize);
        }
    }
    void memset_inited3(const uint32_t times, const uint32_t bufsize)
    {
        BLOCK_COST;
        for (uint32_t i = 0; i < times; i++)
        {
            char buf[BUF_SIZE] = "";
            strncpy(buf, srcbuf, bufsize);
        }
    }
    void memset_noinited(const uint32_t times, const uint32_t bufsize)
    {
        BLOCK_COST;
        for (uint32_t i = 0; i < times; i++)
        {
            char buf[BUF_SIZE];
            strncpy(buf, srcbuf, bufsize);
        }
    }

    void test(const uint32_t times, const uint32_t bufsize)
    {
        memset_inited1(times, bufsize);
        memset_inited2(times, bufsize);
        memset_inited3(times, bufsize);
        memset_noinited(times, bufsize);
    }
}  // namespace ns_test1

namespace test_snprintf
{
#define MAX_CHATINFO 32

    void test1(const char* fomat, ...)
    {
        char    msg[MAX_CHATINFO];
        va_list ap;
        va_start(ap, fomat);
        int size = vsnprintf(msg, MAX_CHATINFO - 1, fomat, ap);
        va_end(ap);

        // std::string str(msg);

        // std::cout << size << ", [" << msg << "]" << std::endl;
        return;
    }

    void test2(const char* fomat, ...)
    {
        char    msg[MAX_CHATINFO] = {};
        va_list ap;
        va_start(ap, fomat);
        int size = vsnprintf(msg, MAX_CHATINFO, fomat, ap);
        va_end(ap);

        // std::string str(msg);

        // std::cout << size << ", [" << msg << "]" << std::endl;
        return;
    }

    void test(const uint32_t times)
    {
        {
            BLOCK_COST;
            for (int i = 0; i < times; i++)
            {
                test1("hello, world! %s", "ceshi");
                test1("hello, world! %s", "ceshic");
                test1("%s", "abcafb");
                test1("%s", "ce");
                test1("%s", "//Flags used by the linker during the creation of static libraries");
                test1("%s", "//Flags used by the linker during the creation of static libraries");
                test1("%s", "//Flags used by the linker during the creation of static libraries");
            }
        }
        {
            BLOCK_COST;
            for (int i = 0; i < times; i++)
            {
                test2("hello, world! %s", "ceshi");
                test2("hello, world! %s", "ceshic");
                test2("%s", "abc\0afb");
                test2("%s", "ce");
                test2("%s", "//Flags used by the linker during the creation of static libraries");
                test2("%s", "//Flags used by the linker during the creation of static libraries");
                test2("%s", "//Flags used by the linker during the creation of static libraries");
            }
        }
    }

}  // namespace test_snprintf

namespace ns_memset
{
    cmdline::parser cmd_parser;

    void parse_line(int argc, char** argv)
    {
        cmd_parser.add<uint32_t>("times", 't', "times of loop", false, 10);
        cmd_parser.add<uint32_t>("bufsize", 's', "size of buff-array", false, 10);

        cmd_parser.parse_check(argc, argv);
    }

    void main(int argc, char** argv)
    {
        parse_line(argc, argv);
        ns_test1::test(cmd_parser.get<uint32_t>("times"), cmd_parser.get<uint32_t>("bufsize"));
        // test_snprintf::test(cmd_parser.get<uint32_t>("times"));
    }
}  // namespace ns_memset
