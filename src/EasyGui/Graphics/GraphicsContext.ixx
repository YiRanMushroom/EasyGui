export module EasyGui.Graphics.GraphicsContext;

import std;
export import EasyGui.Lib;
export import EasyGui.Core.KeyCodes;
export import EasyGui.Core.MouseCodes;
export import EasyGui.Event.AllEvents;

import "EasyGui/Lib/Lib_SDL3.hpp";
import "EasyGui/Lib/Lib_Vulkan.hpp";

namespace EasyGui {
#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif

    export constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;

    struct QueueFamilyIndices {
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        [[nodiscard]] bool IsComplete() const {
            return GraphicsFamily.has_value() && PresentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR Capabilities;
        std::vector<vk::SurfaceFormatKHR> Formats;
        std::vector<vk::PresentModeKHR> PresentModes;
    };

    export class GraphicsContext {
    public:
        GraphicsContext(SDL_Window *window);

        virtual ~GraphicsContext() {
            CleanupSwapChain();
        }

    protected:
        void Init(SDL_Window *window);

        void CreateInstance();

        static bool CheckValidationLayerSupport();

        void SetupDebugMessenger();

        static std::vector<const char *> GetRequiredExtensions();

        static vk::DebugUtilsMessengerCreateInfoEXT PopulateDebugMessengerCreateInfo();

        void PickPhysicalDevice();

        void CreateLogicalDevice();

        void CreateAllocator();

        void CreateSurface(SDL_Window *window);

        QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice physicalDevice);

        bool IsDeviceSuitable(const vk::raii::PhysicalDevice &device);

        [[nodiscard]] bool CheckDeviceExtensionSupport(vk::PhysicalDevice device) const;

        SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device);

        [[nodiscard]] vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<vk::SurfaceFormatKHR> &availableFormats) const;

        [[nodiscard]] vk::PresentModeKHR ChooseSwapPresentMode(
            const std::vector<vk::PresentModeKHR> &availablePresentModes) const;

        [[nodiscard]] vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities,
                                                    SDL_Window *window) const;

        void CreateSwapChain(SDL_Window *window);

        void CreateImageViews();

        void CreateDescriptorPool();

        void CreateSampler();

        void CreateRenderPass();

        void CreateFramebuffers();

        void CreateCommandPool();

        void CreateCommandBuffer();

        void CreateSyncObjects();

    public:
        void RecreateSwapChain(SDL_Window *window);

    protected:
        void CleanupSwapChain();

    protected:
        vk::raii::Context m_Context;
        vk::raii::Instance m_Instance{nullptr};

        vk::raii::SurfaceKHR m_Surface{nullptr};
        vk::raii::DebugUtilsMessengerEXT m_DebugMessenger{nullptr};
        vk::raii::PhysicalDevice m_PhysicalDevice{nullptr};
        vk::raii::Device m_Device{nullptr};

        vma::UniqueAllocator m_Allocator;

        vk::raii::Queue m_GraphicsQueue{nullptr};
        vk::raii::Queue m_PresentQueue{nullptr};
        vk::raii::SwapchainKHR m_SwapChain{nullptr};

        std::vector<vk::Image> m_SwapChainImages;
        vk::Format m_SwapChainImageFormat;
        vk::Extent2D m_SwapChainExtent;
        std::vector<vk::raii::ImageView> m_SwapChainImageViews;

        vk::raii::DescriptorPool m_DescriptorPool{nullptr};
        vk::raii::Sampler m_Sampler{nullptr};

        vk::raii::RenderPass m_RenderPass{nullptr};
        std::vector<vk::raii::Framebuffer> m_SwapChainFramebuffers;
        vk::raii::CommandPool m_CommandPool{nullptr};

        std::vector<vk::raii::Semaphore> m_ImageAvailableSemaphores;
        std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
        std::vector<vk::raii::Fence> m_InFlightFences;
        std::vector<vk::raii::CommandBuffer> m_CommandBuffers;

    protected:
        size_t m_MinImageCount = 0;
        size_t m_ImageCount = 0;
        size_t m_CurrentFrame = 0;
        vk::ClearValue m_ClearColor = vk::ClearColorValue(std::array{0.0f, 0.0f, 0.0f, 1.0f});

    protected:
        const inline static std::vector<const char *> s_ValidationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        const inline static std::vector<const char *> s_DeviceExtensions = {
            vk::KHRSwapchainExtensionName,
            vk::KHRMapMemory2ExtensionName,
            vk::EXTMemoryBudgetExtensionName
        };

    public:
        const vk::raii::Context &GetRaiiContext() { return m_Context; }
        vk::raii::Instance &GetVulkanInstance() { return m_Instance; }
        vk::raii::PhysicalDevice &GetPhysicalDevice() { return m_PhysicalDevice; }
        vk::raii::Device &GetLogicalDevice() { return m_Device; }
        vk::raii::SurfaceKHR &GetSurface() { return m_Surface; }
        vk::raii::Queue &GetGraphicsQueue() { return m_GraphicsQueue; }
        vk::raii::Queue &GetPresentQueue() { return m_PresentQueue; }
        vk::raii::RenderPass &GetRenderPass() { return m_RenderPass; }
        vk::raii::CommandPool &GetCommandPool() { return m_CommandPool; }
        vma::UniqueAllocator &GetAllocator() { return m_Allocator; }

        vk::Extent2D GetSwapChainExtent() const {
            return m_SwapChainExtent;
        }

        void SetClearColor(const vk::ClearColorValue &clearColor) {
            m_ClearColor = clearColor;
        }

        std::vector<vk::raii::Fence> &GetInFlightFences() {
            return m_InFlightFences;
        }

        std::vector<vk::raii::CommandBuffer> &GetCommandBuffers() {
            return m_CommandBuffers;
        }

        std::vector<vk::raii::Semaphore> &GetImageAvailableSemaphores() {
            return m_ImageAvailableSemaphores;
        }

        std::vector<vk::raii::Semaphore> &GetRenderFinishedSemaphores() {
            return m_RenderFinishedSemaphores;
        }

        vk::raii::SwapchainKHR &GetSwapChain() {
            return m_SwapChain;
        }

        vk::ClearValue GetClearColor() const {
            return m_ClearColor;
        }

        std::vector<vk::raii::ImageView> &GetSwapChainImageViews() {
            return m_SwapChainImageViews;
        }

        std::vector<vk::Image> &GetSwapChainImages() {
            return m_SwapChainImages;
        }

        std::vector<vk::raii::Framebuffer> &GetSwapChainFramebuffers() {
            return m_SwapChainFramebuffers;
        }
    };
}
