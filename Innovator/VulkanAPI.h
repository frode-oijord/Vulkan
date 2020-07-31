#pragma once

#include <stdexcept>

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include <dlfcn.h>
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
#pragma warning(disable : 26812)
#endif
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>


class VulkanAPI {
public:
	// VK_core
	PFN_vkCreateInstance CreateInstance;
	PFN_vkDestroyInstance DestroyInstance;
	PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices;
	PFN_vkGetPhysicalDeviceFeatures GetPhysicalDeviceFeatures;
	PFN_vkGetPhysicalDeviceFormatProperties GetPhysicalDeviceFormatProperties;
	PFN_vkGetPhysicalDeviceImageFormatProperties GetPhysicalDeviceImageFormatProperties;
	PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties GetPhysicalDeviceQueueFamilyProperties;
	PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties;
	PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
	PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
	PFN_vkCreateDevice CreateDevice;
	PFN_vkDestroyDevice DestroyDevice;
	PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties;
	PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties;
	PFN_vkEnumerateInstanceLayerProperties EnumerateInstanceLayerProperties;
	PFN_vkEnumerateDeviceLayerProperties EnumerateDeviceLayerProperties;
	PFN_vkGetDeviceQueue GetDeviceQueue;
	PFN_vkQueueSubmit QueueSubmit;
	PFN_vkQueueWaitIdle QueueWaitIdle;
	PFN_vkDeviceWaitIdle DeviceWaitIdle;
	PFN_vkAllocateMemory AllocateMemory;
	PFN_vkFreeMemory FreeMemory;
	PFN_vkMapMemory MapMemory;
	PFN_vkUnmapMemory UnmapMemory;
	PFN_vkFlushMappedMemoryRanges FlushMappedMemoryRanges;
	PFN_vkInvalidateMappedMemoryRanges InvalidateMappedMemoryRanges;
	PFN_vkGetDeviceMemoryCommitment GetDeviceMemoryCommitment;
	PFN_vkBindBufferMemory BindBufferMemory;
	PFN_vkBindImageMemory BindImageMemory;
	PFN_vkGetBufferMemoryRequirements GetBufferMemoryRequirements;
	PFN_vkGetImageMemoryRequirements GetImageMemoryRequirements;
	PFN_vkGetImageSparseMemoryRequirements GetImageSparseMemoryRequirements;
	PFN_vkGetPhysicalDeviceSparseImageFormatProperties GetPhysicalDeviceSparseImageFormatProperties;
	PFN_vkQueueBindSparse QueueBindSparse;
	PFN_vkCreateFence CreateFence;
	PFN_vkDestroyFence DestroyFence;
	PFN_vkResetFences ResetFences;
	PFN_vkGetFenceStatus GetFenceStatus;
	PFN_vkWaitForFences WaitForFences;
	PFN_vkCreateSemaphore CreateSemaphore;
	PFN_vkDestroySemaphore DestroySemaphore;
	PFN_vkCreateEvent CreateEvent;
	PFN_vkDestroyEvent DestroyEvent;
	PFN_vkGetEventStatus GetEventStatus;
	PFN_vkSetEvent SetEvent;
	PFN_vkResetEvent ResetEvent;
	PFN_vkCreateQueryPool CreateQueryPool;
	PFN_vkDestroyQueryPool DestroyQueryPool;
	PFN_vkGetQueryPoolResults GetQueryPoolResults;
	PFN_vkCreateBuffer CreateBuffer;
	PFN_vkDestroyBuffer DestroyBuffer;
	PFN_vkCreateBufferView CreateBufferView;
	PFN_vkDestroyBufferView DestroyBufferView;
	PFN_vkCreateImage CreateImage;
	PFN_vkDestroyImage DestroyImage;
	PFN_vkGetImageSubresourceLayout GetImageSubresourceLayout;
	PFN_vkCreateImageView CreateImageView;
	PFN_vkDestroyImageView DestroyImageView;
	PFN_vkCreateShaderModule CreateShaderModule;
	PFN_vkDestroyShaderModule DestroyShaderModule;
	PFN_vkCreatePipelineCache CreatePipelineCache;
	PFN_vkDestroyPipelineCache DestroyPipelineCache;
	PFN_vkGetPipelineCacheData GetPipelineCacheData;
	PFN_vkMergePipelineCaches MergePipelineCaches;
	PFN_vkCreateGraphicsPipelines CreateGraphicsPipelines;
	PFN_vkCreateComputePipelines CreateComputePipelines;
	PFN_vkDestroyPipeline DestroyPipeline;
	PFN_vkCreatePipelineLayout CreatePipelineLayout;
	PFN_vkDestroyPipelineLayout DestroyPipelineLayout;
	PFN_vkCreateSampler CreateSampler;
	PFN_vkDestroySampler DestroySampler;
	PFN_vkCreateDescriptorSetLayout CreateDescriptorSetLayout;
	PFN_vkDestroyDescriptorSetLayout DestroyDescriptorSetLayout;
	PFN_vkCreateDescriptorPool CreateDescriptorPool;
	PFN_vkDestroyDescriptorPool DestroyDescriptorPool;
	PFN_vkResetDescriptorPool ResetDescriptorPool;
	PFN_vkAllocateDescriptorSets AllocateDescriptorSets;
	PFN_vkFreeDescriptorSets FreeDescriptorSets;
	PFN_vkUpdateDescriptorSets UpdateDescriptorSets;
	PFN_vkCreateFramebuffer CreateFramebuffer;
	PFN_vkDestroyFramebuffer DestroyFramebuffer;
	PFN_vkCreateRenderPass CreateRenderPass;
	PFN_vkDestroyRenderPass DestroyRenderPass;
	PFN_vkGetRenderAreaGranularity GetRenderAreaGranularity;
	PFN_vkCreateCommandPool CreateCommandPool;
	PFN_vkDestroyCommandPool DestroyCommandPool;
	PFN_vkResetCommandPool ResetCommandPool;
	PFN_vkAllocateCommandBuffers AllocateCommandBuffers;
	PFN_vkFreeCommandBuffers FreeCommandBuffers;
	PFN_vkBeginCommandBuffer BeginCommandBuffer;
	PFN_vkEndCommandBuffer EndCommandBuffer;
	PFN_vkResetCommandBuffer ResetCommandBuffer;
	PFN_vkCmdBindPipeline CmdBindPipeline;
	PFN_vkCmdSetViewport CmdSetViewport;
	PFN_vkCmdSetScissor CmdSetScissor;
	PFN_vkCmdSetLineWidth CmdSetLineWidth;
	PFN_vkCmdSetDepthBias CmdSetDepthBias;
	PFN_vkCmdSetBlendConstants CmdSetBlendConstants;
	PFN_vkCmdSetDepthBounds CmdSetDepthBounds;
	PFN_vkCmdSetStencilCompareMask CmdSetStencilCompareMask;
	PFN_vkCmdSetStencilWriteMask CmdSetStencilWriteMask;
	PFN_vkCmdSetStencilReference CmdSetStencilReference;
	PFN_vkCmdBindDescriptorSets CmdBindDescriptorSets;
	PFN_vkCmdBindIndexBuffer CmdBindIndexBuffer;
	PFN_vkCmdBindVertexBuffers CmdBindVertexBuffers;
	PFN_vkCmdDraw CmdDraw;
	PFN_vkCmdDrawIndexed CmdDrawIndexed;
	PFN_vkCmdDrawIndirect CmdDrawIndirect;
	PFN_vkCmdDrawIndexedIndirect CmdDrawIndexedIndirect;
	PFN_vkCmdDispatch CmdDispatch;
	PFN_vkCmdDispatchIndirect CmdDispatchIndirect;
	PFN_vkCmdCopyBuffer CmdCopyBuffer;
	PFN_vkCmdCopyImage CmdCopyImage;
	PFN_vkCmdBlitImage CmdBlitImage;
	PFN_vkCmdCopyBufferToImage CmdCopyBufferToImage;
	PFN_vkCmdCopyImageToBuffer CmdCopyImageToBuffer;
	PFN_vkCmdUpdateBuffer CmdUpdateBuffer;
	PFN_vkCmdFillBuffer CmdFillBuffer;
	PFN_vkCmdClearColorImage CmdClearColorImage;
	PFN_vkCmdClearDepthStencilImage CmdClearDepthStencilImage;
	PFN_vkCmdClearAttachments CmdClearAttachments;
	PFN_vkCmdResolveImage CmdResolveImage;
	PFN_vkCmdSetEvent CmdSetEvent;
	PFN_vkCmdResetEvent CmdResetEvent;
	PFN_vkCmdWaitEvents CmdWaitEvents;
	PFN_vkCmdPipelineBarrier CmdPipelineBarrier;
	PFN_vkCmdBeginQuery CmdBeginQuery;
	PFN_vkCmdEndQuery CmdEndQuery;
	PFN_vkCmdResetQueryPool CmdResetQueryPool;
	PFN_vkCmdWriteTimestamp CmdWriteTimestamp;
	PFN_vkCmdCopyQueryPoolResults CmdCopyQueryPoolResults;
	PFN_vkCmdPushConstants CmdPushConstants;
	PFN_vkCmdBeginRenderPass CmdBeginRenderPass;
	PFN_vkCmdNextSubpass CmdNextSubpass;
	PFN_vkCmdEndRenderPass CmdEndRenderPass;
	PFN_vkCmdExecuteCommands CmdExecuteCommands;

	// VK_KHR_surface
	PFN_vkDestroySurfaceKHR DestroySurfaceKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR;

	// VK_KHR_swapchain
	PFN_vkCreateSwapchainKHR CreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR DestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR AcquireNextImageKHR;
	PFN_vkQueuePresentKHR QueuePresentKHR;

	// VK_KHR_display
	PFN_vkGetPhysicalDeviceDisplayPropertiesKHR GetPhysicalDeviceDisplayPropertiesKHR;
	PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR GetPhysicalDeviceDisplayPlanePropertiesKHR;
	PFN_vkGetDisplayPlaneSupportedDisplaysKHR GetDisplayPlaneSupportedDisplaysKHR;
	PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR;
	PFN_vkCreateDisplayModeKHR CreateDisplayModeKHR;
	PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR;
	PFN_vkCreateDisplayPlaneSurfaceKHR CreateDisplayPlaneSurfaceKHR;

	// VK_KHR_display_swapchain
	PFN_vkCreateSharedSwapchainsKHR CreateSharedSwapchainsKHR;

#ifdef VK_USE_PLATFORM_XLIB_KHR
	// VK_KHR_xlib_surface
	PFN_vkCreateXlibSurfaceKHR CreateXlibSurfaceKHR;
	PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR GetPhysicalDeviceXlibPresentationSupportKHR;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
	// VK_KHR_xcb_surface
	PFN_vkCreateXcbSurfaceKHR CreateXcbSurfaceKHR;
	PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR GetPhysicalDeviceXcbPresentationSupportKHR;
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	// VK_KHR_wayland_surface
	PFN_vkCreateWaylandSurfaceKHR CreateWaylandSurfaceKHR;
	PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR GetPhysicalDeviceWaylandPresentationSupportKHR;
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
	// VK_KHR_android_surface
	PFN_vkCreateAndroidSurfaceKHR CreateAndroidSurfaceKHR;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
	// VK_KHR_win32_surface
	PFN_vkCreateWin32SurfaceKHR CreateWin32SurfaceKHR;
	PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR GetPhysicalDeviceWin32PresentationSupportKHR;
#endif

	PFN_vkGetPhysicalDeviceFeatures2 GetPhysicalDeviceFeatures2;

	VulkanAPI()
	{
#ifdef VK_USE_PLATFORM_ANDROID_KHR
		void* libvulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
		HINSTANCE libvulkan = LoadLibrary("vulkan-1.dll");
#define dlsym GetProcAddress
#endif

		if (!libvulkan) {
			throw std::runtime_error("unable to load Vulkan");
		}

		CreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dlsym(libvulkan, "vkCreateInstance"));
		DestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(dlsym(libvulkan, "vkDestroyInstance"));
		EnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(dlsym(libvulkan, "vkEnumeratePhysicalDevices"));
		GetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(dlsym(libvulkan, "vkGetPhysicalDeviceFeatures"));
		GetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceFormatProperties"));
		GetPhysicalDeviceImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceImageFormatProperties"));
		GetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceProperties"));
		GetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceQueueFamilyProperties"));
		GetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceMemoryProperties"));
		GetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(libvulkan, "vkGetInstanceProcAddr"));
		GetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(dlsym(libvulkan, "vkGetDeviceProcAddr"));
		CreateDevice = reinterpret_cast<PFN_vkCreateDevice>(dlsym(libvulkan, "vkCreateDevice"));
		DestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(dlsym(libvulkan, "vkDestroyDevice"));
		EnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(dlsym(libvulkan, "vkEnumerateInstanceExtensionProperties"));
		EnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(dlsym(libvulkan, "vkEnumerateDeviceExtensionProperties"));
		EnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(dlsym(libvulkan, "vkEnumerateInstanceLayerProperties"));
		EnumerateDeviceLayerProperties = reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(dlsym(libvulkan, "vkEnumerateDeviceLayerProperties"));
		GetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(dlsym(libvulkan, "vkGetDeviceQueue"));
		QueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(dlsym(libvulkan, "vkQueueSubmit"));
		QueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(dlsym(libvulkan, "vkQueueWaitIdle"));
		DeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(dlsym(libvulkan, "vkDeviceWaitIdle"));
		AllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(dlsym(libvulkan, "vkAllocateMemory"));
		FreeMemory = reinterpret_cast<PFN_vkFreeMemory>(dlsym(libvulkan, "vkFreeMemory"));
		MapMemory = reinterpret_cast<PFN_vkMapMemory>(dlsym(libvulkan, "vkMapMemory"));
		UnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(dlsym(libvulkan, "vkUnmapMemory"));
		FlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(dlsym(libvulkan, "vkFlushMappedMemoryRanges"));
		InvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(dlsym(libvulkan, "vkInvalidateMappedMemoryRanges"));
		GetDeviceMemoryCommitment = reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(dlsym(libvulkan, "vkGetDeviceMemoryCommitment"));
		BindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(dlsym(libvulkan, "vkBindBufferMemory"));
		BindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(dlsym(libvulkan, "vkBindImageMemory"));
		GetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(dlsym(libvulkan, "vkGetBufferMemoryRequirements"));
		GetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(dlsym(libvulkan, "vkGetImageMemoryRequirements"));
		GetImageSparseMemoryRequirements = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements>(dlsym(libvulkan, "vkGetImageSparseMemoryRequirements"));
		GetPhysicalDeviceSparseImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>(dlsym(libvulkan, "vk.GetPhysicalDeviceSparseImageFormatProperties"));
		QueueBindSparse = reinterpret_cast<PFN_vkQueueBindSparse>(dlsym(libvulkan, "vkQueueBindSparse"));
		CreateFence = reinterpret_cast<PFN_vkCreateFence>(dlsym(libvulkan, "vkCreateFence"));
		DestroyFence = reinterpret_cast<PFN_vkDestroyFence>(dlsym(libvulkan, "vkDestroyFence"));
		ResetFences = reinterpret_cast<PFN_vkResetFences>(dlsym(libvulkan, "vkResetFences"));
		GetFenceStatus = reinterpret_cast<PFN_vkGetFenceStatus>(dlsym(libvulkan, "vkGetFenceStatus"));
		WaitForFences = reinterpret_cast<PFN_vkWaitForFences>(dlsym(libvulkan, "vkWaitForFences"));
		CreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(dlsym(libvulkan, "vkCreateSemaphore"));
		DestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(dlsym(libvulkan, "vkDestroySemaphore"));
		CreateEvent = reinterpret_cast<PFN_vkCreateEvent>(dlsym(libvulkan, "vkCreateEvent"));
		DestroyEvent = reinterpret_cast<PFN_vkDestroyEvent>(dlsym(libvulkan, "vkDestroyEvent"));
		GetEventStatus = reinterpret_cast<PFN_vkGetEventStatus>(dlsym(libvulkan, "vkGetEventStatus"));
		SetEvent = reinterpret_cast<PFN_vkSetEvent>(dlsym(libvulkan, "vkSetEvent"));
		ResetEvent = reinterpret_cast<PFN_vkResetEvent>(dlsym(libvulkan, "vkResetEvent"));
		CreateQueryPool = reinterpret_cast<PFN_vkCreateQueryPool>(dlsym(libvulkan, "vkCreateQueryPool"));
		DestroyQueryPool = reinterpret_cast<PFN_vkDestroyQueryPool>(dlsym(libvulkan, "vkDestroyQueryPool"));
		GetQueryPoolResults = reinterpret_cast<PFN_vkGetQueryPoolResults>(dlsym(libvulkan, "vkGetQueryPoolResults"));
		CreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(dlsym(libvulkan, "vkCreateBuffer"));
		DestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(dlsym(libvulkan, "vkDestroyBuffer"));
		CreateBufferView = reinterpret_cast<PFN_vkCreateBufferView>(dlsym(libvulkan, "vkCreateBufferView"));
		DestroyBufferView = reinterpret_cast<PFN_vkDestroyBufferView>(dlsym(libvulkan, "vkDestroyBufferView"));
		CreateImage = reinterpret_cast<PFN_vkCreateImage>(dlsym(libvulkan, "vkCreateImage"));
		DestroyImage = reinterpret_cast<PFN_vkDestroyImage>(dlsym(libvulkan, "vkDestroyImage"));
		GetImageSubresourceLayout = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(dlsym(libvulkan, "vkGetImageSubresourceLayout"));
		CreateImageView = reinterpret_cast<PFN_vkCreateImageView>(dlsym(libvulkan, "vkCreateImageView"));
		DestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(dlsym(libvulkan, "vkDestroyImageView"));
		CreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(dlsym(libvulkan, "vkCreateShaderModule"));
		DestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(dlsym(libvulkan, "vkDestroyShaderModule"));
		CreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(dlsym(libvulkan, "vkCreatePipelineCache"));
		DestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(dlsym(libvulkan, "vkDestroyPipelineCache"));
		GetPipelineCacheData = reinterpret_cast<PFN_vkGetPipelineCacheData>(dlsym(libvulkan, "vkGetPipelineCacheData"));
		MergePipelineCaches = reinterpret_cast<PFN_vkMergePipelineCaches>(dlsym(libvulkan, "vkMergePipelineCaches"));
		CreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(dlsym(libvulkan, "vkCreateGraphicsPipelines"));
		CreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(dlsym(libvulkan, "vkCreateComputePipelines"));
		DestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(dlsym(libvulkan, "vkDestroyPipeline"));
		CreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(dlsym(libvulkan, "vkCreatePipelineLayout"));
		DestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(dlsym(libvulkan, "vkDestroyPipelineLayout"));
		CreateSampler = reinterpret_cast<PFN_vkCreateSampler>(dlsym(libvulkan, "vkCreateSampler"));
		DestroySampler = reinterpret_cast<PFN_vkDestroySampler>(dlsym(libvulkan, "vkDestroySampler"));
		CreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(dlsym(libvulkan, "vkCreateDescriptorSetLayout"));
		DestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(dlsym(libvulkan, "vkDestroyDescriptorSetLayout"));
		CreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(dlsym(libvulkan, "vkCreateDescriptorPool"));
		DestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(dlsym(libvulkan, "vkDestroyDescriptorPool"));
		ResetDescriptorPool = reinterpret_cast<PFN_vkResetDescriptorPool>(dlsym(libvulkan, "vkResetDescriptorPool"));
		AllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(dlsym(libvulkan, "vkAllocateDescriptorSets"));
		FreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(dlsym(libvulkan, "vkFreeDescriptorSets"));
		UpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(dlsym(libvulkan, "vkUpdateDescriptorSets"));
		CreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(dlsym(libvulkan, "vkCreateFramebuffer"));
		DestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(dlsym(libvulkan, "vkDestroyFramebuffer"));
		CreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(dlsym(libvulkan, "vkCreateRenderPass"));
		DestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(dlsym(libvulkan, "vkDestroyRenderPass"));
		GetRenderAreaGranularity = reinterpret_cast<PFN_vkGetRenderAreaGranularity>(dlsym(libvulkan, "vkGetRenderAreaGranularity"));
		CreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(dlsym(libvulkan, "vkCreateCommandPool"));
		DestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(dlsym(libvulkan, "vkDestroyCommandPool"));
		ResetCommandPool = reinterpret_cast<PFN_vkResetCommandPool>(dlsym(libvulkan, "vkResetCommandPool"));
		AllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(dlsym(libvulkan, "vkAllocateCommandBuffers"));
		FreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(dlsym(libvulkan, "vkFreeCommandBuffers"));
		BeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(dlsym(libvulkan, "vkBeginCommandBuffer"));
		EndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(dlsym(libvulkan, "vkEndCommandBuffer"));
		ResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(dlsym(libvulkan, "vkResetCommandBuffer"));
		CmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(dlsym(libvulkan, "vkCmdBindPipeline"));
		CmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(dlsym(libvulkan, "vkCmdSetViewport"));
		CmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(dlsym(libvulkan, "vkCmdSetScissor"));
		CmdSetLineWidth = reinterpret_cast<PFN_vkCmdSetLineWidth>(dlsym(libvulkan, "vkCmdSetLineWidth"));
		CmdSetDepthBias = reinterpret_cast<PFN_vkCmdSetDepthBias>(dlsym(libvulkan, "vkCmdSetDepthBias"));
		CmdSetBlendConstants = reinterpret_cast<PFN_vkCmdSetBlendConstants>(dlsym(libvulkan, "vkCmdSetBlendConstants"));
		CmdSetDepthBounds = reinterpret_cast<PFN_vkCmdSetDepthBounds>(dlsym(libvulkan, "vkCmdSetDepthBounds"));
		CmdSetStencilCompareMask = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(dlsym(libvulkan, "vkCmdSetStencilCompareMask"));
		CmdSetStencilWriteMask = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(dlsym(libvulkan, "vkCmdSetStencilWriteMask"));
		CmdSetStencilReference = reinterpret_cast<PFN_vkCmdSetStencilReference>(dlsym(libvulkan, "vkCmdSetStencilReference"));
		CmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(dlsym(libvulkan, "vkCmdBindDescriptorSets"));
		CmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(dlsym(libvulkan, "vkCmdBindIndexBuffer"));
		CmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(dlsym(libvulkan, "vkCmdBindVertexBuffers"));
		CmdDraw = reinterpret_cast<PFN_vkCmdDraw>(dlsym(libvulkan, "vkCmdDraw"));
		CmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(dlsym(libvulkan, "vkCmdDrawIndexed"));
		CmdDrawIndirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(dlsym(libvulkan, "vkCmdDrawIndirect"));
		CmdDrawIndexedIndirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(dlsym(libvulkan, "vkCmdDrawIndexedIndirect"));
		CmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(dlsym(libvulkan, "vkCmdDispatch"));
		CmdDispatchIndirect = reinterpret_cast<PFN_vkCmdDispatchIndirect>(dlsym(libvulkan, "vkCmdDispatchIndirect"));
		CmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(dlsym(libvulkan, "vkCmdCopyBuffer"));
		CmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(dlsym(libvulkan, "vkCmdCopyImage"));
		CmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(dlsym(libvulkan, "vkCmdBlitImage"));
		CmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(dlsym(libvulkan, "vkCmdCopyBufferToImage"));
		CmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(dlsym(libvulkan, "vkCmdCopyImageToBuffer"));
		CmdUpdateBuffer = reinterpret_cast<PFN_vkCmdUpdateBuffer>(dlsym(libvulkan, "vkCmdUpdateBuffer"));
		CmdFillBuffer = reinterpret_cast<PFN_vkCmdFillBuffer>(dlsym(libvulkan, "vkCmdFillBuffer"));
		CmdClearColorImage = reinterpret_cast<PFN_vkCmdClearColorImage>(dlsym(libvulkan, "vkCmdClearColorImage"));
		CmdClearDepthStencilImage = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(dlsym(libvulkan, "vkCmdClearDepthStencilImage"));
		CmdClearAttachments = reinterpret_cast<PFN_vkCmdClearAttachments>(dlsym(libvulkan, "vkCmdClearAttachments"));
		CmdResolveImage = reinterpret_cast<PFN_vkCmdResolveImage>(dlsym(libvulkan, "vkCmdResolveImage"));
		CmdSetEvent = reinterpret_cast<PFN_vkCmdSetEvent>(dlsym(libvulkan, "vkCmdSetEvent"));
		CmdResetEvent = reinterpret_cast<PFN_vkCmdResetEvent>(dlsym(libvulkan, "vkCmdResetEvent"));
		CmdWaitEvents = reinterpret_cast<PFN_vkCmdWaitEvents>(dlsym(libvulkan, "vkCmdWaitEvents"));
		CmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(dlsym(libvulkan, "vkCmdPipelineBarrier"));
		CmdBeginQuery = reinterpret_cast<PFN_vkCmdBeginQuery>(dlsym(libvulkan, "vkCmdBeginQuery"));
		CmdEndQuery = reinterpret_cast<PFN_vkCmdEndQuery>(dlsym(libvulkan, "vkCmdEndQuery"));
		CmdResetQueryPool = reinterpret_cast<PFN_vkCmdResetQueryPool>(dlsym(libvulkan, "vkCmdResetQueryPool"));
		CmdWriteTimestamp = reinterpret_cast<PFN_vkCmdWriteTimestamp>(dlsym(libvulkan, "vkCmdWriteTimestamp"));
		CmdCopyQueryPoolResults = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(dlsym(libvulkan, "vkCmdCopyQueryPoolResults"));
		CmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(dlsym(libvulkan, "vkCmdPushConstants"));
		CmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(dlsym(libvulkan, "vkCmdBeginRenderPass"));
		CmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(dlsym(libvulkan, "vkCmdNextSubpass"));
		CmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(dlsym(libvulkan, "vkCmdEndRenderPass"));
		CmdExecuteCommands = reinterpret_cast<PFN_vkCmdExecuteCommands>(dlsym(libvulkan, "vkCmdExecuteCommands"));
		DestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(dlsym(libvulkan, "vkDestroySurfaceKHR"));
		GetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceSupportKHR"));
		GetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
		GetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
		GetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
		CreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(dlsym(libvulkan, "vkCreateSwapchainKHR"));
		DestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(dlsym(libvulkan, "vkDestroySwapchainKHR"));
		GetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(dlsym(libvulkan, "vkGetSwapchainImagesKHR"));
		AcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(dlsym(libvulkan, "vkAcquireNextImageKHR"));
		QueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(dlsym(libvulkan, "vkQueuePresentKHR"));
		GetPhysicalDeviceDisplayPropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceDisplayPropertiesKHR"));
		GetPhysicalDeviceDisplayPlanePropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR"));
		GetDisplayPlaneSupportedDisplaysKHR = reinterpret_cast<PFN_vkGetDisplayPlaneSupportedDisplaysKHR>(dlsym(libvulkan, "vkGetDisplayPlaneSupportedDisplaysKHR"));
		GetDisplayModePropertiesKHR = reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(dlsym(libvulkan, "vkGetDisplayModePropertiesKHR"));
		CreateDisplayModeKHR = reinterpret_cast<PFN_vkCreateDisplayModeKHR>(dlsym(libvulkan, "vkCreateDisplayModeKHR"));
		GetDisplayPlaneCapabilitiesKHR = reinterpret_cast<PFN_vkGetDisplayPlaneCapabilitiesKHR>(dlsym(libvulkan, "vkGetDisplayPlaneCapabilitiesKHR"));
		CreateDisplayPlaneSurfaceKHR = reinterpret_cast<PFN_vkCreateDisplayPlaneSurfaceKHR>(dlsym(libvulkan, "vkCreateDisplayPlaneSurfaceKHR"));
		CreateSharedSwapchainsKHR = reinterpret_cast<PFN_vkCreateSharedSwapchainsKHR>(dlsym(libvulkan, "vkCreateSharedSwapchainsKHR"));

#ifdef VK_USE_PLATFORM_XLIB_KHR
		CreateXlibSurfaceKHR = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(dlsym(libvulkan, "vkCreateXlibSurfaceKHR"));
		GetPhysicalDeviceXlibPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceXlibPresentationSupportKHR"));
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
		CreateXcbSurfaceKHR = reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(dlsym(libvulkan, "vkCreateXcbSurfaceKHR"));
		GetPhysicalDeviceXcbPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceXcbPresentationSupportKHR"));
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
		CreateWaylandSurfaceKHR = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(dlsym(libvulkan, "vkCreateWaylandSurfaceKHR"));
		GetPhysicalDeviceWaylandPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceWaylandPresentationSupportKHR"));
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
		CreateAndroidSurfaceKHR = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(dlsym(libvulkan, "vkCreateAndroidSurfaceKHR"));
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
		CreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(dlsym(libvulkan, "vkCreateWin32SurfaceKHR"));
		GetPhysicalDeviceWin32PresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));
#endif
		GetPhysicalDeviceFeatures2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(dlsym(libvulkan, "vkGetPhysicalDeviceFeatures2"));
	}
};


static VulkanAPI vk;
