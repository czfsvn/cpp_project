/*
 * @Author: czf
 * @Date: 2022-02-07 10:17:18
 * @LastEditors: czf
 * @LastEditTime: 2022-03-02 17:13:39
 * @FilePath: \cpp_project2022\src\examples\MySkipList\skiplist.cpp
 * @Description:
 *
 * Copyright (c) 2022 by 用户/公司名, All Rights Reserved.
 */
#include "skiplist.h"

namespace ns_redis
{
    void SkipList::Node::print()
    {
        std::cout << "size: " << next_nodes.size() << "/" << next_nodes.capacity() << ", "
                  << this->ele << "-" << this->score << std::endl;
    }

    SkipList::SkipList()
    {
        length_ = 0;
        level_  = 1;
        header_ = zslCreateNode(ZSKIPLIST_MAXLEVEL, 0, "");
    }

    SkipList::~SkipList()
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

    uint16_t SkipList::zslRandomLevel()
    {
        int level = 1;
        while ((cncpp::random() & 0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
            level += 1;
        return (level < ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
    }

    SkipList::Node* SkipList::zslCreateNode(uint16_t level, uint64_t score, const std::string& ele)
    {
        Node* node  = new Node();
        node->score = score;
        node->ele   = ele;
        node->next_nodes.reserve(level);
        return node;
    }

    void SkipList::freeNode(Node* node)
    {
        SAFE_DELETE(node);
    }

    void SkipList::printAll()
    {
        Node* node = header_->next_nodes[0].next;
        std::cout << "level: " << this->level_ << ", length: " << this->length_ << std::endl;
        while (node)
        {
            node->print();
            // Node* next = node->next_nodes[0].next;
            // SAFE_DELETE(node);
            node = node->next_nodes[0].next;
        }
    }

    SkipList::Node* SkipList::zslInsert(const uint64_t score, const std::string& ele)
    {
        Node*    update[ZSKIPLIST_MAXLEVEL] = {};
        uint32_t rank[ZSKIPLIST_MAXLEVEL]   = {};

        Node* node = this->header_;
        for (int16_t i = this->level_ - 1; i >= 0; i--)
        {
            /* store rank that is crossed to reach the insert position */
            rank[i] = i == (this->level_ - 1) ? 0 : rank[i + 1];
            while (node->next_nodes[i].next
                   && (node->next_nodes[i].next->score
                       < score /* ||
(x->next_nodes[i].next->score == score &&
strcmp(x->next_nodes[i].next->ele.c_str(), ele.c_str()) <
0)*/))
            {
                rank[i] += node->next_nodes[i].span;
                node = node->next_nodes[i].next;
            }
            update[i] = node;
        }

        uint16_t new_level = zslRandomLevel();
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
        Node* new_node = zslCreateNode(new_level, score, ele);
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

    /* Internal function used by zslDelete, zslDeleteRangeByScore and
     * zslDeleteRangeByRank. */
    void SkipList::zslDeleteNode(Node* x, Node** update)
    {
        for (int16_t i = 0; i < this->level_; i++)
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
            this->tail_ = x->backward;
        }

        while (this->level_ > 1 && this->header_->next_nodes[this->level_ - 1].next == nullptr)
            this->level_--;

        this->length_--;
    }

    /* Delete an element with matching score/element from the skiplist.
     * The function returns 1 if the node was found and deleted, otherwise
     * 0 is returned.
     *
     * If 'node' is NULL the deleted node is freed by zslFreeNode(), otherwise
     * it is not freed (but just unlinked) and *node is set to the node pointer,
     * so that it is possible for the caller to reuse the node (including the
     * referenced SDS string at node->ele). */
    uint32_t SkipList::zslDelete(uint64_t score, const std::string& ele, Node** node)
    {
        Node* update[ZSKIPLIST_MAXLEVEL] = {};
        // int   i;

        Node* x = this->header_;
        for (int16_t i = this->level_ - 1; i >= 0; i--)
        {
            while (x->next_nodes[i].next && (x->next_nodes[i].next->score < score/* ||
                                                (x->level[i].next->score == score &&
                                                    sdscmp(x->level[i].next->ele, ele) < 0)*/))
            {
                x = x->next_nodes[i].next;
            }
            update[i] = x;
        }

        /* We may have multiple elements with the same score, what we need
         * is to find the element with both the right score and object. */
        x = x->next_nodes[0].next;
        if (x && score == x->score /* && sdscmp(x->ele, ele) == 0*/)
        {
            zslDeleteNode(x, update);
            if (!node)
            {
                SAFE_DELETE(x);  // freeNode(x);
            }
            else
            {
                *node = x;
            }

            return 1;
        }
        return 0; /* not found */
    }

    /* Update the score of an element inside the sorted set skiplist.
     * Note that the element must exist and must match 'score'.
     * This function does not update the score in the hash table side, the
     * caller should take care of it.
     *
     * Note that this function attempts to just update the node, in case after
     * the score update, the node would be exactly at the same position.
     * Otherwise the skiplist is modified by removing and re-adding a new
     * element, which is more costly.
     *
     * The function returns the updated element skiplist node pointer. */
    SkipList::Node* SkipList::zslUpdateScore(
        uint64_t curscore, const std::string& ele, uint64_t newscore)
    {
        Node* update[ZSKIPLIST_MAXLEVEL] = {};
        // int   i;

        /* We need to seek to element to update to start: this is useful anyway,
         * we'll have to update or remove it. */
        Node* x = this->header_;
        for (int16_t i = this->level_ - 1; i >= 0; i--)
        {
            while (x->next_nodes[i].next && (x->next_nodes[i].next->score < curscore/* ||
                                                (x->next_nodes[i].next->score == curscore &&
                                                    sdscmp(x->next_nodes[i].next->ele, ele) < 0)*/))
            {
                x = x->next_nodes[i].next;
            }
            update[i] = x;
        }

        /* Jump to our element: note that this function assumes that the
         * element with the matching score exists. */
        x = x->next_nodes[0].next;

        /* If the node, after the score update, would be still exactly
         * at the same position, we can just update the score without
         * actually removing and re-inserting the element in the skiplist. */
        if ((x->backward == nullptr || x->backward->score < newscore)
            && (x->next_nodes[0].next == nullptr || x->next_nodes[0].next->score > newscore))
        {
            x->score = newscore;
            return x;
        }

        /* No way to reuse the old node: we need to remove and insert a new
         * one at a different place. */
        zslDeleteNode(x, update);
        Node* newnode = zslInsert(newscore, x->ele);
        /* We reused the old node x->ele SDS string, free the node now
         * since zslInsert created a new one. */
        x->ele = "";
        SAFE_DELETE(x);
        return newnode;
    }

    /* Find the rank for an element by both score and key.
     * Returns 0 when the element cannot be found, rank otherwise.
     * Note that the rank is 1-based due to the span of zsl->header to the
     * first element. */
    uint32_t SkipList::zslGetRank(uint64_t score, const std::string& ele)
    {
        uint32_t rank = 0;

        Node* x = this->header_;
        for (int16_t i = this->level_ - 1; i >= 0; i--)
        {
            while (x->next_nodes[i].next && (x->next_nodes[i].next->score < score/* ||
                                                (x->next_nodes[i].next->score == score &&
                                                    sdscmp(x->next_nodes[i].next->ele, ele) <= 0)*/))
            {
                rank += x->next_nodes[i].span;
                x = x->next_nodes[i].next;
            }

            /* x might be equal to zsl->header, so test if obj is non-NULL */
            /* TODO: need fix
             */
            if (!x->ele.empty() /* && sdscmp(x->ele, ele) == 0*/)
            {
                return rank;
            }
        }
        return 0;
    }

    /* Finds an element by its rank. The rank argument needs to be 1-based. */
    SkipList::Node* SkipList::zslGetElementByRank(const uint32_t rank)
    {
        // zskiplistNode* x;
        uint32_t traversed = 0;

        Node* x = this->header_;
        for (int16_t i = this->level_ - 1; i >= 0; i--)
        {
            while (x->next_nodes[i].next && (traversed + x->next_nodes[i].span) <= rank)
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

    /*
    int zslValueGteMin(double value, zrangespec* spec)
    {
        return spec->minex ? (value > spec->min) : (value >= spec->min);
    }

    int zslValueLteMax(double value, zrangespec* spec)
    {
        return spec->maxex ? (value < spec->max) : (value <= spec->max);
    }

    int sdscmplex(sds a, sds b)
    {
        if (a == b) return 0;
        if (a == shared.minstring || b == shared.maxstring) return -1;
        if (a == shared.maxstring || b == shared.minstring) return 1;
        return sdscmp(a,b);
    }

    int zslLexValueGteMin(sds value, zlexrangespec *spec) {
        return spec->minex ?
            (sdscmplex(value,spec->min) > 0) :
            (sdscmplex(value,spec->min) >= 0);
    }

    int zslLexValueLteMax(sds value, zlexrangespec *spec) {
        return spec->maxex ?
            (sdscmplex(value,spec->max) < 0) :
            (sdscmplex(value,spec->max) <= 0);
    }
    */
    /* Returns if there is a part of the zset is in range. */
    uint32_t SkipList::zslIsInRange(const uint32_t min, const uint32_t max)
    {
        /* Test for ranges that will always be empty. */
        if (min > max)
            return 0;

        Node* x = this->tail_;
        if (x == nullptr || x->score < min)
            return 0;

        x = this->header_->next_nodes[0].next;
        if (x == nullptr || x->score > max)
            return 0;

        return 1;
    }

    /* Find the first node that is contained in the specified range.
     * Returns NULL when no element is contained in the range. */
    SkipList::Node* SkipList::zslFirstInRange(const uint32_t min, const uint32_t max)
    {
        /* If everything is out of range, return early. */
        if (!zslIsInRange(min, max))
            return nullptr;

        Node* x = this->header_;
        for (int16_t i = this->level_ - 1; i >= 0; i--)
        {
            /* Go forward while *OUT* of range. */
            // !zslValueGteMin(x->level[i].forward->score,range))
            while (x->next_nodes[i].next && (x->next_nodes[i].next->score > min))
                x = x->next_nodes[i].next;
        }

        /* This is an inner range, so the next node cannot be NULL. */
        x = x->next_nodes[0].next;

        /* Check if score <= max. */
        // if (!zslValueLteMax(x->score,range)) return NULL;
        if (x->score > max)
            return nullptr;

        return x;
    }

    /* Find the last node that is contained in the specified range.
     * Returns NULL when no element is contained in the range. */
    SkipList::Node* SkipList::zslLastInRange(const uint32_t min, const uint32_t max)
    {
        /* If everything is out of range, return early. */
        if (!zslIsInRange(min, max))
            return nullptr;

        Node* x = this->header_;
        for (int16_t i = this->level_ - 1; i >= 0; i--)
        {
            /* Go forward while *IN* range. */
            while (x->next_nodes[i].next && (x->next_nodes[i].next->score < max))
                x = x->next_nodes[i].next;
        }

        /* This is an inner range, so this node cannot be NULL. */
        // serverAssert(x != NULL);

        /* Check if score >= min. */
        if (x->score < min)
            return nullptr;

        return x;
    }

    /* Delete all the elements with score between min and max from the skiplist.
     * Both min and max can be inclusive or exclusive (see range->minex and
     * range->maxex). When inclusive a score >= min && score <= max is deleted.
     * Note that this function takes the reference to the hash table view of the
     * sorted set, in order to remove the elements from the hash table too. */
    uint32_t SkipList::zslDeleteRangeByScore(
        const uint32_t min, const uint32_t max /*, dict* dict*/)
    {
        Node*         update[ZSKIPLIST_MAXLEVEL] = {};
        unsigned long removed                    = 0;

        Node* x = this->header_;
        for (int16_t i = this->level_ - 1; i >= 0; i--)
        {
            while (x->next_nodes[i].next && (x->next_nodes[i].next->score < min))
                x = x->next_nodes[i].next;
            update[i] = x;
        }

        /* Current node is the last with score < or <= min. */
        x = x->next_nodes[0].next;

        /* Delete nodes while in range. */
        while (x && (x->score < max))
        {
            Node* next = x->next_nodes[0].next;
            zslDeleteNode(x, update);
            //  dictDelete(dict, x->ele); ??
            SAFE_DELETE(x); /* zslFreeNode(x);  Here is where x->ele is actually released. */
            removed++;
            x = next;
        }
        return removed;
    }
}  // namespace ns_redis

namespace myskiplist1
{
    /*
    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node*
    SkipList<Key, Comparator>::FindGreaterOrEqual(const Key& key,
                                              Node** prev) const {

    template<typename ITEM>
    typename SkipList<ITEM>::Node* SkipList<ITEM>::Insert(const ITEM& data)
    {
        return nullptr;
    }
    */
}  // namespace myskiplist1

namespace myskiplist2
{
    SkipList::SkipList()
    {
        length_ = 0;
        level_  = 1;
        header_ = CreateNode(ZSKIPLIST_MAXLEVEL, 0, 0);
    }

    SkipList::~SkipList()
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

    uint16_t SkipList::RandomLevel()
    {
        int level = 1;
        while ((cncpp::random() & 0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
            level += 1;

        return (level < ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
    }

    const uint64_t SkipList::size() const
    {
        return length_;
    }

    void SkipList::FreeNode(Node* node)
    {
        SAFE_DELETE(node);
    }

    SkipList::Node* SkipList::CreateNode(uint16_t level, const uint64_t key, const uint64_t score)
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

    SkipList::Node* SkipList::Insert(const uint64_t key, const uint64_t score)
    {
        Node*    update[ZSKIPLIST_MAXLEVEL] = {};
        uint32_t rank[ZSKIPLIST_MAXLEVEL]   = {};

        Node* node = this->header_;
        for (int16_t i = this->level_ - 1; i >= 0; i--)
        {
            /* store rank that is crossed to reach the insert position */
            rank[i] = (i == (this->level_ - 1) ? 0 : rank[i + 1]);
            while (node && node->next_nodes[i].next
                   && (node->next_nodes[i].next->score < score
                       || (node->next_nodes[i].next->score == score
                           && (node->next_nodes[i].next->key < key))))
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
        Node* new_node = CreateNode(new_level, key, score);
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

    void SkipList::Delete(Node* x, Node** update)
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

    int32_t SkipList::DeleteByKey(const uint64_t key)
    {
        const SkipList::Node* node = GetElementByKey(key);
        if (!node)
            return 0;

        return Delete(key, node->score);
    }

    int32_t SkipList::Delete(const uint64_t key, const uint64_t score)
    {
        return Delete(key, score, nullptr);
    }

    int32_t SkipList::Delete(const uint64_t key, const uint64_t score, Node** node)
    {
        Node* update[ZSKIPLIST_MAXLEVEL] = {};

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next
                   && (x->next_nodes[i].next->score < score
                       || (x->next_nodes[i].next->score == score
                           && (x->next_nodes[i].next->key < key))))
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

    SkipList::Node* SkipList::UpdateScore(
        const uint64_t key, const uint64_t curscore, const uint64_t newscore)
    {
        Node* update[ZSKIPLIST_MAXLEVEL] = {};

        /* We need to seek to element to update to start: this is useful anyway,
         * we'll have to update or remove it. */
        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next
                   && (x->next_nodes[i].next->score < curscore
                       || (x->next_nodes[i].next->score == curscore
                           && (x->next_nodes[i].next->key < key))))
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
            && (x->next_nodes[0].next == nullptr || x->next_nodes[0].next->score > newscore))
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

    /* Returns if there is a part of the skiplist is in range.
        check if the minimal element of skiplist is greater than parameter min
        and check if the maximum element of skiplist is less than parameter max
        minscore < std::min(header, tail) < std::max(header, tail) < maxscore
    */
    bool SkipList::IsInRange(const uint64_t minscore, const uint64_t maxscore) const
    {
        /* Test for ranges that will always be empty. */
        if (minscore > maxscore)
            return false;

        Node* x = this->tail_;
        if (x == nullptr || x->score < minscore)
            return false;

        x = header_->next_nodes[0].next;
        if (x == nullptr || (x->score > maxscore))
            return false;

        return true;
    }

    /* Find the last node that is contained in the specified range.
     * Returns NULL when no element is contained in the range.
        Return: the last node greater-equal than minscore and less-equal than maxscore
     */
    SkipList::Node* SkipList::LastInRange(const uint64_t minscore, const uint64_t maxscore) const
    {
        /* If everything is out of range, return early. */
        if (!IsInRange(minscore, maxscore))
            return nullptr;

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            /* Go forward while *IN* range. */
            while (x && x->next_nodes[i].next && x->next_nodes[i].next->score <= maxscore)
                x = x->next_nodes[i].next;
        }

        /* This is an inner range, so this node cannot be NULL. */
        // serverAssert(x != NULL);

        /* Check if score >= min. */
        if (x && x->score < minscore)
            return nullptr;

        return x;
    }

    /* Find the first node that is contained in the specified range.
     * Returns NULL when no element is contained in the range.
            Return: the first node greater-equal than minscore and less-equal than maxscore
     */
    SkipList::Node* SkipList::FirstInRange(const uint64_t minscore, const uint64_t maxscore) const
    {
        /* If everything is out of range, return early. */
        if (!IsInRange(minscore, maxscore))
            return nullptr;

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            /* Go forward while *OUT* of range. */
            while (x && x->next_nodes[i].next && (x->next_nodes[i].next->score < minscore))
                x = x->next_nodes[i].next;
        }

        /* This is an inner range, so the next node cannot be NULL. */
        x = (x ? x->next_nodes[0].next : nullptr);
        // serverAssert(x != NULL);

        /* Check if score <= max. */
        if (x->score > maxscore)
            return nullptr;

        return x;
    }

    /* Delete all the elements with score between min and max from the skiplist.
     * Both min and max can be inclusive or exclusive (see range->minex and
     * range->maxex). When inclusive a score >= min && score <= max is deleted.
     * Note that this function takes the reference to the hash table view of the
     * sorted set, in order to remove the elements from the hash table too. */
    uint64_t SkipList::DeleteRangeByScore(const uint64_t minscore, const uint64_t maxscore)
    {
        Node*    update[ZSKIPLIST_MAXLEVEL] = {};
        uint64_t removed                    = 0;

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next && (x->next_nodes[i].next->score < minscore))
                x = x->next_nodes[i].next;
            update[i] = x;
        }

        /* Current node is the last with score < or <= min. */
        x = (x ? x->next_nodes[0].next : nullptr);

        /* Delete nodes while in range. */
        while (x && (x->score <= maxscore))
        {
            Node* next = x->next_nodes[0].next;
            Delete(x, update);
            FreeNode(x); /* Here is where x->ele is actually released. */
            removed++;
            x = next;
        }
        return removed;
    }

    uint64_t SkipList::DeleteRangeByScoreCB(const uint64_t minscore, const uint64_t maxscore,
        const std::function<bool(const uint64_t key, const uint64_t score)>& cb)
    {
        Node*    update[ZSKIPLIST_MAXLEVEL] = {};
        uint64_t removed                    = 0;

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next && (x->next_nodes[i].next->score < minscore))
                x = x->next_nodes[i].next;
            update[i] = x;
        }

        /* Current node is the last with score < or <= min. */
        x = (x ? x->next_nodes[0].next : nullptr);

        /* Delete nodes while in range. */
        while (x && (x->score <= maxscore))
        {
            Node* next = x->next_nodes[0].next;
            Delete(x, update);
            cb(x->key, x->score);
            FreeNode(x); /* Here is where x->ele is actually released. */
            removed++;
            x = next;
        }
        return removed;
    }

    /* Delete all the elements with rank between start and end from the skiplist.
     * Start and end are inclusive. Note that start and end need to be 1-based */
    uint64_t SkipList::DeleteRangeByRank(uint32_t start, uint32_t end /*, dict* dict*/)
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
            // dictDelete(dict, x->ele);
            FreeNode(x);
            removed++;
            traversed++;
            x = next;
        }
        return removed;
    }

    /* Delete all the elements with rank between start and end from the skiplist.
     * Start and end are inclusive. Note that start and end need to be 1-based */
    uint64_t SkipList::DeleteRangeByRankCB(uint32_t start, uint32_t end,
        const std::function<bool(const uint64_t key, const uint64_t score)>& cb)
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

    /* Find the rank for an element by both score and key.
     * Returns 0 when the element cannot be found, rank otherwise.
     * Note that the rank is 1-based due to the span of zsl->header to the
     * first element. */
    uint64_t SkipList::GetRank(const uint64_t key, const uint64_t score) const
    {
        uint64_t rank = 0;

        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next
                   && (x->next_nodes[i].next->score < score
                       || (x->next_nodes[i].next->score == score
                           && x->next_nodes[i].next->key <= key)))
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

    /* Finds an element by its rank. The rank argument needs to be 1-based. */
    SkipList::Node* SkipList::GetElementByRank(const uint64_t rank) const
    {
        uint64_t traversed = 0;

        Node* x = header_;
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

    uint64_t SkipList::GetKeyByRank(const uint64_t rank) const
    {
        SkipList::Node* node = GetElementByRank(rank);
        if (!node)
            return 0;

        return node->key;
    }

    SkipList::Node* SkipList::GetElementByKey(const uint64_t key) const
    {
        Node* node = header_->next_nodes[0].next;
        while (node)
        {
            if (node->key == key)
                return node;

            node = node->next_nodes[0].next;
        }
        return nullptr;
    }

    bool SkipList::forEach(
        const std::function<bool(const uint64_t key, const uint64_t score)>& cb) const
    {
        Node* node = header_->next_nodes[0].next;
        while (node)
        {
            if (!cb(node->key, node->score))
                return false;

            node = node->next_nodes[0].next;
        }
        return true;
    }

    bool SkipList::forEachDecr(
        const std::function<bool(const uint64_t key, const uint64_t score)>& cb) const
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

    bool SkipList::forEach(
        const std::function<bool(const uint64_t key, const uint64_t score, const uint16_t level)>&
            cb) const
    {
        Node* node = header_->next_nodes[0].next;
        while (node)
        {
            if (!cb(node->key, node->score, node->level))
                return false;

            node = node->next_nodes[0].next;
        }
        return true;
    }

    bool SkipList::forEachRangedScore(const uint64_t minscore, const uint64_t maxscore,
        const std::function<bool(const uint64_t key, const uint64_t score)>& cb) const
    {
        Node* x = header_;
        for (int16_t i = level_ - 1; i >= 0; i--)
        {
            while (x && x->next_nodes[i].next && (x->next_nodes[i].next->score < minscore))
                x = x->next_nodes[i].next;
        }

        /* Current node is the last with score < or <= min. */
        x = (x ? x->next_nodes[0].next : nullptr);

        /* foreach nodes while in range. */
        while (x && (x->score <= maxscore))
        {
            if (!cb(x->key, x->score))
                return false;

            x = x->next_nodes[0].next;
        }
        return true;
    }

    bool SkipList::forEachRangedRank(const uint64_t start, const uint64_t end,
        const std::function<bool(const uint64_t key, const uint64_t score)>& cb) const
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
}  // namespace myskiplist2

namespace simple_toplist
{
    void BaseTopList::addDatas(const uint64_t key, const uint64_t value)
    {
        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
        {
            datas_map_.emplace(key, value);
            skiplist_.Insert(key, value);
        }
        else
        {
            const uint64_t oldvalue = iter->second;
            iter->second            = value;
            skiplist_.UpdateScore(key, oldvalue, iter->second);
        }
    }

    uint64_t BaseTopList::getScore(const uint64_t key) const
    {
        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
            return 0;

        return iter->second;
    }

    void BaseTopList::deleteByKey(const uint64_t key)
    {
        skiplist_.Delete(key, getScore(key));
        datas_map_.erase(key);
    }

    uint64_t BaseTopList::size() const
    {
        return datas_map_.size();
    }

    void BaseTopList::clear()
    {
        skiplist_.DeleteRangeByRank(1, size());
        datas_map_.clear();
    }

    // get the rank of toplist by key on decrease-order
    uint64_t BaseTopList::getDecrRankByKey(const uint64_t key) const
    {
        return SAFE_SUB(size() + 1, getIncrRankByKey(key));
    }

    // get the rank of toplist by key on increase-order
    uint64_t BaseTopList::getIncrRankByKey(const uint64_t key) const
    {
        return skiplist_.GetRank(key, getScore(key));
    }
}  // namespace simple_toplist