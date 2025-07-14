export module EasyGui.Vulkan.Image;

import EasyGui.Lib;
import EasyGui;

namespace EasyGui::Vulkan {
    export class ImGuiImage {
    public:
        ImGuiImage() = default;

        ImGuiImage(ImTextureID &&textureId)
            : m_TextureId(std::exchange(textureId, 0)) {}

        ImGuiImage(const ImGuiImage &) = delete;

        ImGuiImage &operator=(const ImGuiImage &) = delete;

        ImGuiImage(ImGuiImage &&other) noexcept
            : m_TextureId(std::exchange(other.m_TextureId, 0)) {}

        ImGuiImage &operator=(ImGuiImage &&other) noexcept {
            if (this != &other) {
                std::swap(m_TextureId, other.m_TextureId);
            }
            return *this;
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

    private:
        ImTextureID m_TextureId = 0;
    };

    export class PixelImage {
    public:
        PixelImage(vk::UniqueImage &&image,
                   vk::UniqueDeviceMemory &&memory,
                   vk::UniqueImageView &&imageView)
            : m_Image(std::move(image)), m_Memory(std::move(memory)), m_ImageView(std::move(imageView)) {}

        const vk::UniqueImage &GetImage() const {
            return m_Image;
        }

        const vk::UniqueDeviceMemory &GetMemory() const {
            return m_Memory;
        }

        const vk::UniqueImageView &GetImageView() const {
            return m_ImageView;
        }

        ImGuiImage CreateImGuiImage(vk::Sampler sampler) const {
            VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(
                sampler,
                m_ImageView.get(),
                static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)
            );

            return {reinterpret_cast<ImTextureID>(descriptorSet)};
        }

    private:
        vk::UniqueImage m_Image;
        vk::UniqueDeviceMemory m_Memory;
        vk::UniqueImageView m_ImageView;
    };


    export class ImageHelper {
    public:
        ImageHelper(vk::raii::PhysicalDevice *physicalDevice, vk::raii::Device *logicalDevice,
                    vk::raii::CommandPool *commandPool,
                    vk::raii::Queue *graphicsQueue,
                    vk::raii::Instance *instance)
            : m_PhysicalDevice(physicalDevice), m_LogicalDevice(logicalDevice),
              m_CommandPool(commandPool), m_GraphicsQueue(graphicsQueue),
              m_Instance(instance) {}

        std::pair<vk::UniqueImage, vk::UniqueDeviceMemory> CreateImage(
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

            auto image = (**m_LogicalDevice).createImageUnique(imageInfo).value;

            vk::ImageMemoryRequirementsInfo2 memReqInfo{
                .image = *image
            };

            vk::MemoryRequirements2 memReqs = m_LogicalDevice->getImageMemoryRequirements2(memReqInfo);

            vk::MemoryAllocateInfo allocInfo{
                .allocationSize = memReqs.memoryRequirements.size,
                .memoryTypeIndex = FindMemoryType(memReqs.memoryRequirements.memoryTypeBits, properties)
            };

            vk::UniqueDeviceMemory imageMemory = (**m_LogicalDevice).allocateMemoryUnique(allocInfo).value;

            vk::BindImageMemoryInfo bindInfo{
                .image = *image,
                .memory = *imageMemory
            };

            m_LogicalDevice->bindImageMemory2(bindInfo);

            return {
                std::move(image),
                std::move(imageMemory)
            };
        }

        std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory> CreateAndCopyBuffer(
            vk::DeviceSize size,
            const void *data) {
            vk::BufferCreateInfo bufferInfo{
                .size = size,
                .usage = vk::BufferUsageFlagBits::eTransferSrc,
                .sharingMode = vk::SharingMode::eExclusive
            };

            vk::UniqueBuffer stagingBuffer = (**m_LogicalDevice).createBufferUnique(bufferInfo).value;

            vk::MemoryRequirements memRequirements = (**m_LogicalDevice).getBufferMemoryRequirements(*stagingBuffer);

            vk::MemoryAllocateInfo allocInfo{
                .allocationSize = memRequirements.size,
                .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits,
                                                  vk::MemoryPropertyFlagBits::eHostVisible |
                                                  vk::MemoryPropertyFlagBits::eHostCoherent)
            };

            vk::UniqueDeviceMemory stagingBufferMemory = (**m_LogicalDevice).allocateMemoryUnique(allocInfo).value;

            // stagingBuffer.bindMemory(*stagingBufferMemory, 0);
            m_LogicalDevice->bindBufferMemory2({
                vk::BindBufferMemoryInfo{
                    .buffer = *stagingBuffer,
                    .memory = *stagingBufferMemory
                }
            });

            vk::MemoryMapInfo mapInfo{
                .memory = *stagingBufferMemory,
                .size = size
            };

            void *mappedTarget = m_LogicalDevice->mapMemory2(mapInfo);
            std::memcpy(mappedTarget, data, size);

            (**m_LogicalDevice).unmapMemory(*stagingBufferMemory);

            return {
                std::move(stagingBuffer),
                std::move(stagingBufferMemory)
            };
        }

        void CopyBufferToImageWithTransitions(
            vk::UniqueBuffer &buffer,
            vk::UniqueImage &image,
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
                .image = *image,
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
                .image = *image,
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
                *buffer,
                *image,
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
            vk::UniqueImage &image,
            vk::Format format) {
            vk::ImageViewCreateInfo viewInfo{
                .image = *image,
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

            CopyBufferToImageWithTransitions(buffer, image, width, height);

            vk::UniqueImageView imageView{
                CreateImageView(image, format)
            };

            return PixelImage(
                std::move(image),
                std::move(memory),
                std::move(imageView)
            );
        }

    private:
        vk::raii::PhysicalDevice *m_PhysicalDevice;
        vk::raii::Device *m_LogicalDevice;
        vk::raii::CommandPool *m_CommandPool;
        vk::raii::Queue *m_GraphicsQueue;
        vk::raii::Instance *m_Instance;

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

    export vk::raii::Sampler CreateSimpleImageSampler(const vk::raii::Device &logicalDevice) {
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

        return logicalDevice.createSampler(samplerInfo).value();
    }
}
