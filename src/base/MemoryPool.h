#ifndef __MYPOOL1_20211117_H__
#define __MYPOOL1_20211117_H__

#include <assert.h>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include "Misc.h"

using namespace std;

#define HASH_TABLE_SIZE 1638400
#define ADDR_TO_HASH(p) ((reinterpret_cast<uint64_t>(p) >> 8) % (HASH_TABLE_SIZE))

#define ADDR_TO_UINT64(addr) ((uint64_t)addr)

#define INIT_PAGE_SIZE 100
#define MAX_BYTES      2048

#define BLOCK_TIME ns_timer::BlockCost(__FUNCTION__, __LINE__)

#if __USE_MEMORY__
#    define MAKE_SHARED wrap->make_shared
#    define MAKE_UNIQUE wrap->make_unique
#else
#    define MAKE_SHARED std::make_shared
#    define MAKE_UNIQUE std::make_unique
#endif

namespace helper
{
    inline static uint16_t _Index(uint16_t size, uint16_t align)
    {
        uint16_t alignnum = 1 << align; //库里实现的方法
        return ((size + alignnum - 1) >> align) - 1;
    }

    inline static uint16_t _Roundup(uint16_t size, uint16_t align)
    {
        uint16_t alignnum = 1 << align;
        return (size + alignnum - 1) & ~(alignnum - 1);
    }

    // 控制在12%左右的内碎片浪费
    // [1,128]				8byte对齐 freelist[0,16)
    // [129,1024]			16byte对齐 freelist[16,72)
    // [1025,8*1024]		128byte对齐 freelist[72,128)
    // [8*1024+1,64*1024]	1024byte对齐 freelist[128,184)

    inline static uint16_t Index(uint16_t size)
    {
        assert(size <= MAX_BYTES);

        // 每个区间有多少个链
        static int group_array[4] = {16, 56, 56, 56};
        if (size <= 128)
        {
            return _Index(size, 3);
        }
        else if (size <= 1024)
        {
            return _Index(size - 128, 4) + group_array[0];
        }
        else if (size <= 8192)
        {
            return _Index(size - 1024, 7) + group_array[0] + group_array[1];
        }
        else // if (size <= 65536)
        {
            return _Index(size - 8 * 1024, 10) + group_array[0] + group_array[1] + group_array[2];
        }
    }

    // 对齐大小计算，向上取整
    static inline uint16_t Roundup(uint16_t bytes)
    {
        assert(bytes <= MAX_BYTES);

        if (bytes <= 128)
        {
            return _Roundup(bytes, 3);
        }
        else if (bytes <= 2048)
        {
            return _Roundup(bytes, 4);
        }
        else if (bytes <= 8192)
        {
            return _Roundup(bytes, 7);
        }
        else
        { // if (bytes <= 65536){
            return _Roundup(bytes, 10);
        }
    }

} // namespace helper

namespace cncpp
{
    union Block
    {
        Block *next;
        char   data[1];
    };

    struct Page
    {
        void *   start_  = {}; // page 指针
        char *   cur_pos = {}; // 当前可用的位置
        char *   end_pos = {}; // 当前页结束位置
        uint16_t failed  = 0;  // 分配失败的次数

        ~Page();
    };

    class MemoryBlocks
    {
    public:
        MemoryBlocks(uint16_t bsize)
            : size_(bsize)
        {}
        ~MemoryBlocks()
        {
            free_all();
        };

        bool canAllocate(uint32_t need) const;

        void mem_free(void *block);

        void put_block(void *block);

        void *mem_malloc();

        void recode_inused(void *block);

    private:
        void free_all();

    private:
        uint16_t size_         = 0;       // 每个size 大小
        uint32_t free_size_    = 0;       // 可用数量
        uint32_t used_size_    = 0;       // 在用数量
        Block *  free_list_    = nullptr; // 可用列表
        Block *  in_used_list_ = nullptr; // 正在用列表
    };

    class MyMemPool
    {
        using MemoryBlocksPtr = std::shared_ptr<MemoryBlocks>;
        using PagePtr         = std::shared_ptr<Page>;

    public:
        MyMemPool();
        ~MyMemPool();

        void *myalloc(size_t size);
        void  myfree(void *block);

    private:
        void init_pool();

        void *page_alloc(size_t size);

        void trySplitPage(PagePtr page);

        void onUseMemBlock(void *block, const uint16_t size);

        MemoryBlocksPtr getAllocator(const uint16_t size_index);

    private:
        std::map<uint16_t, MemoryBlocksPtr> allocator_map_       = {}; // <size, 分配器>
        std::map<uint64_t, uint16_t>        used_map_            = {}; // <address, size>
        std::list<PagePtr>                  page_list_           = {}; // 页面指针
        std::list<PagePtr>                  exhausted_page_list_ = {};
        uint16_t                            sys_page_size_       = 0; // 页面大小
    };

    class MemPoolWrap
    {
    public:
        /*
        MemPoolWrap() : pool_(new MyMemPool) {}
        ~MemPoolWrap()
        {
            SAFE_DELETE(pool_);
        }
        */

        template<typename T, typename... Args>
        T *New(Args &&...args)
        {
            uint32_t size = sizeof(T);
            void *   data = pool_.myalloc(size);
            if (!data)
                return nullptr;

            T *result = new (data) T(std::forward<Args>(args)...);
            return result;
        }

        template<typename T, typename... Args>
        std::shared_ptr<T> make_shared(Args &&...args)
        {
            T *result = New<T>(std::forward<Args>(args)...);
            return std::shared_ptr<T>(result, [this](T *data) { Delete(data); });
        }

        template<typename T, typename... Args>
        std::unique_ptr<T> make_unique(Args &&...args)
        {
            T *result = New<T>(std::forward<Args>(args)...);
            /*
            return std::unique_ptr<T>(result,
                                      [this](T* data)
                                      {
                                          Delete(data);
                                      });
                                      */
            return std::unique_ptr<T>(result);
        }

        template<typename T>
        void Delete(T *data)
        {
            if (!data)
                return;

            data->~T();
            pool_.myfree((void *)data);
            data = nullptr;
        }

        template<typename T>
        void Delete(T *data, uint32_t size)
        {
            if (!data)
                return;

            data->~T();
            pool_.myfree((void *)size);
            data = nullptr;
        }

    private:
        MyMemPool pool_;
        // = nullptr;
    };
} // namespace cncpp

namespace Test
{
    // std::set<uint32_t> uset;
    // uint32_t           cnt  = uset.count(1);
    // auto               iter = uset.find(1);
    void main();
} // namespace Test

#endif // __MYPOOL1_20211117_H__
