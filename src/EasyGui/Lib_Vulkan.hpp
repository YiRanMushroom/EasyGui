#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>
namespace VULKAN_HPP_NAMESPACE {
    template class UniqueHandle<Buffer, VMA_HPP_NAMESPACE::Dispatcher>;
    template class UniqueHandle<Image, VMA_HPP_NAMESPACE::Dispatcher>;
    template class UniqueHandle<VMA_HPP_NAMESPACE::Allocator, VMA_HPP_NAMESPACE::Dispatcher>;
    template class UniqueHandle<VMA_HPP_NAMESPACE::Pool, VMA_HPP_NAMESPACE::Dispatcher>;
    template class UniqueHandle<VMA_HPP_NAMESPACE::Allocation, VMA_HPP_NAMESPACE::Dispatcher>;
    template class UniqueHandle<VMA_HPP_NAMESPACE::VirtualAllocation, VMA_HPP_NAMESPACE::Dispatcher>;
    template class UniqueHandle<VMA_HPP_NAMESPACE::VirtualBlock, VMA_HPP_NAMESPACE::Dispatcher>;
}

#include "Vulkan/vk_mem_alloc_raii.hpp"

#undef max
#undef min