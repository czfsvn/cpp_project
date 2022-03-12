#include "Global.h"
#include "TimerUsage.h"
#include "TopList.h"
#include "cmdline.h"
#include "skiplist.h"
#include <list>
#include <map>
#include <set>
#include <vector>

namespace test_redis
{
    void test_insert()
    {
        using namespace ns_redis;
        ns_redis::SkipList skl;
        // = new ns_redis::SkipList();
        for (uint32_t idx = 0; idx < 5; idx++)
        {
            std::stringstream oss;
            oss << "name_" << idx;
            skl.zslInsert(cncpp::random() % 20 + 1, oss.str());
        }

        skl.printAll();
    };

    void main()
    {
        test_insert();
    }
}  // namespace test_redis

namespace test_myskiplist1
{
    struct TestData
    {
        uint32_t    uid;
        uint32_t    score = 0;
        std::string dump() const
        {
            std::stringstream oss;
            oss << "[id: " << uid << ", score: " << score << "]";
            return oss.str();
        }
    };

    bool operator<(const TestData& d1, const TestData& d2)
    {
        if (d1.score == d2.score)
            return d1.uid < d2.uid;

        return d1.score < d2.score;
    }

    bool operator<(const std::shared_ptr<TestData>& d1, const std::shared_ptr<TestData>& d2)
    {
        if (d1->score == d2->score)
            return d1->uid < d2->uid;

        return d1->score < d2->score;
    }

    /* wo'nt support raw ptr
    bool operator<(const TestData* d1, const TestData* d2)
    {
        if (d1->score == d2->score)
            return d1->uid < d2->uid;

        return d1->score < d2->score;
    }
    */
    void test_rawdata()
    {
        myskiplist1::SkipList<TestData> sk;
        for (int i = 1; i < 1000; i++)
        {
            TestData data;
            data.uid   = i;
            data.score = cncpp::random() % 100;
            sk.Insert(std::move(data));
        }

        sk.forEach(
            [](const TestData& data)
            {
                INFO("test_rawdata: {}", data.dump());
                return true;
            });
    }

    void test_sharedptr()
    {
        myskiplist1::SkipList<std::shared_ptr<TestData>> sk;
        for (int i = 1; i < 1000; i++)
        {
            std::shared_ptr<TestData> data = std::make_shared<TestData>();
            data->uid                      = i;
            data->score                    = cncpp::random() % 100;
            sk.Insert(std::move(data));
        }

        sk.forEach(
            [](const std::shared_ptr<TestData>& data)
            {
                INFO("test_sharedptr: {}", data->dump());
                return true;
            });
    }

    void test_rawptr()
    {
        myskiplist1::SkipList<TestData*> sk;
        for (int i = 1; i < 1000; i++)
        {
            TestData* data = new TestData;
            data->uid      = i;
            data->score    = cncpp::random() % 100;
            sk.Insert(data);
        }

        sk.forEach(
            [](const TestData* data)
            {
                INFO("test_rawptr: {}", data->dump());
                return true;
            });
    }

    void main()
    {
        test_rawdata();
        test_sharedptr();
        test_rawptr();
    }
}  // namespace test_myskiplist1

namespace test_myskiplist2
{

    struct TestData
    {
        uint32_t uid   = 0;
        uint32_t score = 0;
    };

    bool operator<(std::shared_ptr<TestData> d1, std::shared_ptr<TestData> d2)
    {
        if (!d1 || !d2)
            return false;
        if (d1->score == d2->score)
            return d1->uid < d2->uid;

        return d1->score < d2->score;
    }

    template <typename CONT>
    uint32_t getRank(CONT& cont, const uint32_t key, const uint32_t score)
    {
        uint32_t rank = 0;
        for (const auto& ptr : cont)
        {
            rank++;
            if (!ptr)
                continue;
            if (ptr->uid == key && ptr->score == score)
                return rank;
        }
        return 0;
    }

    static uint32_t                        max_size  = 30000;
    myskiplist2::SkipList                  sk        = {};
    std::vector<std::shared_ptr<TestData>> test_vec  = {};
    std::set<std::shared_ptr<TestData>>    test_set  = {};
    std::map<uint32_t, uint32_t>           datas_map = {};
    std::map<uint32_t, uint32_t>           rank_map  = {};  // <rank, key>

    void init_skiplist();
    void dump_skiplist(const std::string& log);

    void test_updatescore();
    void test_isInRange();
    void test_firstInrange();
    void test_lastInrange();

    void test_deleteRangeByScore();
    void test_deleteRangeByRank();

    void test_foreachRangeByScore();
    void test_foreachRangeByRank();

    void init_skiplist()
    {
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            const uint32_t key   = max_size - idx - 1;
            const uint32_t value = cncpp::random() % 10000 + 1;
            sk.Insert(key, value);
            datas_map.emplace(key, value);

            std::shared_ptr<TestData> tmp = std::make_shared<TestData>();
            tmp->uid                      = key;
            tmp->score                    = value;
            test_vec.emplace_back(tmp);
            test_set.emplace(tmp);
        }
        std::sort(test_vec.begin(), test_vec.end());
        // dump_skiplist("init_skiplist");
    }

    void dump_skiplist(const std::string& log)
    {
        std::cout << "dump_skiplist: " << log << std::endl;
        TRACE("dump_skiplist: {}", log);
        uint32_t idx = 1;
        sk.forEach(
            [&idx](const uint64_t key, const uint64_t score, const uint16_t level)
            {
                std::cout << idx << ", level: " << level << ", key: " << key << ", score: " << score << std::endl;
                TRACE("{}, level: {}, key: {}, score: {}", idx, level, key, score);
                rank_map[idx] = key;
                ++idx;
                return true;
            });
    }

    void test_getrank_sk()
    {
        std::cout << "test skiplist ======================================\n";
        BLOCK_COST;
        for (const auto& item : datas_map)
        {
            // std::cout << "key: " << item.first << ", rank: " << sk.GetRank(item.first, item.second) << std::endl;
            const uint32_t key  = item.first;
            const uint32_t rank = sk.GetRank(item.first, item.second);
            auto           iter = rank_map.find(rank);
            if (iter == rank_map.end())
            {
                std::cout << "error rankinfo, key: " << key << ", score: " << item.second << ", rank: " << rank << std::endl;
                assert(false);
            }

            if (iter->second != key)
            {
                std::cout << "error key info\n";
                assert(false);
            }

            std::cout << rank << ", key: " << key << ", score: " << item.second << std::endl;
        }
    }

    void test_getrank_vector()
    {
        std::cout << "test vector ======================================\n";
        BLOCK_COST;
        for (const auto& item : datas_map)
        {
            // std::cout << "key: " << item.first << ", rank: " << sk.GetRank(item.first, item.second) << std::endl;
            const uint32_t key  = item.first;
            const uint32_t rank = getRank(test_vec, item.first, item.second);
            auto           iter = rank_map.find(rank);
            if (iter == rank_map.end())
            {
                std::cout << "error rankinfo, key: " << key << ", score: " << item.second << ", rank: " << rank << std::endl;
                assert(false);
            }

            if (iter->second != key)
            {
                std::cout << "error key info\n";
                assert(false);
            }

            std::cout << rank << ", key: " << key << ", score: " << item.second << std::endl;
        }
    }
    void test_getrank_set()
    {
        std::cout << "test set ======================================\n";
        BLOCK_COST;
        for (const auto& item : datas_map)
        {
            // std::cout << "key: " << item.first << ", rank: " << sk.GetRank(item.first, item.second) << std::endl;
            const uint32_t key  = item.first;
            const uint32_t rank = getRank(test_set, item.first, item.second);
            auto           iter = rank_map.find(rank);
            if (iter == rank_map.end())
            {
                std::cout << "error rankinfo, key: " << key << ", score: " << item.second << ", rank: " << rank << std::endl;
                assert(false);
            }

            if (iter->second != key)
            {
                std::cout << "error key info\n";
                assert(false);
            }

            std::cout << rank << ", key: " << key << ", score: " << item.second << std::endl;
        }
    }
    void test_getrankcmp()
    {
        BLOCK_COST;
        for (const auto& item : datas_map)
        {
            // std::cout << "key: " << item.first << ", rank: " << sk.GetRank(item.first, item.second) << std::endl;
            const uint32_t key      = item.first;
            const uint32_t rank     = sk.GetRank(item.first, item.second);
            const uint32_t rank_set = getRank(test_set, item.first, item.second);
            const uint32_t rank_vec = getRank(test_vec, item.first, item.second);
            auto           iter     = rank_map.find(rank);
            if (iter == rank_map.end())
            {
                std::cout << "error rankinfo, key: " << key << ", score: " << item.second << ", rank: " << rank << std::endl;
                assert(false);
            }

            if (iter->second != key)
            {
                std::cout << "error key info\n";
                assert(false);
            }

            assert(rank == rank_set);
            assert(rank == rank_vec);
            assert(rank_set == rank_vec);

            // std::cout << rank << ", key: " << key << ", score: " << item.second << std::endl;
        }
    }

    void test_getKeyByRank()
    {
        for (const auto& item : rank_map)
        {
            const uint32_t getkey = sk.GetKeyByRank(item.first);
            assert(getkey == item.second);
        }
    }

    void test_getrank()
    {
        // test_getrank_sk();
        // test_getrank_set();
        // test_getrank_vector();
        test_getKeyByRank();
    }

    void test_rddelete()
    {
        for (uint32_t i = 0; i < 10; i++)
        {
            std::cout << "rddelete " << i << "   ------------------------" << std::endl;
            const uint32_t key  = cncpp::random() % max_size;
            auto           iter = datas_map.find(key);
            if (iter == datas_map.end())
                continue;
            const uint32_t before_rank = sk.GetRank(key, iter->second);

            // std::cout << "rddelete " << i << ", will delete: [" << key << "-" << iter->second << "], rank: " <<
            // before_rank << std::endl; dump_skiplist("before delete");
            TRACE("before delete key: {}, score: {}, rank: {}", key, iter->second, before_rank);
            sk.Delete(key, iter->second);
            // dump_skiplist("after delete");
            const uint32_t after_rank = sk.GetRank(key, iter->second);
            assert(after_rank == 0);
        }
    }

    void test_deleterank() {}

    void test_deleteall() {}

    void test_delete()
    {
        test_rddelete();
    }

    void test_updatescore()
    {
        // dump_skiplist("before update score");
        uint32_t cnt = 0;
        for (auto iter = datas_map.begin(); iter != datas_map.end() && cnt < datas_map.size(); iter++, cnt++)
        {
            const uint32_t ratio = cncpp::random() % 2;
            if (ratio)
                continue;

            const uint32_t new_score = cncpp::random() % 10000 + 1;
            // std::cout << "key: " << iter->first << ", old score: " << iter->second << ", new score: " << new_score <<
            // std::endl;
            sk.UpdateScore(iter->first, iter->second, new_score);

            const uint32_t newrank = sk.GetRank(iter->first, new_score);
            const uint32_t key     = sk.GetKeyByRank(newrank);
            iter->second           = new_score;

            assert(iter->first == key);
        }
        // dump_skiplist("after update score");
    }

    void test_isInRange()
    {
        dump_skiplist("after update score");
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            const uint32_t min = cncpp::random() % 10000;
            const uint32_t max = cncpp::random() % 10000 + min;

            auto node = sk.IsInRange(min, max);
            if (!node)
            {
                std::cout << "min: " << min << ", max: " << max << ", not isInRange" << std::endl;
                assert(false);
            }
            else
            {
                // std::cout << "key: " << node->key << ", score: " << node->score;
                std::cout << "min: " << min << ", max: " << max << ", isInRange" << std::endl;
            }
        }
    }

    void test_firstInrange()
    {
        dump_skiplist("test_firstInrange");
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            const uint32_t min = cncpp::random() % 10000;
            const uint32_t max = cncpp::random() % 10000 + min;

            auto node = sk.FirstInRange(min, max);
            if (!node)
            {
                std::cout << "min: " << min << ", max: " << max << std::endl;
                // assert(false);
            }
            else
            {
                std::cout << "key: " << node->key << ", score: " << node->score;
                std::cout << ", min: " << min << ", max: " << max << std::endl;
            }
        }
    }

    void test_lastInrange()
    {
        dump_skiplist("test_lastInrange");
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            const uint32_t min = cncpp::random() % 10000;
            const uint32_t max = cncpp::random() % 10000 + min;

            auto node = sk.LastInRange(min, max);
            if (!node)
            {
                std::cout << "min: " << min << ", max: " << max << std::endl;
                // assert(false);
            }
            else
            {
                std::cout << "key: " << node->key << ", score: " << node->score;
                std::cout << ", min: " << min << ", max: " << max << std::endl;
            }
        }
    }

    void test_foreachRangeByScore()
    {
        dump_skiplist("test_foreachRangeByScore");
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            const uint32_t min = cncpp::random() % 10000;
            const uint32_t max = cncpp::random() % 10000 + min;

            std::cout << "min: " << min << ", max: " << max << "=========================================\n";
            sk.forEachRangedScore(min, max,
                                  [](const uint64_t key, const uint64_t score)
                                  {
                                      std::cout << "key: " << key << ", score: " << score << std::endl;
                                      return true;
                                  });
        }
    }

    void test_foreachRangeByRank()
    {
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            const uint32_t start = cncpp::random() % 10;
            const uint32_t end   = cncpp::random() % 10 + start;

            std::cout << "start: " << start << ", end: " << end << "=========================================\n";
            sk.forEachRangedScore(start, end,
                                  [](const uint64_t key, const uint64_t score)
                                  {
                                      std::cout << "key: " << key << ", score: " << score << std::endl;
                                      return true;
                                  });
        }
    }

    void main()
    {
        max_size = 20;
        init_skiplist();
        /*
        test_delete();
        test_getrank();

        test_updatescore();

        test_isInRange();
        test_firstInrange();
        test_lastInrange();

        test_deleteRangeByScore();
        // test_deleteRangeByRank();
        */
        test_foreachRangeByScore();
        // test_foreachRangeByRank();
    }

}  // namespace test_myskiplist2

namespace test_toplist
{
    struct Data
    {
        uint32_t key   = 0;
        uint32_t score = 0;

        const uint32_t getKey() const
        {
            return key;
        }

        const uint32_t getScore() const
        {
            return score;
        }

        void setScore(uint32_t sc)
        {
            score = sc;
        }

        Data& operator+=(const Data& other)
        {
            return *this;
        }
    };

    static uint32_t                    max_size   = 0;
    myskiplist2::TopList<Data>         toplist    = {};
    std::vector<Data>                  vec_rank   = {};
    std::unordered_map<uint32_t, Data> orgin_map_ = {};

    void init_toplist();
    void test_updateInfos();

    void test_foreachTopList();
    void test_foreachByRank();
    void test_foreachByRangedScore();
    void test_foreachByRangedRank();

    void test_getRankByKey();
    void test_getRankByScore();  // maybe some record with same score,

    void test_getScore();
    void test_getScoreByRank();

    void test_delete();

    uint32_t vec_getRankByKey(const uint32_t key)
    {
        uint32_t rank = 1;
        for (const auto& item : vec_rank)
        {
            if (item.key == key)
                return rank;

            rank++;
        }
        return 0;
    }

    uint32_t vec_getRankByScore(const uint32_t score)
    {
        uint32_t rank = 1;
        for (const auto& item : vec_rank)
        {
            if (item.score == score)
                return rank;

            rank++;
        }
        return 0;
    }

    void dumpDatas(const std::string& logstr)
    {
        INFO("\n[dumpDatas] [{}] ---------------------------------------------", logstr);
        toplist.forEach(
            [&](const Data& data)
            {
                const uint32_t rank     = toplist.getRankByKey(data.getKey());
                const uint32_t rev_rank = toplist.getRevRankByKey(data.getKey());

                const uint32_t vec_rank1 = vec_getRankByKey(data.getKey());
                const uint32_t vec_rank2 = vec_getRankByScore(data.getScore());
                assert(rank == vec_rank1);
                // assert(rank == vec_rank2);       failed with some records with same rank score
                // assert(vec_rank1 == vec_rank2);  failed with some records with same rank score

                INFO("key: {}, score: {}, rank: {}/{}", data.getKey(), data.getScore(), rank, rev_rank);

                return true;
            });
    }

    void dumpDatasByRank(const std::string& logstr)
    {
        INFO("\n[dumpDatasByRank] [{}] ---------------------------------------------", logstr);

        for (uint32_t idx = 0; idx < max_size + 10; idx++)
        {
            // const Data& tmp = toplist.getRawDataByKey(idx + 1);
            const Data* data = toplist.getDataByRank(idx + 1);
            if (!data || data->getKey() == 0)
            {
                // assert(false);
                continue;
            }

            INFO("rank: {}, key: {}, score: {}", idx + 1, data->getKey(), data->getScore());
        }
    }

    void test1()
    {
        for (uint32_t idx = 0; idx < 10; idx++)
        {
            Data tmp;
            tmp.key   = idx + 100;
            tmp.score = cncpp::random() % 100 + 1;
            toplist.addData(tmp);
        }

        toplist.deleteByKey(1);
    }

    void init_toplist()
    {
        toplist.clear();
        vec_rank.clear();
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            Data tmp;
            tmp.key   = idx + 1;
            tmp.score = cncpp::random() % (max_size * 10) + 1;
            orgin_map_.emplace(tmp.key, tmp);
            toplist.addData(tmp);
            vec_rank.emplace_back(tmp);
        }

        std::sort(vec_rank.begin(), vec_rank.end(),
                  [](const Data& lhs, const Data& rhs)
                  {
                      if (lhs.getScore() == rhs.getScore())
                          return lhs.getKey() < rhs.getKey();

                      return lhs.getScore() < rhs.getScore();
                  });
        dumpDatas("init_toplist");

        dumpDatasByRank("init_toplist");
    }

    void test_updateInfos()
    {
        dumpDatasByRank("before: test_updateInfos");
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            if (cncpp::random() % 2 == 0)
                continue;

            const uint32_t key       = idx + 1;
            const uint32_t old_score = toplist.getScore(key);

            Data tmp;
            tmp.key         = key;
            tmp.score       = cncpp::random() % (max_size * 10) + 1;
            orgin_map_[key] = tmp;
            toplist.updateInfos(tmp);
            INFO("test_updateInfos: {}:{}-->{}", key, old_score, tmp.score);
        }
        dumpDatasByRank("after: test_updateInfos");
    }

    void test_getRankByKey()
    {
        dumpDatasByRank("test_getRankByKey");
        for (const auto& item : orgin_map_)
        {
            const uint32_t rank     = toplist.getRankByKey(item.first);
            const uint32_t vec_rank = vec_getRankByKey(item.first);
            INFO("test_getRankByKey: {}:{}---{}", item.first, item.second.getScore(), rank);
            if (rank != vec_rank)
            {
                // assert(false);
            }
        }
    }

    void test_foreachTopList()
    {
        toplist.forEach(
            [](const Data& data)
            {
                INFO("test_foreachTopList: {}:{}-->{}", data.key, data.score);
                return true;
            });
    }

    void test_foreachByRank()
    {
        INFO("test_foreachByRank: =====================================================");
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            const uint32_t rankfrom = cncpp::random() % max_size + 1;
            const uint32_t rankto   = cncpp::random() % max_size + 1;
            INFO("test_foreachByRank: idx: {}, rank: {}-->{}", idx, rankfrom, rankto);

            toplist.forEachByRank(rankfrom, rankto,
                                  [](const Data& data)
                                  {
                                      INFO("forEachByRank: {}-->{}", data.key, data.score);
                                      return true;
                                  });

            toplist.forEachByRangedRank(rankfrom, rankto,
                                        [](const Data& data)
                                        {
                                            INFO("forEachByRangedRank: {}-->{}", data.key, data.score);
                                            return true;
                                        });
        }
    }

    void test_foreachByRangedRank()
    {
        INFO("test_foreachByRangedRank: =================================================");
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            const uint32_t rankfrom = cncpp::random() % max_size + 1;
            const uint32_t rankto   = cncpp::random() % max_size + 1;
            INFO("test_foreachByRangedRank: idx: {}, rank: {}-->{}", idx, rankfrom, rankto);
        }
    }

    void test2()
    {
        myskiplist2::TopList<Data>& bk = toplist;
        bk.forEach(
            [&](const Data& data)
            {
                INFO("before deleteByRangedRank: key:{}, score:{}", data.key, data.score);
                return true;
            });

        bk.deleteByRangedRank(3, 6);

        bk.forEach(
            [&](const Data& data)
            {
                INFO("after deleteByRangedRank: key:{}, score:{}", data.key, data.score);
                return true;
            });

        bk.clear();

        bk.forEach(
            [&](const Data& data)
            {
                INFO("after clear: key:{}, score:{}", data.key, data.score);
                return true;
            });
    }

    void test_foreachByRangedScore();

    void main()
    {
        max_size = 10;
        // test1();
        init_toplist();

        // test_getRankByKey();

        // test_updateInfos();

        // test_getRankByKey();
        // test_foreachByRank();
        // test_foreachByRangedRank();

        test2();
    }

}  // namespace test_toplist

namespace sharedptr_toplist
{
    void init_toplist();
    void test_addData();
    void test_updateInfo();

    using DataTypePtr = std::shared_ptr<test_toplist::Data>;

    static uint32_t                 max_size  = 10;
    TopList<test_toplist::Data>     toplist   = {};
    std::map<uint32_t, DataTypePtr> datas_map = {};

    void dumpOne(const DataTypePtr& ptr)
    {
        if (!ptr)
            return;

        INFO("dumpone, key:{}, score:{}", ptr->getKey(), ptr->getScore());
    }

    void dumpOne(const uint32_t rank, const DataTypePtr& ptr)
    {
        if (!ptr)
            return;

        INFO("dumpone, rank: {}, key:{}, score:{}, ", rank, ptr->getKey(), ptr->getScore());
    }

    void dumpall(const std::string& logstr)
    {
        INFO("dumpall:{}    ==========================================================", logstr);
        toplist.forEach(
            [](const DataTypePtr& ptr)
            {
                if (!ptr)
                    return true;

                dumpOne(ptr);
                return true;
            });
    }

    void dumpByRank(const std::string& logstr)
    {
        INFO("dumpByRank:{}    ==========================================================", logstr);
        for (uint32_t idx = 1; idx <= max_size; idx++)
        {
            dumpOne(idx, toplist.getDataByRank(idx));
        }
    }

    void init_toplist()
    {
        for (uint32_t idx = 1; idx <= max_size; idx++)
        {
            auto ptr   = std::make_shared<test_toplist::Data>();
            ptr->key   = idx;
            ptr->score = cncpp::random() % (max_size * 10) + 1;
            toplist.addData(ptr);
            datas_map.emplace(idx, ptr);

            // dumpByRank("init_toplist idx ");
        }

        dumpall("init_toplist");
        dumpByRank("init_toplist");
    }

    void test_updateInfo()
    {
        for (uint32_t idx = 1; idx <= max_size; idx++)
        {
            if (cncpp::random() % 2 == 0)
                continue;

            auto iter = datas_map.find(idx);
            if (iter == datas_map.end())
                continue;

            const uint32_t key       = idx;
            const uint32_t score     = cncpp::random() % (max_size * 1) + 1;
            const uint32_t old_score = toplist.getScore(key);

            iter->second->score = score;

            toplist.updateInfos(iter->second);

            INFO("test_updateInfo, {}--{}--->{}", idx, old_score, score);
        }

        dumpall("test_updateInfo");
        dumpByRank("test_updateInfo");
    }

    void main()
    {
        max_size = 20;
        init_toplist();
        // test_updateInfo();
    }
}  // namespace sharedptr_toplist

namespace ns_template
{
    void test()
    {
        using DataType = test_toplist::Data;
        using DataPtr  = std::shared_ptr<DataType>;
        TopListVec<DataType>                     vec_toplist;
        TopListVec<DataType, std::list<DataPtr>> list_toplist;
        TopListSet<DataType, std::set<DataPtr>>  set_toplist;

        std::shared_ptr<DataType> tmp = std::make_shared<DataType>();
        vec_toplist.updateData(tmp);
        list_toplist.updateData(tmp);
        set_toplist.updateData(tmp);

        BaseTopList<DataType>* toplistvec  = new TopListVec<DataType>();
        BaseTopList<DataType>* toplistlist = new TopListVec<DataType, std::list<DataPtr>>();

        BaseTopList<DataType>* toplistset = new TopListSet<DataType>();

        DataPtr ptr = std::make_shared<DataType>();

        toplistvec->addData(ptr);
        toplistlist->addData(ptr);
        toplistset->addData(ptr);

        // toplistvec = toplistset;
    }
}  // namespace ns_template

#if 0
namespace ns_toplist
{
    using DataType = test_toplist::Data;
    using DataPtr  = std::shared_ptr<DataType>;

    void test()
    {
        // TopList<test_toplist::Data, std::vector<std::shared_ptr<test_toplist::Data>>> vec_toplist;
        // TopList<test_toplist::Data, std::set<std::shared_ptr<test_toplist::Data>>>      vec_toplist;
        BaseTopList<DataType>* tp1 = new TopListVec<DataType>();
        BaseTopList<DataType>* tp2 = new TopListSet<DataType>();
        BaseTopList<DataType>* tp3 = new TopListSkip<DataType>();

        DataPtr ptr = std::make_shared<DataType>();
        tp1->addData(ptr);
        tp1->addData(ptr);
        tp3->addData(ptr);
    }

    

    void main(const std::string& type, const uint32_t size)
    {
        max_size = size;
        init_toplist();
    }

    BaseTopList<DataType>* createToplist(const std::string& type)
    {
        if (type == "vec")
            return new TopListVec<DataType>();
        else if (type == "set")
            return new TopListSet<DataType>();
        else if (type == "skip")
            return new TopListSkip<DataType>();

        return nullptr;
    }

    void init_toplist() {}

}  // namespace ns_toplist

#endif

namespace test_skiplist
{
    cmdline::parser cmd_parser;

    void parse_line(int argc, char** argv)
    {
        cmd_parser.add<std::string>("toptype", 't', "toplist type", true, "");
        cmd_parser.add<uint32_t>("size", 's', "size of toplist", false, 10);
        cmd_parser.add<std::string>("case", 'c', "test function case", false, "");

        cmd_parser.parse_check(argc, argv);
    }

    void main_skiplist(int argc, char** argv)
    {
        // std::cout << "skiplist\n";

        parse_line(argc, argv);

        // test_redis::main();
        // test_myskiplist1::main();
        // test_myskiplist2::main();
        // test_toplist::main();
        // sharedptr_toplist::main();

        // ns_template::test();

        // ./bin/example --toptype=vec/set/skip --size=10
        ns_toplist::main(cmd_parser.get<std::string>("toptype"), cmd_parser.get<uint32_t>("size"), cmd_parser.get<std::string>("case"));
        return;
    }
}  // namespace test_skiplist
