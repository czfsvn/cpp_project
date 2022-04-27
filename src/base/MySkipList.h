/*
 * @Author: czf
 * @Date: 2022-04-16 18:47:22
 * @LastEditors: czf
 * @LastEditTime: 2022-04-25 20:42:57
 * @FilePath: \cpp_project2022\src\base\MySkipList.h
 * @Description:
 *
 * Copyright (c) 2022 by 用户/公司名, All Rights Reserved.
 */

#include "Global.h"
#include <assert.h>
#include <functional>

namespace cncpp
{

#define ZSKIPLIST_MAXLEVEL 32
#define ZSKIPLIST_P        0.25

    template <typename KEY, typename DATA>
    class SkipList
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
            DataType              data       = {};  // data
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
        SkipList();
        ~SkipList();

        SkipList(const SkipList&)            = delete;
        SkipList& operator=(const SkipList&) = delete;

        SkipList(SkipList&&)            = delete;
        SkipList& operator=(SkipList&&) = delete;

        void     Clear();
        uint64_t Size() const;

        bool forEach(const std::function<bool(const KeyType& key, const DataType& data)>& cb) const;
        bool forEach(const std::function<bool(const KeyType& key, const DataType& data, const uint16_t level)>& cb) const;

        bool forEachRev(const std::function<bool(const KeyType& key, const DataType& data)>& cb) const;

        bool forEachRangedRank(const uint64_t& rankfrom, const uint64_t& rankto,
            const std::function<bool(const KeyType& key, const DataType& data)>& cb) const;

        Node*   Insert(const KeyType& key, const DataType& data);
        Node*   UpdateData(const KeyType& key, const DataType& curdata, const DataType& newdata);
        int32_t Delete(const KeyType& key, const DataType& data);
        int32_t DeleteByKey(const KeyType& key);

        uint64_t GetRank(const KeyType& key, const DataType& data) const;
        // KeyType&    GetKeyByRank(const uint64_t rank) const;
        DataType GetDataByRank(const uint64_t& rank) const;
        // DataType& getDataByKey(const uint64_t& rank);

        uint64_t DeleteByRank(const uint64_t& rank);
        uint64_t DeleteByRank(const uint64_t& rank, const std::function<bool(const KeyType& key, const DataType& data)>& cb);
        uint64_t DeleteByRangedRank(const uint64_t& start, const uint64_t& end /*, dict* dict*/);
        uint64_t DeleteByRangedRank(
            const uint64_t& start, const uint64_t& end, const std::function<bool(const KeyType& key, const DataType& data)>& cb);

    private:
        uint16_t RandomLevel();
        Node*    CreateNode(uint16_t level, const KeyType& key, const DataType& data);
        void     Delete(Node* x, Node** update);
        int32_t  Delete(const KeyType& key, const DataType& data, Node** node);

        Node* GetNodeByRank(const uint64_t rank) const;
        Node* GetNodeByKey(const KeyType& key) const;

        void FreeNode(Node* node)
        {
            SAFE_DELETE(node);
        }
    };

    template <typename KEY, typename DATA>
    SkipList<KEY, DATA>::SkipList()
    {
        length_ = 0;
        level_  = 1;
        header_ = CreateNode(ZSKIPLIST_MAXLEVEL, {}, {});
    }

    template <typename KEY, typename DATA>
    SkipList<KEY, DATA>::~SkipList()
    {
        Clear();
    }

    template <typename KEY, typename DATA>
    void SkipList<KEY, DATA>::Clear()
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
    uint64_t SkipList<KEY, DATA>::Size() const
    {
        return this->length_;
    }

    template <typename KEY, typename DATA>
    bool SkipList<KEY, DATA>::forEach(const std::function<bool(const KeyType& key, const DataType& data)>& cb) const
    {
        if (!header_)
            return false;

        Node* node = header_->next_nodes[0].next;
        while (node)
        {
            if (!cb(node->key, node->data))
                return false;

            node = node->next_nodes[0].next;
        }

        return true;
    }

    template <typename KEY, typename DATA>
    bool SkipList<KEY, DATA>::forEach(
        const std::function<bool(const KeyType& key, const DataType& data, const uint16_t level)>& cb) const
    {
        if (!header_)
            return false;

        Node* node = header_->next_nodes[0].next;
        while (node)
        {
            if (!cb(node->key, node->data, node->level))
                return false;

            node = node->next_nodes[0].next;
        }

        return true;
    }

    template <typename KEY, typename DATA>
    bool SkipList<KEY, DATA>::forEachRev(const std::function<bool(const KeyType& key, const DataType& data)>& cb) const
    {
        if (!tail_)
            return false;

        Node* node = tail_;
        while (node)
        {
            if (!cb(node->key, node->data))
                return false;

            node = node->backward;
        }
        return true;
    }
    template <typename KEY, typename DATA>
    bool SkipList<KEY, DATA>::forEachRangedRank(
        const uint64_t& start, const uint64_t& end, const std::function<bool(const KeyType& key, const DataType& data)>& cb) const
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
            if (!cb(x->key, x->data))
                return false;

            traversed++;
            x = x->next_nodes[0].next;
        }
        return true;
    }

    template <typename KEY, typename DATA>
    typename SkipList<KEY, DATA>::Node* SkipList<KEY, DATA>::Insert(const KeyType& key, const DataType& data)
    {
        Node*    update[ZSKIPLIST_MAXLEVEL] = {};
        uint32_t rank[ZSKIPLIST_MAXLEVEL]   = {};

        Node* node = this->header_;
        for (int16_t i = this->level_ - 1; i >= 0; i--)
        {
            /* store rank that is crossed to reach the insert position */
            rank[i] = (i == (this->level_ - 1) ? 0 : rank[i + 1]);
            while (node && node->next_nodes[i].next
                   && (node->next_nodes[i].next->data < data
                       || (node->next_nodes[i].next->data == data && (node->next_nodes[i].next->key < key))))
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
    typename SkipList<KEY, DATA>::Node* SkipList<KEY, DATA>::UpdateData(
        const KeyType& key, const DataType& curdata, const DataType& newdata)
    {
        Node* update[ZSKIPLIST_MAXLEVEL] = {};

        /* We need to seek to element to update to start: this is useful anyway,
         * we'll have to update or remove it. */
        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next
                   && (x->next_nodes[i].next->data < curdata
                       || (x->next_nodes[i].next->data == curdata && (x->next_nodes[i].next->key < key))))
            {
                x = x->next_nodes[i].next;
            }
            update[i] = x;
        }

        /* Jump to our element: note that this function assumes that the
         * element with the matching data exists. */
        x = (x ? x->next_nodes[0].next : nullptr);

        /* If the node, after the data update, would be still exactly
         * at the same position, we can just update the data without
         * actually removing and re-inserting the element in the skiplist. */
        if (x && (x->backward == nullptr || x->backward->data < newdata)
            && (x->next_nodes[0].next == nullptr || newdata < x->next_nodes[0].next->data))
        {
            x->data = newdata;
            return x;
        }

        /* No way to reuse the old node: we need to remove and insert a new
         * one at a different place. */
        Delete(x, update);
        Node* newnode = Insert(key, newdata);
        /* We reused the old node x->ele SDS string, free the node now
         * since zslInsert created a new one. */
        // x->key = NULL;
        FreeNode(x);
        return newnode;
    }

    template <typename KEY, typename DATA>
    int32_t SkipList<KEY, DATA>::DeleteByKey(const KeyType& key)
    {
        return 0;
    }

    template <typename KEY, typename DATA>
    typename SkipList<KEY, DATA>::DataType SkipList<KEY, DATA>::GetDataByRank(const uint64_t& rank) const
    {
        Node* node = GetNodeByRank(rank);
        if (!node)
            return nullptr;

        return node->data;
    }

    template <typename KEY, typename DATA>
    uint64_t SkipList<KEY, DATA>::GetRank(const KeyType& key, const DataType& data) const
    {
        uint64_t rank = 0;

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next
                   && (x->next_nodes[i].next->data < data
                       || (x->next_nodes[i].next->data == data && x->next_nodes[i].next->key <= key)))
            {
                rank += x->next_nodes[i].span;
                x = x->next_nodes[i].next;
            }

            /* x might be equal to zsl->header, so test if obj is non-NULL */
            if (x->key == key && x->data == data)
            {
                return rank;
            }
        }
        return 0;
    }

    template <typename KEY, typename DATA>
    typename SkipList<KEY, DATA>::Node* SkipList<KEY, DATA>::GetNodeByRank(const uint64_t rank) const
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
    typename SkipList<KEY, DATA>::Node* SkipList<KEY, DATA>::CreateNode(uint16_t level, const KeyType& key, const DataType& data)
    {
        Node* node  = new Node();
        node->data  = data;
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
    uint16_t SkipList<KEY, DATA>::RandomLevel()
    {
        int level = 1;
        while ((cncpp::random() & 0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
            level += 1;

        return (level < ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
    }

    template <typename KEY, typename DATA>
    void SkipList<KEY, DATA>::Delete(Node* x, Node** update)
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
    int32_t SkipList<KEY, DATA>::Delete(const KeyType& key, const DataType& data)
    {
        return Delete(key, data, nullptr);
    }

    template <typename KEY, typename DATA>
    int32_t SkipList<KEY, DATA>::Delete(const KeyType& key, const DataType& data, Node** node)
    {
        Node* update[ZSKIPLIST_MAXLEVEL] = {};

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next
                   && (x->next_nodes[i].next->data < data
                       || (x->next_nodes[i].next->data == data && (x->next_nodes[i].next->key < key))))
            {
                x = x->next_nodes[i].next;
            }
            update[i] = x;
        }

        /* We may have multiple elements with the same data, what we need
         * is to find the element with both the right data and object. */
        x = (x ? x->next_nodes[0].next : nullptr);
        if (x && data == x->data && (x->key == key))
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
    uint64_t SkipList<KEY, DATA>::DeleteByRank(const uint64_t& rank)
    {
        return DeleteByRangedRank(rank, rank);
    }
    template <typename KEY, typename DATA>
    uint64_t SkipList<KEY, DATA>::DeleteByRank(
        const uint64_t& rank, const std::function<bool(const KeyType& key, const DataType& data)>& cb)
    {
        return DeleteByRangedRank(rank, rank, cb);
    }

    template <typename KEY, typename DATA>
    uint64_t SkipList<KEY, DATA>::DeleteByRangedRank(const uint64_t& start, const uint64_t& end)
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
    uint64_t SkipList<KEY, DATA>::DeleteByRangedRank(
        const uint64_t& start, const uint64_t& end, const std::function<bool(const KeyType& key, const DataType& data)>& cb)
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
            cb(x->key, x->data);
            FreeNode(x);
            removed++;
            traversed++;
            x = next;
        }
        return removed;
    }
}  // namespace cncpp