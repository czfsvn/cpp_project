#include <iostream>

#include "Global.h"
#include "MemoryPool.h"
#include "boost_asio.h"
#include "logger.h"

namespace ns_log
{
    void test_memory()
    {
        // Test::main();
    }

    void main()
    {
        TRACE("examples main test trace");
        CRITICAL("examples main test critical");
        INFO("examples main test info");
        DEBUG("examples main test debug");
        WARN("examples main test warn, {}, {}, {}, {}", 1.0000f, 1010, "abc", 1.0005);
        ERR("examples main test warn, {}, {}, {}, {}", 1.0000f, 1010, "abc", 1.0005);
        test_memory();
    }

}  // namespace ns_log

namespace ns_spdlog
{
    void main()
    {
        // cncpp::globle_init("examples");
        ns_log::main();
        while (1)
        {
        }
        return;
    }
}  // namespace ns_spdlog
