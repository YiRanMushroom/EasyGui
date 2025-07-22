module EasyGui.Window;

import "EasyGui/Lib/Lib.hpp";

namespace EasyGui {
        inline void ImGuiUseStyleColorHazel() {
        auto &colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

        // Headers
        colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Tabs
        // colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        // colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
        colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.185f, 0.19f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.285f, 0.29f, 1.00f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.22f, 0.225f, 0.23f, 1.00f);
        colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.35f, 0.40f, 0.60f, 1.00f);
        colors[ImGuiCol_TabDimmed] = ImVec4(0.13f, 0.135f, 0.14f, 1.00f);
        colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.18f, 0.185f, 0.19f, 1.00f);
        colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.25f, 0.30f, 0.45f, 1.00f);

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Resize Grip
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

        // Scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

        // Check Mark
        colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.0f);

        // Slider
        colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

        // Docking
        // colors[ImGuiCol_DockingPreview] = ImVec4(0.31f, 0.31f, 0.31f, 0.7f);
        // colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.0f);
    }

    void InitImGuiForMyProgram(uint32_t apiVersion,
                               vk::Instance instance, vk::PhysicalDevice physicalDevice,
                               vk::Device device, uint32_t queueFamily, vk::Queue queue,
                               vk::DescriptorPool descriptorPool,
                               vk::RenderPass renderPass, uint32_t minImageCount, uint32_t imageCount) {
        ImGui_ImplVulkan_InitInfo info{};
        info.ApiVersion = apiVersion;
        info.Instance = instance;
        info.PhysicalDevice = physicalDevice;
        info.Device = device;
        info.QueueFamily = queueFamily;
        info.Queue = queue;
        info.DescriptorPool = descriptorPool;
        info.RenderPass = renderPass;
        info.MinImageCount = minImageCount;
        info.ImageCount = imageCount;

        ImGui_ImplVulkan_Init(&info);
    }

    AppGraphicsContext::AppGraphicsContext(SDL_Window *window) : GraphicsContext(window) {
        InitImGui(window);
    }

    void AppGraphicsContext::InitImGui(SDL_Window *window) {
        ImGui::CreateContext();

        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

        ImGuiUseStyleColorHazel();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplSDL3_InitForVulkan(window);

        InitImGuiForMyProgram(
            vk::ApiVersion13,
            *m_Instance,
            *m_PhysicalDevice,
            *m_Device,
            FindQueueFamilies(*m_PhysicalDevice).GraphicsFamily.value(),
            *m_GraphicsQueue,
            *m_DescriptorPool,
            *m_RenderPass,
            m_MinImageCount, m_ImageCount
        );
    }

    AppGraphicsContext::~AppGraphicsContext() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void Window::DispatchNormalEvent(SDL_Event sdlEvent) {
        switch (sdlEvent.type) {
            case SDL_EVENT_KEY_DOWN: {
                KeyPressedEvent event{
                    Key::KeyCode(sdlEvent.key.key),
                    sdlEvent.key.repeat
                };
                for (auto reverseIt = m_Layers.rbegin(); reverseIt != m_Layers.rend(); ++reverseIt) {
                    if ((*reverseIt)->OnEvent(event)) {
                        break;
                    }
                }
                break;
            }
            case SDL_EVENT_KEY_UP: {
                KeyReleasedEvent event{
                    Key::KeyCode(sdlEvent.key.key)
                };
                for (auto reverseIt = m_Layers.rbegin(); reverseIt != m_Layers.rend(); ++reverseIt) {
                    if ((*reverseIt)->OnEvent(event)) {
                        break;
                    }
                }
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                MouseButtonPressedEvent event{
                    Mouse::MouseCode(sdlEvent.button.button)
                };
                for (auto reverseIt = m_Layers.rbegin(); reverseIt != m_Layers.rend(); ++reverseIt) {
                    if ((*reverseIt)->OnEvent(event)) {
                        break;
                    }
                }
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                MouseButtonReleasedEvent event{
                    Mouse::MouseCode(sdlEvent.button.button)
                };
                for (auto reverseIt = m_Layers.rbegin(); reverseIt != m_Layers.rend(); ++reverseIt) {
                    if ((*reverseIt)->OnEvent(event)) {
                        break;
                    }
                }
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                MouseMovedEvent event{
                    sdlEvent.motion.x,
                    sdlEvent.motion.y
                };
                for (auto reverseIt = m_Layers.rbegin(); reverseIt != m_Layers.rend(); ++reverseIt) {
                    if ((*reverseIt)->OnEvent(event)) {
                        break;
                    }
                }
                break;
            }
        }
    }

    void Window::InitializeWindow(const WindowSpec &windowSpec) {
        SDL_WindowFlags windowFlags =
                SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
        m_Window = SDL_CreateWindow(windowSpec.title.c_str(), windowSpec.width, windowSpec.height, windowFlags);
        if (!m_Window) {
            throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
        }
    }

    void Window::PushLayer(std::shared_ptr<IUpdatableLayer> layer) {
        m_Layers.push_back(layer);
    }

    void Window::PopLayer(std::shared_ptr<IUpdatableLayer> layer) {
        auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
        if (it != m_Layers.end()) {
            m_Layers.erase(it);
        }
    }

    vk::raii::PhysicalDevice & Window::GetPhysicalDevice() {
        return m_GraphicsContext->GetPhysicalDevice();
    }

    vk::raii::Device & Window::GetLogicalDevice() {
        return m_GraphicsContext->GetLogicalDevice();
    }

    vk::raii::Queue & Window::GetGraphicsQueue() {
        return m_GraphicsContext->GetGraphicsQueue();
    }

    vk::raii::CommandPool & Window::GetCommandPool() {
        return m_GraphicsContext->GetCommandPool();
    }

    vk::raii::Instance & Window::GetVulkanInstance() {
        return m_GraphicsContext->GetVulkanInstance();
    }

    vma::UniqueAllocator & Window::GetAllocator() {
        return m_GraphicsContext->GetAllocator();
    }

    Window::Window(const WindowSpec &windowSpec) {
        InitializeWindow(windowSpec);
        m_GraphicsContext = std::make_unique<AppGraphicsContext>(m_Window);
    }

    void Window::MainLoop() {
        SDL_SetWindowPosition(m_Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_ShowWindow(m_Window);
        bool done = false;

        while (!done) {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL3_ProcessEvent(&event);
                // rewrite this using switch case
                switch (event.type) {
                    case SDL_EVENT_QUIT: {
                        done = true;
                        WindowCloseEvent event{};
                        for (auto reverseIt = m_Layers.rbegin(); reverseIt != m_Layers.rend(); ++reverseIt) {
                            if ((*reverseIt)->OnEvent(event)) {
                                break;
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                        if (event.window.windowID == SDL_GetWindowID(m_Window)) {
                            done = true;
                            WindowCloseEvent event{};
                            for (auto reverseIt = m_Layers.rbegin(); reverseIt != m_Layers.rend(); ++reverseIt) {
                                if ((*reverseIt)->OnEvent(event)) {
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_WINDOW_RESIZED:
                    case SDL_EVENT_WINDOW_MINIMIZED:
                    case SDL_EVENT_WINDOW_MAXIMIZED: {
                        int width, height;
                        width = event.window.data1;
                        height = event.window.data2;

                        if (width > 0 && height > 0) {
                            m_ShouldUpdate = true;
                            auto [swapChainWidth, swapChainHeight] = m_GraphicsContext->GetSwapChainExtent();
                            if (width != swapChainWidth || height != swapChainHeight) {
                                m_GraphicsContext->RecreateSwapChain(m_Window);
                            }
                        } else {
                            m_ShouldUpdate = false;
                        }

                        WindowResizeEvent resizeEvent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
                        for (auto reverseIt = m_Layers.rbegin(); reverseIt != m_Layers.rend(); ++reverseIt) {
                            if ((*reverseIt)->OnEvent(resizeEvent)) {
                                break;
                            }
                        }
                        break;
                    }
                    case SDL_EVENT_WINDOW_RESTORED: {
                        m_ShouldUpdate = true;
                        break;
                    }
                    default:
                        DispatchNormalEvent(event);
                }
            }

            if (!m_ShouldUpdate) {
                continue;
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
            DrawFrame();

            for (auto& task: m_MainThreadTasks) {
                task();
            }
            m_MainThreadTasks.clear();
        }

        // m_Device.waitIdle();
        m_GraphicsContext->GetLogicalDevice().waitIdle();
        m_Layers.clear();
    }

    void Window::OnUpdate() {
        for (auto &layer: m_Layers) {
            layer->OnUpdate();
        }
    }

    void Window::DrawFrame() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        OnUpdate();
        ImGui::Render();

        auto& device = m_GraphicsContext->GetLogicalDevice();
        auto& inFlightFences = m_GraphicsContext->GetInFlightFences();

        auto waitForFenceResult = device.waitForFences(*inFlightFences[m_CurrentFrame], vk::True,
                                                         std::numeric_limits<uint64_t>::max()
        );

        if (waitForFenceResult != vk::Result::eSuccess) {
            std::cerr << "Failed to wait for fence: " << vk::to_string(waitForFenceResult) << std::endl;
            return;
        }

        auto& swapChain = m_GraphicsContext->GetSwapChain();
        auto& imageAvailableSemaphores = m_GraphicsContext->GetImageAvailableSemaphores();

        auto [resultAcquireImage, imageIndex] = swapChain.acquireNextImage(
            std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[m_CurrentFrame],

            nullptr
        );

        if (resultAcquireImage != vk::Result::eSuccess &&
            resultAcquireImage != vk::Result::eSuboptimalKHR) {
            if (resultAcquireImage == vk::Result::eErrorOutOfDateKHR) {
                m_GraphicsContext->RecreateSwapChain(m_Window);
                return;
            } else {
                std::cerr << "Failed to acquire swap chain image: " << vk::to_string(resultAcquireImage) << std::endl;
                return;
            }
        }

        device.resetFences(*inFlightFences[m_CurrentFrame]);

        // m_ImageViewDependentRenderTargetsPerFrameBuffer[imageIndex] = std::move(m_DependentRenderTargets);

        // m_CommandBufferDependentContexts[m_CurrentFrame].clear();
        auto& commandBuffers = m_GraphicsContext->GetCommandBuffers();
        commandBuffers[m_CurrentFrame].reset();

        vk::CommandBufferBeginInfo beginInfo{
            .pNext = nullptr,
            .flags = {},
            .pInheritanceInfo = nullptr // No inheritance for primary command buffers
        };

        commandBuffers[m_CurrentFrame].begin(beginInfo);

        vk::ClearValue clearColor{
            m_GraphicsContext->GetClearColor()
        };

        auto& frameBuffers = m_GraphicsContext->GetSwapChainFramebuffers();

        vk::RenderPassBeginInfo renderPassInfo{
            .pNext = nullptr,
            .renderPass = m_GraphicsContext->GetRenderPass(),
            .framebuffer = *frameBuffers[imageIndex],
            .renderArea = vk::Rect2D{
                .offset = {0, 0},
                .extent = m_GraphicsContext->GetSwapChainExtent()
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor
        };

        commandBuffers[m_CurrentFrame].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        for (auto reverseIt = m_Layers.rbegin(); reverseIt != m_Layers.rend(); ++reverseIt) {
            auto &layer = *reverseIt;
            layer->OnSubmitCommandBuffer(commandBuffers[m_CurrentFrame]);
        }

        ImDrawData *draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, *commandBuffers[m_CurrentFrame]);

        commandBuffers[m_CurrentFrame].endRenderPass();

        commandBuffers[m_CurrentFrame].end();

        auto& renderFinishedSemaphores = m_GraphicsContext->GetRenderFinishedSemaphores();

        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vk::Semaphore waitSemaphores[] = {*imageAvailableSemaphores[m_CurrentFrame]};
        vk::Semaphore signalSemaphores[] = {*renderFinishedSemaphores[imageIndex]};
        vk::CommandBuffer commandBuffers_[] = {*commandBuffers[m_CurrentFrame]};

        vk::SubmitInfo submitInfo{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = commandBuffers_,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores
        };

        auto &graphicsQueue = m_GraphicsContext->GetGraphicsQueue();

        graphicsQueue.submit(submitInfo, inFlightFences[m_CurrentFrame]);

        vk::SwapchainKHR swapChains[] = {*swapChain};
        vk::PresentInfoKHR presentInfo{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = &imageIndex
        };

        auto &presentQueue = m_GraphicsContext->GetPresentQueue();

        vk::Result presentResult = presentQueue.presentKHR(presentInfo);

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        auto &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

}
