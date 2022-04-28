/*
 * @Author: czf
 * @Date: 2022-04-16 17:49:00
 * @LastEditors: czf
 * @LastEditTime: 2022-04-28 17:56:51
 * @FilePath: \cpp_project2022\src\examples\MySkipList\skiplist4.cpp
 * @Description:
 *
 * Copyright (c) 2022 by 用户/公司名, All Rights Reserved.
 */
#include "MyToplist.h"
#include "StringUtil.h"
#include "cmdline.h"
#include "logger.h"
#include <sstream>

#include "TimerUsage.h"
namespace
{
    struct UserData
    {
        uint32_t userid = 0;
        uint32_t age    = 0;
        uint32_t score  = 0;

        std::string dump() const
        {
            std::stringstream oss;
            oss << "[" << userid << ", " << score << ", " << age << "]";
            return oss.str();
        }

        const uint32_t getKey() const
        {
            return userid;
        }
    };

    using UserDataPtr = std::shared_ptr<UserData>;
    bool operator<(const UserDataPtr& lhs, const UserDataPtr& rhs)
    {
        if (lhs->score != rhs->score)
            return lhs->score < rhs->score;

        if (lhs->age != rhs->age)
            return lhs->age < rhs->age;

        return lhs->userid > rhs->userid;
    }

    UserDataPtr& operator+=(UserDataPtr& lhs, const UserDataPtr& rhs)
    {
        lhs->score += rhs->score;
        return lhs;
    }

    UserDataPtr& operator+=(UserDataPtr& lhs, const UserData& rhs)
    {
        lhs->score += rhs.score;
        return lhs;
    }

    bool equalData(const UserDataPtr& lhs, const UserDataPtr& rhs)
    {
        return lhs->userid == rhs->userid && lhs->score == rhs->score && lhs->age == rhs->age;
    }

    /*
    bool operator==(const UserDataPtr& lhs, const UserDataPtr& rhs)
    {
        return lhs->score == rhs->score && lhs->age == rhs->age;
    }
    */
}  // namespace

namespace
{
#define FUNC log_func(__FUNCTION__)

    uint32_t    max_size     = 0;
    std::string toplist_type = {};

    cncpp::TopList<uint32_t, UserData>* toplist     = nullptr;
    cncpp::TopList<uint32_t, UserData>* toplist2    = nullptr;
    std::map<uint32_t, UserDataPtr>     origin_map  = {};
    std::map<uint32_t, uint32_t>        rank_map    = {};  //< key, rank>
    std::map<uint32_t, uint32_t>        revrank_map = {};  //< key, revrank>

    uint32_t randOne()
    {
        // static cncpp::Random rnd_(0xdeadbeef);
        static cncpp::Random rnd_(time(NULL));
        return rnd_.Next() % max_size + 1;
    }

    void log_func(const std::string& func)
    {
        INFO("toplist type: [{}], function: {}   =====================", toplist_type, func);
    }

    cmdline::parser parse_line(int argc, char** argv)
    {
        cmdline::parser cmd_parser;
        cmd_parser.add<std::string>("type", 't', "toplist type", false, "skip");
        cmd_parser.add<uint32_t>("size", 's', "size of toplist", false, 10);
        cmd_parser.add<std::string>("op", 'o', "test function case", false, "");

        cmd_parser.parse_check(argc, argv);

        return cmd_parser;
    }

    cncpp::TopList<uint32_t, UserData>* createToplist(const std::string& toptype)
    {
        if (toptype == "vec")
        {
            toplist_type = "vector";
            return new cncpp::VecTopList<uint32_t, UserData>();
        }
        else if (toptype == "set")
        {
            toplist_type = "set";
            return new cncpp::SetTopList<uint32_t, UserData>();
        }
        else if (toptype == "skip")
        {
            toplist_type = "skiplist";
            return new cncpp::SkipTopList<uint32_t, UserData>();
        }

        return nullptr;
    }
}  // namespace

namespace ns_toplist4
{
    void init_toplist();
    void test_update();
    void test_deleteByKey();
    void test_deleteByRank();
    void test_deleteByRangedRank();
    void test_getDataByRank();
    void test_getRankByKey();
    void test_getRevRankByKey();
    void test_forEachRev();
    void test_forEachByRangedRank();

    template <typename T>
    void dump(T& cont, const std::string& log)
    {
        INFO("dump: {}", log);
        uint32_t rank = 0;
        cont->forEach(
            [&rank](const UserDataPtr& data)
            {
                INFO("rank: {}, \tdata: {}", ++rank, data->dump());
                return true;
            });
    }

    void cacheRankList()
    {
        // BLOCK_COST;

        rank_map.clear();
        uint32_t rank = 0;
        toplist2->forEach(
            [&rank](const UserDataPtr& data)
            {
                rank_map.emplace(data->userid, ++rank);
                return true;
            });
    }

    void cacheRevRankList()
    {
        // BLOCK_COST;

        revrank_map.clear();
        uint32_t rank = 0;
        toplist2->forEachRev(
            [&rank](const UserDataPtr& data)
            {
                revrank_map.emplace(data->userid, ++rank);
                return true;
            });
    }

    void forEachRank()
    {
        // FUNC;
        //  BLOCK_COST;
        uint32_t rank = 0;
        toplist->forEach(
            [&rank](const UserDataPtr& data)
            {
                INFO("rank: {}, \tdata: {}", ++rank, data->dump());

                auto iter = origin_map.find(data->userid);
                assert(iter != origin_map.end());
                assert(equalData(iter->second, data));
                return true;
            });
    }

#define DMP_CONT(x, y)                                         \
    dump((x), cncpp::format("{}:{}", __FUNCTION__, __LINE__)); \
    dump((y), cncpp::format("{}:{}", __FUNCTION__, __LINE__)); \
    assert(false);                                             \
    exit(1);

    template <typename CONT1, typename CONT2>
    bool checkEqual(CONT1* cont1, CONT2* cont2)
    {
#ifdef _RELEASE_
        // return true;
#endif
        uint32_t rank = 0;
        cont1->forEach(
            [&rank, cont2, cont1](const UserDataPtr& data)
            {
                rank++;
                const uint32_t rank2 = cont2->getRankByKey(data->getKey());
                if (rank != rank2)
                {
                    DMP_CONT(cont1, cont2);
                    return false;
                }

                const UserDataPtr& ptr = cont2->getDataByRank(rank);
                if (!equalData(data, ptr))
                {
                    DMP_CONT(cont1, cont2);
                    return false;
                }

                return true;
            });

        return rank == cont1->size() && rank == cont2->size();
    }

    void test_forEachRev()
    {
        FUNC;
        BLOCK_COST;
        uint32_t rank = 0;
        toplist2->forEachRev(
            [&rank](const UserDataPtr& data)
            {
                INFO("rank: {}, \tdata: {}", ++rank, data->dump());

                auto iter = origin_map.find(data->userid);
                assert(iter != origin_map.end());
                assert(iter->second == data);

                return true;
            });
    }

    void init_toplist()
    {
        FUNC;
        // BLOCK_COST;
        for (uint32_t i = 0; i < max_size; i++)
        {
            const uint32_t new_id = i + 10000;
            UserDataPtr    ptr    = std::make_shared<UserData>();
            ptr->userid           = new_id;
            ptr->age              = randOne();
            ptr->score            = 10;  // randOne();
            toplist->refreshItem(ptr->userid, ptr);
            origin_map.emplace(ptr->userid, ptr);

            {
                UserDataPtr ptr2 = std::make_shared<UserData>();
                ptr2->userid     = ptr->userid;
                ptr2->age        = ptr->age;
                ptr2->score      = ptr->score;
                toplist2->refreshItem(ptr2->userid, ptr2);
            }

            if (!checkEqual(toplist2, toplist))
            {
                DMP_CONT(toplist2, toplist);
            }

            if (!checkEqual(toplist, toplist2))
            {
                DMP_CONT(toplist, toplist2);
            }

            INFO("init_toplist: {}, \tdata: {}", new_id, ptr->dump());
        }
    }

    void test_update()
    {
        FUNC;
        BLOCK_COST;
        for (uint32_t i = 0; i < max_size; i++)
        {
            const uint32_t new_id = i + 10000;
            auto           iter   = origin_map.find(new_id);
            if (iter == origin_map.end())
            {
                assert(false);
                continue;
            }

            const uint32_t old_score = iter->second->score;

            UserDataPtr ptr = std::make_shared<UserData>();
            ptr->userid     = new_id;
            ptr->age        = randOne();
            ptr->score      = randOne();

            toplist->refreshItem(ptr->userid, *ptr);

            {
                UserDataPtr ptr2 = std::make_shared<UserData>();
                ptr2->userid     = ptr->userid;
                ptr2->age        = ptr->age;
                ptr2->score      = ptr->score;
                toplist2->refreshItem(ptr2->userid, *ptr2);
            }

            if (!checkEqual(toplist2, toplist))
            {
                DMP_CONT(toplist, toplist2);
            }

            if (!checkEqual(toplist, toplist2))
            {
                DMP_CONT(toplist2, toplist);
            }

            assert(ptr->score + old_score == iter->second->score);

            INFO("test_update: key:{}, old_score:{}, newscore:{}", new_id, old_score, iter->second->score);
        }
    }

    void test_getDataByRank()
    {
        FUNC;
        BLOCK_COST;

        for (uint32_t i = 0; i < max_size; i++)
        {
            UserDataPtr ptr = toplist->getDataByRank(i + 1);
            if (!ptr)
            {
                assert(false);
                continue;
            }

            auto iter = rank_map.find(ptr->userid);
            if (iter == rank_map.end())
            {
                assert(false);
                continue;
            }

            assert(iter->second == (i + 1));

            {
                UserDataPtr ptr2 = toplist2->getDataByRank(i + 1);
                if (!ptr2)
                {
                    assert(false);
                    continue;
                }

                assert(equalData(ptr, ptr2));
            }
        }
    }

    void test_getRankByKey()
    {
        FUNC;
        BLOCK_COST;

        for (const auto& origin : origin_map)
        {
            const uint32_t rank = toplist->getRankByKey(origin.first);
            auto           iter = rank_map.find(origin.first);
            INFO("test_getRankByKey: {}--{}", origin.first, rank);
            assert(iter != rank_map.end());
            assert(iter->second == rank);

            {
                const uint32_t rank2 = toplist2->getRankByKey(origin.first);
                assert(iter->second == rank2);
            }
        }
    }

    void test_getRevRankByKey()
    {
        FUNC;
        BLOCK_COST;

        for (const auto& origin : origin_map)
        {
            const uint32_t rank = toplist->getRevRankByKey(origin.first);
            auto           iter = revrank_map.find(origin.first);
            INFO("test_getRevRankByKey: {}--{}", origin.first, rank);
            assert(iter != revrank_map.end());
            assert(iter->second == rank);

            {
                const uint32_t rank2 = toplist2->getRevRankByKey(origin.first);
                assert(iter->second == rank2);
            }
        }
    }

    void test_forEachByRangedRank()
    {
        FUNC;
        BLOCK_COST;

        for (uint32_t i = 0; i < max_size + 10; i++)
        {
            if (randOne() % 2 == 0)
                continue;

            const uint32_t from = randOne();
            const uint32_t to   = randOne();

            const uint32_t rankfrom = std::min(from, to);
            const uint32_t rankto   = std::max(from, to);

            INFO("test_forEachByRangedRank: [{}] --> [{}]", rankfrom, rankto);
            toplist->forEachByRangedRank(rankfrom, rankto,
                [](const UserDataPtr& data)
                {
                    INFO("test_forEachByRangedRank {}", data->dump());
                    return true;
                });

            /*
            toplist2->forEachByRangedRank(rankfrom, rankto,
                [](const UserDataPtr& data)
                {
                    INFO("test_forEachByRangedRank {}", data->dump());
                    return true;
                });
                */
        }
    }

    void test_deleteByKey()
    {
        FUNC;
        BLOCK_COST;

        if (!checkEqual(toplist2, toplist))
        {
            DMP_CONT(toplist2, toplist);
        }

        if (!checkEqual(toplist, toplist2))
        {
            DMP_CONT(toplist, toplist2);
        }

        for (const auto& origin : origin_map)
        {
            if (randOne() % 2 == 1)
                continue;

            INFO("test_deleteByKey: {}", origin.second->dump());
            const uint32_t ret = toplist->deleteByKey(origin.first);
            if (ret)
            {
                INFO("test_deleteByKey: {} successs ", origin.second->dump());
                forEachRank();
            }
            else
            {
                assert(false);
            }

            const uint32_t ret2 = toplist2->deleteByKey(origin.first);
            assert(ret2 == ret);

            if (!checkEqual(toplist2, toplist))
            {
                DMP_CONT(toplist2, toplist);
            }

            if (!checkEqual(toplist, toplist2))
            {
                DMP_CONT(toplist, toplist2);
            }
        }
    }

    void test_deleteByRank()
    {
        FUNC;
        BLOCK_COST;

        if (!checkEqual(toplist2, toplist))
        {
            DMP_CONT(toplist2, toplist);
        }

        if (!checkEqual(toplist, toplist2))
        {
            DMP_CONT(toplist, toplist2);
        }

        for (uint32_t i = 0; i < max_size + 1000; i++)
        {
            if (randOne() % 2 == 0)
                continue;

            const uint32_t ret  = toplist->deleteByRank(i);
            const uint32_t ret2 = toplist2->deleteByRank(i);
            assert(ret == ret2);

            if (!checkEqual(toplist2, toplist))
            {
                DMP_CONT(toplist2, toplist);
            }

            if (!checkEqual(toplist, toplist2))
            {
                DMP_CONT(toplist, toplist2);
            }

            /*
            UserDataPtr ptr1 = toplist->getDataByRank(i);
            if (!ptr1)
                continue;

            const uint32_t ret = toplist->deleteByRank(i);

            UserDataPtr ptr2 = toplist->getDataByRank(i);
            if (ptr1 != ptr2)
            {
                assert(ret == 1);
                INFO("test_deleteByRank: success: {}, {}", i, ptr1->dump());
                forEachRank();
            }
            else
            {
                assert(ret == 0);
                INFO("test_deleteByRank: failed: {}, {}", i, ptr1->dump());
                assert(false);
            }

            {
                const uint32_t ret2 = toplist2->deleteByRank(i);
                UserDataPtr    ptr3 = toplist2->getDataByRank(i);
                if (!ptr3 && !ptr2)
                    continue;

                if (!ptr3 && ptr2)
                {
                    assert(false);
                    continue;
                }
                if (ptr3 && !ptr2)
                {
                    assert(false);
                    continue;
                }
                assert(ptr2->userid == ptr3->userid);
                assert(equalData(ptr2, ptr3));
            }
            */
        }
    }

    void test_deleteByRangedRank()
    {
        FUNC;
        BLOCK_COST;

        if (!checkEqual(toplist2, toplist))
        {
            DMP_CONT(toplist2, toplist);
        }

        if (!checkEqual(toplist, toplist2))
        {
            DMP_CONT(toplist, toplist2);
        }

        for (uint32_t i = 0; i < max_size + 10; i++)
        {
            if (randOne() % 2 == 0)
                continue;

            const uint32_t from = randOne();
            const uint32_t to   = randOne();

            const uint32_t rankfrom = std::min(from, to);
            const uint32_t rankto   = std::max(from, to);

            INFO("test_deleteByRangedRank: [{}] --> [{}]", rankfrom, rankto);
            const uint32_t ret  = toplist->deleteByRank(rankfrom, rankto);
            const uint32_t ret2 = toplist2->deleteByRank(rankfrom, rankto);
            assert(ret == ret2);

            if (!checkEqual(toplist2, toplist))
            {
                DMP_CONT(toplist2, toplist);
            }

            if (!checkEqual(toplist, toplist2))
            {
                DMP_CONT(toplist, toplist2);
            }
        }
    }

    void main(int argc, char** argv)
    {
        cmdline::parser cmd_parser = parse_line(argc, argv);

        max_size                   = cmd_parser.get<std::uint32_t>("size");
        const std::string& toptype = cmd_parser.get<std::string>("type");
        const std::string& op      = cmd_parser.get<std::string>("op");

        toplist  = createToplist(toptype);
        toplist2 = createToplist("vec");
        assert(toplist);
        if (!toplist)
            return;

        init_toplist();
        forEachRank();
        cacheRankList();
        cacheRevRankList();

        if (op == "test_update")
        {
            test_update();  // all workd well on size=10
        }
        else if (op == "test_getDataByRank")
        {
            test_getDataByRank();  // all workd well on size=10
        }
        else if (op == "test_getRankByKey")
        {
            test_getRankByKey();  // all workd well on size=10
        }
        else if (op == "test_getRevRankByKey")
        {
            test_getRevRankByKey();  // all workd well on size=10
        }
        else if (op == "test_deleteByKey")
        {
            test_deleteByKey();
        }
        else if (op == "test_deleteByRank")
        {
            test_deleteByRank();
        }
        else if (op == "test_deleteByRangedRank")
        {
            test_deleteByRangedRank();
        }
        else if (op == "test_forEachByRangedRank")
        {
            test_forEachByRangedRank();
        }
        else if (op == "foreach")
        {
            forEachRank();
        }
        forEachRank();
    }
}  // namespace ns_toplist4

namespace ns_skip4
{
    // ./bin/example --type=vec/set/skip --op=insert/update --size=10
    // ./bin/example --type=skip --op=test_update --size=10
    void main(int argc, char** argv)
    {
        ns_toplist4::main(argc, argv);
    }
}  // namespace ns_skip4