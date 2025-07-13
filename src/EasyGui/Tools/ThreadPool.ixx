export module EasyGui.Tools.ThreadPool;

import std.compat;

namespace EasyGui {
    export class ThreadPool {
    public:
        ThreadPool(size_t size = std::jthread::hardware_concurrency() * 2) {
            m_WorkerThreads.reserve(size);
            for (size_t i = 0; i < size; ++i) {
                m_WorkerThreads.emplace_back([this] {
                    while (!m_ShouldStop) {
                        std::function<void()> task;

                        // in a scope
                        {
                            std::unique_lock lock(m_Mutex);
                            m_Condition.wait(lock, [this] { return m_ShouldStop || !m_Tasks.empty(); });

                            if (m_ShouldStop && m_Tasks.empty()) {
                                return;
                            }

                            task = std::move(m_Tasks.front());
                            m_Tasks.pop();
                        }

                        task();
                    }
                });
            }
        }

        template<typename... Args> requires std::invocable<Args...>
        void EnqueueDetached(Args &&... args) {
            std::function<void()> task = [tup = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                std::apply([]<typename... Tps>(const auto &first, Tps &&... rest) {
                    first(std::forward<Tps>(rest)...);
                }, std::move(tup));
            };
            // forward
            {
                std::lock_guard lock(m_Mutex);
                m_Tasks.emplace(std::move(task));
            }
            m_Condition.notify_one();
        }

        template<typename... Args> requires std::invocable<Args...>
        auto Enqueue(Args &&... args) {
            using ReturnType = std::invoke_result_t<Args...>;
            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                [tup = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                    return std::apply([]<typename... Tps>(const auto &first, Tps &&... rest) {
                        return first(std::forward<Tps>(rest)...);
                    }, std::move(tup));
                });

            std::future<ReturnType> res = task->get_future();
            // forward
            {
                std::lock_guard lock(m_Mutex);
                m_Tasks.emplace([task]() { (*task)(); });
            }
            m_Condition.notify_one();
            return res;
        }

        ~ThreadPool() {
            if (m_ShouldStop) {
                m_WorkerThreads.clear();
                return;
            }

            m_ShouldStop = true;
            m_Condition.notify_all();
            for (auto &thread: m_WorkerThreads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        }

        void DetachAll() {
            m_ShouldStop = true;
            m_Condition.notify_all();
            for (auto &thread: m_WorkerThreads) {
                if (thread.joinable()) {
                    thread.detach();
                }
            }
            m_WorkerThreads.clear();
        }

    private:
        std::vector<std::jthread> m_WorkerThreads{};
        std::condition_variable m_Condition{};
        std::mutex m_Mutex{};
        std::queue<std::function<void()>> m_Tasks{};
        std::atomic_bool m_ShouldStop{false};
    };
}
