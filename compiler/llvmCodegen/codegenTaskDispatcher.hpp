//
// Created by XIaokang00010 on 2026/2/8.
//

#ifndef HOSHI_LANG_CODEGEN_TASK_DISPATCHER_HPP
#define HOSHI_LANG_CODEGEN_TASK_DISPATCHER_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace yoi {

    class CodegenTaskDispatcher {
    public:
        explicit CodegenTaskDispatcher(size_t threadCount = 0);
        ~CodegenTaskDispatcher();

        // Disable copy and move
        CodegenTaskDispatcher(const CodegenTaskDispatcher&) = delete;
        CodegenTaskDispatcher& operator=(const CodegenTaskDispatcher&) = delete;

        /**
         * @brief Dispatch a task to the thread pool.
         * @param task The task to execute.
         */
        void dispatch(std::function<void()> task);

        /**
         * @brief Wait for all dispatched tasks to complete.
         */
        void wait();

    private:
        void workerLoop();

        std::vector<std::thread> workers;
        std::queue<std::function<void()>> taskQueue;

        std::mutex queueMutex;
        std::condition_variable queueCondition;

        std::mutex waitMutex;
        std::condition_variable waitCondition;

        std::atomic<size_t> activeTasks{0};
        std::atomic<bool> stop{false};
    };

} // namespace yoi

#endif // HOSHI_LANG_CODEGEN_TASK_DISPATCHER_HPP
