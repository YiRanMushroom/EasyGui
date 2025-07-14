export module EasyGui.Utils.Atomic;

import std.compat;

namespace EasyGui {
    export template<typename T>
    class Atomic {
    public:
        Atomic() = default;
        Atomic(const Atomic&) = delete;
        Atomic(Atomic&&) = delete;

        Atomic(T&& value) : m_Value(std::move(value)) {
        }

        Atomic(const T& value) : m_Value(value) {
        }

        Atomic& operator=(auto&& value) {
            this->GetProxy().Set(std::forward<decltype(value)>(value));
            return *this;
        }

        struct Proxy {
            Atomic* m_Atomic;
            Proxy(Atomic& atomic) : m_Atomic(&atomic) {
            }

            T& operator*() {
                return m_Atomic->m_Value;
            }

            ~Proxy() {
                m_Atomic->m_Mutex.unlock();
            }

            void Set(const T& value) {
                m_Atomic->m_Value = value;
            }

            void Set(T&& value) {
                m_Atomic->m_Value = std::move(value);
            }

            T& Get() {
                return m_Atomic->m_Value;
            }

            T* operator->() {
                return &m_Atomic->m_Value;
            }
        };

        Proxy GetProxy() {
            m_Mutex.lock();
            return Proxy(*this);
        }

        std::optional<Proxy> TryGetProxy() {
            if (m_Mutex.try_lock()) {
                return Proxy(*this);
            }
            return std::nullopt;
        }

    private:
        std::mutex m_Mutex{};
        T m_Value;
    };

    export template<typename T, typename ...Args>
    Atomic<T> MakeAtomic(Args&&... args) {
        return Atomic<T>{std::forward<Args>(args)...};
    }

    export template<typename T>
    class SharedAtomic;

    export template<typename T>
    class WeakAtomic;

    template<typename T>
    class SharedAtomic {
    public:
        SharedAtomic() = default;
        SharedAtomic(const SharedAtomic&) = default;
        SharedAtomic(SharedAtomic&&) = default;

        SharedAtomic(const std::shared_ptr<Atomic<T>>& atomic) : m_Atomic(atomic) {
        }

        SharedAtomic(std::shared_ptr<Atomic<T>>&& atomic) : m_Atomic(std::move(atomic)) {
        }

        explicit operator bool() const {
            return m_Atomic != nullptr;
        }

        bool HasValue() const {
            return m_Atomic != nullptr;
        }

        WeakAtomic<T> ShareWeak() const {
            return {m_Atomic};
        }

        Atomic<T>::Proxy GetProxy() {
            if (!m_Atomic) {
                throw std::runtime_error("SharedAtomic is empty");
            }
            return m_Atomic->GetProxy();
        }

        std::optional<typename Atomic<T>::Proxy> TryGetProxy() {
            if (!m_Atomic) {
                return std::nullopt;
            }
            return m_Atomic->TryGetProxy();
        }

    private:
        std::shared_ptr<Atomic<T>> m_Atomic;
    };

    export template<typename T, typename ...Args>
    SharedAtomic<T> MakeSharedAtomic(Args&&... args) {
        return SharedAtomic<T>{std::make_shared<Atomic<T>>(std::forward<Args>(args)...)};
    }

    template<typename T>
    class WeakAtomic {
    public:
        WeakAtomic() = default;
        WeakAtomic(const WeakAtomic&) = default;
        WeakAtomic(WeakAtomic&&) = default;

        WeakAtomic(const std::weak_ptr<Atomic<T>>& atomic) : m_Atomic(atomic) {
        }

        WeakAtomic(std::weak_ptr<Atomic<T>>&& atomic) : m_Atomic(std::move(atomic)) {
        }

        SharedAtomic<T> Lock() const {
            return {m_Atomic.lock()};
        }

    private:
        std::weak_ptr<Atomic<T>> m_Atomic;
    };
}