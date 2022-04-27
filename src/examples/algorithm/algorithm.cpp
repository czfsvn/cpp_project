/*
 * @Author: czf
 * @Date: 2022-04-21 16:07:17
 * @LastEditors: czf
 * @LastEditTime: 2022-04-21 16:14:09
 * @FilePath: \cpp_project2022\src\examples\algorithm\algorithm.cpp
 * @Description:
 *
 * Copyright (c) 2022 by 用户/公司名, All Rights Reserved.
 */
#include "StringUtil.h"
#include <algorithm>
#include <vector>

namespace ns_advance
{
    void test1()
    {
        std::vector<uint32_t> vec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

        auto iter = vec.begin();
        cncpp::print("begin of iter={}", *iter);

        std::advance(iter, 4);
        cncpp::print("begin of iter+4={}", *iter);
    }
    void main()
    {
        test1();
    }
}  // namespace ns_advance
namespace ns_algorithm
{
    void main()
    {
        ns_advance ::main();
    }
}  // namespace ns_algorithm