module;

export module EasyGui.Utils.Image;

import EasyGui.Lib;
import EasyGui;

namespace EasyGui::Vulkan {
    export class ImGuiImage {
    public:
        ImGuiImage() = default;

        ImGuiImage(ImTextureID &&textureId,
                   size_t width, size_t height)
            : m_TextureId(std::exchange(textureId, 0)),
              m_Width(width), m_Height(height) {}

        ImGuiImage(const ImGuiImage &) = delete;

        ImGuiImage &operator=(const ImGuiImage &) = delete;

        ImGuiImage(ImGuiImage &&other) noexcept
            : m_TextureId(std::exchange(other.m_TextureId, 0)),
              m_Width(other.m_Width), m_Height(other.m_Height) {}

        ImGuiImage &operator=(ImGuiImage &&other) noexcept {
            if (this != &other) {
                std::swap(m_TextureId, other.m_TextureId);
                std::swap(m_Width, other.m_Width);
                std::swap(m_Height, other.m_Height);
            }
            return *this;
        }

        explicit operator bool() const {
            return m_TextureId != 0;
        }

        ~ImGuiImage() {
            if (m_TextureId) {
                ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(m_TextureId));
            }
        }

        ImTextureID Get() const {
            return m_TextureId;
        }

        operator ImTextureID() const {
            return m_TextureId;
        }

        operator ImTextureRef() const {
            return ImTextureRef{m_TextureId};
        }

        size_t GetWidth() const {
            return m_Width;
        }

        size_t GetHeight() const {
            return m_Height;
        }

        std::pair<size_t, size_t> GetSize() const {
            return {m_Width, m_Height};
        }

    private:
        ImTextureID m_TextureId = 0;
        size_t m_Width = 0;
        size_t m_Height = 0;
    };

    export class PixelImage {
    public:
        PixelImage(vma::UniqueImage &&image,
                   vma::UniqueAllocation &&memory,
                   vk::UniqueImageView &&imageView,
                   size_t width, size_t height)
            : m_Image(std::move(image)), m_Memory(std::move(memory)), m_ImageView(std::move(imageView)),
              m_Width(width), m_Height(height) {}

        PixelImage() = default;

        PixelImage(const PixelImage &) = delete;

        PixelImage &operator=(const PixelImage &) = delete;

        PixelImage(PixelImage &&other) noexcept
            : m_Image(std::move(other.m_Image)), m_Memory(std::move(other.m_Memory)),
              m_ImageView(std::move(other.m_ImageView)),
              m_Width(std::exchange(other.m_Width, 0)),
              m_Height(std::exchange(other.m_Height, 0)) {}

        PixelImage &operator=(PixelImage &&other) noexcept {
            if (this != &other) {
                m_Image = std::move(other.m_Image);
                m_Memory = std::move(other.m_Memory);
                m_ImageView = std::move(other.m_ImageView);
                m_Width = std::exchange(other.m_Width, 0);
                m_Height = std::exchange(other.m_Height, 0);
            }
            return *this;
        }

        [[nodiscard]] vk::Image GetImage() const {
            return *m_Image;
        }

        [[nodiscard]] vma::Allocation GetMemory() const {
            return *m_Memory;
        }

        [[nodiscard]] vk::ImageView GetImageView() const {
            return *m_ImageView;
        }

        ImGuiImage CreateImGuiImage(vk::Sampler sampler) const {
            VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(
                sampler,
                m_ImageView.get(),
                static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)
            );

            return {
                std::bit_cast<ImTextureID>(descriptorSet),
                m_Width, m_Height
            };
        }

        size_t GetWidth() const {
            return m_Width;
        }

        size_t GetHeight() const {
            return m_Height;
        }

        std::pair<size_t, size_t> GetSize() const {
            return {m_Width, m_Height};
        }

        explicit operator bool() const {
            return m_Image && m_Memory && m_ImageView;
        }

    private:
        vma::UniqueImage m_Image;
        vma::UniqueAllocation m_Memory;
        vk::UniqueImageView m_ImageView;

        size_t m_Width = 0;
        size_t m_Height = 0;
    };

    export class ImageHelper {
    public:
        ImageHelper(vk::raii::PhysicalDevice* physicalDevice, vk::raii::Device* logicalDevice,
                    vk::raii::CommandPool* commandPool,
                    vk::raii::Queue* graphicsQueue,
                    vk::Instance instance,
                    vma::UniqueAllocator* allocator)
            : m_PhysicalDevice(physicalDevice), m_LogicalDevice(logicalDevice),
              m_CommandPool(commandPool), m_GraphicsQueue(graphicsQueue),
              m_Instance(instance), m_Allocator(**allocator) {}

        std::pair<vma::UniqueImage, vma::UniqueAllocation>
        CreateImage(
            uint32_t width,
            uint32_t height,
            vk::Format format,
            vk::ImageTiling tiling,
            vk::ImageUsageFlags usage,
            vk::MemoryPropertyFlags properties) {
            vk::ImageCreateInfo imageInfo{
                .imageType = vk::ImageType::e2D,
                .format = format,
                .extent = {
                    .width = width,
                    .height = height,
                    .depth = 1
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = vk::SampleCountFlagBits::e1,
                .tiling = tiling,
                .usage = usage,
                .sharingMode = vk::SharingMode::eExclusive,
                .initialLayout = vk::ImageLayout::eUndefined
            };

            vma::AllocationCreateInfo allocInfo{
                .usage = vma::MemoryUsage::eAutoPreferDevice
            };

            return m_Allocator.createImageUnique(
                imageInfo, allocInfo
            ).value;
        }

        std::pair<vma::UniqueBuffer, vma::UniqueAllocation> CreateAndCopyBuffer(
            vk::DeviceSize size,
            const void *data) {
            vk::BufferCreateInfo bufferInfo{
                .size = size,
                .usage = vk::BufferUsageFlagBits::eTransferSrc,
                .sharingMode = vk::SharingMode::eExclusive
            };

            vma::AllocationCreateInfo allocInfo{
                .flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
                .usage = vma::MemoryUsage::eAuto,
            };

            auto [buffer, allocation] = m_Allocator.createBufferUnique(
                bufferInfo, allocInfo
            ).value;

            void *mappedData = m_Allocator.mapMemory(*allocation).value;
            std::memcpy(mappedData, data, size);
            m_Allocator.unmapMemory(*allocation);
            return {std::move(buffer), std::move(allocation)};
        }

        void CopyBufferToImageWithTransitions(
            vk::Buffer buffer,
            vk::Image image,
            uint32_t width,
            uint32_t height) {
            vk::CommandBufferAllocateInfo allocInfo{
                .commandPool = *m_CommandPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1
            };

            vk::raii::CommandBuffer commandBuffer = std::move(
                m_LogicalDevice->allocateCommandBuffers(allocInfo).value().front());

            vk::CommandBufferBeginInfo beginInfo{
                .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
            };

            vk::ImageMemoryBarrier toTransferBarrier{
                .srcAccessMask = vk::AccessFlagBits::eNone,
                .dstAccessMask = vk::AccessFlagBits::eTransferWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
                .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
                .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
                .image = image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            vk::BufferImageCopy region{
                .bufferOffset = 0,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .imageOffset = {0, 0, 0},
                .imageExtent = {width, height, 1}
            };

            vk::ImageMemoryBarrier toShaderBarrier{
                .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
                .dstAccessMask = vk::AccessFlagBits::eShaderRead,
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
                .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
                .image = image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            commandBuffer.begin(beginInfo);

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eTransfer,
                {},
                {},
                {},
                toTransferBarrier
            );

            commandBuffer.copyBufferToImage(
                buffer,
                image,
                vk::ImageLayout::eTransferDstOptimal,
                region
            );

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eFragmentShader,
                {},
                {},
                {},
                toShaderBarrier
            );

            commandBuffer.end();

            vk::SubmitInfo submitInfo{
                .commandBufferCount = 1,
                .pCommandBuffers = &*commandBuffer
            };

            m_GraphicsQueue->submit(submitInfo);
            m_GraphicsQueue->waitIdle();
        }

        vk::UniqueImageView CreateImageView(
            vk::Image image,
            vk::Format format) {
            vk::ImageViewCreateInfo viewInfo{
                .image = image,
                .viewType = vk::ImageViewType::e2D,
                .format = format,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            return (**m_LogicalDevice).createImageViewUnique(
                viewInfo
            ).value;
        }

        PixelImage CreatePixelImage(
            uint32_t width,
            uint32_t height,
            vk::Format format,
            const void *data) {
            auto [image, memory] = CreateImage(
                width, height, format,
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
                vk::MemoryPropertyFlagBits::eDeviceLocal
            );

            auto [buffer, stagingMemory] = CreateAndCopyBuffer(
                width * height * 4, data // Assuming 4 bytes per pixel for RGBA
            );

            CopyBufferToImageWithTransitions(*buffer, *image, width, height);

            vk::UniqueImageView imageView{
                CreateImageView(*image, format)
            };

            return PixelImage(
                std::move(image),
                std::move(memory),
                std::move(imageView),
                width, height
            );
        }

    private:
        vk::raii::PhysicalDevice* m_PhysicalDevice;
        vk::raii::Device* m_LogicalDevice;
        vk::raii::CommandPool* m_CommandPool;
        vk::raii::Queue* m_GraphicsQueue;
        vk::Instance m_Instance;
        vma::Allocator m_Allocator;

        uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
            vk::PhysicalDeviceMemoryProperties memProperties = m_PhysicalDevice->getMemoryProperties();

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

            throw std::runtime_error("failed to find suitable memory type!");
        }
    };

    export vk::UniqueSampler CreateSimpleImageSampler(vk::Device logicalDevice) {
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

        return logicalDevice.createSamplerUnique(samplerInfo).value;
    }
}

namespace EasyGui {
    export class CPUImageData {
    public:
        static std::optional<CPUImageData> LoadFromFile(const std::filesystem::path& path) {
            if (!std::filesystem::exists(path)) {
                return std::nullopt;
            }

            CPUImageData imageData(path);
            if (imageData.m_Width <= 0 || imageData.m_Height <= 0 || imageData.m_Channels <= 0 || !imageData.m_Data) {
                return std::nullopt;
            }

            return imageData;
        }

        int GetWidth() const {
            return m_Width;
        }

        int GetHeight() const {
            return m_Height;
        }

        int GetChannels() const {
            return m_Channels;
        }

        const unsigned char* GetData() const {
            return m_Data.get();
        }

        CPUImageData() = default;

    private:
        CPUImageData(const std::filesystem::path& path) {
            m_Data.reset(stbi_load(
                path.string().c_str(),
                &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha
            ));
        }

        struct STBDeleteType {
            void operator()(unsigned char* data) const {
                stbi_image_free(data);
            }
        };

        std::unique_ptr<unsigned char, STBDeleteType> m_Data;
        int m_Width = 0;
        int m_Height = 0;
        int m_Channels = 0;

    };
}