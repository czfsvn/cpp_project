#include "Global.h"
#include "Misc.h"
#include "ThreadPool.h"
#include <iostream>

using namespace std;
using namespace cncpp;

namespace test_threadpool
{
    uint64_t get_task_idx()
    {
        static uint64_t idx = 0;
        return ++idx;
    }

    class TestTask : public Task
    {
    public:
        TestTask(uint64_t idx) : task_id(idx)
        {
            std::cout << "test task thread: " << getThreadId() << ", construct: " << task_id << std::endl;
        }

        ~TestTask()
        {
            std::cout << "test task thread: " << getThreadId() << ", destruct: " << task_id << std::endl;
        }

        virtual bool work()
        {
            std::cout << "test task thread: " << getThreadId() << ", work: " << task_id << std::endl;
            cncpp::sleepfor_microseconds(rnd.Next() % 1000);
            return true;
        }

        virtual bool done()
        {
            std::cout << "test task thread: " << getThreadId() << ", done: " << task_id << std::endl;
            cncpp::sleepfor_microseconds(rnd.Next() % 1000);
            return true;
        }

    private:
        uint64_t task_id = 0;
    };

    class TestTaskThread : public cncpp::Thread
    {
    public:
        TestTaskThread(cncpp::TaskPoolPtr ptr) : pool_ptr_(ptr) {}
        virtual void run() override
        {
            // pool_ptr_->addNewTask(std::make_shared<TestTask>(get_task_idx()));
            sleepfor_microseconds(10);
        }

    private:
        cncpp::TaskPoolPtr pool_ptr_ = nullptr;

        uint32_t task_count = 0;
    };

    void test()
    {
        sleepfor_seconds(2);
        cncpp::TaskPoolPtr pool = std::make_shared<cncpp::TaskPool>(4);
        pool->init();
        pool->startAll();

        TestTaskThread* test_thrd1 = new TestTaskThread(pool);
        test_thrd1->start();

        TestTaskThread* test_thrd2 = new TestTaskThread(pool);
        test_thrd2->start();

        while (1)
        {
            pool->runInMain();
            sleepfor_microseconds(5);
        }

        test_thrd1->join();
        test_thrd2->join();
        pool->joinAll();
    }

    void main()
    {
        test();
        std::cout << "hello, test_threadpool:: main\n";
    }
}  // namespace test_threadpool

namespace ns_threadpool
{
    void main(int argc, char** argv)
    {
        test_threadpool::main();
    }
}  // namespace ns_threadpool
