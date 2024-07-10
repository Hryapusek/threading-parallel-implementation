#ifndef S0_THREAD_POOL_HPP
#define S0_THREAD_POOL_HPP

#include <vector>
#include <memory>
#include <thread>
#include <future>
#include <condition_variable>
#include <boost/thread/concurrent_queues/sync_queue.hpp>

namespace s0m4b0dY
{
    class ThreadPool
    {
        using Task_t = std::function<void()>;
        using Queue_t = boost::sync_queue<Task_t>;
    public:
        ThreadPool(int nThreads);
        ~ThreadPool();

        template < class Fn, class... Args >
        std::future<std::invoke_result_t<Fn, Args...>> submit(Fn &&func, Args&&... args);

        static void workFunction(Queue_t &tasks_);

    private:
        Queue_t tasks_;
        std::vector<std::jthread> work_threads_;
    };
    
    template <class Fn, class... Args>
    inline std::future<std::invoke_result_t<Fn, Args...>> ThreadPool::submit(Fn &&func, Args &&...args)
    {
        if constexpr (sizeof...(Args) > 0)
        {
            using Result_t = std::invoke_result_t<Fn, Args...>;
            auto task_ptr = std::make_shared<std::packaged_task<Result_t(Args...)>>(
                [func=std::forward<Fn>(func), args=std::forward<Args...>(args...)]() mutable
                {
                    return std::forward<Fn>(func)(std::forward<Args...>(args));
                }
                );
            auto future = task_ptr->get_future();
            tasks_.wait_push([task_ptr=std::move(task_ptr)]() mutable {
                (*task_ptr)();
            });
            return future;
        }
        else
        {
            using Result_t = decltype(std::declval<Fn>()());
            auto task_ptr = std::make_shared<std::packaged_task<Result_t()>>(
                [func=std::forward<Fn>(func)]() mutable
                {
                    return std::forward<Fn>(func)();
                }
                );
            auto future = task_ptr->get_future();
            tasks_.wait_push([task_ptr=std::move(task_ptr)]() mutable {
                (*task_ptr)();
            });
            return future;
        }
    }

    // template <class Fn, class... Args>
    // inline std::future<std::invoke_result_t<Fn, Args...>> ThreadPool::submit(Fn &&func, Args &&...args)
    // {
    //     using Result_t = std::invoke_result_t<Fn, Args...>;
    //     std::promise<int> promise;
    //     promise.se
    //     auto future = promise.get_future();
    //     tasks_.wait_push(
    //         [promise=std::move(promise), func=std::forward<Fn>(func), args=std::forward<Args...>(args)]()
    //         {
    //             promise. std::forward<Fn>(func)()(std::forward<Args...>(args)...);
    //         }
    //     );
    // }
}

#endif
