#pragma once

#include <Innovator/Defines.h>

#include <array>
#include <utility>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include <cstring>

#ifdef VK_USE_PLATFORM_XCB_KHR
#include <xcb/xcb.h>
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include <dlfcn.h>
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#pragma warning(disable : 26812)
#endif
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>


class VulkanAPI {
public:
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	void* libvulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
	HINSTANCE libvulkan = LoadLibrary("vulkan-1.dll");
#define dlsym GetProcAddress
#endif

	// VK_core
	PFN_vkCreateInstance CreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dlsym(libvulkan, "vkCreateInstance"));
	PFN_vkDestroyInstance DestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(dlsym(libvulkan, "vkDestroyInstance"));
	PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(dlsym(libvulkan, "vkEnumeratePhysicalDevices"));
	PFN_vkGetPhysicalDeviceFeatures GetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(dlsym(libvulkan, "vkGetPhysicalDeviceFeatures"));
	PFN_vkGetPhysicalDeviceFormatProperties GetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceFormatProperties"));
	PFN_vkGetPhysicalDeviceImageFormatProperties GetPhysicalDeviceImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceImageFormatProperties"));
	PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceProperties"));
	PFN_vkGetPhysicalDeviceQueueFamilyProperties GetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceQueueFamilyProperties"));
	PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceMemoryProperties"));
	PFN_vkGetInstanceProcAddr GetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(libvulkan, "vkGetInstanceProcAddr"));
	PFN_vkGetDeviceProcAddr GetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(dlsym(libvulkan, "vkGetDeviceProcAddr"));
	PFN_vkCreateDevice CreateDevice = reinterpret_cast<PFN_vkCreateDevice>(dlsym(libvulkan, "vkCreateDevice"));
	PFN_vkDestroyDevice DestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(dlsym(libvulkan, "vkDestroyDevice"));
	PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(dlsym(libvulkan, "vkEnumerateInstanceExtensionProperties"));
	PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(dlsym(libvulkan, "vkEnumerateDeviceExtensionProperties"));
	PFN_vkEnumerateInstanceLayerProperties EnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(dlsym(libvulkan, "vkEnumerateInstanceLayerProperties"));
	PFN_vkEnumerateDeviceLayerProperties EnumerateDeviceLayerProperties = reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(dlsym(libvulkan, "vkEnumerateDeviceLayerProperties"));
	PFN_vkGetDeviceQueue GetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(dlsym(libvulkan, "vkGetDeviceQueue"));
	PFN_vkQueueSubmit QueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(dlsym(libvulkan, "vkQueueSubmit"));
	PFN_vkQueueWaitIdle QueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(dlsym(libvulkan, "vkQueueWaitIdle"));
	PFN_vkDeviceWaitIdle DeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(dlsym(libvulkan, "vkDeviceWaitIdle"));
	PFN_vkAllocateMemory AllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(dlsym(libvulkan, "vkAllocateMemory"));
	PFN_vkFreeMemory FreeMemory = reinterpret_cast<PFN_vkFreeMemory>(dlsym(libvulkan, "vkFreeMemory"));
	PFN_vkMapMemory MapMemory = reinterpret_cast<PFN_vkMapMemory>(dlsym(libvulkan, "vkMapMemory"));
	PFN_vkUnmapMemory UnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(dlsym(libvulkan, "vkUnmapMemory"));
	PFN_vkFlushMappedMemoryRanges FlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(dlsym(libvulkan, "vkFlushMappedMemoryRanges"));
	PFN_vkInvalidateMappedMemoryRanges InvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(dlsym(libvulkan, "vkInvalidateMappedMemoryRanges"));
	PFN_vkGetDeviceMemoryCommitment GetDeviceMemoryCommitment = reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(dlsym(libvulkan, "vkGetDeviceMemoryCommitment"));
	PFN_vkBindBufferMemory BindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(dlsym(libvulkan, "vkBindBufferMemory"));
	PFN_vkBindImageMemory BindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(dlsym(libvulkan, "vkBindImageMemory"));
	PFN_vkGetBufferMemoryRequirements GetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(dlsym(libvulkan, "vkGetBufferMemoryRequirements"));
	PFN_vkGetImageMemoryRequirements GetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(dlsym(libvulkan, "vkGetImageMemoryRequirements"));
	PFN_vkGetImageSparseMemoryRequirements GetImageSparseMemoryRequirements = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements>(dlsym(libvulkan, "vkGetImageSparseMemoryRequirements"));
	PFN_vkGetPhysicalDeviceSparseImageFormatProperties GetPhysicalDeviceSparseImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>(dlsym(libvulkan, "vk.GetPhysicalDeviceSparseImageFormatProperties"));
	PFN_vkQueueBindSparse QueueBindSparse = reinterpret_cast<PFN_vkQueueBindSparse>(dlsym(libvulkan, "vkQueueBindSparse"));
	PFN_vkCreateFence CreateFence = reinterpret_cast<PFN_vkCreateFence>(dlsym(libvulkan, "vkCreateFence"));
	PFN_vkDestroyFence DestroyFence = reinterpret_cast<PFN_vkDestroyFence>(dlsym(libvulkan, "vkDestroyFence"));
	PFN_vkResetFences ResetFences = reinterpret_cast<PFN_vkResetFences>(dlsym(libvulkan, "vkResetFences"));
	PFN_vkGetFenceStatus GetFenceStatus = reinterpret_cast<PFN_vkGetFenceStatus>(dlsym(libvulkan, "vkGetFenceStatus"));
	PFN_vkWaitForFences WaitForFences = reinterpret_cast<PFN_vkWaitForFences>(dlsym(libvulkan, "vkWaitForFences"));
	PFN_vkCreateSemaphore CreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(dlsym(libvulkan, "vkCreateSemaphore"));
	PFN_vkDestroySemaphore DestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(dlsym(libvulkan, "vkDestroySemaphore"));
	PFN_vkCreateEvent CreateEvent = reinterpret_cast<PFN_vkCreateEvent>(dlsym(libvulkan, "vkCreateEvent"));
	PFN_vkDestroyEvent DestroyEvent = reinterpret_cast<PFN_vkDestroyEvent>(dlsym(libvulkan, "vkDestroyEvent"));
	PFN_vkGetEventStatus GetEventStatus = reinterpret_cast<PFN_vkGetEventStatus>(dlsym(libvulkan, "vkGetEventStatus"));
	PFN_vkSetEvent SetEvent = reinterpret_cast<PFN_vkSetEvent>(dlsym(libvulkan, "vkSetEvent"));
	PFN_vkResetEvent ResetEvent = reinterpret_cast<PFN_vkResetEvent>(dlsym(libvulkan, "vkResetEvent"));
	PFN_vkCreateQueryPool CreateQueryPool = reinterpret_cast<PFN_vkCreateQueryPool>(dlsym(libvulkan, "vkCreateQueryPool"));
	PFN_vkDestroyQueryPool DestroyQueryPool = reinterpret_cast<PFN_vkDestroyQueryPool>(dlsym(libvulkan, "vkDestroyQueryPool"));
	PFN_vkGetQueryPoolResults GetQueryPoolResults = reinterpret_cast<PFN_vkGetQueryPoolResults>(dlsym(libvulkan, "vkGetQueryPoolResults"));
	PFN_vkCreateBuffer CreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(dlsym(libvulkan, "vkCreateBuffer"));
	PFN_vkDestroyBuffer DestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(dlsym(libvulkan, "vkDestroyBuffer"));
	PFN_vkCreateBufferView CreateBufferView = reinterpret_cast<PFN_vkCreateBufferView>(dlsym(libvulkan, "vkCreateBufferView"));
	PFN_vkDestroyBufferView DestroyBufferView = reinterpret_cast<PFN_vkDestroyBufferView>(dlsym(libvulkan, "vkDestroyBufferView"));
	PFN_vkCreateImage CreateImage = reinterpret_cast<PFN_vkCreateImage>(dlsym(libvulkan, "vkCreateImage"));
	PFN_vkDestroyImage DestroyImage = reinterpret_cast<PFN_vkDestroyImage>(dlsym(libvulkan, "vkDestroyImage"));
	PFN_vkGetImageSubresourceLayout GetImageSubresourceLayout = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(dlsym(libvulkan, "vkGetImageSubresourceLayout"));
	PFN_vkCreateImageView CreateImageView = reinterpret_cast<PFN_vkCreateImageView>(dlsym(libvulkan, "vkCreateImageView"));
	PFN_vkDestroyImageView DestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(dlsym(libvulkan, "vkDestroyImageView"));
	PFN_vkCreateShaderModule CreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(dlsym(libvulkan, "vkCreateShaderModule"));
	PFN_vkDestroyShaderModule DestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(dlsym(libvulkan, "vkDestroyShaderModule"));
	PFN_vkCreatePipelineCache CreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(dlsym(libvulkan, "vkCreatePipelineCache"));
	PFN_vkDestroyPipelineCache DestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(dlsym(libvulkan, "vkDestroyPipelineCache"));
	PFN_vkGetPipelineCacheData GetPipelineCacheData = reinterpret_cast<PFN_vkGetPipelineCacheData>(dlsym(libvulkan, "vkGetPipelineCacheData"));
	PFN_vkMergePipelineCaches MergePipelineCaches = reinterpret_cast<PFN_vkMergePipelineCaches>(dlsym(libvulkan, "vkMergePipelineCaches"));
	PFN_vkCreateGraphicsPipelines CreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(dlsym(libvulkan, "vkCreateGraphicsPipelines"));
	PFN_vkCreateComputePipelines CreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(dlsym(libvulkan, "vkCreateComputePipelines"));
	PFN_vkDestroyPipeline DestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(dlsym(libvulkan, "vkDestroyPipeline"));
	PFN_vkCreatePipelineLayout CreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(dlsym(libvulkan, "vkCreatePipelineLayout"));
	PFN_vkDestroyPipelineLayout DestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(dlsym(libvulkan, "vkDestroyPipelineLayout"));
	PFN_vkCreateSampler CreateSampler = reinterpret_cast<PFN_vkCreateSampler>(dlsym(libvulkan, "vkCreateSampler"));
	PFN_vkDestroySampler DestroySampler = reinterpret_cast<PFN_vkDestroySampler>(dlsym(libvulkan, "vkDestroySampler"));
	PFN_vkCreateDescriptorSetLayout CreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(dlsym(libvulkan, "vkCreateDescriptorSetLayout"));
	PFN_vkDestroyDescriptorSetLayout DestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(dlsym(libvulkan, "vkDestroyDescriptorSetLayout"));
	PFN_vkCreateDescriptorPool CreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(dlsym(libvulkan, "vkCreateDescriptorPool"));
	PFN_vkDestroyDescriptorPool DestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(dlsym(libvulkan, "vkDestroyDescriptorPool"));
	PFN_vkResetDescriptorPool ResetDescriptorPool = reinterpret_cast<PFN_vkResetDescriptorPool>(dlsym(libvulkan, "vkResetDescriptorPool"));
	PFN_vkAllocateDescriptorSets AllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(dlsym(libvulkan, "vkAllocateDescriptorSets"));
	PFN_vkFreeDescriptorSets FreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(dlsym(libvulkan, "vkFreeDescriptorSets"));
	PFN_vkUpdateDescriptorSets UpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(dlsym(libvulkan, "vkUpdateDescriptorSets"));
	PFN_vkCreateFramebuffer CreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(dlsym(libvulkan, "vkCreateFramebuffer"));
	PFN_vkDestroyFramebuffer DestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(dlsym(libvulkan, "vkDestroyFramebuffer"));
	PFN_vkCreateRenderPass CreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(dlsym(libvulkan, "vkCreateRenderPass"));
	PFN_vkDestroyRenderPass DestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(dlsym(libvulkan, "vkDestroyRenderPass"));
	PFN_vkGetRenderAreaGranularity GetRenderAreaGranularity = reinterpret_cast<PFN_vkGetRenderAreaGranularity>(dlsym(libvulkan, "vkGetRenderAreaGranularity"));
	PFN_vkCreateCommandPool CreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(dlsym(libvulkan, "vkCreateCommandPool"));
	PFN_vkDestroyCommandPool DestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(dlsym(libvulkan, "vkDestroyCommandPool"));
	PFN_vkResetCommandPool ResetCommandPool = reinterpret_cast<PFN_vkResetCommandPool>(dlsym(libvulkan, "vkResetCommandPool"));
	PFN_vkAllocateCommandBuffers AllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(dlsym(libvulkan, "vkAllocateCommandBuffers"));
	PFN_vkFreeCommandBuffers FreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(dlsym(libvulkan, "vkFreeCommandBuffers"));
	PFN_vkBeginCommandBuffer BeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(dlsym(libvulkan, "vkBeginCommandBuffer"));
	PFN_vkEndCommandBuffer EndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(dlsym(libvulkan, "vkEndCommandBuffer"));
	PFN_vkResetCommandBuffer ResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(dlsym(libvulkan, "vkResetCommandBuffer"));
	PFN_vkCmdBindPipeline CmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(dlsym(libvulkan, "vkCmdBindPipeline"));
	PFN_vkCmdSetViewport CmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(dlsym(libvulkan, "vkCmdSetViewport"));
	PFN_vkCmdSetScissor CmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(dlsym(libvulkan, "vkCmdSetScissor"));
	PFN_vkCmdSetLineWidth CmdSetLineWidth = reinterpret_cast<PFN_vkCmdSetLineWidth>(dlsym(libvulkan, "vkCmdSetLineWidth"));
	PFN_vkCmdSetDepthBias CmdSetDepthBias = reinterpret_cast<PFN_vkCmdSetDepthBias>(dlsym(libvulkan, "vkCmdSetDepthBias"));
	PFN_vkCmdSetBlendConstants CmdSetBlendConstants = reinterpret_cast<PFN_vkCmdSetBlendConstants>(dlsym(libvulkan, "vkCmdSetBlendConstants"));
	PFN_vkCmdSetDepthBounds CmdSetDepthBounds = reinterpret_cast<PFN_vkCmdSetDepthBounds>(dlsym(libvulkan, "vkCmdSetDepthBounds"));
	PFN_vkCmdSetStencilCompareMask CmdSetStencilCompareMask = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(dlsym(libvulkan, "vkCmdSetStencilCompareMask"));
	PFN_vkCmdSetStencilWriteMask CmdSetStencilWriteMask = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(dlsym(libvulkan, "vkCmdSetStencilWriteMask"));
	PFN_vkCmdSetStencilReference CmdSetStencilReference = reinterpret_cast<PFN_vkCmdSetStencilReference>(dlsym(libvulkan, "vkCmdSetStencilReference"));
	PFN_vkCmdBindDescriptorSets CmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(dlsym(libvulkan, "vkCmdBindDescriptorSets"));
	PFN_vkCmdBindIndexBuffer CmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(dlsym(libvulkan, "vkCmdBindIndexBuffer"));
	PFN_vkCmdBindVertexBuffers CmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(dlsym(libvulkan, "vkCmdBindVertexBuffers"));
	PFN_vkCmdDraw CmdDraw = reinterpret_cast<PFN_vkCmdDraw>(dlsym(libvulkan, "vkCmdDraw"));
	PFN_vkCmdDrawIndexed CmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(dlsym(libvulkan, "vkCmdDrawIndexed"));
	PFN_vkCmdDrawIndirect CmdDrawIndirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(dlsym(libvulkan, "vkCmdDrawIndirect"));
	PFN_vkCmdDrawIndexedIndirect CmdDrawIndexedIndirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(dlsym(libvulkan, "vkCmdDrawIndexedIndirect"));
	PFN_vkCmdDispatch CmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(dlsym(libvulkan, "vkCmdDispatch"));
	PFN_vkCmdDispatchIndirect CmdDispatchIndirect = reinterpret_cast<PFN_vkCmdDispatchIndirect>(dlsym(libvulkan, "vkCmdDispatchIndirect"));
	PFN_vkCmdCopyBuffer CmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(dlsym(libvulkan, "vkCmdCopyBuffer"));
	PFN_vkCmdCopyImage CmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(dlsym(libvulkan, "vkCmdCopyImage"));
	PFN_vkCmdBlitImage CmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(dlsym(libvulkan, "vkCmdBlitImage"));
	PFN_vkCmdCopyBufferToImage CmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(dlsym(libvulkan, "vkCmdCopyBufferToImage"));
	PFN_vkCmdCopyImageToBuffer CmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(dlsym(libvulkan, "vkCmdCopyImageToBuffer"));
	PFN_vkCmdUpdateBuffer CmdUpdateBuffer = reinterpret_cast<PFN_vkCmdUpdateBuffer>(dlsym(libvulkan, "vkCmdUpdateBuffer"));
	PFN_vkCmdFillBuffer CmdFillBuffer = reinterpret_cast<PFN_vkCmdFillBuffer>(dlsym(libvulkan, "vkCmdFillBuffer"));
	PFN_vkCmdClearColorImage CmdClearColorImage = reinterpret_cast<PFN_vkCmdClearColorImage>(dlsym(libvulkan, "vkCmdClearColorImage"));
	PFN_vkCmdClearDepthStencilImage CmdClearDepthStencilImage = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(dlsym(libvulkan, "vkCmdClearDepthStencilImage"));
	PFN_vkCmdClearAttachments CmdClearAttachments = reinterpret_cast<PFN_vkCmdClearAttachments>(dlsym(libvulkan, "vkCmdClearAttachments"));
	PFN_vkCmdResolveImage CmdResolveImage = reinterpret_cast<PFN_vkCmdResolveImage>(dlsym(libvulkan, "vkCmdResolveImage"));
	PFN_vkCmdSetEvent CmdSetEvent = reinterpret_cast<PFN_vkCmdSetEvent>(dlsym(libvulkan, "vkCmdSetEvent"));
	PFN_vkCmdResetEvent CmdResetEvent = reinterpret_cast<PFN_vkCmdResetEvent>(dlsym(libvulkan, "vkCmdResetEvent"));
	PFN_vkCmdWaitEvents CmdWaitEvents = reinterpret_cast<PFN_vkCmdWaitEvents>(dlsym(libvulkan, "vkCmdWaitEvents"));
	PFN_vkCmdPipelineBarrier CmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(dlsym(libvulkan, "vkCmdPipelineBarrier"));
	PFN_vkCmdBeginQuery CmdBeginQuery = reinterpret_cast<PFN_vkCmdBeginQuery>(dlsym(libvulkan, "vkCmdBeginQuery"));
	PFN_vkCmdEndQuery CmdEndQuery = reinterpret_cast<PFN_vkCmdEndQuery>(dlsym(libvulkan, "vkCmdEndQuery"));
	PFN_vkCmdResetQueryPool CmdResetQueryPool = reinterpret_cast<PFN_vkCmdResetQueryPool>(dlsym(libvulkan, "vkCmdResetQueryPool"));
	PFN_vkCmdWriteTimestamp CmdWriteTimestamp = reinterpret_cast<PFN_vkCmdWriteTimestamp>(dlsym(libvulkan, "vkCmdWriteTimestamp"));
	PFN_vkCmdCopyQueryPoolResults CmdCopyQueryPoolResults = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(dlsym(libvulkan, "vkCmdCopyQueryPoolResults"));
	PFN_vkCmdPushConstants CmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(dlsym(libvulkan, "vkCmdPushConstants"));
	PFN_vkCmdBeginRenderPass CmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(dlsym(libvulkan, "vkCmdBeginRenderPass"));
	PFN_vkCmdNextSubpass CmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(dlsym(libvulkan, "vkCmdNextSubpass"));
	PFN_vkCmdEndRenderPass CmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(dlsym(libvulkan, "vkCmdEndRenderPass"));
	PFN_vkCmdExecuteCommands CmdExecuteCommands = reinterpret_cast<PFN_vkCmdExecuteCommands>(dlsym(libvulkan, "vkCmdExecuteCommands"));


#ifdef VK_KHR_surface
	PFN_vkDestroySurfaceKHR DestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(dlsym(libvulkan, "vkDestroySurfaceKHR"));
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceSupportKHR"));
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
#endif

#ifdef VK_KHR_swapchain
	PFN_vkCreateSwapchainKHR CreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(dlsym(libvulkan, "vkCreateSwapchainKHR"));
	PFN_vkDestroySwapchainKHR DestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(dlsym(libvulkan, "vkDestroySwapchainKHR"));
	PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(dlsym(libvulkan, "vkGetSwapchainImagesKHR"));
	PFN_vkAcquireNextImageKHR AcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(dlsym(libvulkan, "vkAcquireNextImageKHR"));
	PFN_vkQueuePresentKHR QueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(dlsym(libvulkan, "vkQueuePresentKHR"));
#endif

#ifdef VK_KHR_display
	PFN_vkCreateDisplayModeKHR CreateDisplayModeKHR = reinterpret_cast<PFN_vkCreateDisplayModeKHR>(dlsym(libvulkan, "vkCreateDisplayModeKHR"));
	PFN_vkCreateDisplayPlaneSurfaceKHR CreateDisplayPlaneSurfaceKHR = reinterpret_cast<PFN_vkCreateDisplayPlaneSurfaceKHR>(dlsym(libvulkan, "vkCreateDisplayPlaneSurfaceKHR"));
	PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR = reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(dlsym(libvulkan, "vkGetDisplayModePropertiesKHR"));
	PFN_vkGetDisplayPlaneSupportedDisplaysKHR GetDisplayPlaneSupportedDisplaysKHR = reinterpret_cast<PFN_vkGetDisplayPlaneSupportedDisplaysKHR>(dlsym(libvulkan, "vkGetDisplayPlaneSupportedDisplaysKHR"));
	PFN_vkGetPhysicalDeviceDisplayPropertiesKHR GetPhysicalDeviceDisplayPropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceDisplayPropertiesKHR"));
	PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR GetPhysicalDeviceDisplayPlanePropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR"));
	PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR = reinterpret_cast<PFN_vkGetDisplayPlaneCapabilitiesKHR>(dlsym(libvulkan, "vkGetDisplayPlaneCapabilitiesKHR"));
#endif

#ifdef VK_KHR_display_swapchain
	PFN_vkCreateSharedSwapchainsKHR CreateSharedSwapchainsKHR = reinterpret_cast<PFN_vkCreateSharedSwapchainsKHR>(dlsym(libvulkan, "vkCreateSharedSwapchainsKHR"));
#endif

#ifdef VK_KHR_xlib_surface
	PFN_vkCreateXlibSurfaceKHR CreateXlibSurfaceKHR = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(dlsym(libvulkan, "vkCreateXlibSurfaceKHR"));
	PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR GetPhysicalDeviceXcbPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceXcbPresentationSupportKHR"));
#endif

#ifdef VK_KHR_xcb_surface
	PFN_vkCreateXcbSurfaceKHR CreateXcbSurfaceKHR = reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(dlsym(libvulkan, "vkCreateXcbSurfaceKHR"));
	PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR GetPhysicalDeviceXcbPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceXcbPresentationSupportKHR"));
#endif

#ifdef VK_KHR_wayland_surface
	PFN_vkCreateWaylandSurfaceKHR CreateWaylandSurfaceKHR = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(dlsym(libvulkan, "vkCreateWaylandSurfaceKHR"));
	PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR GetPhysicalDeviceWaylandPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceWaylandPresentationSupportKHR"));
#endif

#ifdef VK_KHR_android_surface
	PFN_vkCreateAndroidSurfaceKHR CreateAndroidSurfaceKHR = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(dlsym(libvulkan, "vkCreateAndroidSurfaceKHR"));
#endif

#ifdef VK_KHR_win32_surface
	PFN_vkCreateWin32SurfaceKHR CreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(dlsym(libvulkan, "vkCreateWin32SurfaceKHR"));
	PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR GetPhysicalDeviceWin32PresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));
#endif

#ifdef VK_KHR_get_physical_device_properties2
	PFN_vkGetPhysicalDeviceFeatures2KHR GetPhysicalDeviceFeatures2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(dlsym(libvulkan, "vkGetPhysicalDeviceFeatures2"));
	PFN_vkGetPhysicalDeviceProperties2 GetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(dlsym(libvulkan, "vkGetPhysicalDeviceProperties2"));
#endif
};


inline VulkanAPI vk;

class VulkanObject : public NonCopyable {
public:
	VulkanObject() = default;
	virtual std::string toString() = 0;
};

class VulkanPhysicalDevice {
public:
	VulkanPhysicalDevice(VulkanPhysicalDevice&& self) = default;
	VulkanPhysicalDevice(const VulkanPhysicalDevice& self) = default;
	VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&& self) = delete;
	VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice& self) = delete;
	~VulkanPhysicalDevice() = default;

	explicit VulkanPhysicalDevice(VkPhysicalDevice device) :
		device(device)
	{
		this->device_address_features.pNext = &this->ray_tracing_features;
		this->supported_features2.pNext = &this->device_address_features;
		this->properties2.pNext = &this->ray_tracing_properties;

		vk.GetPhysicalDeviceFeatures2(this->device, &this->supported_features2);
		vk.GetPhysicalDeviceProperties(this->device, &this->properties);
		vk.GetPhysicalDeviceProperties2(this->device, &this->properties2);

		vk.GetPhysicalDeviceMemoryProperties(this->device, &this->memory_properties);

		uint32_t count;
		vk.GetPhysicalDeviceQueueFamilyProperties(this->device, &count, nullptr);
		this->queue_family_properties.resize(count);
		vk.GetPhysicalDeviceQueueFamilyProperties(this->device, &count, this->queue_family_properties.data());

		THROW_ON_ERROR(vk.EnumerateDeviceLayerProperties(this->device, &count, nullptr));
		this->layer_properties.resize(count);
		THROW_ON_ERROR(vk.EnumerateDeviceLayerProperties(this->device, &count, this->layer_properties.data()));

		THROW_ON_ERROR(vk.EnumerateDeviceExtensionProperties(this->device, nullptr, &count, nullptr));
		this->extension_properties.resize(count);
		THROW_ON_ERROR(vk.EnumerateDeviceExtensionProperties(this->device, nullptr, &count, this->extension_properties.data()));
	}

	VkFormatProperties getFormatProperties(VkFormat format)
	{
		VkFormatProperties properties;
		vk.GetPhysicalDeviceFormatProperties(
			this->device,
			format,
			&properties);

		return properties;
	}

	std::vector<VkSparseImageFormatProperties> getSparseImageFormatProperties(
		VkFormat format,
		VkImageType type,
		VkSampleCountFlagBits samples,
		VkImageUsageFlags usage,
		VkImageTiling tiling)
	{
		uint32_t count;
		vk.GetPhysicalDeviceSparseImageFormatProperties(
			this->device,
			format,
			type,
			samples,
			usage,
			tiling,
			&count,
			nullptr);

		if (count == 0) {
			throw std::runtime_error("VulkanPhysicalDevice::getSparseImageFormatProperties: image format not supported for sparse binding");
		}

		std::vector<VkSparseImageFormatProperties> properties(count);
		vk.GetPhysicalDeviceSparseImageFormatProperties(
			this->device,
			format,
			type,
			samples,
			usage,
			tiling,
			&count,
			properties.data());

		return properties;
	}

	uint32_t getQueueIndex(VkQueueFlags required_flags, const std::vector<VkBool32>& filter)
	{
		// check for exact match of required flags
		for (uint32_t queue_index = 0; queue_index < this->queue_family_properties.size(); queue_index++) {
			if (this->queue_family_properties[queue_index].queueFlags == required_flags) {
				if (filter[queue_index]) {
					return queue_index;
				}
			}
		}
		// check for queue with all required flags set
		for (uint32_t queue_index = 0; queue_index < this->queue_family_properties.size(); queue_index++) {
			if ((this->queue_family_properties[queue_index].queueFlags & required_flags) == required_flags) {
				if (filter[queue_index]) {
					return queue_index;
				}
			}
		}
		throw std::runtime_error("VulkanDevice::getQueueIndex: could not find queue with required properties");
	}

	uint32_t getMemoryTypeIndex(uint32_t memory_type, VkMemoryPropertyFlags required_flags) const
	{
		for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
			if (((memory_type >> i) & 1) == 1) { // check for required memory type
				if ((this->memory_properties.memoryTypes[i].propertyFlags & required_flags) == required_flags) {
					return i;
				}
			}
		}
		throw std::runtime_error("VulkanDevice::getMemoryTypeIndex: could not find suitable memory type");
	}

	template<typename T>
	bool supportsFeatures(const T* supported_features, const T* required_features) const
	{
		const size_t struct_size = sizeof(T);
		const size_t feature_count = struct_size / sizeof(VkBool32);

		std::array<VkBool32, feature_count> required;
		std::array<VkBool32, feature_count> supported;

		memcpy(&required, required_features, struct_size);
		memcpy(&supported, supported_features, struct_size);

		for (uint32_t i = 0; i < feature_count; i++) {
			if (required[i] && !supported[i]) {
				return false;
			}
		}
		return true;
	}

	bool supportsFeatures(const VkPhysicalDeviceFeatures2& required_features2) const
	{
		return this->supportsFeatures(&this->supported_features2.features, &required_features2.features);
	}

	VkPhysicalDevice device;

	// Provided by VK_KHR_buffer_device_address
	VkPhysicalDeviceBufferDeviceAddressFeatures device_address_features{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT
	};

	VkPhysicalDeviceRayTracingPropertiesKHR ray_tracing_properties{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_KHR
	};

	VkPhysicalDeviceRayTracingFeaturesKHR ray_tracing_features{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_FEATURES_KHR
	};

	VkPhysicalDeviceFeatures2 supported_features2{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR
	};

	VkPhysicalDeviceProperties2 properties2{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR
	};

	VkPhysicalDeviceProperties properties{};
	VkPhysicalDeviceMemoryProperties memory_properties{};
	std::vector<VkQueueFamilyProperties> queue_family_properties;
	std::vector<VkExtensionProperties> extension_properties;
	std::vector<VkLayerProperties> layer_properties;
};


static VkBool32 DebugUtilsCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_type,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* user_data)
{
	if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		std::cout << "Warning: " << callback_data->messageIdNumber << ":" << callback_data->pMessageIdName << ":" << callback_data->pMessage << std::endl;
	}
	else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		std::cerr << "Error: " << callback_data->messageIdNumber << ":" << callback_data->pMessageIdName << ":" << callback_data->pMessage << std::endl;
	}
	return VK_FALSE;
}

class VulkanInstance : public VulkanObject {
public:
	explicit VulkanInstance(
		const std::string& application_name = "",
		const std::vector<const char*>& required_layers = {},
		const std::vector<const char*>& required_extensions = {})
	{
		uint32_t layer_count;
		THROW_ON_ERROR(vk.EnumerateInstanceLayerProperties(&layer_count, nullptr));
		std::vector<VkLayerProperties> layer_properties(layer_count);
		THROW_ON_ERROR(vk.EnumerateInstanceLayerProperties(&layer_count, layer_properties.data()));

		std::for_each(required_layers.begin(), required_layers.end(), [&](const char* layer_name) {
			for (const auto& properties : layer_properties)
				if (std::strcmp(layer_name, properties.layerName) == 0)
					return;
			throw std::runtime_error("Required instance layer " + std::string(layer_name) + " not supported.");
			});

		uint32_t extension_count;
		THROW_ON_ERROR(vk.EnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));
		std::vector<VkExtensionProperties> extension_properties(extension_count);
		THROW_ON_ERROR(vk.EnumerateInstanceExtensionProperties(nullptr, &extension_count, extension_properties.data()));

		std::for_each(required_extensions.begin(), required_extensions.end(), [&](const char* extension_name) {
			for (const auto& properties : extension_properties)
				if (std::strcmp(extension_name, properties.extensionName) == 0)
					return;
			throw std::runtime_error("Required instance extension " + std::string(extension_name) + " not supported.");
			});

		VkApplicationInfo application_info{
		  .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		  .pNext = nullptr,
		  .pApplicationName = application_name.c_str(),
		  .applicationVersion = 1,
		  .pEngineName = "Innovator",
		  .engineVersion = 1,
		  .apiVersion = VK_API_VERSION_1_0,
		};

		VkInstanceCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pApplicationInfo = &application_info,
			.enabledLayerCount = static_cast<uint32_t>(required_layers.size()),
			.ppEnabledLayerNames = required_layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
			.ppEnabledExtensionNames = required_extensions.data()
		};

		THROW_ON_ERROR(vk.CreateInstance(&create_info, nullptr, &this->instance));

		uint32_t physical_device_count;
		THROW_ON_ERROR(vk.EnumeratePhysicalDevices(this->instance, &physical_device_count, nullptr));

		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		THROW_ON_ERROR(vk.EnumeratePhysicalDevices(this->instance, &physical_device_count, physical_devices.data()));

		for (const auto& physical_device : physical_devices) {
			this->physical_devices.emplace_back(physical_device);
		}

#ifdef VK_EXT_debug_report
		this->vkCreateDebugReportCallbackEXT = this->getProcAddress<PFN_vkCreateDebugReportCallbackEXT>("vkCreateDebugReportCallbackEXT");
		this->vkDestroyDebugReportCallbackEXT = this->getProcAddress<PFN_vkDestroyDebugReportCallbackEXT>("vkDestroyDebugReportCallbackEXT");
#endif
#ifdef VK_KHR_ray_tracing
		this->vkBindAccelerationStructureMemoryKHR = this->getProcAddress<PFN_vkBindAccelerationStructureMemoryKHR>("vkBindAccelerationStructureMemoryKHR");
		this->vkBuildAccelerationStructureKHR = this->getProcAddress<PFN_vkBuildAccelerationStructureKHR>("vkBuildAccelerationStructureKHR");
		this->vkCmdBuildAccelerationStructureIndirectKHR = this->getProcAddress<PFN_vkCmdBuildAccelerationStructureIndirectKHR>("vkCmdBuildAccelerationStructureIndirectKHR");
		this->vkCmdBuildAccelerationStructureKHR = this->getProcAddress<PFN_vkCmdBuildAccelerationStructureKHR>("vkCmdBuildAccelerationStructureKHR");
		this->vkCmdCopyAccelerationStructureKHR = this->getProcAddress<PFN_vkCmdCopyAccelerationStructureKHR>("vkCmdCopyAccelerationStructureKHR");
		this->vkCmdCopyAccelerationStructureToMemoryKHR = this->getProcAddress<PFN_vkCmdCopyAccelerationStructureToMemoryKHR>("vkCmdCopyAccelerationStructureToMemoryKHR");
		this->vkCmdCopyMemoryToAccelerationStructureKHR = this->getProcAddress<PFN_vkCmdCopyMemoryToAccelerationStructureKHR>("vkCmdCopyMemoryToAccelerationStructureKHR");
		this->vkCmdTraceRaysIndirectKHR = this->getProcAddress<PFN_vkCmdTraceRaysIndirectKHR>("vkCmdTraceRaysIndirectKHR");
		this->vkCmdTraceRaysKHR = this->getProcAddress<PFN_vkCmdTraceRaysKHR>("vkCmdTraceRaysKHR");
		this->vkCmdWriteAccelerationStructuresPropertiesKHR = this->getProcAddress<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>("vkCmdWriteAccelerationStructuresPropertiesKHR");
		this->vkCopyAccelerationStructureKHR = this->getProcAddress<PFN_vkCopyAccelerationStructureKHR>("vkCopyAccelerationStructureKHR");
		this->vkCopyAccelerationStructureToMemoryKHR = this->getProcAddress<PFN_vkCopyAccelerationStructureToMemoryKHR>("vkCopyAccelerationStructureToMemoryKHR");
		this->vkCopyMemoryToAccelerationStructureKHR = this->getProcAddress<PFN_vkCopyMemoryToAccelerationStructureKHR>("vkCopyMemoryToAccelerationStructureKHR");
		this->vkCreateAccelerationStructureKHR = this->getProcAddress<PFN_vkCreateAccelerationStructureKHR>("vkCreateAccelerationStructureKHR");
		this->vkCreateRayTracingPipelinesKHR = this->getProcAddress<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR");
		this->vkDestroyAccelerationStructureKHR = this->getProcAddress<PFN_vkDestroyAccelerationStructureKHR>("vkDestroyAccelerationStructureKHR");
		this->vkGetAccelerationStructureDeviceAddressKHR = this->getProcAddress<PFN_vkGetAccelerationStructureDeviceAddressKHR>("vkGetAccelerationStructureDeviceAddressKHR");
		this->vkGetAccelerationStructureMemoryRequirementsKHR = this->getProcAddress<PFN_vkGetAccelerationStructureMemoryRequirementsKHR>("vkGetAccelerationStructureMemoryRequirementsKHR");
		this->vkGetDeviceAccelerationStructureCompatibilityKHR = this->getProcAddress<PFN_vkGetDeviceAccelerationStructureCompatibilityKHR>("vkGetDeviceAccelerationStructureCompatibilityKHR");
		this->vkGetRayTracingCaptureReplayShaderGroupHandlesKHR = this->getProcAddress<PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR>("vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
		this->vkGetRayTracingShaderGroupHandlesKHR = this->getProcAddress<PFN_vkGetRayTracingShaderGroupHandlesKHR>("vkGetRayTracingShaderGroupHandlesKHR");
		this->vkWriteAccelerationStructuresPropertiesKHR = this->getProcAddress<PFN_vkWriteAccelerationStructuresPropertiesKHR>("vkWriteAccelerationStructuresPropertiesKHR");
		this->vkGetBufferDeviceAddressKHR = this->getProcAddress<PFN_vkGetBufferDeviceAddressKHR>("vkGetBufferDeviceAddressKHR");
#endif
	}

	~VulkanInstance()
	{
		vk.DestroyInstance(this->instance, nullptr);
	}

	std::string toString() override
	{
		std::string s;
		for (auto device : this->physical_devices)
		{
			s += std::string(device.properties.deviceName) + '\n';
			for (const auto& extension : device.extension_properties)
			{
				s += std::string(extension.extensionName) + '\n';
			}
		}
		return s;
	}

	template <typename T>
	T getProcAddress(const std::string& name) {
		return reinterpret_cast<T>(vk.GetInstanceProcAddr(this->instance, name.c_str()));
	};

	VulkanPhysicalDevice selectPhysicalDevice(const VkPhysicalDeviceFeatures2& required_features)
	{
		std::vector<VulkanPhysicalDevice> devices;
		for (auto& physical_device : this->physical_devices) {
			if (physical_device.supportsFeatures(required_features)) {
				devices.push_back(physical_device);
			}
		}
		if (devices.empty()) {
			throw std::runtime_error("Could not find physical device with the required features");
		}
		return devices.front();
	}

#ifdef VK_EXT_debug_report
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
#endif

#ifdef VK_KHR_ray_tracing
	PFN_vkBindAccelerationStructureMemoryKHR vkBindAccelerationStructureMemoryKHR;
	PFN_vkBuildAccelerationStructureKHR vkBuildAccelerationStructureKHR;
	PFN_vkCmdBuildAccelerationStructureIndirectKHR vkCmdBuildAccelerationStructureIndirectKHR;
	PFN_vkCmdBuildAccelerationStructureKHR vkCmdBuildAccelerationStructureKHR;
	PFN_vkCmdCopyAccelerationStructureKHR vkCmdCopyAccelerationStructureKHR;
	PFN_vkCmdCopyAccelerationStructureToMemoryKHR vkCmdCopyAccelerationStructureToMemoryKHR;
	PFN_vkCmdCopyMemoryToAccelerationStructureKHR vkCmdCopyMemoryToAccelerationStructureKHR;
	PFN_vkCmdTraceRaysIndirectKHR vkCmdTraceRaysIndirectKHR;
	PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
	PFN_vkCmdWriteAccelerationStructuresPropertiesKHR vkCmdWriteAccelerationStructuresPropertiesKHR;
	PFN_vkCopyAccelerationStructureKHR vkCopyAccelerationStructureKHR;
	PFN_vkCopyAccelerationStructureToMemoryKHR vkCopyAccelerationStructureToMemoryKHR;
	PFN_vkCopyMemoryToAccelerationStructureKHR vkCopyMemoryToAccelerationStructureKHR;
	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
	PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
	PFN_vkGetAccelerationStructureMemoryRequirementsKHR vkGetAccelerationStructureMemoryRequirementsKHR;
	PFN_vkGetDeviceAccelerationStructureCompatibilityKHR vkGetDeviceAccelerationStructureCompatibilityKHR;
	PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR vkGetRayTracingCaptureReplayShaderGroupHandlesKHR;
	PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
	PFN_vkWriteAccelerationStructuresPropertiesKHR vkWriteAccelerationStructuresPropertiesKHR;
	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
#endif
	VkInstance instance{ nullptr };
	std::vector<VulkanPhysicalDevice> physical_devices;
};

class VulkanDevice {
public:
	VulkanDevice() = delete;

	VulkanDevice(
		std::shared_ptr<VulkanInstance> vulkan,
		const VkPhysicalDeviceFeatures2& device_features2,
		const std::vector<const char*>& required_layers = {},
		const std::vector<const char*>& required_extensions = {}) :
		vulkan(std::move(vulkan)),
		physical_device(this->vulkan->selectPhysicalDevice(device_features2))
	{
		std::for_each(required_layers.begin(), required_layers.end(), [&](const char* layer_name) {
			for (const auto& properties : physical_device.layer_properties)
				if (std::strcmp(layer_name, properties.layerName) == 0)
					return;
			throw std::runtime_error("Required device layer " + std::string(layer_name) + " not supported.");
			});

		std::for_each(required_extensions.begin(), required_extensions.end(), [&](const char* extension_name) {
			for (const auto& properties : physical_device.extension_properties)
				if (std::strcmp(extension_name, properties.extensionName) == 0)
					return;
			throw std::runtime_error("Required device extension " + std::string(extension_name) + " not supported.");
			});

		std::array<float, 1> priorities = { 1.0f };
		uint32_t num_queues = static_cast<uint32_t>(this->physical_device.queue_family_properties.size());
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		for (uint32_t queue_index = 0; queue_index < num_queues; queue_index++) {
			queue_create_infos.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = queue_index,
				.queueCount = static_cast<uint32_t>(priorities.size()),
				.pQueuePriorities = priorities.data()
				});
		}

		if (queue_create_infos.empty()) {
			throw std::runtime_error("no queues found");
		}

		VkDeviceCreateInfo device_create_info{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &device_features2,
			.flags = 0,
			.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
			.pQueueCreateInfos = queue_create_infos.data(),
			.enabledLayerCount = static_cast<uint32_t>(required_layers.size()),
			.ppEnabledLayerNames = required_layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
			.ppEnabledExtensionNames = required_extensions.data(),
			.pEnabledFeatures = nullptr,
		};

		THROW_ON_ERROR(vk.CreateDevice(this->physical_device.device, &device_create_info, nullptr, &this->device));

		this->queues.resize(num_queues);
		for (uint32_t i = 0; i < num_queues; i++) {
			vk.GetDeviceQueue(this->device, i, 0, &this->queues[i]);
		}

		VkCommandPoolCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = 0,
		};

		THROW_ON_ERROR(vk.CreateCommandPool(this->device, &create_info, nullptr, &this->default_pool));
	}

	~VulkanDevice()
	{
		vk.DestroyCommandPool(this->device, this->default_pool, nullptr);
		vk.DestroyDevice(this->device, nullptr);
	}

	void bindImageMemory(VkImage image, VkDeviceMemory memory, VkDeviceSize offset)
	{
		THROW_ON_ERROR(vk.BindImageMemory(this->device, image, memory, offset));
	}

	void bindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset)
	{
		THROW_ON_ERROR(vk.BindBufferMemory(this->device, buffer, memory, offset));
	}

	VkQueue getQueue(VkQueueFlags required_flags, VkSurfaceKHR surface = VK_NULL_HANDLE)
	{
		std::vector<VkBool32> filter(physical_device.queue_family_properties.size(), VK_TRUE);
		if (surface != VK_NULL_HANDLE) {
			for (uint32_t i = 0; i < physical_device.queue_family_properties.size(); i++) {
				vk.GetPhysicalDeviceSurfaceSupportKHR(physical_device.device, i, surface, &filter[i]);
			}
		}
		uint32_t queue_index = this->physical_device.getQueueIndex(required_flags, filter);
		return this->queues[queue_index];
	}

#ifdef VK_KHR_ray_tracing
	VkDeviceAddress getDeviceAddress(VkAccelerationStructureKHR as)
	{
		VkAccelerationStructureDeviceAddressInfoKHR device_address_info{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.pNext = nullptr,
			.accelerationStructure = as,
		};
		return this->vulkan->vkGetAccelerationStructureDeviceAddressKHR(this->device, &device_address_info);
	}

	VkDeviceAddress getDeviceAddress(VkBuffer buffer)
	{
		VkBufferDeviceAddressInfo buffer_address_info{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.pNext = nullptr,
			.buffer = buffer,
		};
		return this->vulkan->vkGetBufferDeviceAddressKHR(this->device, &buffer_address_info);
	}
#endif

	VkDevice device{ nullptr };
	std::shared_ptr<VulkanInstance> vulkan;
	VulkanPhysicalDevice physical_device;
	std::vector<VkQueue> queues;
	VkCommandPool default_pool{ 0 };
};

class VulkanMemory {
public:
	VulkanMemory() = delete;

	explicit VulkanMemory(
		std::shared_ptr<VulkanDevice> device,
		VkDeviceSize size,
		uint32_t memory_type_index,
		void* pNext = nullptr);

	~VulkanMemory();

	char* map(VkDeviceSize size, VkDeviceSize offset, VkMemoryMapFlags flags = 0) const;
	void unmap() const;
	void memcpy(const void* src, VkDeviceSize size, VkDeviceSize offset = 0);

	std::shared_ptr<VulkanDevice> device;
	VkDeviceMemory memory{ 0 };
};

static VkBool32 DebugCallback(
	VkFlags flags,
	VkDebugReportObjectTypeEXT,
	uint64_t,
	size_t,
	int32_t,
	const char* layer,
	const char* msg,
	void*)
{
	std::string message;
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		message += "ERROR: ";
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		message += "DEBUG: ";
	}
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		message += "WARNING: ";
	}
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		message += "INFORMATION: ";
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		message += "PERFORMANCE_WARNING: ";
	}
	message += std::string(layer) + " " + std::string(msg);
	std::cout << message << std::endl;
	return VK_FALSE;
}

#ifdef VK_EXT_debug_report
class VulkanDebugCallback {
public:
	VulkanDebugCallback() = delete;

	explicit VulkanDebugCallback(
		std::shared_ptr<VulkanInstance> vulkan,
		VkDebugReportFlagsEXT flags,
		PFN_vkDebugReportCallbackEXT callback = DebugCallback,
		void* userdata = nullptr) :
		vulkan(std::move(vulkan))
	{
		VkDebugReportCallbackCreateInfoEXT create_info{
			.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
			.pNext = nullptr,
			.flags = flags,
			.pfnCallback = callback,
			.pUserData = userdata,
		};

		THROW_ON_ERROR(this->vulkan->vkCreateDebugReportCallbackEXT(this->vulkan->instance, &create_info, nullptr, &this->callback));
	}

	~VulkanDebugCallback()
	{
		this->vulkan->vkDestroyDebugReportCallbackEXT(this->vulkan->instance, this->callback, nullptr);
	}

	std::shared_ptr<VulkanInstance> vulkan;
	VkDebugReportCallbackEXT callback{ nullptr };
};
#endif

class VulkanSemaphore {
public:
	VulkanSemaphore() = delete;

	explicit VulkanSemaphore(std::shared_ptr<VulkanDevice> device)
		: device(std::move(device))
	{
		VkSemaphoreCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0
		};
		THROW_ON_ERROR(vk.CreateSemaphore(this->device->device, &create_info, nullptr, &this->semaphore));
	}

	~VulkanSemaphore()
	{
		vk.DestroySemaphore(this->device->device, this->semaphore, nullptr);
	}

	VkSemaphore semaphore{ 0 };
	std::shared_ptr<VulkanDevice> device;
};

class VulkanSwapchain {
public:
	VulkanSwapchain() = delete;

	VulkanSwapchain(
		std::shared_ptr<VulkanDevice> device,
		VkSurfaceKHR surface,
		uint32_t minImageCount,
		VkFormat imageFormat,
		VkColorSpaceKHR imageColorSpace,
		VkExtent2D imageExtent,
		uint32_t imageArrayLayers,
		VkImageUsageFlags imageUsage,
		VkSharingMode imageSharingMode,
		std::vector<uint32_t> queueFamilyIndices,
		VkSurfaceTransformFlagBitsKHR preTransform,
		VkCompositeAlphaFlagBitsKHR compositeAlpha,
		VkPresentModeKHR presentMode,
		VkBool32 clipped,
		VkSwapchainKHR oldSwapchain) :
		device(std::move(device))
	{
		VkSwapchainCreateInfoKHR create_info{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.surface = surface,
			.minImageCount = minImageCount,
			.imageFormat = imageFormat,
			.imageColorSpace = imageColorSpace,
			.imageExtent = imageExtent,
			.imageArrayLayers = imageArrayLayers,
			.imageUsage = imageUsage,
			.imageSharingMode = imageSharingMode,
			.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size()),
			.pQueueFamilyIndices = queueFamilyIndices.data(),
			.preTransform = preTransform,
			.compositeAlpha = compositeAlpha,
			.presentMode = presentMode,
			.clipped = clipped,
			.oldSwapchain = oldSwapchain
		};

		THROW_ON_ERROR(vk.CreateSwapchainKHR(this->device->device, &create_info, nullptr, &this->swapchain));
	}

	~VulkanSwapchain()
	{
		vk.DestroySwapchainKHR(this->device->device, this->swapchain, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	VkSwapchainKHR swapchain{ 0 };
};


class VulkanSurface {
public:
	VulkanSurface() = delete;

#if defined(VK_KHR_win32_surface)
	VulkanSurface(
		std::shared_ptr<VulkanInstance> vulkan,
		HWND window,
		HINSTANCE hinstance) :
		vulkan(std::move(vulkan))
	{
		VkWin32SurfaceCreateInfoKHR create_info{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.hinstance = hinstance,
			.hwnd = window
		};

		THROW_ON_ERROR(vk.CreateWin32SurfaceKHR(this->vulkan->instance, &create_info, nullptr, &this->surface));
	}

#elif defined(VK_KHR_android_surface)
	VulkanSurface(
		std::shared_ptr<VulkanInstance> vulkan,
		ANativeWindow* window) :
		vulkan(std::move(vulkan))
	{
		VkAndroidSurfaceCreateInfoKHR create_info{
			.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.window = window
		};

		THROW_ON_ERROR(vk.CreateAndroidSurfaceKHR(this->vulkan->instance, &create_info, nullptr, &this->surface));
	}


#elif defined(VK_KHR_xcb_surface)
	VulkanSurface(std::shared_ptr<VulkanInstance> vulkan,
		xcb_window_t window,
		xcb_connection_t* connection) :
		vulkan(std::move(vulkan))
	{
		VkXcbSurfaceCreateInfoKHR create_info{
			.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.connection = connection,
			.window = window,
		};

		THROW_ON_ERROR(vk.CreateXcbSurfaceKHR(this->vulkan->instance, &create_info, nullptr, &this->surface));
	}
#endif

	~VulkanSurface()
	{
		vk.DestroySurfaceKHR(this->vulkan->instance, this->surface, nullptr);
	}

	VkSurfaceFormatKHR getSupportedSurfaceFormat(std::shared_ptr<VulkanDevice> device, VkFormat format)
	{
		uint32_t count;
		THROW_ON_ERROR(vk.GetPhysicalDeviceSurfaceFormatsKHR(device->physical_device.device, surface, &count, nullptr));
		std::vector<VkSurfaceFormatKHR> surface_formats(count);
		THROW_ON_ERROR(vk.GetPhysicalDeviceSurfaceFormatsKHR(device->physical_device.device, surface, &count, surface_formats.data()));

		for (VkSurfaceFormatKHR surface_format : surface_formats) {
			if (surface_format.format == format) {
				return surface_format;
			}
		}
		throw std::runtime_error("surface format not supported!");
	}

	void checkPresentModeSupport(std::shared_ptr<VulkanDevice> device, VkPresentModeKHR present_mode)
	{
		uint32_t count;
		THROW_ON_ERROR(vk.GetPhysicalDeviceSurfacePresentModesKHR(device->physical_device.device, surface, &count, nullptr));
		std::vector<VkPresentModeKHR> present_modes(count);
		THROW_ON_ERROR(vk.GetPhysicalDeviceSurfacePresentModesKHR(device->physical_device.device, surface, &count, present_modes.data()));

		if (std::find(present_modes.begin(), present_modes.end(), present_mode) == present_modes.end()) {
			throw std::runtime_error("surface does not support present mode");
		}
	}

	VkSurfaceCapabilitiesKHR getSurfaceCapabilities(std::shared_ptr<VulkanDevice> device)
	{
		VkSurfaceCapabilitiesKHR surface_capabilities;
		THROW_ON_ERROR(vk.GetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical_device.device, surface, &surface_capabilities));
		return surface_capabilities;
	}


	std::shared_ptr<VulkanInstance> vulkan;
	VkSurfaceKHR surface{ 0 };
};


class VulkanDescriptorPool {
public:
	VulkanDescriptorPool() = delete;

	explicit VulkanDescriptorPool(
		std::shared_ptr<VulkanDevice> device,
		std::vector<VkDescriptorPoolSize> descriptor_pool_sizes) :
		device(std::move(device))
	{
		VkDescriptorPoolCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			.maxSets = static_cast<uint32_t>(descriptor_pool_sizes.size()),
			.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size()),
			.pPoolSizes = descriptor_pool_sizes.data()
		};

		THROW_ON_ERROR(vk.CreateDescriptorPool(this->device->device, &create_info, nullptr, &this->pool));
	}

	~VulkanDescriptorPool()
	{
		vk.DestroyDescriptorPool(this->device->device, this->pool, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	VkDescriptorPool pool{ 0 };
};

class VulkanDescriptorSetLayout {
public:
	VulkanDescriptorSetLayout() = delete;

	VulkanDescriptorSetLayout(
		std::shared_ptr<VulkanDevice> device,
		std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings) :
		device(std::move(device))
	{
		VkDescriptorSetLayoutCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.bindingCount = static_cast<uint32_t>(descriptor_set_layout_bindings.size()),
			.pBindings = descriptor_set_layout_bindings.data()
		};

		THROW_ON_ERROR(vk.CreateDescriptorSetLayout(this->device->device, &create_info, nullptr, &this->layout));
	}

	~VulkanDescriptorSetLayout()
	{
		vk.DestroyDescriptorSetLayout(this->device->device, this->layout, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	VkDescriptorSetLayout layout{ 0 };
};

class VulkanDescriptorSets {
public:
	VulkanDescriptorSets() = delete;

	VulkanDescriptorSets(
		std::shared_ptr<VulkanDevice> device,
		std::shared_ptr<VulkanDescriptorPool> pool,
		const std::vector<VkDescriptorSetLayout>& set_layouts) :
		device(std::move(device)),
		pool(std::move(pool))
	{
		VkDescriptorSetAllocateInfo allocate_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = this->pool->pool,
			.descriptorSetCount = static_cast<uint32_t>(set_layouts.size()),
			.pSetLayouts = set_layouts.data()
		};

		this->descriptor_sets.resize(set_layouts.size());
		THROW_ON_ERROR(vk.AllocateDescriptorSets(this->device->device, &allocate_info, this->descriptor_sets.data()));
	}

	~VulkanDescriptorSets()
	{
		vk.FreeDescriptorSets(
			this->device->device,
			this->pool->pool,
			static_cast<uint32_t>(this->descriptor_sets.size()),
			this->descriptor_sets.data());
	}

	void update(
		const std::vector<VkWriteDescriptorSet>& descriptor_writes,
		const std::vector<VkCopyDescriptorSet>& descriptor_copies = std::vector<VkCopyDescriptorSet>()) const
	{
		vk.UpdateDescriptorSets(
			this->device->device,
			static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(),
			static_cast<uint32_t>(descriptor_copies.size()), descriptor_copies.data());
	}

	std::shared_ptr<VulkanDevice> device;
	std::shared_ptr<VulkanDescriptorPool> pool;
	std::vector<VkDescriptorSet> descriptor_sets;
};

class VulkanFence {
public:
	VulkanFence() = delete;

	explicit VulkanFence(std::shared_ptr<VulkanDevice> device) :
		device(std::move(device))
	{
		VkFenceCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		THROW_ON_ERROR(vk.CreateFence(this->device->device, &create_info, nullptr, &this->fence));
	}

	~VulkanFence()
	{
		vk.DestroyFence(this->device->device, this->fence, nullptr);
	}

	void wait()
	{
		THROW_ON_ERROR(vk.WaitForFences(this->device->device, 1, &this->fence, VK_TRUE, UINT64_MAX));
	}

	void reset()
	{
		THROW_ON_ERROR(vk.ResetFences(this->device->device, 1, &this->fence));
	}

	std::shared_ptr<VulkanDevice> device;
	VkFence fence{ 0 };
};


class VulkanImage {
public:
	VulkanImage() = delete;

	VulkanImage(
		std::shared_ptr<VulkanDevice> device,
		VkImageType image_type,
		VkFormat format,
		VkExtent3D extent,
		uint32_t mip_levels,
		uint32_t array_layers,
		VkSampleCountFlagBits samples,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkSharingMode sharing_mode,
		VkImageCreateFlags flags = 0,
		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
		std::vector<uint32_t> queue_family_indices = std::vector<uint32_t>()) :
		device(std::move(device))
	{
		VkImageCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = flags,
			.imageType = image_type,
			.format = format,
			.extent = extent,
			.mipLevels = mip_levels,
			.arrayLayers = array_layers,
			.samples = samples,
			.tiling = tiling,
			.usage = usage,
			.sharingMode = sharing_mode,
			.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size()),
			.pQueueFamilyIndices = queue_family_indices.data(),
			.initialLayout = initial_layout,
		};

		THROW_ON_ERROR(vk.CreateImage(this->device->device, &create_info, nullptr, &this->image));
	}

	~VulkanImage()
	{
		vk.DestroyImage(this->device->device, this->image, nullptr);
	}

	VkMemoryRequirements getMemoryRequirements()
	{
		VkMemoryRequirements memory_requirements;
		vk.GetImageMemoryRequirements(
			this->device->device,
			this->image,
			&memory_requirements);

		return memory_requirements;
	}

	VkSubresourceLayout getSubresourceLayout(VkImageSubresource& image_subresource)
	{
		VkSubresourceLayout subresource_layout;
		vk.GetImageSubresourceLayout(
			this->device->device,
			this->image,
			&image_subresource,
			&subresource_layout);

		return subresource_layout;
	}

	std::vector<VkSparseImageMemoryRequirements> getSparseMemoryRequirements()
	{
		uint32_t sparse_memory_requirements_count;
		vk.GetImageSparseMemoryRequirements(
			this->device->device,
			this->image,
			&sparse_memory_requirements_count,
			nullptr);

		std::vector<VkSparseImageMemoryRequirements> sparse_memory_requirements(sparse_memory_requirements_count);
		vk.GetImageSparseMemoryRequirements(
			this->device->device,
			this->image,
			&sparse_memory_requirements_count,
			sparse_memory_requirements.data());

		return sparse_memory_requirements;
	}

	VkSparseImageMemoryRequirements getSparseMemoryRequirements(VkImageAspectFlags aspectMask)
	{
		for (auto requirements : this->getSparseMemoryRequirements()) {
			if (requirements.formatProperties.aspectMask & aspectMask) {
				return requirements;
			}
		}
		throw std::runtime_error("Could not find sparse image memory requirements for color aspect bit");
	}

	static VkImageMemoryBarrier MemoryBarrier(
		VkImage image,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageSubresourceRange subresourceRange)
	{
		return {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = srcAccessMask,
			.dstAccessMask = dstAccessMask,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = subresourceRange
		};
	}

	VkImageMemoryBarrier memoryBarrier(
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageSubresourceRange subresourceRange)
	{
		return MemoryBarrier(
			this->image,
			srcAccessMask,
			dstAccessMask,
			oldLayout,
			newLayout,
			subresourceRange);
	}

	std::shared_ptr<VulkanDevice> device;
	VkImage image{ 0 };
};

class VulkanBuffer {
public:
	VulkanBuffer() = delete;

	VulkanBuffer(
		std::shared_ptr<VulkanDevice> device,
		VkBufferCreateFlags flags,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkSharingMode sharingMode,
		const std::vector<uint32_t>& queueFamilyIndices = std::vector<uint32_t>()) :
		device(std::move(device))
	{
		VkBufferCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = flags,
			.size = size,
			.usage = usage,
			.sharingMode = sharingMode,
			.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size()),
			.pQueueFamilyIndices = queueFamilyIndices.data(),
		};

		THROW_ON_ERROR(vk.CreateBuffer(this->device->device, &create_info, nullptr, &this->buffer));
	}

	~VulkanBuffer()
	{
		vk.DestroyBuffer(this->device->device, this->buffer, nullptr);
	}

	VkMemoryRequirements getMemoryRequirements()
	{
		VkMemoryRequirements memory_requirements;
		vk.GetBufferMemoryRequirements(
			this->device->device,
			this->buffer,
			&memory_requirements);

		return memory_requirements;
	}

	std::shared_ptr<VulkanDevice> device;
	VkBuffer buffer{ 0 };
};


class VulkanBufferObject {
public:
	VulkanBufferObject(
		std::shared_ptr<VulkanDevice> device,
		VkBufferCreateFlags createFlags,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkSharingMode sharingMode,
		VkMemoryPropertyFlags memoryFlags)
	{
		this->buffer = std::make_shared<VulkanBuffer>(
			device,
			createFlags,
			size,
			usage,
			sharingMode);

		VkMemoryRequirements memory_requirements = this->buffer->getMemoryRequirements();

		uint32_t memory_type_index = device->physical_device.getMemoryTypeIndex(
			memory_requirements.memoryTypeBits,
			memoryFlags);

		VkMemoryAllocateFlags allocate_flags = 0;
#ifdef VK_USE_PLATFORM_WIN32_KHR
		allocate_flags = (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) ?
			VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR : 0;
#endif
		VkMemoryAllocateFlagsInfo allocate_info{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR,
			.pNext = nullptr,
			.flags = allocate_flags,
			.deviceMask = 0,
		};

		this->memory = std::make_shared<VulkanMemory>(
			device,
			memory_requirements.size,
			memory_type_index,
			&allocate_info);

		device->bindBufferMemory(
			this->buffer->buffer,
			this->memory->memory,
			0);
	}

	~VulkanBufferObject() = default;

	std::shared_ptr<VulkanBuffer> buffer;
	std::shared_ptr<VulkanMemory> memory;
};


class VulkanImageObject {
public:
	VulkanImageObject(
		std::shared_ptr<VulkanDevice> device,
		VkImageType image_type,
		VkFormat format,
		VkExtent3D extent,
		uint32_t levels,
		uint32_t layers,
		VkSampleCountFlagBits samples,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkSharingMode mode,
		VkImageCreateFlags flags,
		VkMemoryPropertyFlags memoryFlags,
		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED)
	{
		this->image = std::make_shared<VulkanImage>(
			device,
			image_type,
			format,
			extent,
			levels,
			layers,
			samples,
			tiling,
			usage,
			mode,
			flags);

		this->memory_requirements = this->image->getMemoryRequirements();

		uint32_t memory_type_index = device->physical_device.getMemoryTypeIndex(
			this->memory_requirements.memoryTypeBits,
			memoryFlags);

		this->memory = std::make_shared<VulkanMemory>(
			device,
			this->memory_requirements.size,
			memory_type_index);

		device->bindImageMemory(
			this->image->image,
			this->memory->memory,
			0);
	}


	std::shared_ptr<VulkanImage> image;
	std::shared_ptr<VulkanMemory> memory;
	VkMemoryRequirements memory_requirements;
};


class VulkanCommandBuffers {
public:
	class Scope {
	public:
		Scope(VulkanCommandBuffers* buffer, size_t index = 0) :
			buffer(buffer),
			index(index)
		{
			this->buffer->begin(this->index);
		}

		~Scope()
		{
			this->buffer->end(this->index);
		}

		VulkanCommandBuffers* buffer;
		size_t index;
	};

	VulkanCommandBuffers() = delete;

	VulkanCommandBuffers(
		std::shared_ptr<VulkanDevice> device,
		size_t count = 1,
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) :
		device(std::move(device))
	{
		VkCommandBufferAllocateInfo allocate_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = this->device->default_pool,
			.level = level,
			.commandBufferCount = static_cast<uint32_t>(count),
		};

		this->buffers.resize(count);
		THROW_ON_ERROR(vk.AllocateCommandBuffers(this->device->device, &allocate_info, this->buffers.data()));
	}

	~VulkanCommandBuffers()
	{
		vk.FreeCommandBuffers(
			this->device->device,
			this->device->default_pool,
			static_cast<uint32_t>(this->buffers.size()),
			this->buffers.data());
	}

	void begin(
		size_t buffer_index = 0,
		VkRenderPass renderpass = 0,
		uint32_t subpass = 0,
		VkFramebuffer framebuffer = 0,
		VkCommandBufferUsageFlags flags = 0)
	{
		VkCommandBufferInheritanceInfo inheritance_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = nullptr,
			.renderPass = renderpass,
			.subpass = subpass,
			.framebuffer = framebuffer,
			.occlusionQueryEnable = VK_FALSE,
			.queryFlags = 0,
			.pipelineStatistics = 0,
		};

		VkCommandBufferBeginInfo begin_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = flags,
			.pInheritanceInfo = &inheritance_info,
		};

		THROW_ON_ERROR(vk.BeginCommandBuffer(this->buffers[buffer_index], &begin_info));
	}

	void end(size_t buffer_index = 0)
	{
		THROW_ON_ERROR(vk.EndCommandBuffer(this->buffers[buffer_index]));
	}

	void submit(
		VkQueue queue,
		VkPipelineStageFlags flags,
		VkFence fence = VK_NULL_HANDLE,
		const std::vector<VkSemaphore>& wait_semaphores = std::vector<VkSemaphore>(),
		const std::vector<VkSemaphore>& signal_semaphores = std::vector<VkSemaphore>())
	{
		this->submit(queue, flags, this->buffers, fence, wait_semaphores, signal_semaphores);
	}

	void submit(
		VkQueue queue,
		VkPipelineStageFlags flags,
		size_t buffer_index,
		VkFence fence = VK_NULL_HANDLE,
		const std::vector<VkSemaphore>& wait_semaphores = std::vector<VkSemaphore>(),
		const std::vector<VkSemaphore>& signal_semaphores = std::vector<VkSemaphore>())
	{
		this->submit(queue, flags, { this->buffers[buffer_index] }, fence, wait_semaphores, signal_semaphores);
	}

	void submit(
		VkQueue queue,
		VkPipelineStageFlags flags,
		const std::vector<VkCommandBuffer>& buffers,
		VkFence fence = VK_NULL_HANDLE,
		const std::vector<VkSemaphore>& wait_semaphores = std::vector<VkSemaphore>(),
		const std::vector<VkSemaphore>& signal_semaphores = std::vector<VkSemaphore>())
	{
		VkSubmitInfo submit_info{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size()),
			.pWaitSemaphores = wait_semaphores.data(),
			.pWaitDstStageMask = &flags,
			.commandBufferCount = static_cast<uint32_t>(buffers.size()),
			.pCommandBuffers = buffers.data(),
			.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size()),
			.pSignalSemaphores = signal_semaphores.data(),
		};

		THROW_ON_ERROR(vk.QueueSubmit(queue, 1, &submit_info, fence));
	}

	VkCommandBuffer buffer(size_t buffer_index = 0)
	{
		return this->buffers[buffer_index];
	}

	static void PipelineBarrier(
		VkCommandBuffer buffer,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		std::vector<VkImageMemoryBarrier> barriers,
		size_t buffer_index = 0)
	{
		vk.CmdPipelineBarrier(
			buffer,
			srcStageMask,
			dstStageMask,
			0, 0, nullptr, 0, nullptr,
			static_cast<uint32_t>(barriers.size()),
			barriers.data());
	}

	void pipelineBarrier(
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		std::vector<VkImageMemoryBarrier> barriers,
		size_t buffer_index = 0)
	{
		PipelineBarrier(
			this->buffers[buffer_index],
			srcStageMask,
			dstStageMask,
			barriers);
	}

	std::shared_ptr<VulkanDevice> device;
	std::vector<VkCommandBuffer> buffers;
};

inline
VulkanMemory::VulkanMemory(
	std::shared_ptr<VulkanDevice> device,
	VkDeviceSize size,
	uint32_t memory_type_index,
	void* pNext) :
	device(std::move(device))
{
	VkMemoryAllocateInfo allocate_info{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = pNext,
		.allocationSize = size,
		.memoryTypeIndex = memory_type_index,
	};

	THROW_ON_ERROR(vk.AllocateMemory(this->device->device, &allocate_info, nullptr, &this->memory));
}

inline
VulkanMemory::~VulkanMemory()
{
	vk.FreeMemory(this->device->device, this->memory, nullptr);
}

inline char*
VulkanMemory::map(VkDeviceSize size, VkDeviceSize offset, VkMemoryMapFlags flags) const
{
	void* data;
	THROW_ON_ERROR(vk.MapMemory(this->device->device, this->memory, offset, size, flags, &data));
	return reinterpret_cast<char*>(data);
}

inline void
VulkanMemory::unmap() const
{
	vk.UnmapMemory(this->device->device, this->memory);
}

class MemoryMap {
public:
	MemoryMap(VulkanMemory* memory, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) :
		memory(memory)
	{
		this->mem = this->memory->map(size, offset);
	}

	~MemoryMap() {
		this->memory->unmap();
	}

	VulkanMemory* memory{ nullptr };
	char* mem;
};

inline void
VulkanMemory::memcpy(const void* src, VkDeviceSize size, VkDeviceSize offset)
{
	MemoryMap memmap(this, size, offset);
	::memcpy(memmap.mem, src, size);
}

class VulkanImageView {
public:
	VulkanImageView() = delete;

	VulkanImageView(
		std::shared_ptr<VulkanDevice> device,
		VkImage image,
		VkImageViewType viewType,
		VkFormat format,
		VkComponentMapping components,
		VkImageSubresourceRange subresourceRange) :
		device(std::move(device))
	{
		VkImageViewCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = image,
			.viewType = viewType,
			.format = format,
			.components = components,
			.subresourceRange = subresourceRange,
		};

		THROW_ON_ERROR(vk.CreateImageView(this->device->device, &create_info, nullptr, &this->view));
	}

	~VulkanImageView()
	{
		vk.DestroyImageView(this->device->device, this->view, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	VkImageView view{ 0 };
};

class VulkanSampler {
public:
	VulkanSampler() = delete;

	VulkanSampler(
		std::shared_ptr<VulkanDevice> device,
		VkFilter magFilter,
		VkFilter minFilter,
		VkSamplerMipmapMode mipmapMode,
		VkSamplerAddressMode addressModeU,
		VkSamplerAddressMode addressModeV,
		VkSamplerAddressMode addressModeW,
		float mipLodBias,
		VkBool32 anisotropyEnable,
		float maxAnisotropy,
		VkBool32 compareEnable,
		VkCompareOp compareOp,
		float minLod,
		float maxLod,
		VkBorderColor borderColor,
		VkBool32 unnormalizedCoordinates) :
		device(std::move(device))
	{
		VkSamplerCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = magFilter,
			.minFilter = minFilter,
			.mipmapMode = mipmapMode,
			.addressModeU = addressModeU,
			.addressModeV = addressModeV,
			.addressModeW = addressModeW,
			.mipLodBias = mipLodBias,
			.anisotropyEnable = anisotropyEnable,
			.maxAnisotropy = maxAnisotropy,
			.compareEnable = compareEnable,
			.compareOp = compareOp,
			.minLod = minLod,
			.maxLod = maxLod,
			.borderColor = borderColor,
			.unnormalizedCoordinates = unnormalizedCoordinates
		};
		THROW_ON_ERROR(vk.CreateSampler(this->device->device, &create_info, nullptr, &this->sampler));
	}

	~VulkanSampler()
	{
		vk.DestroySampler(this->device->device, this->sampler, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	VkSampler sampler{ 0 };
};

class VulkanShaderModule {
public:
	VulkanShaderModule() = delete;

	VulkanShaderModule(
		std::shared_ptr<VulkanDevice> device,
		const std::vector<uint32_t>& code) :
		device(std::move(device))
	{
		VkShaderModuleCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.codeSize = code.size() * sizeof(uint32_t),
			.pCode = code.data(),
		};

		THROW_ON_ERROR(vk.CreateShaderModule(this->device->device, &create_info, nullptr, &this->module));
	}

	~VulkanShaderModule()
	{
		vk.DestroyShaderModule(this->device->device, this->module, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	VkShaderModule module{ 0 };
};


class VulkanPipelineCache {
public:
	VulkanPipelineCache() = delete;

	explicit VulkanPipelineCache(std::shared_ptr<VulkanDevice> device) :
		device(std::move(device))
	{
		VkPipelineCacheCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.initialDataSize = 0,
			.pInitialData = nullptr,
		};

		THROW_ON_ERROR(vk.CreatePipelineCache(this->device->device, &create_info, nullptr, &this->cache));
	}

	~VulkanPipelineCache()
	{
		vk.DestroyPipelineCache(this->device->device, this->cache, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	VkPipelineCache cache{ 0 };
};

class VulkanRenderpass {
public:
	VulkanRenderpass() = delete;

	VulkanRenderpass(
		std::shared_ptr<VulkanDevice> device,
		const std::vector<VkAttachmentDescription>& attachments,
		const std::vector<VkSubpassDescription>& subpasses,
		const std::vector<VkSubpassDependency>& dependencies = std::vector<VkSubpassDependency>()) :
		device(std::move(device))
	{
		VkRenderPassCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = static_cast<uint32_t>(subpasses.size()),
			.pSubpasses = subpasses.data(),
			.dependencyCount = static_cast<uint32_t>(dependencies.size()),
			.pDependencies = dependencies.data(),
		};

		THROW_ON_ERROR(vk.CreateRenderPass(this->device->device, &create_info, nullptr, &this->renderpass));
	}

	~VulkanRenderpass()
	{
		vk.DestroyRenderPass(this->device->device, this->renderpass, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	VkRenderPass renderpass{ 0 };
};

class VulkanRenderPassScope {
public:
	VulkanRenderPassScope() = delete;

	explicit VulkanRenderPassScope(
		VkRenderPass renderpass,
		VkFramebuffer framebuffer,
		const VkRect2D renderarea,
		const std::vector<VkClearValue>& clearvalues,
		VkCommandBuffer command) :
		command(command)
	{
		VkRenderPassBeginInfo begin_info{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = renderpass,
			.framebuffer = framebuffer,
			.renderArea = renderarea,
			.clearValueCount = static_cast<uint32_t>(clearvalues.size()),
			.pClearValues = clearvalues.data()
		};

		vk.CmdBeginRenderPass(
			this->command,
			&begin_info,
			VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	}

	~VulkanRenderPassScope()
	{
		vk.CmdEndRenderPass(this->command);
	}

	VkCommandBuffer command;
};

class VulkanFramebuffer {
public:
	VulkanFramebuffer() = delete;

	VulkanFramebuffer(
		std::shared_ptr<VulkanDevice> device,
		std::shared_ptr<VulkanRenderpass> renderpass,
		std::vector<VkImageView> attachments,
		uint32_t width,
		uint32_t height,
		uint32_t layers) :
		device(std::move(device)),
		renderpass(std::move(renderpass))
	{
		VkFramebufferCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderPass = this->renderpass->renderpass,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.width = width,
			.height = height,
			.layers = layers,
		};

		THROW_ON_ERROR(vk.CreateFramebuffer(this->device->device, &create_info, nullptr, &this->framebuffer));
	}

	~VulkanFramebuffer()
	{
		vk.DestroyFramebuffer(this->device->device, this->framebuffer, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	std::shared_ptr<VulkanRenderpass> renderpass;
	VkFramebuffer framebuffer{ 0 };
};

class VulkanPipelineLayout {
public:
	VulkanPipelineLayout() = delete;

	VulkanPipelineLayout(
		std::shared_ptr<VulkanDevice> device,
		const std::vector<VkDescriptorSetLayout>& setlayouts,
		const std::vector<VkPushConstantRange>& pushconstantranges) :
		device(std::move(device))
	{
		VkPipelineLayoutCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.setLayoutCount = static_cast<uint32_t>(setlayouts.size()),
			.pSetLayouts = setlayouts.data(),
			.pushConstantRangeCount = static_cast<uint32_t>(pushconstantranges.size()),
			.pPushConstantRanges = pushconstantranges.data(),
		};

		THROW_ON_ERROR(vk.CreatePipelineLayout(this->device->device, &create_info, nullptr, &this->layout));
	}

	~VulkanPipelineLayout()
	{
		vk.DestroyPipelineLayout(this->device->device, this->layout, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	VkPipelineLayout layout{ 0 };
};

class VulkanComputePipeline {
public:
	VulkanComputePipeline() = delete;

	VulkanComputePipeline(
		std::shared_ptr<VulkanDevice> device,
		VkPipelineCache pipelineCache,
		const VkPipelineShaderStageCreateInfo& stage,
		VkPipelineLayout layout,
		VkPipeline basePipelineHandle = 0,
		int32_t basePipelineIndex = 0) :
		device(std::move(device))
	{
		VkComputePipelineCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = stage,
			.layout = layout,
			.basePipelineHandle = basePipelineHandle,
			.basePipelineIndex = basePipelineIndex,
		};

		THROW_ON_ERROR(vk.CreateComputePipelines(this->device->device, pipelineCache, 1, &create_info, nullptr, &this->pipeline));
	}

	~VulkanComputePipeline()
	{
		vk.DestroyPipeline(this->device->device, this->pipeline, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	VkPipeline pipeline{ 0 };
};

class VulkanRaytracingPipeline {
public:
	VulkanRaytracingPipeline() = delete;

	VulkanRaytracingPipeline(
		std::shared_ptr<VulkanDevice> device) :
		device(std::move(device))
	{

	}

	std::shared_ptr<VulkanDevice> device;
};

class VulkanGraphicsPipeline {
public:
	VulkanGraphicsPipeline() = delete;

	VulkanGraphicsPipeline(
		std::shared_ptr<VulkanDevice> device,
		VkRenderPass render_pass,
		VkPipelineCache pipeline_cache,
		VkPipelineLayout pipeline_layout,
		VkPrimitiveTopology primitive_topology,
		VkPipelineRasterizationStateCreateInfo rasterization_state,
		const std::vector<VkDynamicState>& dynamic_states,
		const std::vector<VkPipelineShaderStageCreateInfo>& shaderstages,
		const std::vector<VkVertexInputBindingDescription>& binding_descriptions,
		const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions) :
		device(std::move(device))
	{
		VkPipelineVertexInputStateCreateInfo vertex_input_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size()),
			.pVertexBindingDescriptions = binding_descriptions.data(),
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()),
			.pVertexAttributeDescriptions = attribute_descriptions.data(),
		};

		VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.topology = primitive_topology,
			.primitiveRestartEnable = VK_FALSE,
		};

		VkPipelineColorBlendAttachmentState blend_attachment_state{
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_MAX_ENUM,
			.attachmentCount = 1,
			.pAttachments = &blend_attachment_state,
			.blendConstants = {0},
		};

		VkPipelineDynamicStateCreateInfo dynamic_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
			.pDynamicStates = dynamic_states.data(),
		};

		VkPipelineViewportStateCreateInfo viewport_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.viewportCount = 1,
			.pViewports = nullptr,
			.scissorCount = 1,
			.pScissors = nullptr,
		};

		const VkStencilOpState stencil_op_state{
			.failOp = VK_STENCIL_OP_KEEP,
			.passOp = VK_STENCIL_OP_KEEP,
			.depthFailOp = VK_STENCIL_OP_KEEP,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.compareMask = 0,
			.writeMask = 0,
			.reference = 0,
		};

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = stencil_op_state,
			.back = stencil_op_state,
			.minDepthBounds = 0,
			.maxDepthBounds = 0,
		};

		VkPipelineMultisampleStateCreateInfo multi_sample_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 0,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE,
		};

		VkGraphicsPipelineCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stageCount = static_cast<uint32_t>(shaderstages.size()),
			.pStages = shaderstages.data(),
			.pVertexInputState = &vertex_input_state,
			.pInputAssemblyState = &input_assembly_state,
			.pTessellationState = nullptr,
			.pViewportState = &viewport_state,
			.pRasterizationState = &rasterization_state,
			.pMultisampleState = &multi_sample_state,
			.pDepthStencilState = &depth_stencil_state,
			.pColorBlendState = &color_blend_state,
			.pDynamicState = &dynamic_state,
			.layout = pipeline_layout,
			.renderPass = render_pass,
			.subpass = 0,
			.basePipelineHandle = 0,
			.basePipelineIndex = 0,
		};

		THROW_ON_ERROR(vk.CreateGraphicsPipelines(this->device->device, pipeline_cache, 1, &create_info, nullptr, &this->pipeline));
	}

	~VulkanGraphicsPipeline()
	{
		vk.DestroyPipeline(this->device->device, this->pipeline, nullptr);
	}

	std::shared_ptr<VulkanDevice> device;
	VkPipeline pipeline{ 0 };
};

#ifdef VK_KHR_ray_tracing

class VulkanRayTracingPipeline {
public:
	VulkanRayTracingPipeline(
		std::shared_ptr<VulkanInstance> vulkan,
		std::shared_ptr<VulkanDevice> device,
		const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_infos,
		const std::vector<VkRayTracingShaderGroupCreateInfoKHR>& shader_groups,
		VulkanPipelineLayout* pipeline_layout) :
		vulkan(std::move(vulkan)),
		device(std::move(device))
	{
		VkPipelineLibraryCreateInfoKHR libraries{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR,
		};

		VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_create_info{
			.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.stageCount = static_cast<uint32_t>(shader_stage_infos.size()),
			.pStages = shader_stage_infos.data(),
			.groupCount = static_cast<uint32_t>(shader_groups.size()),
			.pGroups = shader_groups.data(),
			.maxRecursionDepth = 1,
			.libraries = libraries,
			.pLibraryInterface = nullptr,
			.layout = pipeline_layout->layout,
		};

		THROW_ON_ERROR(this->vulkan->vkCreateRayTracingPipelinesKHR(
			this->device->device, VK_NULL_HANDLE, 1, &raytracing_pipeline_create_info, nullptr, &this->pipeline));
	}

	~VulkanRayTracingPipeline() = default;

	std::shared_ptr<VulkanInstance> vulkan;
	std::shared_ptr<VulkanDevice> device;
	VkPipeline pipeline;
};


class VulkanAccelerationStructure {
public:
	VulkanAccelerationStructure(
		std::shared_ptr<VulkanInstance> vulkan,
		std::shared_ptr<VulkanDevice> device,
		VkDeviceSize compactedSize,
		VkAccelerationStructureTypeKHR type,
		VkBuildAccelerationStructureFlagsKHR flags,
		std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> geometryInfos,
		VkDeviceAddress deviceAddress) :
		vulkan(std::move(vulkan)),
		device(std::move(device)),
		type(type)
	{
		VkAccelerationStructureCreateInfoKHR create_info{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.compactedSize = compactedSize,
			.type = this->type,
			.flags = flags,
			.maxGeometryCount = static_cast<uint32_t>(geometryInfos.size()),
			.pGeometryInfos = geometryInfos.data(),
			.deviceAddress = deviceAddress
		};

		THROW_ON_ERROR(this->vulkan->vkCreateAccelerationStructureKHR(this->device->device, &create_info, nullptr, &this->as));

		VkMemoryRequirements memory_requirements =
			this->getMemoryRequirements(VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR);

		uint32_t memory_type_index = this->device->physical_device.getMemoryTypeIndex(
			memory_requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		this->memory = std::make_shared<VulkanMemory>(
			this->device,
			memory_requirements.size,
			memory_type_index);

		VkBindAccelerationStructureMemoryInfoKHR memory_info{
			.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR,
			.pNext = nullptr,
			.accelerationStructure = this->as,
			.memory = this->memory->memory,
			.memoryOffset = 0,
			.deviceIndexCount = 0,
			.pDeviceIndices = nullptr,
		};

		this->vulkan->vkBindAccelerationStructureMemoryKHR(this->device->device, 1, &memory_info);
	}


	~VulkanAccelerationStructure()
	{
		this->vulkan->vkDestroyAccelerationStructureKHR(this->device->device, this->as, nullptr);
	}


	VkMemoryRequirements getMemoryRequirements(VkAccelerationStructureMemoryRequirementsTypeKHR type)
	{
		VkMemoryRequirements2 memory_requirements2{
			.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
			.pNext = nullptr,
		};

		VkAccelerationStructureMemoryRequirementsInfoKHR memory_requirements_info{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR,
			.pNext = nullptr,
			.type = type,
			.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			.accelerationStructure = this->as
		};

		this->vulkan->vkGetAccelerationStructureMemoryRequirementsKHR(
			this->device->device, &memory_requirements_info, &memory_requirements2);

		return memory_requirements2.memoryRequirements;
	}


	void build(
		VkCommandBuffer command,
		std::vector<VkAccelerationStructureGeometryKHR>& geometries,
		std::vector<VkAccelerationStructureBuildOffsetInfoKHR>& build_offset_infos)
	{
		VkMemoryRequirements memory_requirements =
			this->getMemoryRequirements(VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR);

		auto scratch_buffer = std::make_shared<VulkanBufferObject>(
			this->device,
			0,
			memory_requirements.size,
			VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceAddress address = this->device->getDeviceAddress(scratch_buffer->buffer->buffer);
		VkAccelerationStructureGeometryKHR* pGeometries = geometries.data();

		VkAccelerationStructureBuildGeometryInfoKHR build_geometry_info{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.pNext = nullptr,
			.type = this->type,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.update = VK_FALSE,
			.srcAccelerationStructure = VK_NULL_HANDLE,
			.dstAccelerationStructure = this->as,
			.geometryArrayOfPointers = VK_FALSE,
			.geometryCount = static_cast<uint32_t>(geometries.size()),
			.ppGeometries = &pGeometries,
			.scratchData = address
		};

		VkAccelerationStructureBuildOffsetInfoKHR* pInfos = build_offset_infos.data();
		this->vulkan->vkCmdBuildAccelerationStructureKHR(
			command,
			static_cast<uint32_t>(build_offset_infos.size()),
			&build_geometry_info,
			&pInfos);

		VkMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
			.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
		};

		vk.CmdPipelineBarrier(
			command,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			0, 1, &barrier, 0, nullptr, 0, nullptr);
	}

	std::shared_ptr<VulkanInstance> vulkan;
	std::shared_ptr<VulkanDevice> device;
	VkAccelerationStructureTypeKHR type;
	VkAccelerationStructureKHR as;
	std::shared_ptr<VulkanMemory> memory;
};
#endif
