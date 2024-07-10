#include "s0_thread_pool.hpp"

s0m4b0dY::ThreadPool::ThreadPool(int nThreads)
{
    work_threads_.emplace_back(
        workFunction, std::ref(tasks_)
    );
}

s0m4b0dY::ThreadPool::~ThreadPool()
{
    tasks_.close();
}

void s0m4b0dY::ThreadPool::workFunction(Queue_t &tasks_)
{
    while (true)
    {
        Task_t task;
        auto status = tasks_.wait_pull(task);
        if (status == boost::concurrent::queue_op_status::closed)
           return; 
        task();
    }
}
