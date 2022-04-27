#pragma once

#include "Global.h"
#include "Misc.h"
#include <assert.h>
#include <functional>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

#define ZSKIPLIST_MAXLEVEL 32
#define ZSKIPLIST_P        0.25

namespace ns_skiplist3
{
    template <typename KEY, typename DATA>
    class SkipList3
    {
    public:
        using KeyType  = KEY;
        using DataType = DATA;

        struct Node
        {

            struct NextNode
            {
                uint64_t span = 0;
                Node*    next = nullptr;
            };

            KeyType               key        = {};  // key
            DataType              score      = {};  // data
            uint16_t              level      = 0;
            Node*                 backward   = nullptr;
            std::vector<NextNode> next_nodes = {};
        };

    private:
        uint32_t length_ = 0;
        uint16_t level_  = 0;
        Node*    header_ = nullptr;
        Node*    tail_   = nullptr;

    public:
        SkipList3();
        ~SkipList3();

        SkipList3(const SkipList3&) = delete;
        SkipList3& operator=(const SkipList3&) = delete;

        SkipList3(SkipList3&&) = delete;
        SkipList3& operator=(SkipList3&&) = delete;

        void     Clear();
        uint64_t Size() const;

        bool forEach(const std::function<bool(const KeyType& key, const DataType& score)>& cb) const;
        bool forEach(const std::function<bool(const KeyType& key, const DataType& score, const uint16_t level)>& cb) const;

        bool forEachRev(const std::function<bool(const KeyType& key, const DataType& score)>& cb) const;

        bool forEachRangedRank(const uint64_t& rankfrom, const uint64_t& rankto,
            const std::function<bool(const KeyType& key, const DataType& score)>& cb) const;

        Node*   Insert(const KeyType& key, const DataType& data);
        Node*   UpdateScore(const KeyType& key, const DataType& curdata, const DataType& newdata);
        int32_t Delete(const KeyType& key, const DataType& score);
        int32_t DeleteByKey(const KeyType& key);

        uint64_t GetRank(const KeyType& key, const DataType& score) const;
        // KeyType&    GetKeyByRank(const uint64_t rank) const;
        DataType GetDataByRank(const uint64_t& rank) const;
        // DataType& getDataByKey(const uint64_t& rank);

        uint64_t DeleteByRank(const uint64_t& rank);
        uint64_t DeleteByRank(const uint64_t& rank, const std::function<bool(const KeyType& key, const DataType& score)>& cb);
        uint64_t DeleteByRangedRank(const uint64_t& start, const uint64_t& end /*, dict* dict*/);
        uint64_t DeleteByRangedRank(
            const uint64_t& start, const uint64_t& end, const std::function<bool(const KeyType& key, const DataType& score)>& cb);

    private:
        uint16_t RandomLevel();
        Node*    CreateNode(uint16_t level, const KeyType& key, const DataType& score);
        void     Delete(Node* x, Node** update);
        int32_t  Delete(const KeyType& key, const DataType& score, Node** node);

        Node* GetNodeByRank(const uint64_t rank) const;
        Node* GetNodeByKey(const KeyType& key) const;

        void FreeNode(Node* node)
        {
            SAFE_DELETE(node);
        }
    };

    template <typename KEY, typename DATA>
    SkipList3<KEY, DATA>::SkipList3()
    {
        length_ = 0;
        level_  = 1;
        header_ = CreateNode(ZSKIPLIST_MAXLEVEL, {}, {});
    }

    template <typename KEY, typename DATA>
    SkipList3<KEY, DATA>::~SkipList3()
    {
        Clear();
    }

    template <typename KEY, typename DATA>
    void SkipList3<KEY, DATA>::Clear()
    {
        if (!header_)
            return;

        if (header_->next_nodes.empty())
        {
            SAFE_DELETE(header_);
            return;
        }

        Node* node = header_->next_nodes[0].next;
        while (node)
        {
            Node* next = node->next_nodes[0].next;
            SAFE_DELETE(node);
            node = next;
        }
        SAFE_DELETE(header_);
    }

    template <typename KEY, typename DATA>
    uint64_t SkipList3<KEY, DATA>::Size() const
    {
        return this->length_;
    }

    template <typename KEY, typename DATA>
    bool SkipList3<KEY, DATA>::forEach(const std::function<bool(const KeyType& key, const DataType& score)>& cb) const
    {
        if (!header_)
            return false;

        Node* node = header_->next_nodes[0].next;
        while (node)
        {
            if (!cb(node->key, node->score))
                return false;

            node = node->next_nodes[0].next;
        }

        return true;
    }

    template <typename KEY, typename DATA>
    bool SkipList3<KEY, DATA>::forEach(
        const std::function<bool(const KeyType& key, const DataType& score, const uint16_t level)>& cb) const
    {
        if (!header_)
            return false;

        Node* node = header_->next_nodes[0].next;
        while (node)
        {
            if (!cb(node->key, node->score, node->level))
                return false;

            node = node->next_nodes[0].next;
        }

        return true;
    }

    template <typename KEY, typename DATA>
    bool SkipList3<KEY, DATA>::forEachRev(const std::function<bool(const KeyType& key, const DataType& score)>& cb) const
    {
        if (!tail_)
            return false;

        Node* node = tail_;
        while (node)
        {
            if (!cb(node->key, node->score))
                return false;

            node = node->backward;
        }
        return true;
    }
    template <typename KEY, typename DATA>
    bool SkipList3<KEY, DATA>::forEachRangedRank(const uint64_t& start, const uint64_t& end,
        const std::function<bool(const KeyType& key, const DataType& score)>& cb) const
    {
        if (start > end)
            return false;

        uint64_t traversed = 0;
        Node*    x         = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x->next_nodes[i].next && (traversed + x->next_nodes[i].span) < start)
            {
                traversed += x->next_nodes[i].span;
                x = x->next_nodes[i].next;
            }
        }

        traversed++;
        x = (x ? x->next_nodes[0].next : nullptr);
        while (x && traversed <= end)
        {
            if (!cb(x->key, x->score))
                return false;

            traversed++;
            x = x->next_nodes[0].next;
        }
        return true;
    }

    template <typename KEY, typename DATA>
    typename SkipList3<KEY, DATA>::Node* SkipList3<KEY, DATA>::Insert(const KeyType& key, const DataType& data)
    {
        Node*    update[ZSKIPLIST_MAXLEVEL] = {};
        uint32_t rank[ZSKIPLIST_MAXLEVEL]   = {};

        Node* node = this->header_;
        for (int16_t i = this->level_ - 1; i >= 0; i--)
        {
            /* store rank that is crossed to reach the insert position */
            rank[i] = (i == (this->level_ - 1) ? 0 : rank[i + 1]);
            while (node && node->next_nodes[i].next
                   && (node->next_nodes[i].next->score < data
                       || (node->next_nodes[i].next->score == data && (node->next_nodes[i].next->key < key))))
            {
                rank[i] += node->next_nodes[i].span;
                node = node->next_nodes[i].next;
            }
            update[i] = node;
        }

        uint16_t new_level = RandomLevel();
        if (new_level > this->level_)
        {
            for (int16_t i = this->level_; i < new_level; i++)
            {
                rank[i]                       = 0;
                update[i]                     = this->header_;
                update[i]->next_nodes[i].span = this->length_;
            }
            this->level_ = new_level;
        }

        Node* new_node = CreateNode(new_level, key, data);
        assert(new_node);
        for (int16_t i = 0; i < new_level; i++)
        {
            new_node->next_nodes[i].next  = update[i]->next_nodes[i].next;
            update[i]->next_nodes[i].next = new_node;

            /* update span covered by update[i] as x is inserted here */
            new_node->next_nodes[i].span  = update[i]->next_nodes[i].span - (rank[0] - rank[i]);
            update[i]->next_nodes[i].span = (rank[0] - rank[i]) + 1;
        }

        /* increment span for untouched levels */
        for (int16_t i = new_level; i < this->level_; i++)
        {
            update[i]->next_nodes[i].span++;
        }

        new_node->backward = (update[0] == this->header_) ? nullptr : update[0];
        if (new_node->next_nodes[0].next)
            new_node->next_nodes[0].next->backward = new_node;
        else
            this->tail_ = new_node;

        this->length_++;
        return new_node;
    }

    template <typename KEY, typename DATA>
    typename SkipList3<KEY, DATA>::Node* SkipList3<KEY, DATA>::UpdateScore(
        const KeyType& key, const DataType& curscore, const DataType& newscore)
    {
        Node* update[ZSKIPLIST_MAXLEVEL] = {};

        /* We need to seek to element to update to start: this is useful anyway,
         * we'll have to update or remove it. */
        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next
                   && (x->next_nodes[i].next->score < curscore
                       || (x->next_nodes[i].next->score == curscore && (x->next_nodes[i].next->key < key))))
            // while (x && x->next_nodes[i].next && (x->next_nodes[i].next->score < curscore))
            {
                x = x->next_nodes[i].next;
            }
            update[i] = x;
        }

        /* Jump to our element: note that this function assumes that the
         * element with the matching score exists. */
        x = (x ? x->next_nodes[0].next : nullptr);

        /* If the node, after the score update, would be still exactly
         * at the same position, we can just update the score without
         * actually removing and re-inserting the element in the skiplist. */
        if (x && (x->backward == nullptr || x->backward->score < newscore)
            && (x->next_nodes[0].next == nullptr || newscore < x->next_nodes[0].next->score))
        {
            x->score = newscore;
            return x;
        }

        /* No way to reuse the old node: we need to remove and insert a new
         * one at a different place. */
        Delete(x, update);
        Node* newnode = Insert(key, newscore);
        /* We reused the old node x->ele SDS string, free the node now
         * since zslInsert created a new one. */
        // x->key = NULL;
        FreeNode(x);
        return newnode;
    }

    template <typename KEY, typename DATA>
    int32_t SkipList3<KEY, DATA>::DeleteByKey(const KeyType& key)
    {
        return 0;
    }

    template <typename KEY, typename DATA>
    typename SkipList3<KEY, DATA>::DataType SkipList3<KEY, DATA>::GetDataByRank(const uint64_t& rank) const
    {
        Node* node = GetNodeByRank(rank);
        if (!node)
            return nullptr;

        return node->score;
    }

    template <typename KEY, typename DATA>
    uint64_t SkipList3<KEY, DATA>::GetRank(const KeyType& key, const DataType& score) const
    {
        uint64_t rank = 0;

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next
                   && (x->next_nodes[i].next->score < score
                       || (x->next_nodes[i].next->score == score /* && x->next_nodes[i].next->key <= key*/)))
            {
                rank += x->next_nodes[i].span;
                x = x->next_nodes[i].next;
            }

            /* x might be equal to zsl->header, so test if obj is non-NULL */
            if (x->key == key && x->score == score)
            {
                return rank;
            }
        }
        return 0;
    }

    template <typename KEY, typename DATA>
    typename SkipList3<KEY, DATA>::Node* SkipList3<KEY, DATA>::GetNodeByRank(const uint64_t rank) const
    {
        uint64_t traversed = 0;
        Node*    x         = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next && (traversed + x->next_nodes[i].span) <= rank)
            {
                traversed += x->next_nodes[i].span;
                x = x->next_nodes[i].next;
            }
            if (traversed == rank)
            {
                return x;
            }
        }
        return nullptr;
    }

    template <typename KEY, typename DATA>
    typename SkipList3<KEY, DATA>::Node* SkipList3<KEY, DATA>::CreateNode(
        uint16_t level, const KeyType& key, const DataType& score)
    {
        Node* node  = new Node();
        node->score = score;
        node->key   = key;
        node->level = level;
        node->next_nodes.reserve(level);
        for (uint16_t idx = 0; idx < level; idx++)
        {
            // node->next_nodes.emplace_back(Node::NextNode());
            node->next_nodes[idx].span = 0;
            node->next_nodes[idx].next = nullptr;
        }
        return node;
    }

    template <typename KEY, typename DATA>
    uint16_t SkipList3<KEY, DATA>::RandomLevel()
    {
        int level = 1;
        while ((cncpp::random() & 0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
            level += 1;

        return (level < ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
    }

    template <typename KEY, typename DATA>
    void SkipList3<KEY, DATA>::Delete(Node* x, Node** update)
    {
        if (!x)
            return;

        for (uint16_t i = 0; i < level_; i++)
        {
            if (update[i]->next_nodes[i].next == x)
            {
                update[i]->next_nodes[i].span += x->next_nodes[i].span - 1;
                update[i]->next_nodes[i].next = x->next_nodes[i].next;
            }
            else
            {
                update[i]->next_nodes[i].span -= 1;
            }
        }
        if (x->next_nodes[0].next)
        {
            x->next_nodes[0].next->backward = x->backward;
        }
        else
        {
            tail_ = x->backward;
        }
        while (level_ > 1 && header_->next_nodes[level_ - 1].next == nullptr)
            level_--;

        length_--;
    }

    template <typename KEY, typename DATA>
    int32_t SkipList3<KEY, DATA>::Delete(const KeyType& key, const DataType& score)
    {
        return Delete(key, score, nullptr);
    }

    template <typename KEY, typename DATA>
    int32_t SkipList3<KEY, DATA>::Delete(const KeyType& key, const DataType& score, Node** node)
    {
        Node* update[ZSKIPLIST_MAXLEVEL] = {};

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next
                   && (x->next_nodes[i].next->score < score
                       || (x->next_nodes[i].next->score == score && (x->next_nodes[i].next->key < key))))
            {
                x = x->next_nodes[i].next;
            }
            update[i] = x;
        }

        /* We may have multiple elements with the same score, what we need
         * is to find the element with both the right score and object. */
        x = (x ? x->next_nodes[0].next : nullptr);
        if (x && score == x->score && (x->key == key))
        {
            Delete(x, update);
            if (!node)
                FreeNode(x);
            else
                *node = x;
            return 1;
        }

        return 0; /* not found */
    }

    template <typename KEY, typename DATA>
    uint64_t SkipList3<KEY, DATA>::DeleteByRank(const uint64_t& rank)
    {
        return DeleteByRangedRank(rank, rank);
    }
    template <typename KEY, typename DATA>
    uint64_t SkipList3<KEY, DATA>::DeleteByRank(
        const uint64_t& rank, const std::function<bool(const KeyType& key, const DataType& score)>& cb)
    {
        return DeleteByRangedRank(rank, rank, cb);
    }

    template <typename KEY, typename DATA>
    uint64_t SkipList3<KEY, DATA>::DeleteByRangedRank(const uint64_t& start, const uint64_t& end)
    {
        Node*    update[ZSKIPLIST_MAXLEVEL] = {};
        uint64_t traversed = 0, removed = 0;

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x->next_nodes[i].next && (traversed + x->next_nodes[i].span) < start)
            {
                traversed += x->next_nodes[i].span;
                x = x->next_nodes[i].next;
            }
            update[i] = x;
        }

        traversed++;
        x = (x ? x->next_nodes[0].next : nullptr);
        while (x && traversed <= end)
        {
            Node* next = x->next_nodes[0].next;
            Delete(x, update);
            FreeNode(x);
            removed++;
            traversed++;
            x = next;
        }
        return removed;
    }
    template <typename KEY, typename DATA>
    uint64_t SkipList3<KEY, DATA>::DeleteByRangedRank(
        const uint64_t& start, const uint64_t& end, const std::function<bool(const KeyType& key, const DataType& score)>& cb)
    {
        Node*    update[ZSKIPLIST_MAXLEVEL] = {};
        uint64_t traversed = 0, removed = 0;

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x->next_nodes[i].next && (traversed + x->next_nodes[i].span) < start)
            {
                traversed += x->next_nodes[i].span;
                x = x->next_nodes[i].next;
            }
            update[i] = x;
        }

        traversed++;
        x = (x ? x->next_nodes[0].next : nullptr);
        while (x && traversed <= end)
        {
            Node* next = x->next_nodes[0].next;
            Delete(x, update);
            cb(x->key, x->score);
            FreeNode(x);
            removed++;
            traversed++;
            x = next;
        }
        return removed;
    }

}  // namespace ns_skiplist3

namespace ns_toplist3
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
        ns_skiplist3::SkipList3<KeyType, DataType> cont_ = {};

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

}  // namespace ns_toplist3
