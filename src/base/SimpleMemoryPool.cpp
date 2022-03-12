//#include "SimpleMemeryPool.h"

namespace cncpp
{
    /*
    BlockPoolManager::BlockPoolManager()
    {
        init();
    }

    BlockPoolManager::~BlockPoolManager()
    {
        block_map_.clear();
    }

    void BlockPoolManager::init()
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
                // allocator_map_.emplace(size, std::make_shared<MemoryBlocks>(size));
                block_map_.emplace(size, std::make_shared<BlockPool<size>>());
            }
        }
    }

    BlockPtr BlockPoolManager::getBlockPtr(uint32_t size)
    {
        const uint16_t size_index = Roundup(size);
        auto           iter       = block_map_.find(size_index);
        if (iter == block_map_.end())
            return BlockPtr();

        return iter->second;
    }

    */
}  // namespace cncpp