#include "TopList.h"
#include "TimerUsage.h"
#include <map>

#define FUNC print_func(__FUNCTION__)

namespace ns_struct
{
    struct Data
    {
        uint32_t key   = 0;
        uint32_t score = 0;

        Data(uint32_t k) : key(k) {}

        uint32_t getKey() const
        {
            return key;
        }

        uint32_t getScore() const
        {
            return score;
        }

        void setScore(uint32_t sc)
        {
            score = sc;
        }

        Data& operator+=(const Data& other)
        {
            this->score += other.score;
            return *this;
        }
    };

    bool operator<(const std::shared_ptr<Data>& lhs, const std::shared_ptr<Data>& rhs)
    {
        if (!lhs || !rhs)
            return false;

        if (lhs->getScore() == rhs->getScore())
            return lhs->getKey() < rhs->getKey();

        return lhs->getScore() < rhs->getScore();
    }
    /*
    bool operator<(const Data& lhs, const Data& rhs)
    {
        if (lhs.getScore() == rhs.getScore())
            return lhs.getKey() < rhs.getKey();

        return lhs.getScore() < rhs.getScore();
    }
    */
}  // namespace ns_struct

using DataType = ns_struct::Data;
using DataPtr  = std::shared_ptr<DataType>;

namespace ns_toplist
{
    void print_func(const std::string& str)
    {
        INFO("[{}] ====================", str);
    }

    BaseTopList<DataType>* createToplist(const std::string& type);

    void dumpAll(const std::string& tag);
    void cachedIncrRank();
    void cachedDecrRank();

    void init_toplist();
    void test_updateInfo();

    void test_deleteByKey();
    void test_deleteByRank();
    void test_deleteByRangedRank();
    void test_deleteByRangedScore();

    void test_getIncrRankByKey();
    void test_getDecrRankByKey();

    void test_getIncrRankByScore();
    void test_getDecrRankByScore();

    void test_incrScore();
    void test_decrScore();

    void test_forEachByIncr();
    void test_forEachByDecr();
    void test_forEachByRangedRank();
    void test_forEachByRangedScore();

    BaseTopList<DataType>*       toplist          = nullptr;
    std::map<uint32_t, DataPtr>  origin_map       = {};
    uint32_t                     max_size         = 0;
    std::map<uint32_t, uint32_t> cached_incr_rank = {};  // increase rank of toplist
    std::map<uint32_t, uint32_t> cached_decr_rank = {};  // decrease rank of toplist

    // ./bin/example --toptype=skip/vec/skiplist --size=50 --case=test_updateInfo
    void main(const std::string& type, const uint32_t size, const std::string& testtype)
    {
        max_size = size;
        toplist  = createToplist(type);
        assert(toplist);
        init_toplist();
        // test_forEachByIncr();
        if (testtype == "test_updateInfo")
        {
            test_updateInfo();
            // test_forEachByIncr();
        }
        else if (testtype == "test_getIncrRankByKey")
            test_getIncrRankByKey();
        else if (testtype == "test_getDecrRankByKey")
            test_getDecrRankByKey();
        // else if (testtype == "test_getIncrRankByScore")
        //     test_getIncrRankByScore();
        // else if (testtype == "test_getDecrRankByScore")
        //     test_getDecrRankByScore();
        else if (testtype == "test_incrScore")
            test_incrScore();
        else if (testtype == "test_decrScore")
            test_decrScore();
        else if (testtype == "test_forEachByIncr")
            test_forEachByIncr();
        else if (testtype == "test_forEachByDecr")
            test_forEachByDecr();
        else if (testtype == "test_forEachByRangedRank")
            test_forEachByRangedRank();
        else if (testtype == "test_forEachByRangedScore")
            test_forEachByRangedScore();
        else if (testtype == "test_deleteByKey")
            test_deleteByKey();
        else if (testtype == "test_deleteByRank")
            test_deleteByRank();
        else if (testtype == "test_deleteByRangedRank")
            test_deleteByRangedRank();
        else if (testtype == "test_deleteByRangedScore")
            test_deleteByRangedScore();
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

    void cachedIncrRank()
    {
        FUNC;
        uint32_t rank = 1;
        cached_incr_rank.clear();
        toplist->forEachByIncr(
            [&rank](const DataPtr& data)
            {
                if (!data)
                    return false;

                INFO("[rank: {}, key: {}, score: {}]", rank, data->getKey(), data->getScore());
                cached_incr_rank.emplace(data->getKey(), rank);
                rank++;
                return true;
            });
    }

    void cachedDecrRank()
    {
        FUNC;
        uint32_t rank = 1;
        cached_decr_rank.clear();
        toplist->forEachByDecr(
            [&rank](const DataPtr& data)
            {
                if (!data)
                    return false;

                INFO("[rank: {}, key: {}, score: {}]", rank, data->getKey(), data->getScore());
                cached_decr_rank.emplace(data->getKey(), rank);
                rank++;
                return true;
            });
    }

    void dumpIncrAll(const std::string& tag)
    {
        INFO("[dumpIncrAll] {} begin----------------------------------", tag);
        uint32_t rank = 0;
        toplist->forEachByIncr(
            [&rank](const DataPtr& data)
            {
                if (!data)
                    return false;

                INFO("[rank: {}, key: {}, score: {}]", ++rank, data->getKey(), data->getScore());
                return true;
            });
        // INFO("[dumpAll] {} end----------------------------------", tag);
    }

    void dumpDecrAll(const std::string& tag)
    {
        INFO("[dumpDecrAll] {} begin----------------------------------", tag);
        uint32_t rank = 0;
        toplist->forEachByDecr(
            [&rank](const DataPtr& data)
            {
                if (!data)
                    return false;

                INFO("[rank: {}, key: {}, score: {}]", ++rank, data->getKey(), data->getScore());
                return true;
            });
        // INFO("[dumpAll] {} end----------------------------------", tag);
    }

    void init_toplist()
    {
        // BLOCK_COST;
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            DataPtr ptr = std::make_shared<DataType>(idx + 1);
            ptr->setScore(cncpp::random() % max_size + 1 + 1000);
            toplist->addData(ptr);
            origin_map.emplace(ptr->getKey(), ptr);
        }

        // dumpIncrAll("after init toplist");
        // dumpDecrAll("after init toplist");
    }

    void test_updateInfo()
    {
        FUNC;
        BLOCK_COST;
        for (auto& item : origin_map)
        {
            const uint32_t old_score = item.second->getScore();
            const uint32_t new_score = cncpp::random() % max_size + 1 + 1000;
            item.second->setScore(new_score);
            toplist->updateInfos(item.second);
            assert(new_score == item.second->getScore());
            INFO(" [key: {}, score: {} --> {}]", item.second->getKey(), old_score, item.second->getScore());
        }

        // dumpIncrAll("after test_updateInfo");

        // test_getIncrRankByKey();
    }

    void test_getIncrRankByKey()
    {
        FUNC;
        cachedIncrRank();
        BLOCK_COST;
        for (const auto& item : origin_map)
        {
            auto iter = cached_incr_rank.find(item.first);
            assert(iter != cached_incr_rank.end());

            const uint32_t rank = toplist->getIncrRankByKey(item.first);
            assert(rank == iter->second);
        }
    }

    void test_getDecrRankByKey()
    {
        FUNC;
        cachedDecrRank();
        BLOCK_COST;
        for (const auto& item : origin_map)
        {
            auto iter = cached_decr_rank.find(item.first);
            assert(iter != cached_decr_rank.end());

            const uint32_t rank = toplist->getDecrRankByKey(item.first);
            assert(rank == iter->second);
        }
    }

    void test_getIncrRankByScore()
    {
        FUNC;
        cachedIncrRank();
        BLOCK_COST;
        for (const auto& item : origin_map)
        {
            auto iter = cached_incr_rank.find(item.first);
            assert(iter != cached_incr_rank.end());

            const uint32_t rank = toplist->getIncrRankByScore(item.second->getScore());
            // assert(rank == iter->second);  // some key with same score
            INFO("[test_getIncrRankByScore] key:{}, score: {}, keyrank: {}, scorerank: {}", item.first, item.second->getScore(),
                iter->second, rank);
        }
    }

    void test_getDecrRankByScore()
    {
        FUNC;
        cachedDecrRank();
        BLOCK_COST;
        for (const auto& item : origin_map)
        {
            auto iter = cached_decr_rank.find(item.first);
            assert(iter != cached_decr_rank.end());

            const uint32_t rank = toplist->getDecrRankByKey(item.second->getScore());
            // assert(rank == iter->second);

            INFO("[test_getDecrRankByScore] key:{}, score: {}, keyrank: {}, scorerank: {}", item.first, item.second->getScore(),
                iter->second, rank);
        }
    }

    void test_incrScore()
    {
        FUNC;
        BLOCK_COST;
        // cachedIncrRank();
        for (const auto& item : origin_map)
        {
            const uint32_t add_score = cncpp::random() % max_size + 1;
            const uint32_t old_score = item.second->getScore();
            toplist->incrScore(item.first, add_score);
            assert(item.second->getScore() == (add_score + old_score));
            INFO("[test_incrScore] key: {}, score: {} --> {}", item.first, old_score, item.second->getScore());
        }

        // test_getIncrRankByKey();
    }

    void test_decrScore()
    {
        FUNC;
        BLOCK_COST;
        // cachedDecrRank();
        for (const auto& item : origin_map)
        {
            const uint32_t sub_score = cncpp::random() % max_size + 1;
            const uint32_t old_score = item.second->getScore();
            toplist->decrScore(item.first, sub_score);
            assert(item.second->getScore() == SAFE_SUB(old_score, sub_score));
            INFO("[test_decrScore] key: {}, score: {} --> {}", item.first, old_score, item.second->getScore());
        }

        // test_getDecrRankByKey();
    }

    void test_forEachByIncr()
    {
        FUNC;
        BLOCK_COST;
        uint32_t rank = 1;
        toplist->forEachByIncr(
            [&rank](const DataPtr& data)
            {
                if (!data)
                    return false;

                cncpp::sleepfor_nanoseconds(1);
                INFO("[rank: {}, key: {}, score: {}]", rank, data->getKey(), data->getScore());
                rank++;
                return true;
            });
    }

    void test_forEachByDecr()
    {
        FUNC;
        BLOCK_COST;
        uint32_t rank = 1;
        toplist->forEachByDecr(
            [&rank](const DataPtr& data)
            {
                if (!data)
                    return false;

                cncpp::sleepfor_nanoseconds(1);
                INFO("[rank: {}, key: {}, score: {}]", rank++, data->getKey(), data->getScore());
                rank++;
                return true;
            });
    }

    void test_forEachByRangedRank()
    {
        FUNC;
        BLOCK_COST;
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            const uint32_t rankfrom = std::max<uint32_t>(1, cncpp::random() % max_size + 1);
            const uint32_t rankto   = std::min<uint32_t>(max_size, cncpp::random() % max_size + 1);

            const uint32_t from = std::min<uint32_t>(rankto, rankfrom);
            const uint32_t to   = std::max<uint32_t>(rankto, rankfrom);

            INFO("rank: {} ---> {}", from, to);
            toplist->forEachByRangedRank(from, to,
                [](const DataPtr& data)
                {
                    cncpp::sleepfor_nanoseconds(1);
                    INFO("[key: {}, score: {}]", data->getKey(), data->getScore());
                    return true;
                });
        }
    }
    void test_forEachByRangedScore()
    {
        FUNC;
        BLOCK_COST;
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            const uint32_t minscore = cncpp::random() % max_size + 1000;
            const uint32_t maxscore = cncpp::random() % max_size + 1000;

            const uint32_t from = std::min<uint32_t>(minscore, maxscore);
            const uint32_t to   = std::max<uint32_t>(minscore, maxscore);

            INFO("score: {} ---> {}", from, to);
            toplist->forEachByRangedScore(from, to,
                [](const DataPtr& data)
                {
                    // cncpp::sleepfor_nanoseconds(1);
                    INFO("[key: {}, score: {}]", data->getKey(), data->getScore());
                    return true;
                });
        }
    }

    void test_deleteByKey()
    {
        FUNC;
        BLOCK_COST;
        for (const auto& item : origin_map)
        {
            if (cncpp::random() % 3 != 1)
                continue;

            toplist->deleteByKey(item.second->getKey());
        }
    }
    void test_deleteByRank()
    {
        FUNC;
        BLOCK_COST;
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            if (cncpp::random() % 3 != 1)
                continue;

            toplist->deleteByRank(cncpp::random() % max_size + 1);
        }
    }

    void test_deleteByRangedRank()
    {
        FUNC;
        BLOCK_COST;
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            if (cncpp::random() % 3 != 1)
                continue;

            const uint32_t from = cncpp::random() % max_size + 1;
            const uint32_t to   = cncpp::random() % max_size + 1;

            const uint32_t rf = std::min<uint32_t>(from, to);
            const uint32_t rt = std::min<uint32_t>(from, to);
            toplist->deleteByRangedRank(rf, rt);
        }
    }
    void test_deleteByRangedScore()
    {
        FUNC;
        BLOCK_COST;
        for (uint32_t idx = 0; idx < max_size; idx++)
        {
            if (cncpp::random() % 3 != 1)
                continue;

            const uint32_t minscore = cncpp::random() % max_size + 1000;
            const uint32_t maxscore = cncpp::random() % max_size + 1000;

            const uint32_t from = std::min<uint32_t>(minscore, maxscore);
            const uint32_t to   = std::max<uint32_t>(minscore, maxscore);
            toplist->deleteByRangedScore(from, to);
        }
    }
}  // namespace ns_toplist