export module EasyGui.Utils.Atomic;

import std.compat;

export import EasyGui.Utils.Flags;
import EasyGui.Tools.ThreadPool;
import EasyGui.std_extensions;

namespace EasyGui {
    export class SpinLock {
    public:
        void lock() {
            bool expected = false;

            if (!m_Flag.compare_exchange_strong(expected, true, std::memory_order_acquire)) {
                expected = false;
                std::this_thread::yield();
            } else {
                m_Flag.store(true, std::memory_order_relaxed);
                return;
            }

            while (!m_Flag.compare_exchange_strong(expected, true, std::memory_order_relaxed)) {
                expected = false;
                std::this_thread::yield();
            }
            m_Flag.store(true, std::memory_order_relaxed);
        }

        void Lock() {
            lock();
        }

        void unlock() {
            m_Flag.store(false, std::memory_order_release);
        }

        void Unlock() {
            unlock();
        }

    private:
        std::atomic_bool m_Flag{};
    };

    export class RecursiveSpinLock {
    public:
        void lock() {
            std::thread::id this_id = std::this_thread::get_id();
            if (m_Owner == this_id) {
                ++m_RecursionCount;
                return;
            }

            m_Mutex.lock();
            m_Owner = this_id;
            m_RecursionCount = 1;
        }

        void Lock() {
            lock();
        }

        void unlock() {
            if (m_Owner != std::this_thread::get_id()) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
            }
            if (--m_RecursionCount == 0) {
                m_Owner = {};
                m_Mutex.unlock();
            }
        }

        void Unlock() {
            unlock();
        }

    private:
        std::thread::id m_Owner{};
        size_t m_RecursionCount{};
        SpinLock m_Mutex{};
    };

    export template<typename T, typename LockType = SpinLock>
    class Atomic {
    public:
        Atomic() = default;

        Atomic(const Atomic &) = delete;

        Atomic(Atomic &&) = delete;

        Atomic(T &&value) : m_Value(std::move(value)) {
        }

        Atomic(const T &value) : m_Value(value) {
        }

        Atomic &operator=(auto &&value) {
            this->GetProxy().Set(std::forward<decltype(value)>(value));
            return *this;
        }

        struct Proxy {
            Atomic *m_Atomic;

            Proxy(Atomic &atomic) : m_Atomic(&atomic) {
            }

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
        LockType m_Mutex{};
        T m_Value;
    };

    export template<typename T, typename LockType = SpinLock, typename... Args>
    Atomic<T, LockType> MakeAtomic(Args &&... args) {
        return Atomic<T, LockType>{std::forward<Args>(args)...};
    }

    export template<typename T, typename LockType = SpinLock>
    class SharedAtomic;

    export template<typename T, typename LockType = SpinLock>
    class WeakAtomic;

    template<typename T, typename LockType>
    class SharedAtomic {
    public:
        SharedAtomic() = default;

        SharedAtomic(const SharedAtomic &) = default;

        SharedAtomic(SharedAtomic &&) = default;

        SharedAtomic(const std::shared_ptr<Atomic<T, LockType>> &atomic) : m_Atomic(atomic) {
        }

        SharedAtomic(std::shared_ptr<Atomic<T, LockType>> &&atomic) : m_Atomic(std::move(atomic)) {
        }

        explicit operator bool() const {
            return m_Atomic != nullptr;
        }

        bool HasValue() const {
            return m_Atomic != nullptr;
        }

        WeakAtomic<T, LockType> ShareWeak() const {
            return {m_Atomic};
        }

        Atomic<T, LockType>::Proxy GetProxy() {
            if (!m_Atomic) {
                throw std::runtime_error("SharedAtomic is empty");
            }
            return m_Atomic->GetProxy();
        }

        std::optional<typename Atomic<T, LockType>::Proxy> TryGetProxy() {
            if (!m_Atomic) {
                return std::nullopt;
            }
            return m_Atomic->TryGetProxy();
        }

    private:
        std::shared_ptr<Atomic<T, LockType>> m_Atomic;
    };

    export template<typename T, typename LockType, typename... Args>
    SharedAtomic<T, LockType> MakeSharedAtomic(Args &&... args) {
        return SharedAtomic<T, LockType>{std::make_shared<Atomic<T, LockType>>(std::forward<Args>(args)...)};
    }

    template<typename T, typename LockType>
    class WeakAtomic {
    public:
        WeakAtomic() = default;

        WeakAtomic(const WeakAtomic &) = default;

        WeakAtomic(WeakAtomic &&) = default;

        WeakAtomic(const std::weak_ptr<Atomic<T>> &atomic) : m_Atomic(atomic) {
        }

        WeakAtomic(std::weak_ptr<Atomic<T>> &&atomic) : m_Atomic(std::move(atomic)) {
        }

        SharedAtomic<T, LockType> Lock() const {
            return {m_Atomic.lock()};
        }

    private:
        std::weak_ptr<Atomic<T, LockType>> m_Atomic;
    };

    export template<typename T, typename LockType = SpinLock>
    class ARCMutex;

    export template<typename T, typename LockType = SpinLock>
    class WeakMutex;

    template<typename T, typename LockType>
    struct MutexPair {
        LockType Mutex{};
        T Value;

        explicit MutexPair(T &&value) : Value(std::move(value)) {
        }

        MutexPair() = default;
    };

    template<typename T, typename LockType>
    class ReadProxy {
    private:
        MutexPair<T, LockType> *m_Pair{};

    public:
        ReadProxy() = delete;

        ReadProxy(const ReadProxy &) = delete;

        ReadProxy(ReadProxy &&other) = delete;

        ReadProxy &operator=(const ReadProxy &) = delete;

        ReadProxy &operator=(ReadProxy &&other) = delete;

    public:
        ReadProxy(MutexPair<T, LockType> &pair) : m_Pair(&pair) {
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

        T *operator->() {
            return &Get();
        }

        const T *operator->() const {
            return &Get();
        }

        T &operator*() {
            return Get();
        }

        const T &operator*() const {
            return Get();
        }
    };

    template<typename T, typename LockType>
    class ARCMutex {
        friend class WeakMutex<T, LockType>;

    public:
        using value_type = T;
        using stored_type = std::shared_ptr<MutexPair<T, LockType>>;

        ARCMutex(DefaultConstructed) : m_Ptr(std::make_shared<MutexPair<T, LockType>>()) {
        }

        explicit ARCMutex(EmptyConstructed) : m_Ptr(nullptr) {
        }

        template<typename... Args>
        explicit ARCMutex(ForwardConstructorParameters, Args &&... args)
            : m_Ptr(std::make_shared<MutexPair<T, LockType>>(T{std::forward<Args>(args)...})) {
        }

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

        ReadProxy<T, LockType> Read() {
            return ReadProxy<T, LockType>{*m_Ptr};
        }

        WeakMutex<T, LockType> ShareWeak() const {
            return {m_Ptr};
        }

        bool HasValue() const {
            return m_Ptr != nullptr;
        }

        operator bool() const {
            return HasValue();
        }

        auto DeferredSet(Fn<T()> auto &&func, IThreadPool *pool = GlobalThreadPool()) {
            return pool->Enqueue([self = *this, provider = std::forward<decltype(func)>(func)]() mutable {
                self = provider();
            });
        }

    private:
        stored_type m_Ptr;
    };

    template<typename T, typename LockType>
    class WeakMutex {
        friend class ARCMutex<T, LockType>;

    public:
        using value_type = T;
        using stored_type = std::weak_ptr<MutexPair<T, LockType>>;

    public:
        WeakMutex() = default;

        WeakMutex(const WeakMutex &) = default;

        WeakMutex &operator=(const WeakMutex &) = default;

        ARCMutex<T, LockType> Lock() const {
            return {m_Ptr.lock()};
        }

    private:
        stored_type m_Ptr;
    };
}
