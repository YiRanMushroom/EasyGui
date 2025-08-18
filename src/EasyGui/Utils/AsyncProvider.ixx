export module EasyGui.Utils.AsyncProvider;

import std.compat;
export import EasyGui.Utils.Flags;

namespace EasyGui {
    using namespace std::chrono_literals;

    export template<typename T>
    class AsyncProvider {
    public:
        using value_type = T;

        void Update() {
            if (m_CompletableFuture && m_CompletableFuture->wait_for(0ms) == std::future_status::ready) {
                m_Value = std::move(m_CompletableFuture->get());
                m_CompletableFuture.reset();
            }
        }

        void Wait() {
            if (m_CompletableFuture) {
                m_CompletableFuture->wait();
                m_Value = std::move(m_CompletableFuture->get());
                m_CompletableFuture.reset();
            }
        }

        T &Get() {
            if (!m_Value && !m_CompletableFuture) {
                throw std::runtime_error("Value is not set and no future is available.");
            }

            Update();

            if (m_Value) {
                return *m_Value;
            }

            Wait();

            if (!m_Value) {
                throw std::runtime_error("Value is still not set after waiting.");
            }

            return *m_Value;
        }

        T &GetRaw() const {
            if (!m_Value) {
                throw std::runtime_error("Value is not set.");
            }
            return *m_Value;
        }

        operator T &() {
            return Get();
        }

        T &operator*() {
            return Get();
        }

        T &operator()() {
            return Get();
        }

        void SetValue(T &&value) {
            m_Value = std::move(value);
        }

        template<typename F> requires requires(F func) {
            { func() } -> std::convertible_to<T>;
        }
        void SetFuture(F &&func) {
            Update();
            m_CompletableFuture = std::make_optional<std::future<T>>(
                std::async(std::launch::async, std::forward<F>(func)));
        }

        template<typename F> requires requires(F func) {
            { func() } -> std::convertible_to<T>;
        }
        AsyncProvider(F &&func) : AsyncProvider{} {
            SetFuture(std::forward<F>(func));
        }

        template<typename F> requires requires(F func) {
            { func() } -> std::convertible_to<T>;
        }
        AsyncProvider(DefaultConstructed, F &&func) : AsyncProvider{T{}} {
            SetFuture(std::forward<F>(func));
        }

        AsyncProvider(T &&value) : m_Value(std::move(value)), m_CompletableFuture{} {}

        AsyncProvider(const T &value) : m_Value(value), m_CompletableFuture{} {}

        AsyncProvider(DefaultConstructed) : AsyncProvider{T{}} {}

        AsyncProvider(EmptyConstructed) : AsyncProvider{} {}

    private:
        AsyncProvider() = default;

        std::optional<T> m_Value;
        std::optional<std::future<T>> m_CompletableFuture;
    };
}
