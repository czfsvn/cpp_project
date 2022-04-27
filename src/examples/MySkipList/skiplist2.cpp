/*
 * @Author: czf
 * @Date: 2022-03-01 10:17:01
 * @LastEditors: czf
 * @LastEditTime: 2022-04-16 18:09:31
 * @FilePath: \cpp_project2022\src\examples\MySkipList\skiplist2.cpp
 * @Description:
 *
 * Copyright (c) 2022 by 用户/公司名, All Rights Reserved.
 */
#include "skiplist2.h"
#include "StringUtil.h"
#include "cmdline.h"
#include "logger.h"
#include <sstream>

using namespace ns_toplist3;

struct UserData
{
    uint32_t userid = 0;
    uint32_t age    = 0;
    uint32_t score  = 0;

    std::string dump() const
    {
        std::stringstream oss;
        oss << "[" << userid << ", \t" << score << ", \t" << age << "]";
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

    return lhs->userid < rhs->userid;
}

UserDataPtr& operator+=(UserDataPtr& lhs, const UserDataPtr& rhs)
{
    lhs->score += rhs->score;
    return lhs;
}

bool operator==(const UserDataPtr& lhs, const UserDataPtr& rhs)
{
    return lhs->score == rhs->score && lhs->age == rhs->age;
}

namespace
{
    uint32_t    max_size     = 0;
    std::string toplist_type = {};

    ns_toplist3::TopList<uint32_t, UserData>* toplist     = nullptr;
    std::map<uint32_t, UserDataPtr>           origin_map  = {};
    std::map<uint32_t, uint32_t>              rank_map    = {};  //< key, rank>
    std::map<uint32_t, uint32_t>              revrank_map = {};  //< key, revrank>

    uint32_t randOne()
    {
        const uint32_t ret = cncpp::random() % max_size + 1;
        return ret;
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
        cmd_parser.add<std::string>("op", 'c', "test function case", false, "");

        cmd_parser.parse_check(argc, argv);

        return cmd_parser;
    }

    TopList<uint32_t, UserData>* createToplist(const std::string& toptype)
    {
        if (toptype == "vec")
        {
            toplist_type = "vector";
            return new VecTopList<uint32_t, UserData>();
        }
        else if (toptype == "set")
        {
            toplist_type = "set";
            return new SetTopList<uint32_t, UserData>();
        }
        else if (toptype == "skip")
        {
            toplist_type = "skiplist";
            return new SkipTopList<uint32_t, UserData>();
        }

        return nullptr;
    }

    void test_fmt()
    {
        std::string           toptype = "vec";
        std::string           op      = "insert";
        std::vector<uint32_t> vec{ 0, 2, 3, 4 };
        const std::string     vec_str = cncpp::format("vec={}", vec);
        const std::string     fmtsrt  = cncpp::format("main_toplist3_v2 max_size={}, toptype={}, op={}", max_size, toptype, op);
        cncpp::print("main_toplist3_v2 1 max_size={}, toptype={}, op={}\n", max_size, toptype, op);
        cncpp::print("vec={}\n", vec);
    }
}  // namespace

#if 0
namespace ns_skiplist3
{
    bool operator<(const UserData& lhs, const UserData& rhs)
    {
        if (lhs.score != rhs.score)
            return lhs.score < rhs.score;

        if (lhs.age != rhs.age)
            return lhs.age < rhs.age;

        return lhs.userid < rhs.userid;
    }

    UserData& operator+=(UserData& lhs, const UserData& rhs)
    {
        lhs.score += rhs.score;
        return lhs;
    }

    bool operator==(const UserData& lhs, const UserData& rhs)
    {
        return lhs.score == rhs.score && lhs.age == rhs.age;
    }

    ns_skiplist3::SkipList3<uint32_t, UserData> skiplist   = {};
    std::map<uint32_t, UserData>                origin_map = {};
    uint32_t                                    times      = 0;

    void dump_foreach()
    {
        uint32_t rank = 0;
        skiplist.forEach(
            [&rank](const uint32_t& key, const UserData& data, uint16_t level)
            {
                rank++;
                std::cout << rank << ", \t" << level << ", \t" << data.dump() << std::endl;
                return true;
            });
    }

    void dump_foreachRev()
    {
        uint32_t rank = 0;
        skiplist.forEachRev(
            [&rank](const uint32_t& key, const UserData& data)
            {
                rank++;
                std::cout << rank << ", \t" << data.dump() << std::endl;
                return true;
            });
    }

    void test_insert()
    {
        origin_map.clear();
        for (uint32_t i = 0; i < times; i++)
        {
            UserData tmp;
            tmp.userid = i + 1;
            tmp.age    = cncpp::random() % times + 10;
            tmp.score  = cncpp::random() % times + 10;
            skiplist.Insert(tmp.userid, tmp);
            origin_map.emplace(tmp.userid, tmp);
        }

        dump_foreach();
        // dump_foreachRev();
    }

    void test_updatescore()
    {
        test_insert();

        std::cout << "test_updatescore ======================\n";
        for (auto& item : origin_map)
        {
            const uint32_t old_score = item.second.score;

            UserData tmp = item.second;
            tmp.score    = cncpp::random() % times + 0;
            tmp.age      = cncpp::random() % times + 0;
            skiplist.UpdateScore(tmp.userid, item.second, tmp);
            std::cout << "old: " << item.second.dump() << " ---> " << tmp.dump() << std::endl;
            origin_map[item.first] = tmp;
        }

        dump_foreach();
    }

    void test_delete()
    {
        test_insert();

        std::cout << "test_delete ======================\n";
        for (auto iter = origin_map.begin(); iter != origin_map.end(); /*iter++*/)
        {
            if (cncpp::random() % 2 == 0)
            {
                iter++;
                continue;
            }

            std::cout << "delete: " << iter->second.dump() << std::endl;
            skiplist.Delete(iter->second.userid, iter->second);
            iter = origin_map.erase(iter);
        }

        dump_foreach();
    }

    void test_forcompile()
    {
        ns_skiplist3::SkipList3<uint32_t, UserData> skip1;
        skip1.forEach(
            [](uint64_t key, UserData s, uint16_t)
            {
                return true;
            });

        skip1.forEach(
            [](uint64_t key, UserData s, uint16_t level)
            {
                return true;
            });

        UserData data;
        skip1.Insert(data.userid, data);
        skip1.UpdateScore(data.userid, data, data);

        skip1.Delete(data.userid, data);
    }

    void main()
    {
        test_forcompile();
    }
}  // namespace ns_skiplist3
#endif

namespace ns_toplist3
{
#define FUNC log_func(__FUNCTION__)

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

    void cacheRankList()
    {
        rank_map.clear();
        uint32_t rank = 0;
        toplist->forEach(
            [&rank](const UserDataPtr& data)
            {
                rank_map.emplace(data->userid, ++rank);
                return true;
            });
    }

    void cacheRevRankList()
    {
        revrank_map.clear();
        uint32_t rank = 0;
        toplist->forEachRev(
            [&rank](const UserDataPtr& data)
            {
                revrank_map.emplace(data->userid, ++rank);
                return true;
            });
    }

    void forEachRank()
    {
        FUNC;
        uint32_t rank = 0;
        toplist->forEach(
            [&rank](const UserDataPtr& data)
            {
                INFO("rank: {}, \tdata: {}", ++rank, data->dump());

                auto iter = origin_map.find(data->userid);
                assert(iter != origin_map.end());
                assert(iter->second == data);
                return true;
            });
    }

    void test_forEachRev()
    {
        FUNC;
        uint32_t rank = 0;
        toplist->forEachRev(
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
        for (uint32_t i = 0; i < max_size; i++)
        {
            const uint32_t new_id = i + 10000;
            UserDataPtr    ptr    = std::make_shared<UserData>();
            ptr->userid           = new_id;
            ptr->age              = randOne();
            ptr->score            = randOne();
            toplist->insert(ptr->userid, ptr);
            origin_map.emplace(ptr->userid, ptr);

            INFO("init_toplist: {}, \tdata: {}", new_id, ptr->dump());
        }
    }

    void main_toplist3(int argc, char** argv)
    {
        cmdline::parser cmd_parser = parse_line(argc, argv);

        max_size                   = cmd_parser.get<std::uint32_t>("size");
        const std::string& toptype = cmd_parser.get<std::string>("type");
        const std::string& op      = cmd_parser.get<std::string>("op");

        toplist = createToplist(toptype);
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

        forEachRank();
    }

    void test_update()
    {
        FUNC;
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

            toplist->insert(ptr->userid, ptr);

            assert(ptr->score + old_score == iter->second->score);

            INFO("test_update: key:{}, old_score:{}, newscore:{}", new_id, old_score, iter->second->score);
        }
    }

    void test_getDataByRank()
    {
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
        }
    }

    void test_getRankByKey()
    {
        for (const auto& origin : origin_map)
        {
            const uint32_t rank = toplist->getRankByKey(origin.first);
            auto           iter = rank_map.find(origin.first);
            assert(iter != rank_map.end());
            assert(iter->second == rank);
        }
    }

    void test_getRevRankByKey()
    {
        for (const auto& origin : origin_map)
        {
            const uint32_t rank = toplist->getRevRankByKey(origin.first);
            auto           iter = revrank_map.find(origin.first);
            assert(iter != revrank_map.end());
            assert(iter->second == rank);
        }
    }

    void test_forEachByRangedRank()
    {
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
        }
    }

    void test_deleteByKey()
    {
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
        }
    }

    void test_deleteByRank()
    {
        for (uint32_t i = 0; i < max_size + 1000; i++)
        {
            if (randOne() % 2 == 0)
                continue;

            UserDataPtr ptr1 = toplist->getDataByRank(i);
            if (!ptr1)
                continue;

            const uint32_t ret = toplist->deleteByRank(i);

            UserDataPtr ptr2 = toplist->getDataByRank(i);
            if (ptr1 != ptr2)
            {
                assert(ret == 1);
                INFO("test_deleteByRank: success: {}, {}", i, ptr1->dump());
                // forEachRank();
            }
            else
            {
                assert(ret == 0);
                INFO("test_deleteByRank: failed: {}, {}", i, ptr1->dump());
                assert(false);
            }
        }
    }

    void test_deleteByRangedRank()
    {
        for (uint32_t i = 0; i < max_size + 10; i++)
        {
            if (randOne() % 2 == 0)
                continue;

            const uint32_t from = randOne();
            const uint32_t to   = randOne();

            const uint32_t rankfrom = std::min(from, to);
            const uint32_t rankto   = std::max(from, to);

            INFO("test_deleteByRangedRank: [{}] --> [{}]", rankfrom, rankto);
            const uint32_t ret = toplist->deleteByRank(rankfrom, rankto);
            if (ret)
            {
                forEachRank();
                // INFO("test_deleteByRangedRank: [{}] --> [{}]", rankfrom, rankto);
            }
            else
            {
                // assert(false);
            }
        }
    }

}  // namespace ns_toplist3

#if 0
namespace ns_toplist3_v2
{
#define FUNC log_func(__FUNCTION__)

    // uint32_t    max_size     = 0;
    std::string toplist_type = {};

    TopList<uint32_t, UserData>*    toplist     = nullptr;
    std::map<uint32_t, UserDataPtr> origin_map  = {};
    std::map<uint32_t, uint32_t>    rank_map    = {};  //< key, rank>
    std::map<uint32_t, uint32_t>    revrank_map = {};  //< key, revrank>

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

    void main_toplist3_v2(int argc, char** argv)
    {
        cmdline::parser cmd_parser = parse_line(argc, argv);

        max_size                   = cmd_parser.get<std::uint32_t>("size");
        const std::string& toptype = cmd_parser.get<std::string>("type");
        const std::string& op      = cmd_parser.get<std::string>("op");

        toplist = createToplist(toptype);
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
        /*
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
        */
        forEachRank();
    }

    void init_toplist()
    {
        FUNC;
        for (uint32_t i = 0; i < max_size; i++)
        {
            const uint32_t new_id = i + 10000;
            UserDataPtr    ptr    = std::make_shared<UserData>();
            ptr->userid           = new_id;
            ptr->age              = randOne() % 5 + 1;
            ptr->score            = randOne() % 5 + 1;
            toplist->insert(ptr->userid, ptr);
            origin_map.emplace(ptr->userid, ptr);

            INFO("init_toplist: {}, \tdata: {}", new_id, ptr->dump());
        }
    }

    void test_update()
    {
        FUNC;
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

            toplist->insert(ptr->userid, ptr);

            assert(ptr->score + old_score == iter->second->score);

            INFO("test_update: key:{}, old_score:{}, newscore:{}", new_id, old_score, iter->second->score);
        }
    }
}  // namespace ns_toplist3_v2
#endif

namespace ns_skip3
{
    // ./bin/example --type=vec/set/skip --op=insert/update --size=10
    // ./bin/example --type=skip --op=insert --size=10
    void main_skip3(int argc, char** argv)
    {
        ns_toplist3::main_toplist3(argc, argv);  // cost time test
        // ns_toplist3_v2::main_toplist3_v2(argc, argv);
    }
}  // namespace ns_skip3