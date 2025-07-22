#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>
export namespace VMA_HPP_NAMESPACE {
    template class VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Buffer, VMA_HPP_NAMESPACE::Dispatcher>;
    template class VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Image, VMA_HPP_NAMESPACE::Dispatcher>;
    template class VULKAN_HPP_NAMESPACE::UniqueHandle<VMA_HPP_NAMESPACE::Allocator, VMA_HPP_NAMESPACE::Dispatcher>;
    template class VULKAN_HPP_NAMESPACE::UniqueHandle<VMA_HPP_NAMESPACE::Pool, VMA_HPP_NAMESPACE::Dispatcher>;
    template class VULKAN_HPP_NAMESPACE::UniqueHandle<VMA_HPP_NAMESPACE::Allocation, VMA_HPP_NAMESPACE::Dispatcher>;
    template class VULKAN_HPP_NAMESPACE::UniqueHandle<VMA_HPP_NAMESPACE::VirtualAllocation, VMA_HPP_NAMESPACE::Dispatcher>;
    template class VULKAN_HPP_NAMESPACE::UniqueHandle<VMA_HPP_NAMESPACE::VirtualBlock, VMA_HPP_NAMESPACE::Dispatcher>;

    template class VMA_HPP_NAMESPACE::Deleter<class VULKAN_HPP_NAMESPACE::Image,class VMA_HPP_NAMESPACE::Allocator>;
    template class VMA_HPP_NAMESPACE::Deleter<class VULKAN_HPP_NAMESPACE::Buffer,class VMA_HPP_NAMESPACE::Allocator>;
    template class VMA_HPP_NAMESPACE::Deleter<class VMA_HPP_NAMESPACE::Pool,class VMA_HPP_NAMESPACE::Allocator>;
    template class VMA_HPP_NAMESPACE::Deleter<class VMA_HPP_NAMESPACE::Allocation,class VMA_HPP_NAMESPACE::Allocator>;
}

#undef max
#undef min