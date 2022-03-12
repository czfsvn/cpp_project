#ifndef __SKIPLIST_20220206_H__
#define __SKIPLIST_20220206_H__

#include "Global.h"
#include "Misc.h"
#include "logger.h"
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

#define ZSKIPLIST_MAXLEVEL 32
#define ZSKIPLIST_P        0.25

namespace ns_leveldb
{
    template <typename Key, class Comparator>
    class SkipList
    {
    private:
        template <typename ITEM>
        class Node
        {
            ITEM key;
        };

    public:
        SkipList(const SkipList&) = delete;
        SkipList& operator=(const SkipList&) = delete;

        void Insert(const Key& key);

        bool Erase(const Key& key);

        bool Contains(const Key& key) const;

        bool ForEach(std::function<bool(const Key& key)>& cb);

        uint32_t GetRank(const Key& key);
    };

}  // namespace ns_leveldb

namespace ns_redis
{
    class SkipList
    {
        struct Node
        {
            void print();

            struct NextNode
            {
                uint32_t span = 0;
                Node*    next = nullptr;
            };

            std::string           ele        = {};
            uint64_t              score      = 0;  // key
            Node*                 backward   = nullptr;
            std::vector<NextNode> next_nodes = {};
        };

    public:
        SkipList();
        SkipList(const SkipList&) = delete;
        SkipList& operator=(const SkipList&) = delete;

        ~SkipList();

        void printAll();

        Node*    zslInsert(const uint64_t score, const std::string& ele);
        Node*    zslUpdateScore(uint64_t curscore, const std::string& ele, uint64_t newscore);
        uint32_t zslDelete(uint64_t score, const std::string& ele, Node** node);
        void     zslDeleteNode(Node* x, Node** update);

        uint32_t zslGetRank(uint64_t score, const std::string& ele);
        Node*    zslGetElementByRank(const uint32_t rank);

        uint32_t zslIsInRange(const uint32_t min, const uint32_t max);
        Node*    zslFirstInRange(const uint32_t min, const uint32_t max);
        Node*    zslLastInRange(const uint32_t min, const uint32_t max);
        uint32_t zslDeleteRangeByScore(const uint32_t min, const uint32_t max /*, dict* dict*/);
        uint32_t zslDeleteRangeByLex(const uint32_t min, const uint32_t max /*, dict* dict*/);

    private:
        uint16_t zslRandomLevel();

        Node* zslCreateNode(uint16_t level, uint64_t score, const std::string& ele);
        void  freeNode(Node* x);

    private:
        uint32_t length_ = 0;
        uint16_t level_  = 0;
        Node*    header_ = nullptr;
        Node*    tail_   = nullptr;
    };
}  // namespace ns_redis

namespace myskiplist1
{
    template <typename ITEM>
    class SkipList
    {
    public:
        template <typename DATA>
        struct Node
        {
            struct NextNode
            {
                uint32_t    span = 0;
                Node<DATA>* next = nullptr;
            };

            DATA                  data       = {};
            Node<DATA>*           backward   = nullptr;
            std::vector<NextNode> next_nodes = {};
            uint32_t              level      = 0;  // for debug

            void dump()
            {
                INFO("level:{}, data:{}", level, data.dump());
            }
        };

    private:
        uint16_t randomLevel()
        {
            int level = 1;
            while ((cncpp::random() & 0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
                level += 1;

            return (level < ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
        }

        Node<ITEM>* createNode(const uint16_t lv, const ITEM& data)
        {
            Node<ITEM>* node = new Node<ITEM>();
            node->data       = data;
            node->level      = lv;
            node->next_nodes.reserve(lv);
            return node;
        }

    public:
        SkipList()
        {
            length_ = 0;
            level_  = 1;
            header_ = createNode(ZSKIPLIST_MAXLEVEL, ITEM());
        }

        ~SkipList()
        {
            if (!header_)
                return;

            if (header_->next_nodes.empty())
            {
                SAFE_DELETE(header_);
                return;
            }

            Node<ITEM>* node = header_->next_nodes[0].next;
            while (node)
            {
                Node<ITEM>* next = node->next_nodes[0].next;
                SAFE_DELETE(node);
                node = next;
            }
            SAFE_DELETE(header_);
        }

        void dump() const
        {
            Node<ITEM>* node = header_->next_nodes[0].next;
            while (node)
            {
                node->dump();
                node = node->next_nodes[0].next;
            }
        }

        bool forEach(const std::function<bool(const ITEM& data)>& cb) const
        {
            Node<ITEM>* node = header_->next_nodes[0].next;
            while (node)
            {
                // node->dump();
                if (!cb(node->data))
                    return false;

                node = node->next_nodes[0].next;
            }
            return true;
        }

        Node<ITEM>* Insert(const ITEM& data)
        {
            Node<ITEM>* update[ZSKIPLIST_MAXLEVEL] = {};
            uint32_t    rank[ZSKIPLIST_MAXLEVEL]   = {};

            Node<ITEM>* node = this->header_;
            for (int16_t i = this->level_ - 1; i >= 0; i--)
            {
                /* store rank that is crossed to reach the insert position */
                rank[i] = i == (this->level_ - 1) ? 0 : rank[i + 1];
                while (node->next_nodes[i].next && ((node->next_nodes[i].next->data) < data))
                {
                    rank[i] += node->next_nodes[i].span;
                    node = node->next_nodes[i].next;
                }
                update[i] = node;
            }

            uint16_t new_level = randomLevel();
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
            Node<ITEM>* new_node = createNode(new_level, data);
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

        bool     Update(const ITEM& data);
        bool     Delete(const ITEM& data);
        bool     DeleteByKey(const uint64_t key);
        bool     DeleteByRank(const uint64_t rank);
        uint64_t getRankByKey(const uint64_t key) const;

    private:
        uint32_t    length_ = 0;
        uint16_t    level_  = 0;
        Node<ITEM>* header_ = nullptr;
        Node<ITEM>* tail_   = nullptr;
    };
}  // namespace myskiplist1

namespace myskiplist2
{
    class SkipList
    {
        struct Node
        {
            void print();

            struct NextNode
            {
                uint32_t span = 0;
                Node*    next = nullptr;
            };

            uint64_t              key        = 0;  // Ψһֵ
            uint64_t              score      = 0;  // ����ֵ
            uint16_t              level      = 0;
            Node*                 backward   = nullptr;
            std::vector<NextNode> next_nodes = {};
        };

    public:
        SkipList();
        ~SkipList();

        SkipList(const SkipList&) = delete;
        SkipList& operator=(const SkipList&) = delete;

        void dumpAll();
        bool forEach(const std::function<bool(const uint64_t key, const uint64_t score)>& cb) const;
        bool forEach(const std::function<bool(const uint64_t key, const uint64_t score, const uint16_t level)>& cb) const;

        bool forEachDecr(const std::function<bool(const uint64_t key, const uint64_t score)>& cb) const;

        // todo: need test unit
        bool forEachRangedScore(const uint64_t minscore, const uint64_t maxscore, const std::function<bool(const uint64_t key, const uint64_t score)>& cb) const;
        bool forEachRangedRank(const uint64_t start, const uint64_t end, const std::function<bool(const uint64_t key, const uint64_t score)>& cb) const;

        Node*   Insert(const uint64_t key, const uint64_t score);
        Node*   UpdateScore(const uint64_t key, const uint64_t curscore, const uint64_t newscore);
        int32_t Delete(const uint64_t key, const uint64_t score);

        int32_t DeleteByKey(const uint64_t key);

        uint64_t GetRank(const uint64_t key, const uint64_t score) const;

        uint64_t GetKeyByRank(const uint64_t rank) const;

        bool     IsInRange(const uint64_t min, const uint64_t max) const;
        Node*    FirstInRange(const uint64_t min, const uint64_t max) const;
        Node*    LastInRange(const uint64_t min, const uint64_t max) const;
        uint64_t DeleteRangeByScore(const uint64_t min, const uint64_t max /*, dict* dict*/);
        uint64_t DeleteRangeByScoreCB(const uint64_t min, const uint64_t max, const std::function<bool(const uint64_t key, const uint64_t score)>& cb);

        uint64_t DeleteRangeByRank(uint32_t start, uint32_t end /*, dict* dict*/);
        uint64_t DeleteRangeByRankCB(uint32_t start, uint32_t end, const std::function<bool(const uint64_t key, const uint64_t score)>& cb);

        const uint64_t size() const;

    private:
        uint16_t RandomLevel();

        Node*   CreateNode(uint16_t level, const uint64_t key, const uint64_t score);
        void    FreeNode(Node* x);
        int32_t Delete(const uint64_t key, const uint64_t score, Node** node);
        void    Delete(Node* x, Node** update);
        Node*   GetElementByRank(const uint64_t rank) const;
        Node*   GetElementByKey(const uint64_t key) const;

    private:
        uint32_t length_ = 0;
        uint16_t level_  = 0;
        Node*    header_ = nullptr;
        Node*    tail_   = nullptr;
    };

    template <typename DATA>
    class TopList
    {
    public:
        // for load and no save
        void addData(const DATA& data, bool neesSave = false);
        void deleteByKey(const uint64_t key);
        void deleteByRank(const uint64_t rank);
        void deleteByRangedRank(const uint64_t rankfrom, const uint64_t rankto);
        void deleteByRangedRank2(const uint64_t rankfrom, const uint64_t rankto);

        uint64_t getRankByKey(const uint64_t key);
        uint64_t getRevRankByKey(const uint64_t key);

        bool     incrScore(const uint64_t key, const uint64_t score);
        bool     decrScore(const uint64_t key, const uint64_t score);
        uint32_t getScore(const uint64_t key);
        DATA*    getDataByKey(const uint64_t key);
        DATA*    getDataByRank(const uint64_t rank);

        DATA getRawDataByKey(const uint64_t key);
        DATA getRawDataByRank(const uint64_t rank);

        // attention: may exist some records with same rank score
        uint32_t getRankByScore(const uint64_t score);

        // need save
        void     updateInfos(const DATA& data);
        uint64_t size() const;
        void     clear();

        bool forEach(const std::function<bool(const DATA&)>& cb);

        bool forEachByRangedScore(const uint64_t minscore, const uint64_t maxscore, const std::function<bool(const DATA&)>& cb);
        bool forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, const std::function<bool(const DATA&)>& cb);
        bool forEachByRank(const uint64_t rankfrom, const uint64_t rankto, const std::function<bool(const DATA&)>& cb);

    private:
        SkipList                           skiplist_   = {};
        std::unordered_map<uint64_t, DATA> datas_map_  = {};
        std::set<uint64_t>                 dirty_sets_ = {};
    };

    template <typename DATA>
    void TopList<DATA>::addData(const DATA& data, bool neesSave /* = false*/)
    {
        auto iter = datas_map_.find(data.getKey());
        if (iter == datas_map_.end())
        {
            datas_map_[data.getKey()] = data;
            skiplist_.Insert(data.getKey(), data.getScore());

            if (neesSave)
            {
                dirty_sets_.emplace(data.getKey());
            }
        }
        else
        {
            if (iter->second.getScore() != data.getScore())
            {
                skiplist_.UpdateScore(data.getKey(), iter->second.getScore(), data.getScore());
            }
            datas_map_[data.getKey()] = data;

            if (neesSave)
            {
                dirty_sets_.emplace(data.getKey());
            }
        }
    }

    template <typename DATA>
    void TopList<DATA>::updateInfos(const DATA& data)
    {
        return addData(data, true);
    }

    template <typename DATA>
    bool TopList<DATA>::incrScore(const uint64_t key, const uint64_t score)
    {
        if (!score || !key)
            return false;

        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
            return false;

        const uint64_t old_score = iter->second->score;
        iter->second->score += score;

        dirty_sets_.emplace(key);
        skiplist_.UpdateScore(key, old_score, iter->second.getScore());
        return true;
    }

    template <typename DATA>
    bool TopList<DATA>::decrScore(const uint64_t key, const uint64_t score)
    {
        if (!score || !key)
            return false;

        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
            return false;

        const uint64_t old_score = iter->second->score;
        iter->second->score      = SAFE_SUB(iter->second->score, score);
        dirty_sets_.emplace(key);
        skiplist_.UpdateScore(key, old_score, iter->second.getScore());
        return true;
    }

    template <typename DATA>
    void TopList<DATA>::deleteByKey(const uint64_t key)
    {
        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
            return;

        skiplist_.Delete(key, iter->second.getScore());
        datas_map_.erase(iter);
    }

    template <typename DATA>
    void TopList<DATA>::deleteByRank(const uint64_t rank)
    {
        deleteByKey(getDataByKey(rank));
    }

    template <typename DATA>
    void TopList<DATA>::deleteByRangedRank2(const uint64_t rankfrom, const uint64_t rankto)
    {
        std::unordered_map<uint64_t, uint64_t> tmpmap = {};
        for (uint64_t rank = rankfrom; rank <= rankto; rank++)
        {
            // deleteByRank(rank);
            tmpmap.emplace(skiplist_.GetKeyByRank(rank), rank);
        }

        for (const auto& item : tmpmap)
        {
            datas_map_.erase(item.first);
            skiplist_.Delete(item.first, item.second);
        }
    }

    template <typename DATA>
    void TopList<DATA>::deleteByRangedRank(const uint64_t rankfrom, const uint64_t rankto)
    {
        skiplist_.DeleteRangeByRankCB(rankfrom, rankto,
                                      [this](const uint64_t key, const uint64_t score)
                                      {
                                          datas_map_.erase(key);
                                          return true;
                                      });
    }

    template <typename DATA>
    uint64_t TopList<DATA>::getRankByKey(const uint64_t key)
    {
        return skiplist_.GetRank(key, getScore(key));
    }

    template <typename DATA>
    uint64_t TopList<DATA>::getRevRankByKey(const uint64_t key)
    {
        return size() + 1 - skiplist_.GetRank(key, getScore(key));
    }

    template <typename DATA>
    uint64_t TopList<DATA>::size() const
    {
        return skiplist_.size();
    }

    template <typename DATA>
    void TopList<DATA>::clear()
    {
        skiplist_.DeleteRangeByRank(1, size());
        datas_map_.clear();
    }

    template <typename DATA>
    uint32_t TopList<DATA>::getScore(const uint64_t key)
    {
        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
            return 0;

        return iter->second.getScore();
    }

    template <typename DATA>
    DATA* TopList<DATA>::getDataByKey(const uint64_t key)
    {
        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
            return nullptr;

        return &iter->second;
    }

    template <typename DATA>
    DATA* TopList<DATA>::getDataByRank(const uint64_t rank)
    {
        return getDataByKey(skiplist_.GetKeyByRank(rank));
    }

    template <typename DATA>
    DATA TopList<DATA>::getRawDataByKey(const uint64_t key)
    {
        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
            return DATA();

        return iter->second;
    }

    template <typename DATA>
    DATA TopList<DATA>::getRawDataByRank(const uint64_t rank)
    {
        return getRawDataByKey(skiplist_.GetKeyByRank(rank));
    }

    template <typename DATA>
    bool TopList<DATA>::forEach(const std::function<bool(const DATA&)>& cb)
    {
        for (const auto& item : datas_map_)
        {
            if (!cb(item.second))
                return false;
        }
        return true;
    }

    template <typename DATA>
    bool TopList<DATA>::forEachByRank(const uint64_t rankfrom, const uint64_t rankto, const std::function<bool(const DATA&)>& cb)
    {
        std::unordered_map<uint64_t, uint64_t> tmpmap = {};
        for (uint64_t rank = rankfrom; rank <= rankto; rank++)
        {
            if (!cb(*getDataByKey(skiplist_.GetKeyByRank(rank))))
                return false;
        }
        return true;
    }

    template <typename DATA>
    bool TopList<DATA>::forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, const std::function<bool(const DATA&)>& cb)
    {
        skiplist_.forEachRangedRank(rankfrom, rankto,
                                    [&cb, this](const uint64_t key, const uint64_t score)
                                    {
                                        DATA* data = getDataByKey(key);
                                        if (!data || !cb(*data))
                                            return false;

                                        return true;
                                    });
        return true;
    }

    template <typename DATA>
    bool TopList<DATA>::forEachByRangedScore(const uint64_t minscore, const uint64_t maxscore, const std::function<bool(const DATA&)>& cb)

    {
        skiplist_.forEachRangedScore(minscore, maxscore,
                                     [&cb, this](const uint64_t key, const uint64_t score)
                                     {
                                         DATA* data = getDataByKey(key);
                                         if (!data || !cb(*data))
                                             return false;

                                         return true;
                                     });
        return true;
    }

}  // namespace myskiplist2

namespace sharedptr_toplist
{
    template <typename DATA>
    class TopList
    {
    public:
        typedef std::shared_ptr<DATA> DataType;

    private:
        myskiplist2::SkipList                  skiplist_   = {};
        std::unordered_map<uint32_t, DataType> datas_map_  = {};
        std::set<uint32_t>                     dirty_sets_ = {};

    public:
        void deleteByKey(const uint64_t key);
        void deleteByRank(const uint64_t rank);
        void deleteByRangedRank(const uint64_t rankfrom, const uint64_t rankto);
        void deleteByRangedRank2(const uint64_t rankfrom, const uint64_t rankto);

        uint64_t getIncrRankByKey(const uint64_t key);
        uint64_t getDecrRankByKey(const uint64_t key);

        bool     incrScore(const uint64_t key, const uint64_t score);
        bool     decrScore(const uint64_t key, const uint64_t score);
        uint32_t getScore(const uint64_t key);

        uint32_t getRankByScore(const uint64_t score);

        uint64_t size() const;
        void     clear();

    public:
        void addData(const DataType& data, bool needsave = false)
        {
            if (!data)
                return;

            auto iter = datas_map_.find(data->getKey());
            if (iter == datas_map_.end())
            {
                datas_map_[data->getKey()] = data;
                skiplist_.Insert(data->getKey(), data->getScore());

                if (needsave)
                {
                    dirty_sets_.emplace(data->getKey());
                }
            }
            else
            {
                // check if need to plus score and old
                if (iter->second->getScore() != data->getScore())
                {
                    skiplist_.UpdateScore(data->getKey(), iter->second->getScore(), data->getScore());
                }
                datas_map_[data->getKey()] = data;

                if (needsave)
                {
                    dirty_sets_.emplace(data->getKey());
                }
            }
        }

        // need save
        void updateInfos(const DataType& data)
        {
            addData(data, true);
        }

        DataType getDataByKey(const uint64_t key)
        {
            auto iter = datas_map_.find(key);
            if (iter == datas_map_.end())
                return DataType();

            return iter->second;
        }

        DataType getDataByRank(const uint64_t rank)
        {
            return getDataByKey(skiplist_.GetKeyByRank(rank));
        }

        template <typename FUNC>
        bool forEach(FUNC&& cb)
        {
            for (const auto& item : datas_map_)
            {
                if (!cb(item.second))
                    return false;
            }
            return true;
        }

        template <typename FUNC>
        bool forEachByRangedRank(const uint64_t rankfrom, const uint64_t rankto, FUNC&& cb)
        {
            skiplist_.forEachRangedRank(rankfrom, rankto,
                                        [&cb, this](const uint64_t key, const uint64_t score)
                                        {
                                            DataType data = getDataByKey(key);
                                            if (!data || !cb(data))
                                                return false;

                                            return true;
                                        });
            return true;
        }

        template <typename FUNC>
        bool forEachByRangedScore(const uint64_t minscore, const uint64_t maxscore, FUNC&& cb)
        {
            skiplist_.forEachRangedScore(minscore, maxscore,
                                         [&cb, this](const uint64_t key, const uint64_t score)
                                         {
                                             DataType data = getDataByKey(key);
                                             if (!data || !cb(data))
                                                 return false;

                                             return true;
                                         });
            return true;
        }
    };

    template <typename DATA>
    bool TopList<DATA>::incrScore(const uint64_t key, const uint64_t score)
    {
        if (!score || !key)
            return false;

        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
            return false;

        const uint64_t old_score = iter->second->getScore();
        iter->second->setScore(old_score + score);

        dirty_sets_.emplace(key);
        skiplist_.UpdateScore(key, old_score, iter->second.getScore());
        return true;
    }

    template <typename DATA>
    bool TopList<DATA>::decrScore(const uint64_t key, const uint64_t score)
    {
        if (!score || !key)
            return false;

        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
            return false;

        const uint64_t old_score = iter->second->getScore();
        iter->second->setScore(SAFE_SUB(old_score, score));
        dirty_sets_.emplace(key);
        skiplist_.UpdateScore(key, old_score, iter->second.getScore());
        return true;
    }

    template <typename DATA>
    void TopList<DATA>::deleteByKey(const uint64_t key)
    {
        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
            return;

        skiplist_.Delete(key, iter->second->getScore());
        datas_map_.erase(iter);
    }

    template <typename DATA>
    void TopList<DATA>::deleteByRank(const uint64_t rank)
    {
        deleteByKey(getDataByKey(rank));
    }

    template <typename DATA>
    void TopList<DATA>::deleteByRangedRank(const uint64_t rankfrom, const uint64_t rankto)
    {
        std::unordered_map<uint64_t, uint64_t> tmpmap = {};
        for (uint64_t rank = rankfrom; rank <= rankto; rank++)
        {
            // deleteByRank(rank);
            tmpmap.emplace(skiplist_.GetKeyByRank(rank), rank);
        }

        for (const auto& item : tmpmap)
        {
            datas_map_.erase(item.first);
            skiplist_.Delete(item.first, item.second);
        }
    }

    template <typename DATA>
    void TopList<DATA>::deleteByRangedRank2(const uint64_t rankfrom, const uint64_t rankto)
    {
        skiplist_.DeleteRangeByRankCB(rankfrom, rankto,
                                      [this](const uint64_t key, const uint64_t score)
                                      {
                                          datas_map_.erase(key);
                                      });
    }

    template <typename DATA>
    uint64_t TopList<DATA>::getIncrRankByKey(const uint64_t key)
    {
        return skiplist_.GetRank(key, getScore(key));
    }

    template <typename DATA>
    uint64_t TopList<DATA>::getDecrRankByKey(const uint64_t key)
    {
        return size() + 1 - skiplist_.GetRank(key, getScore(key));
    }

    template <typename DATA>
    uint64_t TopList<DATA>::size() const
    {
        return skiplist_.size();
    }

    template <typename DATA>
    void TopList<DATA>::clear()
    {
        skiplist_.DeleteRangeByRank(1, size() + 1);
        datas_map_.clear();
    }

    template <typename DATA>
    uint32_t TopList<DATA>::getScore(const uint64_t key)
    {
        auto iter = datas_map_.find(key);
        if (iter == datas_map_.end())
            return 0;

        return iter->second->getScore();
    }

}  // namespace sharedptr_toplist

namespace simple_toplist
{
    class BaseTopList
    {
    private:
        std::unordered_map<uint64_t, uint64_t> datas_map_ = {};
        myskiplist2::SkipList                  skiplist_  = {};

    public:
        void addDatas(const uint64_t key, const uint64_t value);

        uint64_t getScore(const uint64_t key) const;

        // get the rank of toplist by key of decrease
        uint64_t getDecrRankByKey(const uint64_t key) const;

        // get the rank of toplist by key of increase
        uint64_t getIncrRankByKey(const uint64_t key) const;

        void deleteByKey(const uint64_t key);

        void deleteByScore(const uint64_t score);

        uint64_t size() const;

        void clear();

        template <typename FUNCTOR>
        bool forEachTopNofDecr(const uint64_t topN, FUNCTOR&& cb)
        {
            const uint64_t from = SAFE_SUB(size() + 1, topN);
            const uint64_t to   = size();
            if (from > to)
                return false;

            skiplist_.forEachRangedRank(from, to,
                                        [&cb](const uint64_t key, const uint64_t score)
                                        {
                                            if (!cb(key, score))
                                                return false;

                                            return true;
                                        });
            return true;
        }

        template <typename FUNCTOR>
        bool forEachTopNOfIncr(const uint64_t topN, FUNCTOR&& cb)
        {
            if (topN <= 0)
                return false;

            skiplist_.forEachRangedRank(1, topN,
                                        [&cb](const uint64_t key, const uint64_t score)
                                        {
                                            if (!cb(key, score))
                                                return false;

                                            return true;
                                        });
            return true;
        }
    };

}  // namespace simple_toplist

namespace test_skiplist
{
    void main_skiplist(int argc, char** argv);
}

#endif
