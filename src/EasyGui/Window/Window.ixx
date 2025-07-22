export module EasyGui.Window;

import std;
export import EasyGui.Lib;
export import EasyGui.Core.KeyCodes;
export import EasyGui.Core.MouseCodes;
export import EasyGui.Event.AllEvents;
import EasyGui.Graphics.GraphicsContext;

import "EasyGui/Lib/Lib_SDL3.hpp";
import "EasyGui/Lib/Lib_Vulkan.hpp";

namespace EasyGui {
    export class IUpdatableLayer {
    public:
        virtual ~IUpdatableLayer() = default;

        virtual void OnUpdate() {}

        virtual void OnSubmitCommandBuffer(vk::CommandBuffer commandBuffer) {}

        virtual bool OnEvent(const Event &event) {
            return false;
        }
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

    export struct WindowSpec {
        std::string title{"Default Title"};
        int width = 1920;
        int height = 1080;
    };

    export class AppGraphicsContext : public GraphicsContext {
    public:
        AppGraphicsContext(SDL_Window *window);

        void InitImGui(SDL_Window *window);

        virtual ~AppGraphicsContext() override;
    };

    export class Window {
    public:
        explicit Window(const WindowSpec &windowSpec);

    private:
        void DispatchNormalEvent(SDL_Event sdlEvent);

    public:
        void MainLoop();

        void OnUpdate();

        void DrawFrame();

    private:
        void InitializeWindow(const WindowSpec &windowSpec);

        SDL_Window *m_Window{nullptr};
        std::unique_ptr<AppGraphicsContext> m_GraphicsContext;
        size_t m_CurrentFrame = 0;
        bool m_ShouldUpdate = true;

        std::vector<std::shared_ptr<IUpdatableLayer>> m_Layers;

        std::vector<std::function<void()>> m_MainThreadTasks;

    public:
        template<std::derived_from<IUpdatableLayer> T>
        std::shared_ptr<T> EmplaceLayer(auto &&... args) {
            auto layer = std::make_shared<T>(std::forward<decltype(args)>(args)...);
            PushLayer(layer);
            return layer;
        }

        void SubmitToMainThread(auto &&task) {
            m_MainThreadTasks.emplace_back(std::forward<decltype(task)>(task));
        }

        // override all IBasicContext methods
        [[nodiscard]] SDL_Window *GetWindow() const { return m_Window; }

        void PushLayer(std::shared_ptr<IUpdatableLayer> layer);

        void PopLayer(std::shared_ptr<IUpdatableLayer> layer);

        vk::raii::PhysicalDevice &GetPhysicalDevice();

        vk::raii::Device &GetLogicalDevice();

        vk::raii::Queue &GetGraphicsQueue();

        vk::raii::CommandPool &GetCommandPool();

        vk::raii::Instance &GetVulkanInstance();

        vma::UniqueAllocator &GetAllocator();
    };


#undef CreateWindow

    export std::shared_ptr<Window> CreateWindow(const WindowSpec &windowSpec) {
        return std::make_shared<Window>(windowSpec);
    }
}
