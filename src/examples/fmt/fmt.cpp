/*
 * @Author: czf
 * @Date: 2022-04-13 21:41:08
 * @LastEditors: czf
 * @LastEditTime: 2022-04-15 22:55:47
 * @FilePath: \cpp_project2022\src\examples\fmt\fmt.cpp
 * @Description:
 *
 * Copyright (c) 2022 by 用户/公司名, All Rights Reserved.
 */

// helpurl:  https://fmt.dev/latest/index.html
//  examples: https://fmt.dev/latest/syntax.html#format-examples

#include "StringUtil.h"
#include <list>
#include <map>
#include <set>
#include <spdlog/fmt/bundled/chrono.h>
#include <spdlog/fmt/bundled/color.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/fmt/bundled/os.h>
#include <spdlog/fmt/bundled/ranges.h>
#include <vector>

namespace ns_fmt_test
{
    void test1()
    {

        fmt::print("Hello, world!\n");

        std::string s = fmt::format("The answer is {}.", 42);
        // s == "The answer is 42."

        fmt::print("answer: {}\n", fmt::format("The answer is {}.", 42));

        std::string str = fmt::format("I'd rather be {1} than {0}.", "right", "happy");

        fmt::print("sentense： {}!\n", str);
    }

    void test_chrono()
    {
        using namespace std::literals::chrono_literals;
        fmt::print("Default format: {} {}\n", 42s, 100ms);
        fmt::print("strftime-like format: {:%H:%M:%S}\n", 3h + 15min + 30s);
    }

    template <typename CONT>
    void print_cont(const CONT& cont)
    {
        cncpp::print("print_cont: {}\n", cont);

        const std::string str = cncpp::format("{}", cont);
        cncpp::print("print_cont: {}\n", cont);

        fmt::print("{}", fmt::join(cont, ", "));
    }

    void test_cont()
    {
        {
            cncpp::print("test vector ============\n");
            std::vector<int> v = { 1, 2, 3 };
            print_cont(v);
        }

        {
            cncpp::print("test set ============\n");
            std::set<int> v = { 1, 2, 3 };
            print_cont(v);
        }

        {
            cncpp::print("test list ============\n");
            std::list<int> v = { 1, 2, 3 };
            print_cont(v);
        }

        {
            cncpp::print("test map ============\n");
            std::map<uint32_t, std::string> m = { { 1, "a" }, { 2, "b" }, { 3, "c" } };
            print_cont(m);
        }

        {
            cncpp::print("test map ============\n");
            std::map<uint32_t, std::vector<uint32_t>> m = { { 1, { 10, 2, 3 } }, { 2, { 111, 2, 3 } }, { 3, { 101, 2, 3 } } };
            print_cont(m);
        }
    }

    void test_os()
    {
        // auto out = fmt::output_file("guide.txt");
        // out.print("Don't {}", "Panic");
    }

    void test_color()
    {
        fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "Hello, {}!\n", "world");
        fmt::print(fg(fmt::color::floral_white) | bg(fmt::color::slate_gray) | fmt::emphasis::underline, "Hello, {}!\n", "мир");
        fmt::print(fg(fmt::color::steel_blue) | fmt::emphasis::italic, "Hello, {}!\n", "世界");
    }
}  // namespace ns_fmt_test

namespace ns_fmt
{
    void main()
    {
        ns_fmt_test::test1();
        ns_fmt_test::test_chrono();
        ns_fmt_test::test_cont();
        ns_fmt_test::test_color();
    }
}  // namespace ns_fmt
