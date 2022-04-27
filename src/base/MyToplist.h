/*
 * @Author: czf
 * @Date: 2022-04-16 18:49:54
 * @LastEditors: czf
 * @LastEditTime: 2022-04-16 19:12:21
 * @FilePath: \cpp_project2022\src\base\MyToplist.h
 * @Description:
 *
 * Copyright (c) 2022 by 用户/公司名, All Rights Reserved.
 */

#include "Misc.h"
#include "MySkipList.h"

#include <memory>
#include <set>
#include <vector>

namespace cncpp
{
    template <typename KEY, typename DATA>
    class TopList
    {
    public:
        using KeyType  = KEY;
        using DataType = std::shared_ptr<DATA>;
        using FuncCB   = std::function<bool(const DataType&)>;

    protected:
        std::unordered_map<uint64_t, DataType> datas_map_ = {};

    public:
        virtual void insert(const KeyType& key, const DataType& data) = 0;
        // virtual bool updateData(const KeyType& key, const DataType& data) = 0;

        virtual uint32_t deleteByKey(const KeyType& key) = 0;
        // virtual uint32_t deleteByData(const DataType& key)                              = 0;
        virtual uint32_t deleteByRank(const uint64_t& rank)                             = 0;
        virtual uint32_t deleteByRank(const uint64_t& rankfrom, const uint64_t& rankto) = 0;

        virtual DataType getDataByRank(const uint64_t& rank) const = 0;
        virtual uint64_t getRankByKey(const KeyType& key) const    = 0;

        virtual void clear() = 0;

        virtual bool forEach(FuncCB&& cb)                                                             = 0;
        virtual bool forEachRev(FuncCB&& cb)                                                          = 0;
        virtual bool forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FuncCB&& cb) = 0;

        uint64_t size() const
        {
            return datas_map_.size();
        }

        virtual DataType getData(const KeyType& key) const
        {
            auto iter = this->datas_map_.find(key);
            if (iter != this->datas_map_.end())
                return nullptr;

            return iter->second;
        }

        virtual uint64_t getRevRankByKey(const KeyType& key) const
        {
            const uint64_t rank = getRankByKey(key);
            return rank ? SAFE_SUB(this->size() + 1, rank) : 0;
        }

    };  // namespace SkipTopList3

    template <typename KEY, typename DATA>
    class VecTopList : public TopList<KEY, DATA>
    {
    public:
        using KeyType  = typename TopList<KEY, DATA>::KeyType;
        using DataType = typename TopList<KEY, DATA>::DataType;
        using FuncCB   = typename TopList<KEY, DATA>::FuncCB;

    private:
        std::vector<DataType> cont_ = {};

    public:
        void insert(const KeyType& key, const DataType& data) override;

        uint32_t deleteByKey(const KeyType& key) override;
        // uint32_t deleteByData(const DataType& key) override;
        uint32_t deleteByRank(const uint64_t& rank) override;
        uint32_t deleteByRank(const uint64_t& rankfrom, const uint64_t& rankto) override;

        DataType getDataByRank(const uint64_t& rank) const override;
        uint64_t getRankByKey(const KeyType& key) const override;

        bool forEach(FuncCB&& cb) override;
        bool forEachRev(FuncCB&& cb) override;
        bool forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FuncCB&& cb) override;

        void clear() override;

    private:
        void sortall();
    };

    template <typename KEY, typename DATA>
    class SetTopList : public TopList<KEY, DATA>
    {
    public:
        using KeyType  = typename TopList<KEY, DATA>::KeyType;
        using DataType = typename TopList<KEY, DATA>::DataType;
        using FuncCB   = typename TopList<KEY, DATA>::FuncCB;

    private:
        std::set<DataType> cont_ = {};

    public:
        void insert(const KeyType& key, const DataType& data) override;

        uint32_t deleteByKey(const KeyType& key) override;
        // uint32_t deleteByData(const DataType& key) override;
        uint32_t deleteByRank(const uint64_t& rank) override;
        uint32_t deleteByRank(const uint64_t& rankfrom, const uint64_t& rankto) override;

        DataType getDataByRank(const uint64_t& rank) const override;
        uint64_t getRankByKey(const KeyType& key) const override;

        bool forEach(FuncCB&& cb) override;
        bool forEachRev(FuncCB&& cb) override;
        bool forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FuncCB&& cb) override;

        void clear() override;
    };

    template <typename KEY, typename DATA>
    class SkipTopList : public TopList<KEY, DATA>
    {
    public:
        using KeyType  = typename TopList<KEY, DATA>::KeyType;
        using DataType = typename TopList<KEY, DATA>::DataType;
        using FuncCB   = typename TopList<KEY, DATA>::FuncCB;

    private:
        SkipList<KeyType, DataType> cont_ = {};

    public:
        void insert(const KeyType& key, const DataType& data) override;

        uint32_t deleteByKey(const KeyType& key) override;
        // uint32_t deleteByData(const DataType& key) override;
        uint32_t deleteByRank(const uint64_t& rank) override;
        uint32_t deleteByRank(const uint64_t& rankfrom, const uint64_t& rankto) override;

        DataType getDataByRank(const uint64_t& rank) const override;
        uint64_t getRankByKey(const KeyType& key) const override;

        bool forEach(FuncCB&& cb) override;
        bool forEachRev(FuncCB&& cb) override;
        bool forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FuncCB&& cb) override;

        void clear() override;
    };

    template <typename KEY, typename DATA>
    void VecTopList<KEY, DATA>::sortall()
    {
        std::sort(this->cont_.begin(), this->cont_.end());  // vector
    }

    template <typename KEY, typename DATA>
    void VecTopList<KEY, DATA>::insert(const KeyType& key, const DataType& data)
    {
        auto iter = this->datas_map_.find(key);
        if (iter == this->datas_map_.end())
        {
            this->datas_map_.emplace(key, data);
            cont_.emplace_back(data);
            sortall();
        }
        else
        {
            // it worked well !!
            iter->second += data;
            sortall();
        }
    }

    template <typename KEY, typename DATA>
    uint32_t VecTopList<KEY, DATA>::deleteByKey(const KeyType& key)
    {
        auto it = this->datas_map_.find(key);
        if (it == this->datas_map_.end())
            return 0;

        for (auto iter = this->cont_.begin(); iter != this->cont_.end(); ++iter)
        {
            if ((*iter)->getKey() != key)
                continue;

            this->cont_.erase(iter);
            this->datas_map_.erase(it);
            return 1;
        }
        return 0;
    }

    template <typename KEY, typename DATA>
    uint32_t VecTopList<KEY, DATA>::deleteByRank(const uint64_t& rank)
    {
        return deleteByRank(rank, rank);
    }

    template <typename KEY, typename DATA>
    uint32_t VecTopList<KEY, DATA>::deleteByRank(const uint64_t& rankfrom, const uint64_t& rankto)
    {
        if (!rankfrom || rankfrom > rankto || rankfrom > cont_.size())
            return 0;

        const uint64_t will_rmcount = SAFE_SUB(std::min<uint64_t>(this->size(), rankto), rankfrom) + 1;

        auto iter = this->cont_.begin();
        std::advance(iter, SAFE_SUB(rankfrom, 1));

        uint64_t removed = 0;
        while (iter != cont_.end() && removed < will_rmcount)
        {
            this->datas_map_.erase((*iter)->getKey());
            iter = this->cont_.erase(iter);
            removed++;
        }

        return removed;
    }

    template <typename KEY, typename DATA>
    typename VecTopList<KEY, DATA>::DataType VecTopList<KEY, DATA>::getDataByRank(const uint64_t& rank) const
    {
        if (!rank || rank > cont_.size() || cont_.empty())
            return {};

        return cont_[SAFE_SUB(rank, 1)];
    }

    template <typename KEY, typename DATA>
    uint64_t VecTopList<KEY, DATA>::getRankByKey(const KeyType& key) const
    {
        uint64_t rank = 0;
        for (const auto& item : this->cont_)
        {
            rank++;
            if (item->getKey() != key)
                continue;

            return rank;
        }

        return 0;
    }

    template <typename KEY, typename DATA>
    void VecTopList<KEY, DATA>::clear()
    {
        this->cont_.clear();
        this->datas_map_.clear();
    }

    template <typename KEY, typename DATA>
    bool VecTopList<KEY, DATA>::forEach(FuncCB&& cb)
    {
        for (const auto& item : cont_)
        {
            if (!cb(item))
                return false;
        }

        return true;
    }

    template <typename KEY, typename DATA>
    bool VecTopList<KEY, DATA>::forEachRev(FuncCB&& cb)
    {
        for (auto iter = this->cont_.rbegin(); iter != this->cont_.rend(); ++iter)
        {
            if (!cb(*iter))
                return false;
        }
        return true;
    }

    template <typename KEY, typename DATA>
    bool VecTopList<KEY, DATA>::forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FuncCB&& cb)
    {
        if (!rankfrom || rankfrom > rankto || rankfrom > cont_.size())
            return false;

        const uint64_t will_loop = SAFE_SUB(std::min<uint64_t>(this->size(), rankto), rankfrom) + 1;

        auto iter = this->cont_.begin();
        std::advance(iter, SAFE_SUB(rankfrom, 1));

        uint64_t loop = 0;
        while (iter != cont_.end() && loop < will_loop)
        {
            if (!cb(*iter))
                return false;

            iter++;
            loop++;
        }

        return loop > 0;
    }

    // set
    template <typename KEY, typename DATA>
    void SetTopList<KEY, DATA>::insert(const KeyType& key, const DataType& data)
    {
        auto iter = this->datas_map_.find(key);
        if (iter == this->datas_map_.end())
        {
            this->datas_map_.emplace(key, data);
            this->cont_.emplace(data);
        }
        else
        {
            // it worked well!!
            this->cont_.erase(iter->second);
            iter->second += data;
            this->cont_.emplace(iter->second);
        }
    }

    template <typename KEY, typename DATA>
    uint32_t SetTopList<KEY, DATA>::deleteByKey(const KeyType& key)
    {
        auto iter = this->datas_map_.find(key);
        if (iter == this->datas_map_.end())
            return 0;

        // todo: check worked ?
        this->cont_.erase(iter->second);
        this->datas_map_.erase(iter);
        return 1;
    }

    template <typename KEY, typename DATA>
    uint32_t SetTopList<KEY, DATA>::deleteByRank(const uint64_t& rank)
    {
        return deleteByRank(rank, rank);
    }

    template <typename KEY, typename DATA>
    uint32_t SetTopList<KEY, DATA>::deleteByRank(const uint64_t& rankfrom, const uint64_t& rankto)
    {
        if (!rankfrom || rankfrom > rankto || rankfrom > cont_.size())
            return 0;

        const uint64_t will_rmcount = SAFE_SUB(std::min<uint64_t>(this->size(), rankto), rankfrom) + 1;

        auto iter = this->cont_.begin();
        std::advance(iter, SAFE_SUB(rankfrom, 1));

        uint64_t removed = 0;
        while (iter != cont_.end() && removed < will_rmcount)
        {
            this->datas_map_.erase((*iter)->getKey());
            iter = this->cont_.erase(iter);
            removed++;
        }

        return removed;
    }

    template <typename KEY, typename DATA>
    typename SetTopList<KEY, DATA>::DataType SetTopList<KEY, DATA>::getDataByRank(const uint64_t& rank) const
    {
        if (!rank || rank > cont_.size() || cont_.empty())
            return {};

        auto iter = this->cont_.begin();
        std::advance(iter, SAFE_SUB(rank, 1));

        if (iter == this->cont_.end())
            return {};

        return *iter;
    }

    template <typename KEY, typename DATA>
    uint64_t SetTopList<KEY, DATA>::getRankByKey(const KeyType& key) const
    {
        uint64_t tmp_rank = 0;
        for (auto iter = cont_.begin(); iter != cont_.end(); ++iter)
        {
            ++tmp_rank;
            if ((*iter)->getKey() == key)
                break;
        }
        return tmp_rank;
    }

    template <typename KEY, typename DATA>
    bool SetTopList<KEY, DATA>::forEach(FuncCB&& cb)
    {
        for (const auto& item : cont_)
        {
            if (!cb(item))
                return false;
        }

        return true;
    }

    template <typename KEY, typename DATA>
    bool SetTopList<KEY, DATA>::forEachRev(FuncCB&& cb)
    {
        for (auto iter = this->cont_.rbegin(); iter != this->cont_.rend(); ++iter)
        {
            if (!cb(*iter))
                return false;
        }
        return true;
    }

    template <typename KEY, typename DATA>
    bool SetTopList<KEY, DATA>::forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FuncCB&& cb)
    {
        if (!rankfrom || rankfrom > rankto || rankfrom > cont_.size())
            return false;

        const uint64_t will_loop = SAFE_SUB(std::min<uint64_t>(this->size(), rankto), rankfrom) + 1;

        auto iter = this->cont_.begin();
        std::advance(iter, SAFE_SUB(rankfrom, 1));

        uint64_t loop = 0;
        while (iter != cont_.end() && loop < will_loop)
        {
            if (!cb(*iter))
                return false;

            iter++;
            loop++;
        }

        return loop > 0;
    }

    template <typename KEY, typename DATA>
    void SetTopList<KEY, DATA>::clear()
    {
        this->cont_.clear();
        this->datas_map_.clear();
    }

    // skiplist ----------------------------------------------------------------

    template <typename KEY, typename DATA>
    void SkipTopList<KEY, DATA>::insert(const KeyType& key, const DataType& data)
    {
        auto iter = this->datas_map_.find(key);
        if (iter == this->datas_map_.end())
        {
            this->datas_map_.emplace(key, data);
            this->cont_.Insert(key, data);
        }
        else
        {
            this->cont_.Delete(key, iter->second);
            iter->second += data;
            this->cont_.Insert(key, iter->second);
        }
    }

    template <typename KEY, typename DATA>
    uint32_t SkipTopList<KEY, DATA>::deleteByKey(const KeyType& key)
    {
        auto iter = this->datas_map_.find(key);
        if (iter == this->datas_map_.end())
            return 0;

        this->cont_.Delete(key, iter->second);
        this->datas_map_.erase(iter);
        return 1;
    }

    template <typename KEY, typename DATA>
    uint32_t SkipTopList<KEY, DATA>::deleteByRank(const uint64_t& rank)
    {
        uint64_t removed = 0;
        this->cont_.DeleteByRank(rank,
            [this, &removed](const KeyType& key, const DataType& data)
            {
                this->datas_map_.erase(key);
                removed++;
                return true;
            });

        return removed;
    }

    template <typename KEY, typename DATA>
    uint32_t SkipTopList<KEY, DATA>::deleteByRank(const uint64_t& rankfrom, const uint64_t& rankto)
    {
        uint64_t removed = 0;
        this->cont_.DeleteByRangedRank(rankfrom, rankto,
            [this, &removed](const KeyType& key, const DataType& data)
            {
                this->datas_map_.erase(key);
                removed++;
                return true;
            });

        return removed;
    }

    template <typename KEY, typename DATA>
    typename SkipTopList<KEY, DATA>::DataType SkipTopList<KEY, DATA>::getDataByRank(const uint64_t& rank) const
    {
        return this->cont_.GetDataByRank(rank);
    }

    template <typename KEY, typename DATA>
    uint64_t SkipTopList<KEY, DATA>::getRankByKey(const KeyType& key) const
    {
        auto iter = this->datas_map_.find(key);
        if (iter == this->datas_map_.end())
            return 0;

        return this->cont_.GetRank(key, iter->second);
    }

    template <typename KEY, typename DATA>
    bool SkipTopList<KEY, DATA>::forEach(FuncCB&& cb)
    {
        return this->cont_.forEach(
            [cb](const KeyType& key, const DataType& data)
            {
                if (!cb(data))
                    return false;

                return true;
            });
    }

    template <typename KEY, typename DATA>
    bool SkipTopList<KEY, DATA>::forEachRev(FuncCB&& cb)
    {
        return this->cont_.forEachRev(
            [cb](const KeyType& key, const DataType& data)
            {
                if (!cb(data))
                    return false;

                return true;
            });
    }

    template <typename KEY, typename DATA>
    bool SkipTopList<KEY, DATA>::forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FuncCB&& cb)
    {
        return this->cont_.forEachRangedRank(rankfrom, rankto,
            [cb](const KeyType& key, const DataType& data)
            {
                if (!cb(data))
                    return false;

                return true;
            });
    }

    template <typename KEY, typename DATA>
    void SkipTopList<KEY, DATA>::clear()
    {
        this->datas_map_.clear();
        this->cont_.Clear();
    }
}  // namespace cncpp