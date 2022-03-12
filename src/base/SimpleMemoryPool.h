#ifndef __SIMPLE_MEMORY_POOL_H__
#define __SIMPLE_MEMORY_POOL_H__

#include "logger.h"
#include <list>
#include <map>
#include <unordered_map>

#include "Singleton.h"

const uint32_t kCachedSize = 100;

namespace cncpp
{
    class BlockPool;
    using BlockPtr = std::shared_ptr<BlockPool>;

    class BlockPool
    {
    public:
        BlockPool(uint32_t size) : align_size_(size)
        {
            reAlloc();
        }

        void* allocOne()
        {
            if (pool_.empty())
                reAlloc();

            if (pool_.empty())
                return nullptr;

            void* rt = pool_.back();
            pool_.pop_back();
            return rt;
        }
        void freeOne(void* block)
        {
            if (!block)
                return;

            pool_.emplace_front(block);
            checkAndFreeSome();
        }

    private:
        void reAlloc()
        {
            for (uint32_t cnt = 0; cnt < kCachedSize; cnt++)
            {
                pool_.emplace_front(::malloc(align_size_));
            }
        }
        void checkAndFreeSome() {}

    private:
        std::list<void*> pool_       = {};
        const uint32_t   align_size_ = 0;
    };

    class BlockPoolManager : public cncpp::Singleton<BlockPoolManager>
    {
    public:
        BlockPoolManager()
        {
            init();
        }

        void init()
        {
            const static std::map<uint16_t, std::pair<uint16_t, uint16_t>> aligns_map = {
                { 8, { 8, 16 } },      // 8字节对齐, 分配大小列表是 8 + 8 * 15       1~~128
                { 16, { 144, 120 } },  // 16字节对齐, 分配大小列表是 144 + 16 * 120   129~~2048
            };

            for (const auto& kvp : aligns_map)
            {
                const uint32_t align = kvp.first;
                for (uint16_t idx = 0; idx < kvp.second.second; idx++)
                {
                    const uint16_t usize = kvp.second.first + align * idx;
                    block_map_.emplace(usize, std::make_shared<BlockPool>(usize));
                    TRACE("BlockPoolManager init: {}", usize);
                }
            }
        }

        void* allocOne(const uint32_t size)
        {
            const uint16_t size_index = Roundup(size);
            auto           iter       = block_map_.find(size_index);
            if (iter == block_map_.end())
                return nullptr;

            return iter->second->allocOne();
        }

        void freeOne(void* block, uint32_t size)
        {
            if (!block)
                return;

            const uint16_t size_index = Roundup(size);
            auto           iter       = block_map_.find(size_index);
            if (iter == block_map_.end())
                return;

            return iter->second->freeOne(block);
        }

    public:
        template <typename T, typename... Args>
        T* New(Args&&... args)
        {
            const uint32_t size = sizeof(T);
            void*          data = allocOne(size);
            if (!data)
                return nullptr;

            T* result = new (data) T(std::forward<Args>(args)...);
            return result;
        }

        template <typename T, typename... Args>
        std::shared_ptr<T> make_shared(Args&&... args)
        {
            T* result = New<T>(std::forward<Args>(args)...);
            return std::shared_ptr<T>(result,
                                      [this](T* data)
                                      {
                                          Delete(data);
                                      });
        }

        template <typename T, typename... Args>
        std::unique_ptr<T> make_unique(Args&&... args)
        {
            T* result = New<T>(std::forward<Args>(args)...);

            return std::unique_ptr<T>(result,
                                      [this](T* data)
                                      {
                                          Delete(data);
                                      });
        }

        template <typename T>
        void Delete(T* data)
        {
            if (!data)
                return;

            data->~T();

            const uint32_t size = sizeof(T);
            BlockPoolManager::getMe().freeOne(( void* )data, size);

            data = nullptr;
        }

    private:
        BlockPtr getBlockPtr(uint32_t size)
        {
            const uint16_t size_index = Roundup(size);
            auto           iter       = block_map_.find(size_index);
            if (iter == block_map_.end())
                return BlockPtr();

            return iter->second;
        }

    private:
        std::unordered_map<uint32_t, BlockPtr> block_map_ = {};
    };

    class SimpleWrap
    {
    public:
    };
}  // namespace cncpp

#endif