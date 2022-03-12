#ifndef __SIMPLE_MEMORY_POOL_H__
#define __SIMPLE_MEMORY_POOL_H__

#include <list>
#include <unordered_map>

#include "Singleton.h"

const uint32_t kCachedSize = 100;

namespace cncpp
{
    class BlockBase;
    using BlockPtr = std::shared_ptr<BlockBase>;

    class BlockBase
    {
    public:
        void* allocOne()           = 0;
        void  freeOne(void* block) = 0;
    };

    template <uint32_t kSize>
    class BlockPool : public BlockBase
    {
    public:
        BlockPool();
        ~BlockPool();

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
                pool_.emplace_front(::malloc(kSize));
            }
        }
        void checkAndFreeSome();

    private:
        std::list<void*> pool_ = {};
    };

    class BlockPoolManager : public Singleton<BlockPoolManager>
    {
    public:
        BlockPoolManager();
        ~BlockPoolManager();

        void init();

    public:
        template <typename T, typename... Args>
        T* New(Args&&... args)
        {
            const uint32_t size = sizeof(T);
            BlockPtr       ptr  = getBlockPtr(size);
            if (!ptr)
            {
                return new T(std::forward<Args>(args)...);
            }

            void* data = pool_.myalloc(size);
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
            BlockPtr       ptr  = getBlockPtr(size);
            if (!ptr)
            {
                free(data);
            }
            else
            {
                ptr->freeOne(( void* )data);
            }

            data = nullptr;
        }

    private:
        BlockPtr getBlockPtr(uint32_t size);

    private:
        std::unordered_map<uint32_t, BlockPtr> block_map_ = {};
    };
}  // namespace cncpp

#endif