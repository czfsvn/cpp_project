#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "Thread.h"
#include <vector>
#include "SyncQueue.h"

namespace cncpp
{
    class Task;
    class TaskPool;
    class TaskThread;
    using TaskPtr       = std::shared_ptr<Task>;
    using TaskPoolPtr   = std::shared_ptr<TaskPool>;
    using TaskThreadPtr = std::shared_ptr<TaskThread>;

    class Task
    {
    public:
        virtual bool work() = 0;
        virtual bool done()
        {
            return true;
        }
    };

    class TaskThread : public Thread
    {
    public:
        TaskThread(const TaskPoolPtr &ptr);
        bool addTask(const TaskPtr &task);
        void run() override;

    private:
        TaskPoolPtr        pool_ptr_   = nullptr;
        SyncQueue<TaskPtr> task_queue_ = {};
    };

    class TaskPool : public std::enable_shared_from_this<TaskPool>
    {
    public:
        TaskPool(uint16_t size);

        bool init();

        void addDone(TaskPtr &task);

        bool addNewTask(const TaskPtr &task);

        bool addNewTask(Task *task);

        void runInMain();

        void joinAll();

        void startAll();

    private:
        TaskThreadPtr getThread();

    private:
        std::atomic<uint32_t>      cur_thread_idx_; // = 0;
        uint16_t                   thread_size_ = 0;
        std::vector<TaskThreadPtr> thread_vec_  = {};
        SyncQueue<TaskPtr>         task_done_   = {};
    };
} // namespace cncpp
#endif // !__THREADPOOL_H__
