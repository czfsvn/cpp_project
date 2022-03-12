#include "MemoryPool.h"
#include <algorithm>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <vector>

#include "logger.h"

#define SPLIT_PAGE_COUNT 5
const static bool use_page_alloc = true;

namespace cncpp
{
    //  MemoryBlocks /////////////////////////
    bool MemoryBlocks::canAllocate(uint32_t need) const
    {
        return free_size_ >= need;
    }

    void MemoryBlocks::mem_free(void* block)
    {
        if (!block)
            return;

        Block* newblock = ( Block* )block;
        newblock->next  = free_list_;
        free_list_      = newblock;

        {
            used_size_ = SAFE_SUB(used_size_, 1);
            free_size_ += 1;
        }

        // DEBUG("free: {}, size: {}", ADDR_TO_UINT64(block), size_);
    }

    void MemoryBlocks::put_block(void* block)
    {
        if (!block)
            return;

        Block* newblock = ( Block* )block;
        newblock->next  = free_list_;
        free_list_      = newblock;

        free_size_ += 1;

        // DEBUG("put_block: {}, size: {}", ADDR_TO_UINT64(block), size_);
    }

    void* MemoryBlocks::mem_malloc()
    {
        if (!free_list_ || !free_size_)
            return nullptr;

        Block* newblock = free_list_;
        free_list_      = newblock->next;

        {
            free_size_ = SAFE_SUB(free_size_, 1);
            used_size_ += 1;
        }

        // DEBUG("reuse: {}, size: {}", ADDR_TO_UINT64(newblock), size_);

        return newblock;
    }

    void MemoryBlocks::recode_inused(void* block)
    {
        if (!block)
            return;

        used_size_++;  // 暂时记录已使用的数量
    }

    void MemoryBlocks::free_all()
    {
        if (use_page_alloc)
            return;

        while (free_list_)
        {
            Block* newblock = ( Block* )free_list_;
            free_list_      = free_list_->next;
            if (newblock)
            {
                free(( void* )newblock);
                newblock = nullptr;
            }
        }

        while (in_used_list_)
        {
            Block* newblock = in_used_list_;
            in_used_list_   = in_used_list_->next;
            if (newblock)
            {
                free(( void* )newblock);
                newblock = nullptr;
            }
        }
        free_size_ = 0;
        used_size_ = 0;
    }

    // Page //////////////////////////////
    Page::~Page()
    {
        if (use_page_alloc && start_)
        {
            free(start_);
        }
    }

    //  MyMemPool /////////////////////////

    MyMemPool::MyMemPool()
    {
        init_pool();
    }

    MyMemPool::~MyMemPool()
    {
        allocator_map_.clear();
        used_map_.clear();

        page_list_.clear();
        exhausted_page_list_.clear();
    };

    void MyMemPool::init_pool()
    {
        const static std::map<uint16_t, std::pair<uint16_t, uint16_t>> aligns_map = {
            { 8, { 8, 16 } },      // 8字节对齐, 分配大小列表是 8 + 8 * 15       1~~128
            { 16, { 144, 120 } },  // 16字节对齐, 分配大小列表是 144 + 16 * 120   129~~2048
        };

        sys_page_size_ = getpagesize();
        for (const auto& kvp : aligns_map)
        {
            const uint32_t align = kvp.first;
            for (uint16_t idx = 0; idx < kvp.second.second; idx++)
            {
                const uint16_t size = kvp.second.first + align * idx;
                allocator_map_.emplace(size, std::make_shared<MemoryBlocks>(size));
            }
        }
    }

    void MyMemPool::myfree(void* block)
    {
        if (!block)
            return;

        uint64_t size_index = ADDR_TO_UINT64(block);
        auto     iter       = used_map_.find(size_index);
        if (iter == used_map_.end())
        {
            free(block);
            return;
        }

        MemoryBlocksPtr alloc_ptr = getAllocator(iter->second);
        if (alloc_ptr)
        {
            alloc_ptr->mem_free(block);
        }
        else
        {
            free(block);
        }

        used_map_.erase(iter);
    }

    void* MyMemPool::myalloc(size_t size)
    {
        if (!size)
            return nullptr;

        // DEBUG("MyMemPool::myalloc =====================================");
        // DEBUG("MyMemPool::myalloc size: {}", size);

        if (size > MAX_BYTES)
        {  // large memory use system malloc
            void* result = malloc(size);
            onUseMemBlock(result, Roundup(size));
            return result;
        }

        uint16_t        size_index = Roundup(size);
        MemoryBlocksPtr alloctor   = getAllocator(size_index);
        if (!alloctor)
        {
            // no memory allocator use system malloc
            return malloc(size);
        }

        if (alloctor->canAllocate(1))
        {
            char* result = static_cast<char*>(alloctor->mem_malloc());
            if (result)
            {
                onUseMemBlock(result, size_index);
                return result;
            }
        }

        // allocator malloc faild use Page allocate
        return page_alloc(size_index);
    }

    void* MyMemPool::page_alloc(size_t size)
    {
        // INFO("MyMemPool::page_alloc: {}", size);
        const uint16_t size_index = Roundup(size);
        for (auto iter = page_list_.begin(); iter != page_list_.end(); /* ++iter*/)
        {
            PagePtr page = *iter;
            if (!page)
            {
                iter = page_list_.erase(iter);
                continue;
            }

            char* will_endpos = page->cur_pos + size;
            if (will_endpos <= page->end_pos)
            {
                char* result = page->cur_pos;
                page->cur_pos += size;
                onUseMemBlock(result, size_index);
                return result;
            }

            if (++page->failed >= SPLIT_PAGE_COUNT)
            {
                // try split to pool
                trySplitPage(page);
                exhausted_page_list_.emplace_back(page);
                iter = page_list_.erase(iter);
            }
            else
            {
                iter++;
            }
        }

        PagePtr newpage = std::make_shared<Page>();
        if (!newpage)
        {
            assert(false);
            return nullptr;
        }
        {
            uint16_t page_size = sys_page_size_ ? sys_page_size_ : INIT_PAGE_SIZE;
            void*    buff      = malloc(page_size);
            assert(buff);

            newpage->start_  = ( char* )buff;
            newpage->cur_pos = ( char* )buff;
            newpage->end_pos = ( char* )buff + page_size;

            // INFO("MyMemPool::new_page : {}, page_size: {}", ADDR_TO_UINT64(((void*)buff)), size);
        }

        page_list_.emplace_back(newpage);

        // const size_t len = ( size_t )(newpage->end_pos - newpage->cur_pos);
        const char* will_endpos = newpage->cur_pos + size;
        if (will_endpos < newpage->end_pos)
        {
            char* result = newpage->cur_pos;
            newpage->cur_pos += size;

            onUseMemBlock(result, size_index);
            return result;
        }

        return nullptr;
    }

    void MyMemPool::trySplitPage(PagePtr page)
    {
        if (!page)
            return;

        for (auto iter = allocator_map_.begin(); iter != allocator_map_.end(); ++iter)
        {
            if (!iter->second)
                continue;

            while (page->cur_pos + iter->first < page->end_pos)
            {
                // DEBUG("MyMemPool::trySplitPage : {}", iter->first);
                iter->second->put_block(page->cur_pos);
                page->cur_pos = page->cur_pos + iter->first;
            }

            if (page->cur_pos >= page->end_pos)
                return;
        }
    }

    void MyMemPool::onUseMemBlock(void* block, const uint16_t size_index)
    {
        if (!block || !size_index)
            return;

        used_map_.emplace(ADDR_TO_UINT64(block), size_index);

        MemoryBlocksPtr alloc_ptr = getAllocator(size_index);
        if (!alloc_ptr)
            return;

        alloc_ptr->recode_inused(block);
    }

    MyMemPool::MemoryBlocksPtr MyMemPool::getAllocator(const uint16_t size_index)
    {
        auto iter = allocator_map_.find(size_index);
        if (iter == allocator_map_.end())
            return MemoryBlocksPtr();

        return iter->second;
    }

}  // namespace cncpp
