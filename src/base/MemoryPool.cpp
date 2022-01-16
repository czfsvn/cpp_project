#include "MemoryPool.h"
#include <algorithm>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <vector>

#define SPLIT_PAGE_COUNT 5
const static bool use_page_alloc = true;

namespace cncpp
{
    //  MemoryBlocks /////////////////////////
    bool MemoryBlocks::canAllocate(uint32_t need) const
    {
        return free_size_ >= need;
    }

    void MemoryBlocks::mem_free(void *block)
    {
        if (!block)
            return;

        Block *newblock = (Block *)block;
        newblock->next  = free_list_;
        free_list_      = newblock;

        {
            used_size_ = SAFE_SUB(used_size_, 1);
            free_size_ += 1;
        }

        std::cout << "free : " << ADDR_TO_UINT64(block) << ", size： " << size_ << std::endl;
        // std::cout << "MyMemPool::mem_free size: " << size_ << ", freesize: " << free_size_ << ",
        // used: " << used_size_ << ", block: " << block << std::endl;
    }

    void MemoryBlocks::put_block(void *block)
    {
        if (!block)
            return;

        Block *newblock = (Block *)block;
        newblock->next  = free_list_;
        free_list_      = newblock;

        free_size_ += 1;
        std::cout << "put_block : " << ADDR_TO_UINT64(block) << ", size： " << size_ << std::endl;

        // std::cout << "MyMemPool::put_block size: " << size_ << ", freesize: " << free_size_ << ",
        // used: " << used_size_ << ", block: " << block << std::endl;
    }

    void *MemoryBlocks::mem_malloc()
    {
        if (!free_list_ || !free_size_)
            return nullptr;

        Block *newblock = free_list_;
        free_list_      = newblock->next;

        {
            free_size_ = SAFE_SUB(free_size_, 1);
            used_size_ += 1;
        }

        // std::cout << "MyMemPool::mem_malloc size: " << size_ << ", freesize: " << free_size_ <<
        // ", used: " << used_size_ << ", newblock: " << newblock << std::endl;

        std::cout << "reuse : " << ADDR_TO_UINT64(((void *)newblock)) << ", size： " << size_
                  << std::endl;

        return newblock;
    }

    void MemoryBlocks::recode_inused(void *block)
    {
        if (!block)
            return;

        used_size_++; // 暂时记录已使用的数量

        // std::cout << "MyMemPool::recode_inused size: " << size_ << ", freesize: " << free_size_
        // << ", used: " << used_size_ << ", block: " << block << std::endl;
    }

    void MemoryBlocks::free_all()
    {
        if (use_page_alloc)
            return;

        while (free_list_)
        {
            Block *newblock = (Block *)free_list_;
            free_list_      = free_list_->next;
            if (newblock)
            {
                free((void *)newblock);
                newblock = nullptr;
            }
        }

        while (in_used_list_)
        {
            Block *newblock = in_used_list_;
            in_used_list_   = in_used_list_->next;
            if (newblock)
            {
                free((void *)newblock);
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
    const static std::map<uint16_t, std::pair<uint16_t, uint16_t>> aligns_map = {
        {8, {8, 16}},     // 8字节对齐, 分配大小列表是 8 + 8 * 15       1~~128
        {16, {144, 120}}, // 16字节对齐, 分配大小列表是 144 + 16 * 120   129~~2048
    };

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
        sys_page_size_ = getpagesize();
        for (const auto &kvp : aligns_map)
        {
            const uint32_t align = kvp.first;
            for (uint16_t idx = 0; idx < kvp.second.second; idx++)
            {
                const uint16_t size = kvp.second.first + align * idx;
                allocator_map_.emplace(size, std::make_shared<MemoryBlocks>(size));
                // std::cout << "make pool align: " << align << ", size: " << size << std::endl;
            }
        }
    }

    void MyMemPool::myfree(void *block)
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

    void *MyMemPool::myalloc(size_t size)
    {
        if (!size)
            return nullptr;

        std::cout << "\n MyMemPool::myalloc=====================================" << size
                  << std::endl;
        std::cout << "MyMemPool::myalloc size: " << size << std::endl;
        if (size > MAX_BYTES)
        { // large memory use system malloc
            void *result = malloc(size);
            onUseMemBlock(result, helper::Roundup(size));
            return result;
        }

        uint16_t        size_index = helper::Roundup(size);
        MemoryBlocksPtr alloctor   = getAllocator(size_index);
        if (!alloctor)
        {
            // no memory allocator use system malloc
            return malloc(size);
        }

        if (alloctor->canAllocate(1))
        {
            char *result = static_cast<char *>(alloctor->mem_malloc());
            if (result)
            {
                onUseMemBlock(result, size_index);
                return result;
            }
        }

        // allocator malloc faild use Page allocate
        return page_alloc(size_index);
    }

    void *MyMemPool::page_alloc(size_t size)
    {
        std::cout << "MyMemPool::page_alloc =" << size << std::endl;
        const uint16_t size_index = helper::Roundup(size);
        for (auto iter = page_list_.begin(); iter != page_list_.end(); /* ++iter*/)
        {
            PagePtr page = *iter;
            if (!page)
            {
                iter = page_list_.erase(iter);
                continue;
            }

            char *will_endpos = page->cur_pos + size;
            if (will_endpos <= page->end_pos)
            {
                char *result = page->cur_pos;
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
            void *   buff      = malloc(page_size);
            assert(buff);

            newpage->start_  = (char *)buff;
            newpage->cur_pos = (char *)buff;
            newpage->end_pos = (char *)buff + page_size;
            // std::cout << "new page: from(" << newpage->cur_pos << ", " << newpage->end_pos << "),
            // size: " << (newpage->end_pos - newpage->cur_pos) << std::endl;
            std::cout << "MyMemPool::new_page : " << ADDR_TO_UINT64(((void *)buff))
                      << ", page_size " << page_size << std::endl;
        }

        page_list_.emplace_back(newpage);

        // const size_t len = ( size_t )(newpage->end_pos - newpage->cur_pos);
        const char *will_endpos = newpage->cur_pos + size;
        if (will_endpos < newpage->end_pos)
        {
            char *result = newpage->cur_pos;
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

        for (auto iter = allocator_map_.rbegin(); iter != allocator_map_.rend(); ++iter)
        {
            if (!iter->second)
                continue;

            while (page->cur_pos + iter->first < page->end_pos)
            {
                std::cout << "MyMemPool::trySplitPage : " << iter->first << std::endl;
                iter->second->put_block(page->cur_pos);
                page->cur_pos = page->cur_pos + iter->first;
            }

            if (page->cur_pos >= page->end_pos)
                return;
        }
    }

    void MyMemPool::onUseMemBlock(void *block, const uint16_t size_index)
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

} // namespace cncpp

namespace Test
{
    class TestBase1
    {
    public:
        TestBase1()
        {
            std::cout << "TestBase1 default\n";
        }

        TestBase1(uint32_t i)
            : i32_value(i)
            , i64_value(i)
        {
            std::cout << "TestBase1 construct: " << i32_value << ", this: " << this << std::endl;
        }
        ~TestBase1()
        {
            std::cout << "TestBase1 destruct: " << i32_value << ", this: " << this << std::endl;
        }

        void Print() const
        {
            std::cout << "TestBase1 print: " << i32_value << ", i64_value: " << i64_value
                      << ", addr: " << this << std::endl;
        }

    private:
        uint32_t i32_value = 100;
        char     c_value   = '3';
        uint64_t i64_value = 101;
        // std::string str_valu1 = "abdafdfasdfasdfasdfasdfsdfsdfasdfasdfas";
        // std::string str_valu2 = "abdafdfasdfasdfasdfasdfsdfsdfasdfasdfas";
    };

    void test_index()
    {
        for (uint32_t idx = 0; idx < 200; idx++)
        {
            std::cout << "idx :" << idx << "--" << helper::Index(idx) << std::endl;
        }
    }

    void test_round()
    {
        std::map<uint32_t, std::vector<uint32_t>> tmpmap;
        for (uint32_t idx = 0; idx < 3000; idx++)
        {
            // std::cout << "idx :" << idx << "--" << helper::Roundup(idx) << std::endl;
            tmpmap[helper::Roundup(idx)].emplace_back(idx);
        }

        for (const auto &item : tmpmap)
        {
            std::stringstream oss;
            oss << "(";
            for (const auto &v : item.second)
            {
                // assert(item.first < v);
                oss << v << ",";
            }
            oss << ")";

            std::cout << "key: " << item.first << oss.str() << std::endl;
        }
    }

    void test_toAddr()
    {
        uint32_t *value1 = new uint32_t(100);
        uint32_t *value2 = new uint32_t(1200);

        std::cout << "value1: " << value1 << ", addr: " << ADDR_TO_UINT64(value1)
                  << ", uint: " << uint64_t(value1) << std::endl;
        std::cout << "value2: " << value2 << ", addr: " << ADDR_TO_UINT64(value2)
                  << ", uint: " << uint64_t(value2) << std::endl;
    }

    void test_charAddr()
    {
        const uint16_t arr_size = 20;
        char *         addr     = new char[arr_size];
        for (uint16_t i = 0; i < arr_size; i++)
        {
            addr[i] = 'c';
        }
    }

    void test_one_alloc()
    {
        cncpp::MyMemPool pool;
        TestBase1 *      base = new (pool.myalloc(sizeof(TestBase1))) TestBase1();
        if (!base)
        {
            std::cout << "malloc failed\n";
        }
        else
        {
            pool.myfree(base);
        }
        std::cout << "test\n";
    }

    void test_some_alloc()
    {
        cncpp::MyMemPool       pool;
        std::list<TestBase1 *> test_list = {};
        const uint32_t         size      = 10;
        for (uint32_t i = 0; i < size; i++)
        {
            test_list.emplace_back(new (pool.myalloc(sizeof(TestBase1))) TestBase1(i));
        }

        std::cout << "test_some_alloc, first allocte\n";
        std::for_each(test_list.begin(), test_list.end(), [](TestBase1 *base) {
            if (!base)
                return;

            base->Print();
        });

        while (test_list.size() >= 6)
        {
            TestBase1 *base = test_list.front();
            test_list.pop_front();
            pool.myfree(base);
        }

        std::cout << "test_some_alloc, after erase\n";
        std::for_each(test_list.begin(), test_list.end(), [](TestBase1 *base) {
            if (!base)
                return;

            base->Print();
        });

        for (uint32_t i = 0; i < size; i++)
        {
            test_list.emplace_back(new (pool.myalloc(sizeof(TestBase1))) TestBase1(i + 1000));
        }

        std::cout << "test_some_alloc, after alloc\n";
        std::for_each(test_list.begin(), test_list.end(), [](TestBase1 *base) {
            if (!base)
                return;

            base->Print();
        });

        while (test_list.size() >= 6)
        {
            TestBase1 *base = test_list.front();
            test_list.pop_front();
            pool.myfree(base);
        }

        std::cout << "test_some_alloc, after erase\n";
        std::for_each(test_list.begin(), test_list.end(), [](TestBase1 *base) {
            if (!base)
                return;

            base->Print();
        });
    }

    /*----------------------------------------------------------------
    内存池测试相关
    1、相同大小单个对象指针的分配和释放
    2、多个对象指针的分配和释放
    3、内存池从重复利用
    4、page 中剩余内存的split 到内存池
    5、不同大小对象的混合存储释放
    6、数组分配释放

    MemPoolWrap
    1、智能指针的存储和释放
    2、智能指针数组的存储和释放
    */

    void main()
    {
        std::cout << "test\n";
        // test_alloc();
        test_some_alloc();
        // test_round();
        // test_toAddr();
        // test_charAddr();
    }

} // namespace Test
