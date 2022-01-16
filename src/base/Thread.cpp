#include "Thread.h"
#include <functional>

namespace cncpp
{
    Thread::Thread()
    {
        stopped_.store(true);
    }

    Thread::~Thread() {}

    // base option
    void Thread::start()
    {
        stopped_ = false;
        if (!thrd_ptr_)
        {
            thrd_ptr_ = std::unique_ptr<std::thread>(new std::thread(std::bind(&Thread::thrdloop, this)));
        }
    }

    void Thread::stop()
    {
        stopped_ = true;
    }

    void Thread::join()
    {
        if (thrd_ptr_ && thrd_ptr_->joinable())
        {
            thrd_ptr_->join();
        }
    }

    void Thread::thrdloop()
    {
        while (!isStop())
        {
            run();
            // sleepfor_microseconds(1);
        }
    }

    bool Thread::isStop()
    {
        return stopped_;
    }
}
