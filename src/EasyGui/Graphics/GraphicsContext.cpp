module GraphicsContext;

import std;

import "EasyGui/Lib/Lib.hpp";

namespace EasyGui {
    GraphicsContext::GraphicsContext(SDL_Window *window) {
        Init(window);
    }

    void GraphicsContext::Init(SDL_Window* window) {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface(window);
        PickPhysicalDevice();
        CreateLogicalDevice();

        CreateAllocator();

        CreateSwapChain(window);
        CreateImageViews();

        CreateDescriptorPool();

        CreateSampler();

        CreateRenderPass();
        CreateFramebuffers();

        CreateCommandPool();
        CreateSyncObjects();
        CreateCommandBuffer();
    }

    void GraphicsContext::CreateInstance() {
        if (enableValidationLayers && !CheckValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        vk::ApplicationInfo appInfo{
            .sType = vk::StructureType::eApplicationInfo,
            .pNext = nullptr,
            .pApplicationName = "Hello Triangle",
            .applicationVersion = vk::makeVersion(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = vk::makeVersion(1, 0, 0),
            .apiVersion = vk::ApiVersion13
        };

        auto extensions = GetRequiredExtensions();

        auto debugCreateInfo = PopulateDebugMessengerCreateInfo();

        // flags,
        vk::InstanceCreateInfo createInfo{
            .sType = vk::StructureType::eInstanceCreateInfo,
            .pNext = enableValidationLayers ? static_cast<const void *>(&debugCreateInfo) : nullptr,
            .flags = vk::InstanceCreateFlags{},
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(s_ValidationLayers.size()) : 0,
            .ppEnabledLayerNames = enableValidationLayers ? s_ValidationLayers.data() : nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
        };

        m_Instance = m_Context.createInstance(createInfo).value();
    }

    bool GraphicsContext::CheckValidationLayerSupport() {
        auto availableLayers = vk::enumerateInstanceLayerProperties().value;

        for (const auto &layerName: s_ValidationLayers) {
            bool layerFound = false;
            for (const auto &layerProperties: availableLayers) {
                if (std::string_view(layerProperties.layerName) == layerName) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                std::cerr << "Validation layer " << layerName << " not found!" << std::endl;
                return false;
            }
        }

        // std::cout << "Validation layers are supported." << std::endl;

        return true;
    }

    void GraphicsContext::SetupDebugMessenger() {
        if constexpr (!enableValidationLayers) return;

        m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(PopulateDebugMessengerCreateInfo()).value();
    }

    std::vector<const char *> GraphicsContext::GetRequiredExtensions() {
        std::vector<const char *> extensions;
        uint32_t sdl_extensions_count = 0;
        const char *const *sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);
        for (uint32_t n = 0; n < sdl_extensions_count; n++)
            extensions.push_back(sdl_extensions[n]);

        if (enableValidationLayers) {
            extensions.push_back(vk::EXTDebugUtilsExtensionName);
        }

        return extensions;
    }

    inline vk::Bool32 DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                    vk::DebugUtilsMessageTypeFlagsEXT messageType,
                                    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                    void *pUserData) {
        if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        } else {
            std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
        }

        return vk::False;
    }

    vk::DebugUtilsMessengerCreateInfoEXT GraphicsContext::PopulateDebugMessengerCreateInfo() {
        return vk::DebugUtilsMessengerCreateInfoEXT{
            .sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT,
            .pNext = nullptr,
            .flags = vk::DebugUtilsMessengerCreateFlagsEXT{},
            .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            .pfnUserCallback = DebugCallback,
            .pUserData = nullptr
        };
    }

    void GraphicsContext::PickPhysicalDevice() {
        auto physicalDevices = m_Instance.enumeratePhysicalDevices().value();
        if (physicalDevices.empty()) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        for (const auto &device: physicalDevices) {
            if (IsDeviceSuitable(device)) {
                m_PhysicalDevice = device;
                // std::cout << "Physical device selected: " << device.getProperties().deviceName << std::endl;
                return;
            }
        }
    }

    void GraphicsContext::CreateLogicalDevice() {
        if (!*m_PhysicalDevice) {
            throw std::runtime_error("Physical device not selected.");
        }

        auto queueFamilies = FindQueueFamilies(m_PhysicalDevice);

        float queuePriorities[1] = {1.0f};

        std::unordered_set<uint32_t> uniqueQueueFamilies = {
            queueFamilies.GraphicsFamily.value(),
            queueFamilies.PresentFamily.value()
        };

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        queueCreateInfos.reserve(uniqueQueueFamilies.size());
        for (const auto &queueFamily: uniqueQueueFamilies) {
            queueCreateInfos.push_back({
                .pNext = nullptr,
                .flags = {},
                .queueFamilyIndex = queueFamily,
                .queueCount = 1,
                .pQueuePriorities = queuePriorities
            });
        }

        vk::PhysicalDeviceFeatures deviceFeatures{};

        vk::DeviceCreateInfo deviceCreateInfo{
            .pNext = nullptr,
            .flags = {},
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(s_ValidationLayers.size()) : 0,
            .ppEnabledLayerNames = enableValidationLayers ? s_ValidationLayers.data() : nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(s_DeviceExtensions.size()),
            .ppEnabledExtensionNames = s_DeviceExtensions.data(),
            .pEnabledFeatures = &deviceFeatures
        };

        m_Device = m_PhysicalDevice.createDevice(deviceCreateInfo).value();
        m_GraphicsQueue = m_Device.getQueue(queueFamilies.GraphicsFamily.value(), 0).value();
        m_PresentQueue = m_Device.getQueue(queueFamilies.PresentFamily.value(), 0).value();
    }

    void GraphicsContext::CreateAllocator() {
        vma::AllocatorCreateInfo allocatorInfo{
            .flags = vma::AllocatorCreateFlagBits::eExtMemoryBudget,
            .physicalDevice = *m_PhysicalDevice,
            .device = *m_Device,
            .instance = *m_Instance,
            .vulkanApiVersion = vk::ApiVersion13
        };

        m_Allocator = vma::createAllocatorUnique(allocatorInfo).value;
    }

    void GraphicsContext::CreateSurface(SDL_Window *window) {
        auto sdlWindowProperties = SDL_GetWindowProperties(window);
        // HWND hwnd = SDL_Vulkan_GetVkGetInstanceProcAddr();
        /*
        And, in case you missed it, you can get the HWND with the SDL_PROP_WINDOW_WIN32_HWND_POINTER property with the object returned by SDL_GetWindowProperties.
         */

        // extern SDL_DECLSPEC void * SDLCALL SDL_GetPointerProperty(SDL_PropertiesID props, const char *name, void *default_value);
        HWND hwnd = static_cast<HWND>(
            SDL_GetPointerProperty(sdlWindowProperties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
        vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo{
            .pNext = nullptr,
            .flags = {},
            .hinstance = GetModuleHandle(nullptr),
            .hwnd = hwnd
        };

        m_Surface = m_Instance.createWin32SurfaceKHR(surfaceCreateInfo).value();
    }

    QueueFamilyIndices GraphicsContext::FindQueueFamilies(vk::PhysicalDevice physicalDevice) {
        QueueFamilyIndices indices;

        auto queueFamilies = physicalDevice.getQueueFamilyProperties();

        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            const auto &queueFamily = queueFamilies[i];

            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.GraphicsFamily = i;
            }

            if (physicalDevice.getSurfaceSupportKHR(i, *m_Surface).value) {
                indices.PresentFamily = i;
            }

            if (indices.IsComplete()) {
                break;
            }
        }

        return indices;
    }

    bool GraphicsContext::IsDeviceSuitable(const vk::raii::PhysicalDevice &device) {
        auto queueFamilies = FindQueueFamilies(device);

        bool extensionSupported = CheckDeviceExtensionSupport(device);

        bool swapChainAdequate = false;

        if (extensionSupported) {
            auto swapChainSupport = QuerySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
        }

        return queueFamilies.IsComplete() && extensionSupported && swapChainAdequate;
    }

    bool GraphicsContext::CheckDeviceExtensionSupport(vk::PhysicalDevice device) const {
        auto availableExtensions = device.enumerateDeviceExtensionProperties().value;
        std::set<std::string> requiredExtensions(s_DeviceExtensions.begin(), s_DeviceExtensions.end());
        for (const auto &extension: availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }
        if (requiredExtensions.empty()) {
            // std::cout << "All required device extensions are supported." << std::endl;
            return true;
        }
        std::cerr << "Not all required device extensions are supported!" << std::endl;
        for (const auto &ext: requiredExtensions) {
            std::cerr << "Missing extension: " << ext << std::endl;
        }
        return false;
    }

    SwapChainSupportDetails GraphicsContext::QuerySwapChainSupport(vk::PhysicalDevice device) {
        SwapChainSupportDetails details;

        auto capabilities = device.getSurfaceCapabilitiesKHR(*m_Surface).value;
        auto formats = device.getSurfaceFormatsKHR(*m_Surface).value;
        auto presentModes = device.getSurfacePresentModesKHR(*m_Surface).value;

        return {
            .Capabilities = std::move(capabilities),
            .Formats = std::move(formats),
            .PresentModes = std::move(presentModes)
        };
    }

    vk::SurfaceFormatKHR GraphicsContext::ChooseSwapSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR> &availableFormats) const {
        for (const auto &availableFormat: availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
                availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }

        return availableFormats.front();
    }

    vk::PresentModeKHR GraphicsContext::ChooseSwapPresentMode(
        const std::vector<vk::PresentModeKHR> &availablePresentModes) const {
        return vk::PresentModeKHR::eFifo; // FIFO is guaranteed to be supported
    }

    vk::Extent2D GraphicsContext::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, SDL_Window* window) const {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()

        ) {
            return capabilities.currentExtent;
        }

        int width, height;
        // Get frame buffer size from SDL
        SDL_GetWindowSize(window, &width, &height);

        vk::Extent2D actualExtent = {
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }

    void GraphicsContext::CreateSwapChain(SDL_Window* window) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(*m_PhysicalDevice);

        auto surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
        auto presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
        auto extent = ChooseSwapExtent(swapChainSupport.Capabilities, window);

        uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
        m_MinImageCount = swapChainSupport.Capabilities.minImageCount;
        m_ImageCount = imageCount;
        if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.
            maxImageCount) {
            imageCount = swapChainSupport.Capabilities.maxImageCount;
        }

        auto queueFamilies = FindQueueFamilies(*m_PhysicalDevice);

        uint32_t queueFamilyIndices[] = {
            queueFamilies.GraphicsFamily.value(),
            queueFamilies.PresentFamily.value()
        };

        vk::SwapchainCreateInfoKHR swapChainCreateInfo{
            .pNext = nullptr,
            .flags = {},
            .surface = *m_Surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = queueFamilies.GraphicsFamily == queueFamilies.PresentFamily
                                    ? vk::SharingMode::eExclusive
                                    : vk::SharingMode::eConcurrent,
            .queueFamilyIndexCount = static_cast<uint32_t>((queueFamilies.GraphicsFamily == queueFamilies.PresentFamily)
                                                               ? 0
                                                               : 2),
            .pQueueFamilyIndices = (queueFamilies.GraphicsFamily == queueFamilies.PresentFamily)
                                       ? nullptr
                                       : queueFamilyIndices,
            .preTransform = swapChainSupport.Capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque, // Opaque is a common choice
            .presentMode = presentMode,
            .clipped = vk::True,
            .oldSwapchain = m_SwapChain.release()
        };

        m_SwapChain = m_Device.createSwapchainKHR(swapChainCreateInfo).value();

        m_SwapChainImages = m_SwapChain.getImages();
        m_SwapChainImageFormat = surfaceFormat.format;
        m_SwapChainExtent = extent;
    }

    void GraphicsContext::CreateImageViews() {
        m_SwapChainImageViews.reserve(m_SwapChainImages.size());

        for (const auto &image: m_SwapChainImages) {
            vk::ImageViewCreateInfo viewInfo{
                .pNext = nullptr,
                .flags = {},
                .image = image,
                .viewType = vk::ImageViewType::e2D,
                .format = m_SwapChainImageFormat,
                .components = {
                    .r = vk::ComponentSwizzle::eIdentity,
                    .g = vk::ComponentSwizzle::eIdentity,
                    .b = vk::ComponentSwizzle::eIdentity,
                    .a = vk::ComponentSwizzle::eIdentity
                },
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            m_SwapChainImageViews.push_back(m_Device.createImageView(viewInfo).value());
        }
    }

    void GraphicsContext::CreateDescriptorPool() {
        vk::DescriptorPoolSize poolSize{
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 10
        };

        vk::DescriptorPoolCreateInfo poolInfo{
            .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .maxSets = 10,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize
        };

        m_DescriptorPool = m_Device.createDescriptorPool(poolInfo).value();
    }

    void GraphicsContext::CreateSampler() {
        vk::SamplerCreateInfo samplerInfo{
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
            .anisotropyEnable = vk::False,
            .borderColor = vk::BorderColor::eIntOpaqueBlack,
            .unnormalizedCoordinates = vk::False,
        };

        m_Sampler = m_Device.createSampler(samplerInfo).value();
    }

    void GraphicsContext::CreateRenderPass() {
        // disable v-sync
        vk::AttachmentDescription colorAttachment{
            .flags = {},
            .format = m_SwapChainImageFormat,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR
        };

        vk::AttachmentReference colorAttachmentRef{
            .attachment = 0, // Index of the attachment in the render pass
            .layout = vk::ImageLayout::eColorAttachmentOptimal
        };

        vk::SubpassDescription subpass{
            .flags = {},
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
        };

        vk::SubpassDependency dependency{
            .srcSubpass = vk::SubpassExternal, // No previous subpass
            .dstSubpass = 0, // Our subpass
            .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .srcAccessMask = {},
            .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
            .dependencyFlags = {}
        };

        vk::RenderPassCreateInfo renderPassInfo{
            .pNext = nullptr,
            .flags = {},
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
        };

        m_RenderPass = m_Device.createRenderPass(renderPassInfo).value();
    }

    void GraphicsContext::CreateFramebuffers() {
        m_SwapChainFramebuffers.reserve(m_SwapChainImageViews.size());

        for (const auto &imageView: m_SwapChainImageViews) {
            vk::ImageView attachments[] = {
                *imageView // Single attachment for color
            };

            vk::FramebufferCreateInfo framebufferInfo{
                .pNext = nullptr,
                .flags = {},
                .renderPass = *m_RenderPass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = m_SwapChainExtent.width,
                .height = m_SwapChainExtent.height,
                .layers = 1 // Single layer for 2D images
            };

            m_SwapChainFramebuffers.push_back(
                m_Device.createFramebuffer(framebufferInfo).value()
            );
        }
    }

    void GraphicsContext::CreateCommandPool() {
        auto queueFamilies = FindQueueFamilies(*m_PhysicalDevice);
        if (!queueFamilies.GraphicsFamily.has_value()) {
            throw std::runtime_error("Graphics queue family not found.");
        }
        vk::CommandPoolCreateInfo poolInfo{
            .pNext = nullptr,
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = queueFamilies.GraphicsFamily.value()
        };

        m_CommandPool = m_Device.createCommandPool(poolInfo).value();
    }

    void GraphicsContext::CreateCommandBuffer() {
        vk::CommandBufferAllocateInfo allocInfo{
            .pNext = nullptr,
            .commandPool = *m_CommandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_CommandBuffers.push_back(std::move(m_Device.allocateCommandBuffers(allocInfo).value().front()));
        }

        // m_CommandBufferDependentContexts.resize(MAX_FRAMES_IN_FLIGHT);
    }

    void GraphicsContext::CreateSyncObjects() {
        vk::SemaphoreCreateInfo semaphoreInfo{
            .pNext = nullptr,
            .flags = {}
        };
        vk::FenceCreateInfo fenceInfo{
            .pNext = nullptr,
            .flags = vk::FenceCreateFlagBits::eSignaled // Start with the fence signaled
        };

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_ImageAvailableSemaphores.push_back(m_Device.createSemaphore(semaphoreInfo).value());
            m_InFlightFences.push_back(m_Device.createFence(fenceInfo).value());
        }

        // m_InFlightFence = m_Device.createFence(fenceInfo).value();
        for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
            m_RenderFinishedSemaphores.push_back(m_Device.createSemaphore(semaphoreInfo).value());
        }
    }

    void GraphicsContext::RecreateSwapChain(SDL_Window* window) {
        m_Device.waitIdle();

        CleanupSwapChain();

        CreateSwapChain(window);
        CreateImageViews();
        CreateFramebuffers();

        // std::cout << "Swap chain recreated successfully." << std::endl;
    }

    void GraphicsContext::CleanupSwapChain() {
        m_SwapChainFramebuffers.clear();
        m_SwapChainImageViews.clear();
        m_SwapChainImages.clear();

        m_SwapChain.clear();
    }
}