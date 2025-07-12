export module EasyGui;

import std;
export import EasyGui.Lib;
export import EasyGui.Core.KeyCodes;
export import EasyGui.Core.MouseCodes;
export import EasyGui.Event.AllEvents;

import <EasyGui/Lib_SDL3.hpp>;
import <EasyGui/Lib_Vulkan.hpp>;

namespace EasyGui {
    export constexpr bool IsPresented() {
        return true;
    }

    export class IUpdatableLayer {
    public:
        virtual ~IUpdatableLayer() = default;

        virtual void OnUpdate() = 0;

        virtual void OnSubmitCommandBuffer(vk::CommandBuffer commandBuffer, std::vector<std::any> &dependentContexts) =
        0;

        virtual bool OnEvent(const Event &event) = 0;
    };

    export class GlobalContext {
    public:
        static void Init() {
            if (!s_Instance) {
                s_Instance = new GlobalContext();
            } else throw std::runtime_error("GlobalContext is already initialized.");
        }

        static void Shutdown() {
            delete s_Instance;
            s_Instance = nullptr;
        }

        GlobalContext(const GlobalContext &) = delete;

        GlobalContext &operator=(const GlobalContext &) = delete;

        GlobalContext(GlobalContext &&) = delete;

        GlobalContext &operator=(GlobalContext &&) = delete;

    private:
        GlobalContext() {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
                std::println("SDL initialized successfully.");
            } else {
                throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
            }
        }

        ~GlobalContext() {
            SDL_Quit();
        }

        static GlobalContext &GetInstance() {
            if (!s_Instance) {
                throw std::runtime_error("GlobalContext is not initialized. Call Init() first.");
            }
            return *s_Instance;
        }

        inline static GlobalContext *s_Instance;
    };

#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif

    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;

    export struct WindowSpec {
        std::string title{"Default Title"};
        int width = 1920;
        int height = 1080;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        [[nodiscard]] bool IsComplete() const;
    };

    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR Capabilities;
        std::vector<vk::SurfaceFormatKHR> Formats;
        std::vector<vk::PresentModeKHR> PresentModes;
    };

    export class Window {
    public:
        explicit Window(const WindowSpec &windowSpec);

    private:
        void InitializeWindow(const WindowSpec &windowSpec);

        void InitVulkan();

        void CreateInstance();

        static bool CheckValidationLayerSupport();

        void SetupDebugMessenger();

        static std::vector<const char *> GetRequiredExtensions();

        static vk::DebugUtilsMessengerCreateInfoEXT PopulateDebugMessengerCreateInfo();

        void PickPhysicalDevice();

        void CreateLogicalDevice();

        void CreateSurface();

        QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice physicalDevice);

        bool IsDeviceSuitable(const vk::raii::PhysicalDevice &device);

        [[nodiscard]] bool CheckDeviceExtensionSupport(vk::PhysicalDevice device) const;

        SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device);

        [[nodiscard]] vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<vk::SurfaceFormatKHR> &availableFormats) const;

        [[nodiscard]] vk::PresentModeKHR ChooseSwapPresentMode(
            const std::vector<vk::PresentModeKHR> &availablePresentModes) const;

        [[nodiscard]] vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) const;

        void CreateSwapChain();

        void CreateImageViews();

        // void CreateSampler();

        void InitImGui();

        void BeginImGuiFrame();

        void CreateRenderPass();

        void CreateFramebuffers();

        void CreateCommandPool();

        void CreateCommandBuffer();

        void CreateSyncObjects();

        void DrawFrame();

        void RecreateSwapChain();

        void CleanupSwapChain();

        void DispatchNormalEvent(SDL_Event sdlEvent);

    public:
        void MainLoop();

        void Cleanup();

        void OnUpdate();

    private:
        vk::raii::Context m_Context;
        vk::raii::Instance m_Instance{nullptr};

        SDL_Window *m_Window{nullptr};
        vk::raii::SurfaceKHR m_Surface{nullptr};
        vk::raii::DebugUtilsMessengerEXT m_DebugMessenger{nullptr};
        vk::raii::PhysicalDevice m_PhysicalDevice{nullptr};
        vk::raii::Device m_Device{nullptr};
        vk::raii::Queue m_GraphicsQueue{nullptr};
        vk::raii::Queue m_PresentQueue{nullptr};
        vk::raii::SwapchainKHR m_SwapChain{nullptr};

        std::vector<vk::Image> m_SwapChainImages;
        vk::Format m_SwapChainImageFormat;
        vk::Extent2D m_SwapChainExtent;
        std::vector<vk::raii::ImageView> m_SwapChainImageViews;

        vk::raii::RenderPass m_RenderPass{nullptr};
        std::vector<vk::raii::Framebuffer> m_SwapChainFramebuffers;
        vk::raii::CommandPool m_CommandPool{nullptr};

        std::vector<vk::raii::Semaphore> m_ImageAvailableSemaphores;
        std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
        std::vector<vk::raii::Fence> m_InFlightFences;
        size_t m_CurrentFrame = 0;
        std::vector<vk::raii::CommandBuffer> m_CommandBuffers;
        bool m_ShouldUpdate = true;

        std::vector<std::vector<std::any>> m_CommandBufferDependentContexts;

        std::vector<std::shared_ptr<IUpdatableLayer>> m_Layers;

    private:
        size_t m_MinImageCount = 0;
        size_t m_ImageCount = 0;
        vk::ClearColorValue m_ClearColor = vk::ClearColorValue(std::array{.8f, .2f, 0.8f, 1.0f});

    private:
        const inline static std::vector<const char *> s_ValidationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        const inline static std::vector<const char *> s_DeviceExtensions = {
            vk::KHRSwapchainExtensionName
        };

    public:
        template<std::derived_from<IUpdatableLayer> T>
        std::shared_ptr<T> EmplaceLayer(auto &&... args) {
            auto layer = std::make_shared<T>(std::forward<decltype(args)>(args)...);
            PushLayer(layer);
            return layer;
        }

        // override all IBasicContext methods
        SDL_Window *GetWindow() const { return m_Window; }
        const vk::raii::Context &GetRaiiContext() { return m_Context; }
        vk::raii::Instance &GetVulkanInstance() { return m_Instance; }
        vk::raii::PhysicalDevice &GetPhysicalDevice() { return m_PhysicalDevice; }
        vk::raii::Device &GetLogicalDevice() { return m_Device; }
        vk::raii::SurfaceKHR &GetSurface() { return m_Surface; }
        vk::raii::Queue &GetGraphicsQueue() { return m_GraphicsQueue; }
        vk::raii::Queue &GetPresentQueue() { return m_PresentQueue; }
        vk::raii::RenderPass &GetRenderPass() { return m_RenderPass; }
        vk::raii::CommandPool &GetCommandPool() { return m_CommandPool; }
        void SetClearColor(const vk::ClearColorValue &clearColor) { m_ClearColor = clearColor; }

        void PushLayer(std::shared_ptr<IUpdatableLayer> layer) {
            m_Layers.push_back(layer);
        }

        void PopLayer(std::shared_ptr<IUpdatableLayer> layer) {
            auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
            if (it != m_Layers.end()) {
                m_Layers.erase(it);
            }
        }
    };

#undef CreateWindow

    export std::shared_ptr<Window> CreateWindow(const WindowSpec &windowSpec) {
        return std::make_shared<Window>(windowSpec);
    }
}
