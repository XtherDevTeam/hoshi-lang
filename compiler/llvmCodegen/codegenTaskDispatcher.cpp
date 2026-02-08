//
// Created by XIaokang00010 on 2026/2/8.
//

#include "codegenTaskDispatcher.hpp"

namespace yoi {

    CodegenTaskDispatcher::CodegenTaskDispatcher(size_t threadCount) {
        // automatically use hardware cores from system info
        if (threadCount == 0)
            threadCount = std::thread::hardware_concurrency();
        for (size_t i = 0; i < threadCount; ++i) {
            workers.emplace_back(&CodegenTaskDispatcher::workerLoop, this);
        }
    }

    CodegenTaskDispatcher::~CodegenTaskDispatcher() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        queueCondition.notify_all();
        for (std::thread &worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    void CodegenTaskDispatcher::dispatch(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            activeTasks++;
            taskQueue.push(std::move(task));
        }
        queueCondition.notify_one();
    }

    void CodegenTaskDispatcher::wait() {
        std::unique_lock<std::mutex> lock(waitMutex);
        waitCondition.wait(lock, [this]() { return activeTasks == 0; });
    }

    void CodegenTaskDispatcher::workerLoop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCondition.wait(lock, [this]() { return stop || !taskQueue.empty(); });
                if (stop && taskQueue.empty()) {
                    return;
                }
                task = std::move(taskQueue.front());
                taskQueue.pop();
            }

            task();

            {
                std::unique_lock<std::mutex> lock(waitMutex);
                activeTasks--;
                if (activeTasks == 0) {
                    waitCondition.notify_all();
                }
            }
        }
    }

} // namespace yoi
