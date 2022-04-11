#ifndef __TOPLIST_20220215_H__
#define __TOPLIST_20220215_H__

#include "skiplist.h"

namespace ns_template
{
    template <typename DATA>
    class BaseTopList
    {
    public:
        using DATA_PTR = std::shared_ptr<DATA>;
        // typedef std::shared_ptr<DATA> DATA_PTR;
        // typedef typename std::shared_ptr<DATA> DATA_PTR;
        std::unordered_map<uint64_t, DATA_PTR> datas_map = {};

        virtual void addData(const DATA_PTR& data)    = 0;
        virtual void updateData(const DATA_PTR& data) = 0;
    };

    template <typename DATA, typename CONT = std::vector<std::shared_ptr<DATA>>>
    class TopListVec : public BaseTopList<DATA>
    {
    public:
        CONT cont = {};
        // 这样声明才可使用父类的定义
        using DATA_PTR = typename BaseTopList<DATA>::DATA_PTR;

        void addData(const DATA_PTR& data)
        {
            // 访问父类的变量：需要加this 或者 父类的方式
            if (this->datas_map.empty())
                cont.emplace_back(data);

            if (BaseTopList<DATA>::datas_map.empty())
                cont.emplace_back(data);

            typename BaseTopList<DATA>::DATA_PTR data_ptr;
            if (!data_ptr)
                return;
        }

        void updateData(const DATA_PTR& data)
        {
            if (this->datas_map.empty())
                cont.emplace_back(data);

            if (BaseTopList<DATA>::datas_map.empty())
                cont.emplace_back(data);

            typename BaseTopList<DATA>::DATA_PTR data_ptr;
            if (!data_ptr)
                return;
        }
    };

    template <typename DATA, typename CONT = std::set<std::shared_ptr<DATA>>>
    class TopListSet : public BaseTopList<DATA>
    {
    public:
        CONT cont = {};
        // 这样声明才可使用父类的定义
        using DATA_PTR = typename BaseTopList<DATA>::DATA_PTR;

        void addData(const DATA_PTR& data)
        {
            // 访问父类的变量：需要加this 或者 父类的方式
            if (this->datas_map.empty())
                cont.emplace(data);

            if (BaseTopList<DATA>::datas_map.empty())
                cont.emplace(data);

            typename BaseTopList<DATA>::DATA_PTR data_ptr;
            if (!data_ptr)
                return;
        }

        void updateData(const DATA_PTR& data)
        {
            if (this->datas_map.empty())
                cont.emplace(data);

            if (BaseTopList<DATA>::datas_map.empty())
                cont.emplace(data);

            typename BaseTopList<DATA>::DATA_PTR data_ptr;
            if (!data_ptr)
                return;
        }
    };

}  // namespace ns_template

namespace ns_toplist
{
    void main(const std::string& type, const uint32_t size, const std::string& testcase);

    template <typename DATA>
    class BaseTopList
    {
    public:
        using DataSharedPtr = std::shared_ptr<DATA>;
        using FuncCB        = std::function<bool(const DataSharedPtr&)>;

    protected:
        std::unordered_map<uint32_t, DataSharedPtr> datas_map_ = {};

    public:
        virtual void deleteByKey(const uint64_t key)                                       = 0;
        virtual void deleteByRank(const uint64_t rank)                                     = 0;
        virtual void deleteByRangedRank(const uint64_t rankfrom, const uint64_t rankto)    = 0;
        virtual void deleteByRangedScore(const uint64_t minscore, const uint64_t maxscore) = 0;

        virtual uint64_t getIncrRankByKey(const uint64_t key) = 0;
        virtual uint64_t getDecrRankByKey(const uint64_t key)
        {
            const uint64_t incr_rank = getIncrRankByKey(key);
            return incr_rank ? SAFE_SUB(this->size() + 1, incr_rank) : incr_rank;
        }

        // return min rank if exist same score
        virtual uint32_t getIncrRankByScore(const uint64_t score) = 0;
        virtual uint32_t getDecrRankByScore(const uint64_t score) = 0;

        virtual bool incrScore(const uint64_t key, const uint64_t score) = 0;
        virtual bool decrScore(const uint64_t key, const uint64_t score) = 0;

        uint32_t getScore(const uint64_t key)
        {
            auto iter = datas_map_.find(key);
            if (iter == datas_map_.end() || !iter->second)
                return 0;

            return iter->second->getScore();
        }

        virtual uint64_t size()
        {
            return datas_map_.size();
        }

        virtual void clear() = 0;
        virtual void sortAll() {}

    public:
        virtual void          addData(const DataSharedPtr& data, bool overlap = true) = 0;
        virtual void          updateInfos(const DataSharedPtr& data)                  = 0;
        virtual DataSharedPtr getDataByRank(const uint64_t rank)                      = 0;

        DataSharedPtr getDataByKey(const uint64_t key)
        {
            auto iter = datas_map_.find(key);
            return iter == datas_map_.end() ? nullptr : iter->second;
        }

        template <typename FUNC>
        bool forEach(FUNC&& cb)
        {
            for (const auto& data : datas_map_)
            {
                if (!cb(data.second))
                    return false;
            }

            return true;
        }

        virtual bool forEachByIncr(FuncCB&& cb)                                                       = 0;
        virtual bool forEachByDecr(FuncCB&& cb)                                                       = 0;
        virtual bool forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FuncCB&& cb) = 0;

        virtual bool forEachByRangedScore(const uint64_t minscore, const uint64_t maxscore, FuncCB&& cb) = 0;

        /*
             'virtual' cannot be specified on member function templates  ??
        template <typename FUNC>
        virtual bool forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FUNC&& cb) = 0;

        template <typename FUNC>
        virtual bool forEachByRangedScore(const uint64_t minscore, const uint64_t maxscore, FUNC&& cb) = 0;
        */
    };

    template <typename DATA, typename CONT = std::vector<std::shared_ptr<DATA>>>
    class TopListVec : public BaseTopList<DATA>
    {
    private:
        CONT sort_datas_;
        using DataSharedPtr = typename BaseTopList<DATA>::DataSharedPtr;
        using FuncCB        = typename BaseTopList<DATA>::FuncCB;

    public:
        void clear() override
        {
            this->datas_map_.clear();
            sort_datas_.clear();
        }

        void sortAll() override
        {
            std::sort(sort_datas_.begin(), sort_datas_.end());
        }

        void addData(const DataSharedPtr& data, bool overlap = true) override
        {
            auto iter = this->datas_map_.find(data->getKey());
            if (iter == this->datas_map_.end())
            {
                // not existing
                this->datas_map_.emplace(data->getKey(), data);
                sort_datas_.emplace_back(data);
                sortAll();
            }
            else
            {
                // todo:
                // add log recorded error
            }
        }

        void updateInfos(const DataSharedPtr& data) override
        {
            if (!data)
                return;

            bool overlapped = false;
            auto iter       = this->datas_map_.find(data->getKey());
            if (iter != this->datas_map_.end())
            {
                /*
                if (iter->second.get() == data.get())
                {
                    sortAll();
                    overlapped = true;
                }
                else
                */
                {
                    for (auto it = sort_datas_.begin(); it != sort_datas_.end(); it++)
                    {
                        DataSharedPtr ptr = *it;
                        if (!ptr || data->getKey() != ptr->getKey())
                            continue;

                        *it          = data;
                        iter->second = data;
                        sortAll();
                        overlapped = true;
                        break;
                    }
                }
            }

            if (!overlapped)
            {
                addData(data);
            }
        }

        bool incrScore(const uint64_t key, const uint64_t score) override
        {
            auto iter = this->datas_map_.find(key);
            if (iter == this->datas_map_.end())
                return false;

            const uint64_t newscore = iter->second->getScore() + score;
            iter->second->setScore(newscore);
            sortAll();
            return true;
        }

        bool decrScore(const uint64_t key, const uint64_t score) override
        {
            auto iter = this->datas_map_.find(key);
            if (iter == this->datas_map_.end())
                return false;

            const uint64_t newscore = SAFE_SUB(iter->second->getScore(), score);
            iter->second->setScore(newscore);
            sortAll();
            return true;
        }

        void deleteByKey(const uint64_t key) override
        {
            auto it = this->datas_map_.find(key);
            if (it == this->datas_map_.end())
                return;

            this->datas_map_.erase(it);
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if ((*iter)->getKey() != key)
                    continue;

                sort_datas_.erase(iter);
                return;
            }
        }

        void deleteByRank(const uint64_t rank) override
        {
            if (rank > this->size())
                return;

            uint32_t idx = 0;
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if (++idx != rank)
                    continue;

                this->datas_map_.erase((*iter)->getKey());
                sort_datas_.erase(iter);
                return;
            }
        }

        void deleteByRangedRank(const uint64_t rankfrom, const uint64_t rankto) override
        {
            const uint64_t to = std::min(rankto, this->size());
            if (rankfrom > to || rankfrom > this->size())
                return;

            uint64_t removed_count = 0;
            uint64_t will_rmcount  = SAFE_SUB(to, rankfrom);
            uint32_t idx           = 0;

            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end() && removed_count > will_rmcount;)
            {
                if (++idx < rankfrom)
                {
                    iter++;
                    continue;
                }

                removed_count++;
                this->datas_map_.erase((*iter)->getKey());
                iter = sort_datas_.erase(iter);
            }
        }

        void deleteByRangedScore(const uint64_t minscore, const uint64_t maxscore) override
        {
            if (minscore > maxscore)
                return;

            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end();)
            {
                if ((*iter)->getScore() < minscore || (*iter)->getScore() > maxscore)
                    continue;

                this->datas_map_.erase((*iter)->getKey());
                iter = sort_datas_.erase(iter);
            }
        }

        uint64_t getIncrRankByKey(const uint64_t key) override
        {
            auto it = this->datas_map_.find(key);
            if (it == this->datas_map_.end())
                return 0;

            uint32_t idx = 1;
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if ((*iter)->getKey() != key)
                {
                    idx++;
                    continue;
                }

                return idx;
            }

            return 0;
        }

        uint32_t getIncrRankByScore(const uint64_t score) override
        {
            uint32_t idx = 1;
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if ((*iter)->getScore() < score)
                {
                    idx++;
                    continue;
                }

                if ((*iter)->getScore() > score)
                    return 0;

                return idx;
            }
            return 0;
        }

        uint32_t getDecrRankByScore(const uint64_t score) override
        {
            uint32_t idx = 1;
            for (auto iter = sort_datas_.rbegin(); iter != sort_datas_.rend(); ++iter)
            {
                if ((*iter)->getScore() > score)
                {
                    idx++;
                    continue;
                }

                if ((*iter)->getScore() < score)
                    return 0;

                return idx;
            }
            return 0;
        }

        DataSharedPtr getDataByRank(const uint64_t rank) override
        {
            if (sort_datas_.empty() || rank > sort_datas_.size())
                return nullptr;

            return sort_datas_[SAFE_SUB(rank, 1)];
            /*
            uint32_t idx = 0;
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if (++idx != rank)
                    continue;

                return *iter;
            }
            return nullptr;
            */
        }

        bool forEachByIncr(FuncCB&& cb) override
        {
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if (!cb(*iter))
                    return false;
            }
            return true;
        }

        bool forEachByDecr(FuncCB&& cb) override
        {
            for (auto iter = sort_datas_.rbegin(); iter != sort_datas_.rend(); ++iter)
            {
                if (!cb(*iter))
                    return false;
            }
            return true;
        }

        bool forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FuncCB&& cb) override
        {
            if (rankfrom > rankto)
                return false;

            if (sort_datas_.empty() || rankfrom > sort_datas_.size())
                return false;

            for (uint32_t rank = rankfrom; rank < rankto && rank < sort_datas_.size(); ++rank)
            {
                if (!cb(sort_datas_[SAFE_SUB(rank, 1)]))
                    return false;
            }

            return true;

            /*
            uint32_t rank = 0;
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                rank++;
                if (rank < rankfrom)
                    continue;

                if (rank > rankto)
                    break;

                if (!cb(*iter))
                    return false;
            }

            return true;
            */
        }

        bool forEachByRangedScore(const uint64_t minscore, const uint64_t maxscore, FuncCB&& cb) override
        {
            if (minscore > maxscore)
                return false;

            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                DataSharedPtr data = *iter;
                if (!data)
                    continue;

                if (data->getScore() < minscore)
                    continue;

                if (data->getScore() > maxscore)
                    break;

                if (!cb(data))
                    return false;
            }
            return true;
        }
    };

    template <typename DATA, typename CONT = std::set<std::shared_ptr<DATA>>>
    class TopListSet : public BaseTopList<DATA>
    {
    private:
        CONT sort_datas_;
        using DataSharedPtr = typename BaseTopList<DATA>::DataSharedPtr;
        using FuncCB        = typename BaseTopList<DATA>::FuncCB;

    public:
        void clear() override
        {
            this->datas_map_.clear();
            sort_datas_.clear();
        }

        void sortAll() override {}

        void addData(const DataSharedPtr& data, bool overlap = true) override
        {
            auto iter = this->datas_map_.find(data->getKey());
            if (iter == this->datas_map_.end())
            {
                // not existing
                this->datas_map_.emplace(data->getKey(), data);
                sort_datas_.emplace(data);
            }
            else
            {
                // todo:
                // add log recorded error
            }
        }

        void updateInfos(const DataSharedPtr& data) override
        {
            if (!data)
                return;

            auto iter = this->datas_map_.find(data->getKey());
            if (iter != this->datas_map_.end())
            {
                for (auto it = sort_datas_.begin(); it != sort_datas_.end(); it++)
                {
                    DataSharedPtr ptr = *it;
                    if (!ptr || data->getKey() != ptr->getKey())
                        continue;

                    sort_datas_.erase(it);
                    this->datas_map_.erase(iter);
                    break;
                }
            }

            addData(data);
        }

        bool incrScore(const uint64_t key, const uint64_t score) override
        {
            auto iter = this->datas_map_.find(key);
            if (iter == this->datas_map_.end())
                return false;

            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end();)
            {
                DataSharedPtr data = *iter;
                if (!data)
                {
                    iter++;
                    continue;
                }

                if (data->getKey() != key)
                {
                    iter++;
                    continue;
                }

                sort_datas_.erase(iter);
                const uint64_t newscore = data->getScore() + score;
                data->setScore(newscore);
                sort_datas_.emplace(data);
                return true;
            }
            return false;
        }

        bool decrScore(const uint64_t key, const uint64_t score) override
        {
            auto iter = this->datas_map_.find(key);
            if (iter == this->datas_map_.end())
                return false;

            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end();)
            {
                DataSharedPtr data = *iter;
                if (!data)
                {
                    iter++;
                    continue;
                }

                if (data->getKey() != key)
                {
                    iter++;
                    continue;
                }

                sort_datas_.erase(iter);
                const uint64_t newscore = SAFE_SUB(data->getScore(), score);
                data->setScore(newscore);
                sort_datas_.emplace(data);
                return true;
            }
            return false;
        }

        void deleteByKey(const uint64_t key) override
        {
            auto it = this->datas_map_.find(key);
            if (it == this->datas_map_.end())
                return;

            this->datas_map_.erase(it);
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if ((*iter)->getKey() != key)
                    continue;

                sort_datas_.erase(iter);
                return;
            }
        }

        void deleteByRank(const uint64_t rank) override
        {
            if (rank > this->size())
                return;

            uint32_t idx = 0;
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if (++idx != rank)
                    continue;

                this->datas_map_.erase((*iter)->getKey());
                sort_datas_.erase(iter);
                return;
            }
        }

        void deleteByRangedRank(const uint64_t rankfrom, const uint64_t rankto) override
        {
            const uint64_t to = std::min(rankto, this->size());
            if (rankfrom > to || rankfrom > this->size())
                return;

            uint64_t removed_count = 0;
            uint64_t will_rmcount  = SAFE_SUB(to, rankfrom);
            uint32_t idx           = 0;

            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end() && removed_count > will_rmcount;)
            {
                if (++idx < rankfrom)
                {
                    iter++;
                    continue;
                }

                removed_count++;
                this->datas_map_.erase((*iter)->getKey());
                iter = sort_datas_.erase(iter);
            }
        }

        void deleteByRangedScore(const uint64_t minscore, const uint64_t maxscore) override
        {
            if (minscore > maxscore)
                return;

            // todo : there maybe some wrong operation , need  check ! ! !
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end();)
            {
                if ((*iter)->getScore() < minscore || (*iter)->getScore() > maxscore)
                    continue;

                this->datas_map_.erase((*iter)->getKey());
                iter = sort_datas_.erase(iter);
            }
        }

        uint64_t getIncrRankByKey(const uint64_t key) override
        {
            auto it = this->datas_map_.find(key);
            if (it == this->datas_map_.end())
                return 0;

            uint32_t idx = 1;
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if ((*iter)->getKey() != key)
                {
                    idx++;
                    continue;
                }

                return idx;
            }

            return 0;
        }

        uint32_t getIncrRankByScore(const uint64_t score) override
        {
            uint32_t idx = 1;
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if ((*iter)->getScore() < score)
                {
                    idx++;
                    continue;
                }

                if ((*iter)->getScore() > score)
                    return 0;

                return idx;
            }
            return 0;
        }

        uint32_t getDecrRankByScore(const uint64_t score) override
        {
            uint32_t idx = 1;
            for (auto iter = sort_datas_.rbegin(); iter != sort_datas_.rend(); ++iter)
            {
                if ((*iter)->getScore() > score)
                {
                    idx++;
                    continue;
                }

                if ((*iter)->getScore() < score)
                    return 0;

                return idx;
            }
            return 0;
        }

        DataSharedPtr getDataByRank(const uint64_t rank) override
        {
            uint32_t idx = 0;
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if (++idx != rank)
                    continue;

                return *iter;
            }
            return nullptr;
        }

        bool forEachByIncr(FuncCB&& cb) override
        {
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                if (!cb(*iter))
                    return false;
            }
            return true;
        }

        bool forEachByDecr(FuncCB&& cb) override
        {
            for (auto iter = sort_datas_.rbegin(); iter != sort_datas_.rend(); ++iter)
            {
                if (!cb(*iter))
                    return false;
            }
            return true;
        }

        bool forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FuncCB&& cb) override
        {
            if (rankfrom > rankto)
                return false;

            uint32_t rank = 0;
            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                rank++;
                if (rank < rankfrom)
                    continue;

                if (rank > rankto)
                    break;

                if (!cb(*iter))
                    return false;
            }

            return true;
        }

        bool forEachByRangedScore(const uint64_t minscore, const uint64_t maxscore, FuncCB&& cb) override
        {
            if (minscore > maxscore)
                return false;

            for (auto iter = sort_datas_.begin(); iter != sort_datas_.end(); ++iter)
            {
                DataSharedPtr data = *iter;
                if (!data)
                    continue;
                if (data->getScore() < minscore)
                    continue;

                if (data->getScore() > maxscore)
                    break;

                if (!cb(data))
                    return false;
            }
            return true;
        }
    };

    template <typename DATA>
    class TopListSkip : public BaseTopList<DATA>
    {
    private:
        using DataSharedPtr = typename BaseTopList<DATA>::DataSharedPtr;
        using FuncCB        = typename BaseTopList<DATA>::FuncCB;

    private:
        myskiplist2::SkipList sort_datas_ = {};

    public:
        void clear() override
        {
            this->datas_map_.clear();
            sort_datas_.DeleteRangeByRank(1, this->size() + 1);
        }

        void addData(const DataSharedPtr& data, bool overlap = true) override
        {
            auto iter = this->datas_map_.find(data->getKey());
            if (iter == this->datas_map_.end())
            {
                this->datas_map_.emplace(data->getKey(), data);
                sort_datas_.Insert(data->getKey(), data->getScore());
            }
            else
            {

                assert(false);
                const uint64_t old_score = iter->second->getScore();
                if (overlap)
                {
                    if (iter->second.get() != data.get())
                        (*iter->second) = (*data);
                }
                else
                {
                    (*iter->second) += (*data);
                }

                if (old_score == iter->second->getScore())
                    return;

                sort_datas_.UpdateScore(data->getKey(), old_score, iter->second->getScore());
            }
        }
        void updateInfos(const DataSharedPtr& data) override
        {
            sort_datas_.DeleteByKey(data->getKey());
            this->datas_map_.erase(data->getKey());
            addData(data);
        }

        bool incrScore(const uint64_t key, const uint64_t score) override
        {
            auto iter = this->datas_map_.find(key);
            if (iter == this->datas_map_.end())
                return false;

            const uint64_t old_score = iter->second->getScore();
            iter->second->setScore(old_score + score);
            sort_datas_.UpdateScore(key, old_score, iter->second->getScore());
            return true;
        }

        bool decrScore(const uint64_t key, const uint64_t score) override
        {
            auto iter = this->datas_map_.find(key);
            if (iter == this->datas_map_.end())
                return false;

            const uint64_t old_score = iter->second->getScore();
            iter->second->setScore(SAFE_SUB(old_score, score));
            sort_datas_.UpdateScore(key, old_score, iter->second->getScore());
            return true;
        }

        void deleteByKey(const uint64_t key) override
        {
            auto iter = this->datas_map_.find(key);
            if (iter == this->datas_map_.end())
                return;

            sort_datas_.Delete(key, iter->second->getScore());
            this->datas_map_.erase(iter);
        }

        void deleteByRank(const uint64_t rank) override
        {
            deleteByKey(sort_datas_.GetKeyByRank(rank));
        }

        void deleteByRangedRank(const uint64_t rankfrom, const uint64_t rankto) override
        {
            sort_datas_.DeleteRangeByRankCB(rankfrom, rankto,
                [this](const uint64_t key, const uint64_t score)
                {
                    this->datas_map_.erase(key);
                    return true;
                });
        }

        void deleteByRangedScore(const uint64_t minscore, const uint64_t maxscore) override
        {
            sort_datas_.DeleteRangeByScoreCB(minscore, maxscore,
                [this](const uint64_t key, const uint64_t score)
                {
                    this->datas_map_.erase(key);
                    return true;
                });
        }

        uint64_t getIncrRankByKey(const uint64_t key) override
        {
            auto it = this->datas_map_.find(key);
            if (it == this->datas_map_.end())
                return 0;

            return sort_datas_.GetRank(key, it->second->getScore());
        }

        uint32_t getIncrRankByScore(const uint64_t score) override
        {
            // todo: finish it
            assert(false);
            return 0;
        }

        uint32_t getDecrRankByScore(const uint64_t score) override
        {
            // todo: finish it
            assert(false);
            return 0;
        }

        DataSharedPtr getDataByRank(const uint64_t rank) override
        {
            return this->getDataByKey(sort_datas_.GetKeyByRank(rank));
        }

        bool forEachByIncr(FuncCB&& cb) override
        {
            sort_datas_.forEach(
                [this, &cb](const uint64_t key, const uint64_t score)
                {
                    DataSharedPtr ptr = this->getDataByKey(key);
                    if (!ptr || !cb(ptr))
                        return false;

                    return true;
                });

            return true;
        }

        bool forEachByDecr(FuncCB&& cb) override
        {
            // todo: finish it
            sort_datas_.forEachDecr(
                [this, &cb](const uint64_t key, const uint64_t score)
                {
                    DataSharedPtr ptr = this->getDataByKey(key);
                    if (!ptr || !cb(ptr))
                        return false;

                    return true;
                });

            return true;
        }

        bool forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FuncCB&& cb) override
        {
            if (rankfrom > rankto)
                return false;

            sort_datas_.forEachRangedRank(rankfrom, rankto,
                [this, &cb](const uint64_t key, const uint64_t score)
                {
                    DataSharedPtr ptr = this->getDataByKey(key);
                    if (!ptr || !cb(ptr))
                        return false;

                    return true;
                });

            return true;
        }

        virtual bool forEachByRangedScore(const uint64_t minscore, const uint64_t maxscore, FuncCB&& cb) override
        {
            if (minscore > maxscore)
                return false;

            sort_datas_.forEachRangedScore(minscore, maxscore,
                [this, &cb](const uint64_t key, const uint64_t score)
                {
                    DataSharedPtr ptr = this->getDataByKey(key);
                    if (!ptr || !cb(ptr))
                        return false;

                    return true;
                });
            return true;
        }
    };

}  // namespace ns_toplist

#endif