// File License: CC0 1.0 Universal
//
// Adds RAII wrapper to VulkanMemoryAllocator-Hpp similar to vk::raii from <vulkan/vulkan_raii.hpp> (Vulkan-Headers).
// - https://github.com/YaaZ/VulkanMemoryAllocator-Hpp
// - https://github.com/KhronosGroup/Vulkan-Headers/blob/v1.3.301/include/vulkan/vulkan_raii.hpp
//
// UNTESTED, use at your own risk.

#ifndef VULKAN_MEMORY_ALLOCATOR_RAII_HPP
#define VULKAN_MEMORY_ALLOCATOR_RAII_HPP

//#if !defined(VULKAN_MEMORY_ALLOCATOR_HPP)
#include "vk_mem_alloc.hpp"
//#endif

//#if !defined(VULKAN_RAII_HPP)
#include <vulkan/vulkan_raii.hpp>
//#endif

#if !defined(VMA_RAII_HPP_NAMESPACE)
#define VMA_RAII_HPP_NAMESPACE raii
#endif

#if !defined( VULKAN_HPP_DISABLE_ENHANCED_MODE )
namespace
VMA_HPP_NAMESPACE {
    namespace
    VMA_RAII_HPP_NAMESPACE {
        namespace details {
            template<typename T>
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<T>::type
            createRaiiResult(auto&& device, auto &&result) {
                if constexpr (requires {
                    result.value;
                    result.result;
                }) {
                    return {
                        std::move(result.result),
                        T(device, std::move(result.value)),
                    };
                } else {
                    return T(device, std::move(result));
                }
            }

            auto getResultValue(auto &&result) {
                if constexpr (requires { result.value; }) {
                    if (result.result != VULKAN_HPP_NAMESPACE::Result::eSuccess) {
                        throw std::runtime_error(std::format(
                            "VMA operation failed with result: {}",
                            VULKAN_HPP_NAMESPACE::to_string(result.result)
                        ));
                    }
                    return std::move(result.value);
                } else {
                    return std::move(result);
                }
            }

            template<typename T>
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<T>::type
            emplaceResult(auto &&result, auto &&... args) {
                if constexpr (requires {
                    result.value;
                    result.result;
                }) {
                    return {
                        std::move(result.result),
                        T(std::forward<decltype(args)>(args)..., std::move(result.value)),
                    };
                } else {
                    return T(std::forward<decltype(args)>(args)..., std::move(result));
                }
            }
        }

        struct Pool;
        struct Allocation;
        struct StatsString;
        struct AllocatedBuffer;
        struct AllocatedImage;
        struct Defragmentation;
        struct Allocator;

        struct Pool {
        private:
            VMA_HPP_NAMESPACE::Allocator m_allocator;
            VMA_HPP_NAMESPACE::Pool m_pool;

        public:
            Pool() : m_allocator(nullptr), m_pool(nullptr) {}

            Pool(
                VMA_HPP_NAMESPACE::Allocator allocator,
                VMA_HPP_NAMESPACE::Pool pool
            ) : m_allocator(allocator), m_pool(pool) {}

            Pool(
                VMA_HPP_NAMESPACE::Allocator allocator,
                const VMA_HPP_NAMESPACE::PoolCreateInfo &createInfo
            ) : m_allocator(allocator), m_pool(details::getResultValue(m_allocator.createPool(createInfo))) {}

            // Pool(
            //     VMA_HPP_NAMESPACE::Allocator allocator,
            //     VMA_HPP_NAMESPACE::Pool&& pool
            // ) : m_allocator(allocator), m_pool(std::move(pool)) {}

            // create pool function
            static VULKAN_HPP_NAMESPACE::ResultValueType<Pool>::type create(
                VMA_HPP_NAMESPACE::Allocator allocator,
                const VMA_HPP_NAMESPACE::PoolCreateInfo &createInfo
            ) {
                auto res = allocator.createPool(createInfo);
                return details::emplaceResult<Pool>(std::move(res), allocator);
            }

            ~Pool() {
                if (m_pool) {
                    m_allocator.destroyPool(m_pool);
                }

                m_allocator = nullptr;
                m_pool = nullptr;
            }

            // No copy
            Pool(Pool &) = delete;

            Pool &operator=(Pool &) = delete;

            // Move
            Pool(Pool &&other) noexcept : Pool() {
                swap(other);
            }

            Pool &operator=(Pool &&other) noexcept {
                Pool newObj{std::move(other)};
                swap(newObj);
                return *this;
            }

            void swap(Pool &other) noexcept {
                std::ranges::swap(m_allocator, other.m_allocator);
                std::ranges::swap(m_pool, other.m_pool);
            }

        public:
            VMA_HPP_NAMESPACE::Allocator allocator() const { return m_allocator; }

        public:
            VULKAN_HPP_TYPESAFE_EXPLICIT operator VmaPool() const VULKAN_HPP_NOEXCEPT {
                return m_pool;
            }

            VULKAN_HPP_TYPESAFE_EXPLICIT operator vma::Pool() const VULKAN_HPP_NOEXCEPT {
                return m_pool;
            }

        public:
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VMA_HPP_NAMESPACE::Statistics getStatistics() const {
                return m_allocator.getPoolStatistics(m_pool);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VMA_HPP_NAMESPACE::DetailedStatistics calculateStatistics() const {
                return m_allocator.calculatePoolStatistics(m_pool);
            }

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type checkCorruption() const {
                return m_allocator.checkPoolCorruption(m_pool);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS const char *getName() const {
                return m_allocator.getPoolName(m_pool);
            }

            void setName(
                const char *name
            ) const {
                m_allocator.setPoolName(m_pool, name);
            }

            void setName(
                const std::string &name
            ) const {
                setName(name.c_str());
            }
        };

        struct Allocation {
        private:
            VMA_HPP_NAMESPACE::Allocator m_allocator;
            VMA_HPP_NAMESPACE::Allocation m_allocation;

        public:
            Allocation() : m_allocator(nullptr), m_allocation(nullptr) {}

            Allocation(
                VMA_HPP_NAMESPACE::Allocator allocator,
                VMA_HPP_NAMESPACE::Allocation allocation
            ) : m_allocator(allocator), m_allocation(allocation) {}

            Allocation(
                VMA_HPP_NAMESPACE::Allocator allocator,
                const VULKAN_HPP_NAMESPACE::MemoryRequirements &vkMemoryRequirements,
                const VMA_HPP_NAMESPACE::AllocationCreateInfo &createInfo,
                VULKAN_HPP_NAMESPACE::Optional<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo = nullptr
            ) : m_allocator(allocator),
                m_allocation (details::getResultValue(m_allocator.allocateMemory(vkMemoryRequirements, createInfo, allocationInfo))) {}

            // Allocation(
            //     VMA_HPP_NAMESPACE::Allocator allocator,
            //     VMA_HPP_NAMESPACE::Allocation &&allocation
            // ) : m_allocator(allocator), m_allocation(std::move(allocation)) {}

            static VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type create(
                VMA_HPP_NAMESPACE::Allocator allocator,
                const VULKAN_HPP_NAMESPACE::MemoryRequirements &vkMemoryRequirements,
                const VMA_HPP_NAMESPACE::AllocationCreateInfo &createInfo,
                VULKAN_HPP_NAMESPACE::Optional<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo = nullptr
            ) {
                auto res = allocator.allocateMemory(vkMemoryRequirements, createInfo, allocationInfo);
                return details::emplaceResult<Allocation>(std::move(res), allocator);
            }

            ~Allocation() {
                if (m_allocation) {
                    m_allocator.freeMemory(m_allocation);
                }

                m_allocator = nullptr;
                m_allocation = nullptr;
            }

            // No copy
            Allocation(Allocation &) = delete;

            Allocation &operator=(Allocation &) = delete;

            // Move
            Allocation(Allocation &&other) noexcept : Allocation() {
                swap(other);
            }

            Allocation &operator=(Allocation &&other) noexcept {
                Allocation newObj{std::move(other)};
                swap(newObj);
                return *this;
            }

            void swap(Allocation &other) noexcept {
                std::ranges::swap(m_allocator, other.m_allocator);
                std::ranges::swap(m_allocation, other.m_allocation);
            }

        public:
            VMA_HPP_NAMESPACE::Allocator allocator() const { return m_allocator; }

        public:
            VULKAN_HPP_TYPESAFE_EXPLICIT operator VmaAllocation() const VULKAN_HPP_NOEXCEPT {
                return m_allocation;
            }

            VULKAN_HPP_TYPESAFE_EXPLICIT operator VMA_HPP_NAMESPACE::Allocation() const VULKAN_HPP_NOEXCEPT {
                return m_allocation;
            }

        public:
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VMA_HPP_NAMESPACE::AllocationInfo getInfo() const {
                return m_allocator.getAllocationInfo(m_allocation);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VMA_HPP_NAMESPACE::AllocationInfo2 getInfo2() const {
                return m_allocator.getAllocationInfo2(m_allocation);
            }

            void setUserData(void *userData) const {
                m_allocator.setAllocationUserData(m_allocation, userData);
            }

            void setName(const char *name) const {
                m_allocator.setAllocationName(m_allocation, name);
            }

            void setName(const std::string &name) const {
                setName(name.c_str());
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_NAMESPACE::MemoryPropertyFlags
            getMemoryProperties() const {
                return m_allocator.getAllocationMemoryProperties(m_allocation);
            }

#pragma region Memory Mapping
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void *>::type
            map() const {
                return m_allocator.mapMemory(m_allocation);
            }

            void unmap() const {
                m_allocator.unmapMemory(m_allocation);
            }

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type flush(
                Allocation allocation,
                VULKAN_HPP_NAMESPACE::DeviceSize offset = 0,
                VULKAN_HPP_NAMESPACE::DeviceSize size = VK_WHOLE_SIZE
            ) const {
                return m_allocator.flushAllocation(m_allocation, offset, size);
            }

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type invalidate(
                Allocation allocation,
                VULKAN_HPP_NAMESPACE::DeviceSize offset = 0,
                VULKAN_HPP_NAMESPACE::DeviceSize size = VK_WHOLE_SIZE
            ) const {
                return m_allocator.invalidateAllocation(m_allocation, offset, size);
            }
#pragma endregion

#pragma region Copy To/From Memory
            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type copyMemoryToAllocation(
                const void *srcHostPointer,
                VULKAN_HPP_NAMESPACE::DeviceSize dstAllocationLocalOffset,
                VULKAN_HPP_NAMESPACE::DeviceSize size
            ) const {
                return m_allocator.copyMemoryToAllocation(srcHostPointer, m_allocation, dstAllocationLocalOffset, size);
            }

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type copyAllocationToMemory(
                VULKAN_HPP_NAMESPACE::DeviceSize srcAllocationLocalOffset,
                void *dstHostPointer,
                VULKAN_HPP_NAMESPACE::DeviceSize size
            ) const {
                return m_allocator.copyAllocationToMemory(m_allocation, srcAllocationLocalOffset, dstHostPointer, size);
            }
#pragma endregion

#pragma region Buffer
            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindBufferMemory(
                VULKAN_HPP_NAMESPACE::Buffer buffer) const {
                return m_allocator.bindBufferMemory(m_allocation, buffer);
            }

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindBufferMemory2(
                VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                VULKAN_HPP_NAMESPACE::Buffer buffer,
                const void *next
            ) const {
                return m_allocator.bindBufferMemory2(m_allocation, allocationLocalOffset, buffer, next);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>::type createAliasingBuffer(
                const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
                const VULKAN_HPP_NAMESPACE::BufferCreateInfo &bufferCreateInfo
            ) const {
                // VULKAN_HPP_NAMESPACE::Buffer buff = m_allocator.createAliasingBuffer(m_allocation, bufferCreateInfo);
                // return VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer(device, buff);
                return details::createRaiiResult<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>(
                    device,
                    m_allocator.createAliasingBuffer(m_allocation, bufferCreateInfo)
                );
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>::type createAliasingBuffer2(
                const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
                VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                const VULKAN_HPP_NAMESPACE::BufferCreateInfo &bufferCreateInfo
            ) const {
                // VULKAN_HPP_NAMESPACE::Buffer buff = m_allocator.createAliasingBuffer2(m_allocation, allocationLocalOffset, bufferCreateInfo);
                // return VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer(device, buff);
                return details::createRaiiResult<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>(
                    device,
                    m_allocator.createAliasingBuffer2(m_allocation, allocationLocalOffset, bufferCreateInfo)
                );
            }
#pragma endregion

#pragma region Image
            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindImageMemory(
                VULKAN_HPP_NAMESPACE::Image image) const {
                return m_allocator.bindImageMemory(m_allocation, image);
            }

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindImageMemory2(
                VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                VULKAN_HPP_NAMESPACE::Image image,
                const void *next
            ) const {
                return m_allocator.bindImageMemory2(m_allocation, allocationLocalOffset, image, next);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>::type createAliasingImage(
                const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
                const VULKAN_HPP_NAMESPACE::ImageCreateInfo &imageCreateInfo
            ) const {
                return details::createRaiiResult<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>(
                    device,
                    m_allocator.createAliasingImage(m_allocation, imageCreateInfo)
                );
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>::type createAliasingImage2(
                const VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device &device,
                VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                const VULKAN_HPP_NAMESPACE::ImageCreateInfo &imageCreateInfo
            ) const {
                // VULKAN_HPP_NAMESPACE::Image img = m_allocator.createAliasingImage2(m_allocation, allocationLocalOffset, imageCreateInfo);
                // return VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image(device, img);
                return details::createRaiiResult<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>(
                    device,
                    m_allocator.createAliasingImage2(m_allocation, allocationLocalOffset, imageCreateInfo)
                );
            }
#pragma endregion
        };

        struct StatsString {
        private:
            VMA_HPP_NAMESPACE::Allocator m_allocator;
            char *m_str;

        public:
            StatsString() : m_allocator(nullptr), m_str(nullptr) {}

            StatsString(
                VMA_HPP_NAMESPACE::Allocator allocator,
                char *str
            ) : m_allocator(allocator), m_str(str) {}

            StatsString(
                VMA_HPP_NAMESPACE::Allocator allocator,
                bool detailedMap
            ) : m_allocator(allocator), m_str(allocator.buildStatsString(detailedMap)) {}

            ~StatsString() {
                if (m_allocator && m_str)
                    m_allocator.freeStatsString(m_str);

                m_allocator = nullptr;
                m_str = nullptr;
            }

            // No copy
            StatsString(StatsString &) = delete;

            StatsString &operator=(StatsString &) = delete;

            // Move
            StatsString(StatsString &&other) noexcept : StatsString() {
                swap(other);
            }

            StatsString &operator=(StatsString &&other) noexcept {
                StatsString newObj{std::move(other)};
                swap(newObj);
                return *this;
            }

            void swap(StatsString &other) noexcept {
                std::ranges::swap(m_allocator, other.m_allocator);
                std::ranges::swap(m_str, other.m_str);
            }

        public:
            VMA_HPP_NAMESPACE::Allocator allocator() const { return m_allocator; }

        public:
            char *c_str() const { return m_str; }
            operator std::string() const { return m_str; }
        };

        struct AllocatedBuffer {
        private:
            VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::detail::DeviceDispatcher const *m_dispatcher;
            VULKAN_HPP_NAMESPACE::Device m_device;

            VMA_HPP_NAMESPACE::Allocator m_allocator;
            VULKAN_HPP_NAMESPACE::Buffer m_buffer;
            VMA_HPP_NAMESPACE::Allocation m_allocation;

        public:
            AllocatedBuffer() : m_dispatcher(nullptr), m_device(nullptr), m_allocator(nullptr), m_buffer(nullptr),
                                m_allocation(nullptr) {}

            AllocatedBuffer(
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device const &device,
                VMA_HPP_NAMESPACE::Allocator allocator,
                VULKAN_HPP_NAMESPACE::Buffer buffer,
                VMA_HPP_NAMESPACE::Allocation allocation
            ) : m_dispatcher(device.getDispatcher()), m_device(device), m_allocator(allocator), m_buffer(buffer),
                m_allocation(allocation) {
                assert(m_dispatcher);
                assert(m_device);

                assert(m_buffer);
                assert(m_allocation);
            }

            AllocatedBuffer(
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device const &device,
                VMA_HPP_NAMESPACE::Allocator allocator,
                const VULKAN_HPP_NAMESPACE::BufferCreateInfo &bufferCreateInfo,
                const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo,
                VULKAN_HPP_NAMESPACE::Optional<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo = nullptr
            ) : m_dispatcher(device.getDispatcher()), m_device(device), m_allocator(allocator), m_buffer(nullptr),
                m_allocation(nullptr) {
                auto buff = details::getResultValue(
                    m_allocator.createBuffer(bufferCreateInfo, allocationCreateInfo, allocationInfo));
                m_buffer = buff.first;
                m_allocation = buff.second;

                assert(m_dispatcher);
                assert(m_device);

                assert(m_buffer);
                assert(m_allocation);
            }

            AllocatedBuffer(
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device const &device,
                VMA_HPP_NAMESPACE::Allocator allocator,
                const VULKAN_HPP_NAMESPACE::BufferCreateInfo &bufferCreateInfo,
                const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo,
                VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                VULKAN_HPP_NAMESPACE::Optional<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo = nullptr
            ) : m_dispatcher(device.getDispatcher()), m_device(device), m_allocator(allocator), m_buffer(nullptr),
                m_allocation(nullptr) {
                auto buff = details::getResultValue(
                    m_allocator.createBufferWithAlignment(bufferCreateInfo, allocationCreateInfo, minAlignment,
                                                          allocationInfo));
                m_buffer = buff.first;
                m_allocation = buff.second;

                assert(m_dispatcher);
                assert(m_device);

                assert(m_buffer);
                assert(m_allocation);
            }

            ~AllocatedBuffer() {
                if (m_allocator) {
                    assert(m_buffer);
                    assert(m_allocation);
                    m_allocator.destroyBuffer(m_buffer, m_allocation);
                }

                m_dispatcher = nullptr;
                m_device = nullptr;

                m_allocator = nullptr;
                m_buffer = nullptr;
                m_allocation = nullptr;
            }

            // No copy
            AllocatedBuffer(AllocatedBuffer &) = delete;

            AllocatedBuffer &operator=(AllocatedBuffer &) = delete;

            // Move
            AllocatedBuffer(AllocatedBuffer &&other) noexcept : AllocatedBuffer() {
                swap(other);
            }

            AllocatedBuffer &operator=(AllocatedBuffer &&other) noexcept {
                AllocatedBuffer newObj{std::move(other)};
                swap(newObj);
                return *this;
            }

            void swap(AllocatedBuffer &other) noexcept {
                std::ranges::swap(m_dispatcher, other.m_dispatcher);
                std::ranges::swap(m_device, other.m_device);

                std::ranges::swap(m_allocator, other.m_allocator);
                std::ranges::swap(m_buffer, other.m_buffer);
                std::ranges::swap(m_allocation, other.m_allocation);
            }

        public:
            operator VkBuffer() const { return m_buffer; }
            operator VULKAN_HPP_NAMESPACE::Buffer() const { return m_buffer; }

            operator VmaAllocation() const { return m_allocation; }
            operator VMA_HPP_NAMESPACE::Allocation() const { return m_allocation; }

            VULKAN_HPP_NAMESPACE::Device const &getDevice() const { return m_device; }
            VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::detail::DeviceDispatcher const *getDispatcher() const {
                return m_dispatcher;
            }

#pragma region Buffer

        public:
            //=== VK_VERSION_1_0 ===

            //void bindMemory( VULKAN_HPP_NAMESPACE::DeviceMemory memory, VULKAN_HPP_NAMESPACE::DeviceSize memoryOffset ) const;

            VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::MemoryRequirements
            getMemoryRequirements() const VULKAN_HPP_NOEXCEPT {
                return m_device.getBufferMemoryRequirements(m_buffer, *m_dispatcher);
            }
#pragma endregion

#pragma region Allocation

        public:
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VMA_HPP_NAMESPACE::AllocationInfo getInfo() const {
                return m_allocator.getAllocationInfo(m_allocation);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VMA_HPP_NAMESPACE::AllocationInfo2 getInfo2() const {
                return m_allocator.getAllocationInfo2(m_allocation);
            }

            void setUserData(void *userData) const {
                m_allocator.setAllocationUserData(m_allocation, userData);
            }

            void setName(const char *name) const {
                m_allocator.setAllocationName(m_allocation, name);
            }

            void setName(const std::string &name) const {
                setName(name.c_str());
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_NAMESPACE::MemoryPropertyFlags
            getMemoryProperties() const {
                return m_allocator.getAllocationMemoryProperties(m_allocation);
            }

#pragma region Memory Mapping
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void *>::type
            map() const {
                return m_allocator.mapMemory(m_allocation);
            }

            void unmap() const {
                m_allocator.unmapMemory(m_allocation);
            }

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type flush(
                Allocation allocation,
                VULKAN_HPP_NAMESPACE::DeviceSize offset = 0,
                VULKAN_HPP_NAMESPACE::DeviceSize size = VK_WHOLE_SIZE
            ) const {
                return m_allocator.flushAllocation(m_allocation, offset, size);
            }

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type invalidate(
                Allocation allocation,
                VULKAN_HPP_NAMESPACE::DeviceSize offset = 0,
                VULKAN_HPP_NAMESPACE::DeviceSize size = VK_WHOLE_SIZE
            ) const {
                return m_allocator.invalidateAllocation(m_allocation, offset, size);
            }
#pragma endregion
#pragma endregion
        };

        struct AllocatedImage {
        private:
            VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::detail::DeviceDispatcher const *m_dispatcher;
            VULKAN_HPP_NAMESPACE::Device m_device;

            VMA_HPP_NAMESPACE::Allocator m_allocator;
            VULKAN_HPP_NAMESPACE::Image m_image;
            VMA_HPP_NAMESPACE::Allocation m_allocation;

        public:
            AllocatedImage() : m_dispatcher(nullptr), m_device(nullptr), m_allocator(nullptr), m_image(nullptr),
                               m_allocation(nullptr) {}

            AllocatedImage(
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device const &device,
                VMA_HPP_NAMESPACE::Allocator allocator,
                VULKAN_HPP_NAMESPACE::Image image,
                VMA_HPP_NAMESPACE::Allocation allocation
            ) : m_dispatcher(device.getDispatcher()), m_device(device), m_allocator(allocator), m_image(image),
                m_allocation(allocation) {
                assert(m_dispatcher);
                assert(m_device);

                assert(m_allocator);
                assert(m_image);
                assert(m_allocation);
            }

            AllocatedImage(
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device const &device,
                VMA_HPP_NAMESPACE::Allocator allocator,
                const VULKAN_HPP_NAMESPACE::ImageCreateInfo &imageCreateInfo,
                const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo,
                VULKAN_HPP_NAMESPACE::Optional<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo = nullptr
            ) : m_dispatcher(device.getDispatcher()), m_device(device), m_allocator(allocator), m_image(nullptr),
                m_allocation(nullptr) {
                assert(m_dispatcher);
                assert(m_device); {
                    assert(m_allocator);

                    auto buff = details::getResultValue(
                        m_allocator.createImage(imageCreateInfo, allocationCreateInfo, allocationInfo));
                    m_image = buff.first;
                    m_allocation = buff.second;

                    assert(m_image);
                    assert(m_allocation);
                }
            }

            ~AllocatedImage() {
                if (m_allocator) {
                    assert(m_image);
                    assert(m_allocation);
                    m_allocator.destroyImage(m_image, m_allocation);
                }

                m_dispatcher = nullptr;
                m_device = nullptr;

                m_allocator = nullptr;
                m_image = nullptr;
                m_allocation = nullptr;
            }

            // No copy
            AllocatedImage(AllocatedImage &) = delete;

            AllocatedImage &operator=(AllocatedImage &) = delete;

            // Move
            AllocatedImage(AllocatedImage &&other) noexcept : AllocatedImage() {
                swap(other);
            }

            AllocatedImage &operator=(AllocatedImage &&other) noexcept {
                AllocatedImage newObj{std::move(other)};
                swap(newObj);
                return *this;
            }

            void swap(AllocatedImage &other) noexcept {
                std::ranges::swap(m_dispatcher, other.m_dispatcher);
                std::ranges::swap(m_device, other.m_device);

                std::ranges::swap(m_allocator, other.m_allocator);
                std::ranges::swap(m_image, other.m_image);
                std::ranges::swap(m_allocation, other.m_allocation);
            }

        public:
            operator VkImage() const { return m_image; }
            operator VULKAN_HPP_NAMESPACE::Image() const { return m_image; }

            operator VmaAllocation() const { return m_allocation; }
            operator VMA_HPP_NAMESPACE::Allocation() const { return m_allocation; }

            VULKAN_HPP_NAMESPACE::Device const &getDevice() const { return m_device; }
            VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::detail::DeviceDispatcher const *getDispatcher() const {
                return m_dispatcher;
            }

#pragma region Image

        public:
            //=== VK_VERSION_1_0 ===

            //void bindMemory( VULKAN_HPP_NAMESPACE::DeviceMemory memory, VULKAN_HPP_NAMESPACE::DeviceSize memoryOffset ) const;

            VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::MemoryRequirements
            getMemoryRequirements() const VULKAN_HPP_NOEXCEPT {
                return m_device.getImageMemoryRequirements(m_image, *m_dispatcher);
            }

            VULKAN_HPP_NODISCARD std::vector<VULKAN_HPP_NAMESPACE::SparseImageMemoryRequirements>
            getSparseMemoryRequirements() const {
                return m_device.getImageSparseMemoryRequirements(m_image, *m_dispatcher);
            }

            VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::SubresourceLayout getSubresourceLayout(
                const VULKAN_HPP_NAMESPACE::ImageSubresource &subresource) const VULKAN_HPP_NOEXCEPT {
                return m_device.getImageSubresourceLayout(m_image, subresource, *m_dispatcher);
            }

            //=== VK_EXT_image_drm_format_modifier ===

            VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ImageDrmFormatModifierPropertiesEXT
            getDrmFormatModifierPropertiesEXT() const {
                return details::getResultValue(m_device.getImageDrmFormatModifierPropertiesEXT(m_image, *m_dispatcher));
            }

            //=== VK_EXT_host_image_copy ===

            VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::SubresourceLayout2KHR getSubresourceLayout2EXT(
                const VULKAN_HPP_NAMESPACE::ImageSubresource2KHR &subresource) const VULKAN_HPP_NOEXCEPT {
                return m_device.getImageSubresourceLayout2EXT(m_image, subresource, *m_dispatcher);
            }

            template<typename X, typename Y, typename... Z>
            VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::StructureChain<X, Y, Z...> getSubresourceLayout2EXT(
                const VULKAN_HPP_NAMESPACE::ImageSubresource2KHR &subresource) const VULKAN_HPP_NOEXCEPT {
                return m_device.getImageSubresourceLayout2EXT<X, Y, Z...>(m_image, subresource, *m_dispatcher);
            }

            //=== VK_KHR_maintenance5 ===

            VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::SubresourceLayout2KHR getSubresourceLayout2KHR(
                const VULKAN_HPP_NAMESPACE::ImageSubresource2KHR &subresource) const VULKAN_HPP_NOEXCEPT {
                return m_device.getImageSubresourceLayout2KHR(m_image, subresource, *m_dispatcher);
            }

            template<typename X, typename Y, typename... Z>
            VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::StructureChain<X, Y, Z...> getSubresourceLayout2KHR(
                const VULKAN_HPP_NAMESPACE::ImageSubresource2KHR &subresource) const VULKAN_HPP_NOEXCEPT {
                return m_device.getImageSubresourceLayout2KHR<X, Y, Z...>(m_image, subresource, *m_dispatcher);
            }
#pragma endregion

#pragma region Allocation

        public:
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VMA_HPP_NAMESPACE::AllocationInfo getInfo() const {
                return m_allocator.getAllocationInfo(m_allocation);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VMA_HPP_NAMESPACE::AllocationInfo2 getInfo2() const {
                return m_allocator.getAllocationInfo2(m_allocation);
            }

            void setUserData(void *userData) const {
                m_allocator.setAllocationUserData(m_allocation, userData);
            }

            void setName(const char *name) const {
                m_allocator.setAllocationName(m_allocation, name);
            }

            void setName(const std::string &name) const {
                setName(name.c_str());
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_NAMESPACE::MemoryPropertyFlags
            getMemoryProperties() const {
                return m_allocator.getAllocationMemoryProperties(m_allocation);
            }

#pragma region Memory Mapping
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void *>::type
            map() const {
                return m_allocator.mapMemory(m_allocation);
            }

            void unmap() const {
                m_allocator.unmapMemory(m_allocation);
            }

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type flush(
                Allocation allocation,
                VULKAN_HPP_NAMESPACE::DeviceSize offset = 0,
                VULKAN_HPP_NAMESPACE::DeviceSize size = VK_WHOLE_SIZE
            ) const {
                return m_allocator.flushAllocation(m_allocation, offset, size);
            }

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type invalidate(
                Allocation allocation,
                VULKAN_HPP_NAMESPACE::DeviceSize offset = 0,
                VULKAN_HPP_NAMESPACE::DeviceSize size = VK_WHOLE_SIZE
            ) const {
                return m_allocator.invalidateAllocation(m_allocation, offset, size);
            }
#pragma endregion
#pragma endregion
        };

        struct Defragmentation {
        private:
            VMA_HPP_NAMESPACE::Allocator m_allocator;
            VMA_HPP_NAMESPACE::DefragmentationContext m_context;

        public:
            Defragmentation() : m_allocator(nullptr), m_context(nullptr) {}

            Defragmentation(
                VMA_HPP_NAMESPACE::Allocator allocator,
                VMA_HPP_NAMESPACE::DefragmentationContext context
            ) : m_allocator(allocator), m_context(context) {}

            Defragmentation(
                VMA_HPP_NAMESPACE::Allocator allocator,
                const DefragmentationInfo &info
            ) : m_allocator(allocator), m_context(details::getResultValue(allocator.beginDefragmentation(info))) {}

            ~Defragmentation() {
                if (m_allocator && m_context)
                    m_allocator.endDefragmentation(m_context);

                m_allocator = nullptr;
                m_context = nullptr;
            }

            // No copy
            Defragmentation(Defragmentation &) = delete;

            Defragmentation &operator=(Defragmentation &) = delete;

            // Move
            Defragmentation(Defragmentation &&other) noexcept : Defragmentation() {
                swap(other);
            }

            Defragmentation &operator=(Defragmentation &&other) noexcept {
                Defragmentation newObj{std::move(other)};
                swap(newObj);
                return *this;
            }

            void swap(Defragmentation &other) noexcept {
                std::ranges::swap(m_allocator, other.m_allocator);
                std::ranges::swap(m_context, other.m_context);
            }

        public:
            VMA_HPP_NAMESPACE::Allocator allocator() const { return m_allocator; }

        public:
            operator VmaDefragmentationContext() const { return m_context; }
            operator VMA_HPP_NAMESPACE::DefragmentationContext() const { return m_context; }

        public:
            operator bool() const { return m_context; }

        public:
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<
                VMA_HPP_NAMESPACE::DefragmentationPassMoveInfo>::type beginPass() const {
                return m_allocator.beginDefragmentationPass(m_context);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<
                VMA_HPP_NAMESPACE::DefragmentationPassMoveInfo>::type endPass() const {
                return m_allocator.endDefragmentationPass(m_context);
            }
        };

        struct Allocator {
        private:
            VMA_HPP_NAMESPACE::Allocator m_allocator;

        public:
            VULKAN_HPP_CONSTEXPR Allocator() VULKAN_HPP_NOEXCEPT : m_allocator(nullptr) {}
            VULKAN_HPP_TYPESAFE_EXPLICIT
            Allocator(VmaAllocator allocator) VULKAN_HPP_NOEXCEPT : m_allocator(allocator) {}
            VULKAN_HPP_TYPESAFE_EXPLICIT
            Allocator(VMA_HPP_NAMESPACE::Allocator allocator) VULKAN_HPP_NOEXCEPT : m_allocator(allocator) {}
            VULKAN_HPP_TYPESAFE_EXPLICIT Allocator(const AllocatorCreateInfo &createInfo) : m_allocator(
                details::getResultValue(createAllocator(createInfo))) {}

            ~Allocator() {
                if (m_allocator)
                    m_allocator.destroy();

                m_allocator = nullptr;
            }

            // No copy
            Allocator(Allocator &) = delete;

            Allocator &operator=(Allocator &) = delete;

            // Move
            Allocator(Allocator &&other) noexcept : Allocator() {
                swap(other);
            }

            Allocator &operator=(Allocator &&other) noexcept {
                Allocator newObj{std::move(other)};
                swap(newObj);
                return *this;
            }

            void swap(Allocator &other) noexcept {
                std::ranges::swap(m_allocator, other.m_allocator);
            }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
            auto operator<=>(Allocator const &) const = default;
#else
            bool operator==(Allocator const &rhs) const VULKAN_HPP_NOEXCEPT {
                return m_allocator == rhs.m_allocator;
            }
#endif

            VULKAN_HPP_TYPESAFE_EXPLICIT operator VmaAllocator() const VULKAN_HPP_NOEXCEPT {
                return m_allocator;
            }

            VULKAN_HPP_TYPESAFE_EXPLICIT operator VMA_HPP_NAMESPACE::Allocator() const VULKAN_HPP_NOEXCEPT {
                return m_allocator;
            }

            explicit operator bool() const VULKAN_HPP_NOEXCEPT {
                return m_allocator.operator bool();
            }

            bool operator!() const VULKAN_HPP_NOEXCEPT {
                return m_allocator.operator!();
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VMA_HPP_NAMESPACE::AllocatorInfo getAllocatorInfo() const {
                return m_allocator.getAllocatorInfo();
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties *
            getPhysicalDeviceProperties() const {
                return m_allocator.getPhysicalDeviceProperties();
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties *
            getMemoryProperties() const {
                return m_allocator.getMemoryProperties();
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_NAMESPACE::MemoryPropertyFlags getMemoryTypeProperties(
                uint32_t memoryTypeIndex) const {
                return m_allocator.getMemoryTypeProperties(memoryTypeIndex);
            }

            void setCurrentFrameIndex(uint32_t frameIndex) const {
                m_allocator.setCurrentFrameIndex(frameIndex);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS TotalStatistics calculateStatistics() const {
                return m_allocator.calculateStatistics();
            }

            template<typename VectorAllocator = std::allocator<VMA_HPP_NAMESPACE::Budget>,
                typename B = VectorAllocator,
                typename std::enable_if<std::is_same<typename B::value_type, VMA_HPP_NAMESPACE::Budget>::value,
                    int>::type = 0>
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS std::vector<VMA_HPP_NAMESPACE::Budget, VectorAllocator>
            getHeapBudgets(VectorAllocator &vectorAllocator) const {
                return m_allocator.getHeapBudgets(vectorAllocator);
            }

            template<typename VectorAllocator = std::allocator<VMA_HPP_NAMESPACE::Budget>>
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS std::vector<VMA_HPP_NAMESPACE::Budget, VectorAllocator>
            getHeapBudgets() const {
                return m_allocator.getHeapBudgets();
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type
            findMemoryTypeIndex(
                uint32_t memoryTypeBits,
                const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
            ) const {
                return m_allocator.findMemoryTypeIndex(memoryTypeBits, allocationCreateInfo);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type
            findMemoryTypeIndexForBufferInfo(
                const VULKAN_HPP_NAMESPACE::BufferCreateInfo &bufferCreateInfo,
                const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
            ) const {
                return m_allocator.findMemoryTypeIndexForBufferInfo(bufferCreateInfo, allocationCreateInfo);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type
            findMemoryTypeIndexForImageInfo(
                const VULKAN_HPP_NAMESPACE::ImageCreateInfo &imageCreateInfo,
                const VMA_HPP_NAMESPACE::AllocationCreateInfo &allocationCreateInfo
            ) const {
                return m_allocator.findMemoryTypeIndexForImageInfo(imageCreateInfo, allocationCreateInfo);
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<Pool>::type
            createPool(const VMA_HPP_NAMESPACE::PoolCreateInfo &createInfo) const {
                // return Pool(this->operator VmaAllocator(), createInfo);
                return Pool::create(
                    this->operator VmaAllocator(),
                    createInfo
                );
            }

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type
            allocateMemory(
                const VULKAN_HPP_NAMESPACE::MemoryRequirements &vkMemoryRequirements,
                const VMA_HPP_NAMESPACE::AllocationCreateInfo &createInfo,
                VULKAN_HPP_NAMESPACE::Optional<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo = nullptr
            ) const {
                // return Allocation(this->operator VmaAllocator(), vkMemoryRequirements, createInfo, allocationInfo);
                return Allocation::create(
                    this->operator VmaAllocator(),
                    vkMemoryRequirements,
                    createInfo,
                    allocationInfo
                );
            }

#pragma region Allocations
            template<typename VectorAllocator = std::allocator<Allocation>,
                typename B = VectorAllocator,
                typename std::enable_if<std::is_same<typename B::value_type, Allocation>::value, int>::type = 0>
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<
                Allocation, VectorAllocator>>::type allocateMemoryPages(
                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> vkMemoryRequirements,
                VULKAN_HPP_NAMESPACE::ArrayProxy<const VMA_HPP_NAMESPACE::AllocationCreateInfo> createInfo,
                VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo,
                VectorAllocator &vectorAllocator
            ) const; //TODO

            template<typename VectorAllocator = std::allocator<Allocation>>
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<
                Allocation, VectorAllocator>>::type allocateMemoryPages(
                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> vkMemoryRequirements,
                VULKAN_HPP_NAMESPACE::ArrayProxy<const VMA_HPP_NAMESPACE::AllocationCreateInfo> createInfo,
                VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo =
                        nullptr
            ) const; //TODO

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type
            allocateMemoryForBuffer(
                VULKAN_HPP_NAMESPACE::Buffer buffer,
                const VMA_HPP_NAMESPACE::AllocationCreateInfo &createInfo,
                VULKAN_HPP_NAMESPACE::Optional<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo = nullptr
            ) const; //TODO

            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type
            allocateMemoryForImage(
                VULKAN_HPP_NAMESPACE::Image image,
                const VMA_HPP_NAMESPACE::AllocationCreateInfo &createInfo,
                VULKAN_HPP_NAMESPACE::Optional<VMA_HPP_NAMESPACE::AllocationInfo> allocationInfo = nullptr
            ) const; //TODO

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type flushAllocations(
                VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> allocations,
                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> offsets,
                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> sizes
            ) const {
                std::vector<VMA_HPP_NAMESPACE::Allocation> alloc{};
                alloc.reserve(allocations.size());
                for (const auto &allocation: allocations) {
                    assert(allocation.allocator() == m_allocator);
                    alloc.emplace_back(allocation.operator VMA_HPP_NAMESPACE::Allocation());
                }

                return m_allocator.flushAllocations(
                    alloc,
                    offsets,
                    sizes
                );
            }

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type invalidateAllocations(
                VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> allocations,
                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> offsets,
                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> sizes
            ) const {
                std::vector<VMA_HPP_NAMESPACE::Allocation> alloc{};
                alloc.reserve(allocations.size());
                for (const auto &allocation: allocations) {
                    assert(allocation.allocator() == m_allocator);
                    alloc.emplace_back(allocation.operator VMA_HPP_NAMESPACE::Allocation());
                }

                return m_allocator.invalidateAllocations(
                    alloc,
                    offsets,
                    sizes
                );
            }
#pragma endregion

            typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type checkCorruption(uint32_t memoryTypeBits) const {
                return m_allocator.checkCorruption(memoryTypeBits);
            }

#pragma region Stats String
            VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS StatsString buildStatsString(
                VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const {
                return StatsString(*this, detailedMap);
            }
#pragma endregion
        };
    }
}
#endif

#endif
