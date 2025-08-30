export module EasyGui.Utils.Atomic;

import std.compat;

export import EasyGui.Utils.Flags;
import EasyGui.Tools.ThreadPool;

namespace EasyGui {
    export template<typename T>
    class Atomic {
    public:
        Atomic() = default;

        Atomic(const Atomic &) = delete;

        Atomic(Atomic &&) = delete;

        Atomic(T &&value) : m_Value(std::move(value)) {}

        Atomic(const T &value) : m_Value(value) {}

        Atomic &operator=(auto &&value) {
            this->GetProxy().Set(std::forward<decltype(value)>(value));
            return *this;
        }

        struct Proxy {
            Atomic *m_Atomic;
            Proxy(Atomic &atomic) : m_Atomic(&atomic) {}

            T &operator*() {
                return m_Atomic->m_Value;
            }

            ~Proxy() {
                m_Atomic->m_Mutex.unlock();
            }

            void Set(const T &value) {
                m_Atomic->m_Value = value;
            }

            void Set(T &&value) {
                m_Atomic->m_Value = std::move(value);
            }

            T &Get() {
                return m_Atomic->m_Value;
            }

            T *operator->() {
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

    export template<typename T, typename... Args>
    Atomic<T> MakeAtomic(Args &&... args) {
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

        SharedAtomic(const SharedAtomic &) = default;

        SharedAtomic(SharedAtomic &&) = default;

        SharedAtomic(const std::shared_ptr<Atomic<T>> &atomic) : m_Atomic(atomic) {}

        SharedAtomic(std::shared_ptr<Atomic<T>> &&atomic) : m_Atomic(std::move(atomic)) {}

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

    export template<typename T, typename... Args>
    SharedAtomic<T> MakeSharedAtomic(Args &&... args) {
        return SharedAtomic<T>{std::make_shared<Atomic<T>>(std::forward<Args>(args)...)};
    }

    template<typename T>
    class WeakAtomic {
    public:
        WeakAtomic() = default;

        WeakAtomic(const WeakAtomic &) = default;

        WeakAtomic(WeakAtomic &&) = default;

        WeakAtomic(const std::weak_ptr<Atomic<T>> &atomic) : m_Atomic(atomic) {}

        WeakAtomic(std::weak_ptr<Atomic<T>> &&atomic) : m_Atomic(std::move(atomic)) {}

        SharedAtomic<T> Lock() const {
            return {m_Atomic.lock()};
        }

    private:
        std::weak_ptr<Atomic<T>> m_Atomic;
    };

    export template<typename T>
    class ARCMutex;

    export template<typename T>
    class WeakMutex;

    template<typename T>
    struct MutexPair {
        std::mutex Mutex{};
        T Value;

        explicit MutexPair(T &&value) : Value(std::move(value)) {}

        MutexPair() = default;
    };

    template<typename T>
    class ReadProxy {
    private:
        MutexPair<T> *m_Pair{};

    public:
        ReadProxy() = delete;

        ReadProxy(const ReadProxy &) = delete;

        ReadProxy(ReadProxy &&other) = delete;

        ReadProxy &operator=(const ReadProxy &) = delete;

        ReadProxy &operator=(ReadProxy &&other) = delete;

    public:
        ReadProxy(MutexPair<T> &pair) : m_Pair(&pair) {
            m_Pair->Mutex.lock();
        }

        ~ReadProxy() {
            m_Pair->Mutex.unlock();
        }

    public:
        T &Get() {
            return m_Pair->Value;
        }

        const T &Get() const {
            return m_Pair->Value;
        }

        operator T &() {
            return Get();
        }

        operator const T &() const {
            return Get();
        }

        T* operator->() {
            return &Get();
        }

        const T* operator->() const {
            return &Get();
        }

        T& operator*() {
            return Get();
        }

        const T& operator*() const {
            return Get();
        }
    };

    template<typename T>
    class ARCMutex {
        friend class WeakMutex<T>;

    public:
        using value_type = T;
        using stored_type = std::shared_ptr<MutexPair<T>>;

        ARCMutex(DefaultConstructed) : m_Ptr(std::make_shared<MutexPair<T>>()) {}
        explicit ARCMutex(EmptyConstructed) : m_Ptr(nullptr) {}

        template<typename... Args>
        explicit ARCMutex(ForwardConstructorParameters, Args &&... args)
            : m_Ptr(std::make_shared<MutexPair<T>>(T{std::forward<Args>(args)...})) {}

    public:
        ARCMutex(const ARCMutex &) = default;

        ARCMutex &operator=(const ARCMutex &) = default;

    public:
        ARCMutex &operator=(auto &&other) {
            auto &[mutex, value] = *m_Ptr;
            std::lock_guard lock(mutex);
            value = std::forward<decltype(other)>(other);
            return *this;
        }

        template<typename Func>
        decltype(auto) Let(Func &&func) {
            auto &[mutex, value] = *m_Ptr;
            std::lock_guard lock(mutex);
            return func(value);
        }

        ReadProxy<T> Read() {
            return ReadProxy<T>{*m_Ptr};
        }

        WeakMutex<T> ShareWeak() const {
            return {m_Ptr};
        }

        bool HasValue() const {
            return m_Ptr != nullptr;
        }

        operator bool() const {
            return HasValue();
        }

        template<typename Func>
        auto DeferredSet(Func &&func, IThreadPool *pool = GlobalThreadPool()) {
            return pool->Enqueue([self = *this, provider = std::forward<Func>(func)]() mutable {
                self = provider();
            });
        }

    private:
        stored_type m_Ptr;
    };

    template<typename T>
    class WeakMutex {
        friend class ARCMutex<T>;

    public:
        using value_type = T;
        using stored_type = std::weak_ptr<MutexPair<T>>;

    public:
        WeakMutex() = default;

        WeakMutex(const WeakMutex &) = default;

        WeakMutex &operator=(const WeakMutex &) = default;

        ARCMutex<T> Lock() const {
            return {m_Ptr.lock()};
        }

    private:
        stored_type m_Ptr;
    };
}
