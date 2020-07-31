#pragma once

#include <Innovator/Defines.h>

#include <array>
#include <utility>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>

#include <cstring>

class VulkanObject {
public:
	virtual std::string toString() = 0;
};

class VulkanPhysicalDevice {
public:
	VulkanPhysicalDevice(VulkanPhysicalDevice&& self) = default;
	VulkanPhysicalDevice(const VulkanPhysicalDevice& self) = default;
	VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&& self) = delete;
	VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice& self) = delete;
	~VulkanPhysicalDevice() = default;

	explicit VulkanPhysicalDevice(VkPhysicalDevice device)
		: device(device)
	{
		this->features2.pNext = &this->device_address_features;

		vk.GetPhysicalDeviceFeatures(this->device, &this->features);
		vk.GetPhysicalDeviceFeatures2(this->device, &this->features2);
		vk.GetPhysicalDeviceProperties(this->device, &this->properties);
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

	bool supportsFeatures(const VkPhysicalDeviceFeatures& required_features) const
	{
		return
			this->features.logicOp >= required_features.logicOp &&
			this->features.wideLines >= required_features.wideLines &&
			this->features.depthClamp >= required_features.depthClamp &&
			this->features.alphaToOne >= required_features.alphaToOne &&
			this->features.depthBounds >= required_features.depthBounds &&
			this->features.largePoints >= required_features.largePoints &&
			this->features.shaderInt64 >= required_features.shaderInt64 &&
			this->features.shaderInt16 >= required_features.shaderInt16 &&
			this->features.dualSrcBlend >= required_features.dualSrcBlend &&
			this->features.multiViewport >= required_features.multiViewport &&
			this->features.shaderFloat64 >= required_features.shaderFloat64 &&
			this->features.sparseBinding >= required_features.sparseBinding &&
			this->features.imageCubeArray >= required_features.imageCubeArray &&
			this->features.geometryShader >= required_features.geometryShader &&
			this->features.depthBiasClamp >= required_features.depthBiasClamp &&
			this->features.independentBlend >= required_features.independentBlend &&
			this->features.fillModeNonSolid >= required_features.fillModeNonSolid &&
			this->features.inheritedQueries >= required_features.inheritedQueries &&
			this->features.sampleRateShading >= required_features.sampleRateShading &&
			this->features.multiDrawIndirect >= required_features.multiDrawIndirect &&
			this->features.samplerAnisotropy >= required_features.samplerAnisotropy &&
			this->features.robustBufferAccess >= required_features.robustBufferAccess &&
			this->features.tessellationShader >= required_features.tessellationShader &&
			this->features.shaderClipDistance >= required_features.shaderClipDistance &&
			this->features.shaderCullDistance >= required_features.shaderCullDistance &&
			this->features.fullDrawIndexUint32 >= required_features.fullDrawIndexUint32 &&
			this->features.textureCompressionBC >= required_features.textureCompressionBC &&
			this->features.shaderResourceMinLod >= required_features.shaderResourceMinLod &&
			this->features.occlusionQueryPrecise >= required_features.occlusionQueryPrecise &&
			this->features.sparseResidencyBuffer >= required_features.sparseResidencyBuffer &&
			this->features.textureCompressionETC2 >= required_features.textureCompressionETC2 &&
			this->features.sparseResidencyImage2D >= required_features.sparseResidencyImage2D &&
			this->features.sparseResidencyImage3D >= required_features.sparseResidencyImage3D &&
			this->features.sparseResidencyAliased >= required_features.sparseResidencyAliased &&
			this->features.pipelineStatisticsQuery >= required_features.pipelineStatisticsQuery &&
			this->features.shaderResourceResidency >= required_features.shaderResourceResidency &&
			this->features.sparseResidency2Samples >= required_features.sparseResidency2Samples &&
			this->features.sparseResidency4Samples >= required_features.sparseResidency4Samples &&
			this->features.sparseResidency8Samples >= required_features.sparseResidency8Samples &&
			this->features.variableMultisampleRate >= required_features.variableMultisampleRate &&
			this->features.sparseResidency16Samples >= required_features.sparseResidency16Samples &&
			this->features.fragmentStoresAndAtomics >= required_features.fragmentStoresAndAtomics &&
			this->features.drawIndirectFirstInstance >= required_features.drawIndirectFirstInstance &&
			this->features.shaderImageGatherExtended >= required_features.shaderImageGatherExtended &&
			this->features.textureCompressionASTC_LDR >= required_features.textureCompressionASTC_LDR &&
			this->features.shaderStorageImageMultisample >= required_features.shaderStorageImageMultisample &&
			this->features.vertexPipelineStoresAndAtomics >= required_features.vertexPipelineStoresAndAtomics &&
			this->features.shaderStorageImageExtendedFormats >= required_features.shaderStorageImageExtendedFormats &&
			this->features.shaderStorageImageReadWithoutFormat >= required_features.shaderStorageImageReadWithoutFormat &&
			this->features.shaderStorageImageWriteWithoutFormat >= required_features.shaderStorageImageWriteWithoutFormat &&
			this->features.shaderTessellationAndGeometryPointSize >= required_features.shaderTessellationAndGeometryPointSize &&
			this->features.shaderSampledImageArrayDynamicIndexing >= required_features.shaderSampledImageArrayDynamicIndexing &&
			this->features.shaderStorageImageArrayDynamicIndexing >= required_features.shaderStorageImageArrayDynamicIndexing &&
			this->features.shaderUniformBufferArrayDynamicIndexing >= required_features.shaderUniformBufferArrayDynamicIndexing &&
			this->features.shaderStorageBufferArrayDynamicIndexing >= required_features.shaderStorageBufferArrayDynamicIndexing;
	}

	VkPhysicalDevice device;
	VkPhysicalDeviceFeatures features{};

	// Provided by VK_KHR_buffer_device_address
	VkPhysicalDeviceBufferDeviceAddressFeaturesEXT device_address_features{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT
	};

	VkPhysicalDeviceFeatures2 features2{ 
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR 
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
	NO_COPY_OR_ASSIGNMENT(VulkanInstance)

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
			for (auto properties : layer_properties)
				if (std::strcmp(layer_name, properties.layerName) == 0)
					return;
			throw std::runtime_error("Required instance layer " + std::string(layer_name) + " not supported.");
			});

		uint32_t extension_count;
		THROW_ON_ERROR(vk.EnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));
		std::vector<VkExtensionProperties> extension_properties(extension_count);
		THROW_ON_ERROR(vk.EnumerateInstanceExtensionProperties(nullptr, &extension_count, extension_properties.data()));

		std::for_each(required_extensions.begin(), required_extensions.end(), [&](const char* extension_name) {
			for (auto properties : extension_properties)
				if (std::strcmp(extension_name, properties.extensionName) == 0)
					return;
			throw std::runtime_error("Required instance extension " + std::string(extension_name) + " not supported.");
			});

		VkApplicationInfo application_info{
			VK_STRUCTURE_TYPE_APPLICATION_INFO,						// sType
			nullptr,												// pNext
			application_name.c_str(),								// pApplicationName
			1,														// applicationVersion
			"Innovator",											// pEngineName
			1,														// engineVersion
			VK_API_VERSION_1_0,										// apiVersion
		};

		VkInstanceCreateInfo create_info{
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,					// sType 
			nullptr,												// pNext 
			0,														// flags
			&application_info,										// pApplicationInfo
			static_cast<uint32_t>(required_layers.size()),			// enabledLayerCount
			required_layers.data(),									// ppEnabledLayerNames
			static_cast<uint32_t>(required_extensions.size()),		// enabledExtensionCount
			required_extensions.data()								// ppEnabledExtensionNames
		};

		THROW_ON_ERROR(vk.CreateInstance(&create_info, nullptr, &this->instance));

		uint32_t physical_device_count;
		THROW_ON_ERROR(vk.EnumeratePhysicalDevices(this->instance, &physical_device_count, nullptr));

		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		THROW_ON_ERROR(vk.EnumeratePhysicalDevices(this->instance, &physical_device_count, physical_devices.data()));

		for (const auto& physical_device : physical_devices) {
			this->physical_devices.emplace_back(physical_device);
		}

		this->vkCreateDebugReportCallbackEXT = this->getProcAddress<PFN_vkCreateDebugReportCallbackEXT>("vkCreateDebugReportCallbackEXT");
		this->vkDestroyDebugReportCallbackEXT = this->getProcAddress<PFN_vkDestroyDebugReportCallbackEXT>("vkDestroyDebugReportCallbackEXT");
#ifdef VK_USE_PLATFORM_WIN32_KHR
		// VK_KHR_ray_tracing extension
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
			for (auto extension : device.extension_properties)
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

	VulkanPhysicalDevice selectPhysicalDevice(const VkPhysicalDeviceFeatures& required_features)
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

	VkSurfaceCapabilitiesKHR getPhysicalDeviceSurfaceCapabilities(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		VkSurfaceCapabilitiesKHR surface_capabilities;
		THROW_ON_ERROR(vk.GetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surface_capabilities));
		return surface_capabilities;
	}

	std::vector<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormats(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		uint32_t count;
		THROW_ON_ERROR(vk.GetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr));
		std::vector<VkSurfaceFormatKHR> surface_formats(count);
		THROW_ON_ERROR(vk.GetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, surface_formats.data()));
		return surface_formats;
	}

	std::vector<VkPresentModeKHR> getPhysicalDeviceSurfacePresentModes(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		uint32_t count;
		THROW_ON_ERROR(vk.GetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr));
		std::vector<VkPresentModeKHR> present_modes(count);
		THROW_ON_ERROR(vk.GetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, present_modes.data()));
		return present_modes;
	}

	// VK_EXT_debug_report
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;

	// VK_KHR_ray_tracing extension
#ifdef VK_USE_PLATFORM_WIN32_KHR
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
	NO_COPY_OR_ASSIGNMENT(VulkanDevice)
	VulkanDevice() = delete;

	VulkanDevice(
		std::shared_ptr<VulkanInstance> vulkan,
		const VkPhysicalDeviceFeatures2& device_features2,
		const std::vector<const char*>& required_layers = {},
		const std::vector<const char*>& required_extensions = {}) :
			physical_device(vulkan->selectPhysicalDevice(device_features2.features))
	{
		std::for_each(required_layers.begin(), required_layers.end(), [&](const char* layer_name) {
			for (auto properties : physical_device.layer_properties)
				if (std::strcmp(layer_name, properties.layerName) == 0)
					return;
			throw std::runtime_error("Required device layer " + std::string(layer_name) + " not supported.");
			});

		std::for_each(required_extensions.begin(), required_extensions.end(), [&](const char* extension_name) {
			for (auto properties : physical_device.extension_properties)
				if (std::strcmp(extension_name, properties.extensionName) == 0)
					return;
			throw std::runtime_error("Required device extension " + std::string(extension_name) + " not supported.");
			});

		std::array<float, 1> priorities = { 1.0f };
		uint32_t num_queues = static_cast<uint32_t>(this->physical_device.queue_family_properties.size());
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		for (uint32_t queue_index = 0; queue_index < num_queues; queue_index++) {
			queue_create_infos.push_back({
			  VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,			// sType
			  nullptr,												// pNext
			  0,													// flags
			  queue_index,											// queueFamilyIndex
			  static_cast<uint32_t>(priorities.size()),				// queueCount   
			  priorities.data()										// pQueuePriorities
				});
		}

		if (queue_create_infos.empty()) {
			throw std::runtime_error("no queues found");
		}

		VkDeviceCreateInfo device_create_info{
		  VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,						// sType
		  &device_features2,										// pNext
		  0,														// flags
		  static_cast<uint32_t>(queue_create_infos.size()),			// queueCreateInfoCount
		  queue_create_infos.data(),								// pQueueCreateInfos
		  static_cast<uint32_t>(required_layers.size()),			// enabledLayerCount
		  required_layers.data(),									// ppEnabledLayerNames
		  static_cast<uint32_t>(required_extensions.size()),		// enabledExtensionCount
		  required_extensions.data(),								// ppEnabledExtensionNames
		  nullptr,													// pEnabledFeatures
		};

		THROW_ON_ERROR(vk.CreateDevice(this->physical_device.device, &device_create_info, nullptr, &this->device));

		this->queues.resize(num_queues);
		for (uint32_t i = 0; i < num_queues; i++) {
			vk.GetDeviceQueue(this->device, i, 0, &this->queues[i]);
		}

		VkCommandPoolCreateInfo create_info{
		  VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,			// sType
		  nullptr,												// pNext 
		  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,		// flags
		  0,													// queueFamilyIndex 
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

	VkDevice device{ nullptr };
	VulkanPhysicalDevice physical_device;
	std::vector<VkQueue> queues;
	VkCommandPool default_pool{ 0 };
};

class VulkanMemory {
public:
	NO_COPY_OR_ASSIGNMENT(VulkanMemory)
		VulkanMemory() = delete;

	explicit VulkanMemory(
		std::shared_ptr<VulkanDevice> device,
		VkDeviceSize size,
		uint32_t memory_type_index,
		void* pNext = nullptr);

	~VulkanMemory();

	char* map(VkDeviceSize size, VkDeviceSize offset, VkMemoryMapFlags flags = 0) const;
	void unmap() const;
	void memcpy(const void* src, VkDeviceSize size, VkDeviceSize offset);

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

#ifdef VK_USE_PLATFORM_WIN32_KHR
class VulkanDebugCallback {
public:
	NO_COPY_OR_ASSIGNMENT(VulkanDebugCallback)
		VulkanDebugCallback() = delete;

	explicit VulkanDebugCallback(
		std::shared_ptr<VulkanInstance> vulkan,
		VkDebugReportFlagsEXT flags,
		PFN_vkDebugReportCallbackEXT callback = DebugCallback,
		void* userdata = nullptr) :
		vulkan(std::move(vulkan))
	{
		VkDebugReportCallbackCreateInfoEXT create_info{
			VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,		// sType
			nullptr,											// pNext
			flags,												// flags
			callback,											// pfnCallback
			userdata,											// pUserData
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
	NO_COPY_OR_ASSIGNMENT(VulkanSemaphore)
		VulkanSemaphore() = delete;

	explicit VulkanSemaphore(std::shared_ptr<VulkanDevice> device)
		: device(std::move(device))
	{
		VkSemaphoreCreateInfo create_info{
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,	// sType
			nullptr,									// pNext
			0											// flags (reserved for future use)
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
	NO_COPY_OR_ASSIGNMENT(VulkanSwapchain)
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
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,	// sType
			nullptr,										// pNext
			0,												// flags (reserved for future use)
			surface,
			minImageCount,
			imageFormat,
			imageColorSpace,
			imageExtent,
			imageArrayLayers,
			imageUsage,
			imageSharingMode,
			static_cast<uint32_t>(queueFamilyIndices.size()),
			queueFamilyIndices.data(),
			preTransform,
			compositeAlpha,
			presentMode,
			clipped,
			oldSwapchain
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

class VulkanDescriptorPool {
public:
	NO_COPY_OR_ASSIGNMENT(VulkanDescriptorPool)
		VulkanDescriptorPool() = delete;

	explicit VulkanDescriptorPool(
		std::shared_ptr<VulkanDevice> device,
		std::vector<VkDescriptorPoolSize> descriptor_pool_sizes) :
		device(std::move(device))
	{
		VkDescriptorPoolCreateInfo create_info{
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,			// sType 
			nullptr,												// pNext
			VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,		// flags
			static_cast<uint32_t>(descriptor_pool_sizes.size()),	// maxSets
			static_cast<uint32_t>(descriptor_pool_sizes.size()),	// poolSizeCount
			descriptor_pool_sizes.data()							// pPoolSizes
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
	NO_COPY_OR_ASSIGNMENT(VulkanDescriptorSetLayout)
		VulkanDescriptorSetLayout() = delete;

	VulkanDescriptorSetLayout(
		std::shared_ptr<VulkanDevice> device,
		std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings) :
		device(std::move(device))
	{
		VkDescriptorSetLayoutCreateInfo create_info{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,			// sType
			nullptr,														// pNext
			0,																// flags
			static_cast<uint32_t>(descriptor_set_layout_bindings.size()),	// bindingCount
			descriptor_set_layout_bindings.data()							// pBindings
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
	NO_COPY_OR_ASSIGNMENT(VulkanDescriptorSets)
		VulkanDescriptorSets() = delete;

	VulkanDescriptorSets(
		std::shared_ptr<VulkanDevice> device,
		std::shared_ptr<VulkanDescriptorPool> pool,
		const std::vector<VkDescriptorSetLayout>& set_layouts) :
		device(std::move(device)),
		pool(std::move(pool))
	{
		VkDescriptorSetAllocateInfo allocate_info{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,		// sType
			nullptr,											// pNext
			this->pool->pool,									// descriptorPool
			static_cast<uint32_t>(set_layouts.size()),			// descriptorSetCount
			set_layouts.data()									// pSetLayouts
		};

		this->descriptor_sets.resize(set_layouts.size());
		THROW_ON_ERROR(vk.AllocateDescriptorSets(this->device->device, &allocate_info, this->descriptor_sets.data()));
	}

	~VulkanDescriptorSets()
	{
		vk.FreeDescriptorSets(this->device->device,
			this->pool->pool,
			static_cast<uint32_t>(this->descriptor_sets.size()),
			this->descriptor_sets.data());
	}

	void update(const std::vector<VkWriteDescriptorSet>& descriptor_writes,
		const std::vector<VkCopyDescriptorSet>& descriptor_copies = std::vector<VkCopyDescriptorSet>()) const
	{
		vk.UpdateDescriptorSets(this->device->device,
			static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(),
			static_cast<uint32_t>(descriptor_copies.size()), descriptor_copies.data());
	}

	std::shared_ptr<VulkanDevice> device;
	std::shared_ptr<VulkanDescriptorPool> pool;
	std::vector<VkDescriptorSet> descriptor_sets;
};

class VulkanFence {
public:
	NO_COPY_OR_ASSIGNMENT(VulkanFence)
		VulkanFence() = delete;

	explicit VulkanFence(std::shared_ptr<VulkanDevice> device) :
		device(std::move(device))
	{
		VkFenceCreateInfo create_info{
			VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,	// sType
			nullptr,								// pNext
			VK_FENCE_CREATE_SIGNALED_BIT			// flags
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
	NO_COPY_OR_ASSIGNMENT(VulkanImage)
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
		std::vector<uint32_t> queue_family_indices = std::vector<uint32_t>(),
		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED) :
		device(std::move(device))
	{
		VkImageCreateInfo create_info{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,					// sType
			nullptr,												// pNext
			flags,													// flags
			image_type,												// imageType
			format,													// format
			extent,													// extent
			mip_levels,												// mipLevels
			array_layers,											// arrayLayers
			samples,												// samples
			tiling,													// tiling
			usage,													// usage
			sharing_mode,											// sharingMode
			static_cast<uint32_t>(queue_family_indices.size()),		// queueFamilyIndexCount
			queue_family_indices.data(),							// pQueueFamilyIndices
			initial_layout,											// initialLayout
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
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			srcAccessMask,
			dstAccessMask,
			oldLayout,
			newLayout,
			0,
			0,
			image,
			subresourceRange
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
	NO_COPY_OR_ASSIGNMENT(VulkanBuffer)
		VulkanBuffer() = delete;

	VulkanBuffer(
		std::shared_ptr<VulkanInstance> vulkan,
		std::shared_ptr<VulkanDevice> device,
		VkBufferCreateFlags flags,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkSharingMode sharingMode,
		const std::vector<uint32_t>& queueFamilyIndices = std::vector<uint32_t>()) :
			vulkan(std::move(vulkan)),
			device(std::move(device))
	{
		VkBufferCreateInfo create_info{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,				// sType  
			nullptr,											// pNext  
			flags,												// flags  
			size,												// size  
			usage,												// usage  
			sharingMode,										// sharingMode  
			static_cast<uint32_t>(queueFamilyIndices.size()),	// queueFamilyIndexCount  
			queueFamilyIndices.data(),							// pQueueFamilyIndices  
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
#ifdef VK_USE_PLATFORM_WIN32_KHR
	VkDeviceAddress getDeviceAddress(VkDevice device, VkBuffer buffer)
	{
		VkBufferDeviceAddressInfo buffer_address_info{
			VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			nullptr,
			buffer
		};
		return this->vulkan->vkGetBufferDeviceAddressKHR(device, &buffer_address_info);
	}

	VkDeviceAddress getDeviceAddress()
	{
		return getDeviceAddress(this->device->device, this->buffer);
	}
#endif

	std::shared_ptr<VulkanInstance> vulkan;
	std::shared_ptr<VulkanDevice> device;
	VkBuffer buffer{ 0 };
};


class VulkanBufferObject {
public:
	VulkanBufferObject(
		std::shared_ptr<VulkanInstance> vulkan,
		std::shared_ptr<VulkanDevice> device,
		VkBufferCreateFlags createFlags,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkSharingMode sharingMode,
		VkMemoryPropertyFlags memoryFlags)
	{
		this->buffer = std::make_shared<VulkanBuffer>(
			vulkan,
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
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR,
			nullptr,
			allocate_flags,								// flags
			0,											// deviceMask
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
		VkMemoryPropertyFlags memoryFlags)
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
	NO_COPY_OR_ASSIGNMENT(VulkanCommandBuffers)

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
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,		// sType 
			nullptr,											// pNext 
			this->device->default_pool,							// commandPool 
			level,												// level 
			static_cast<uint32_t>(count),						// commandBufferCount 
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
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,	// sType
			nullptr,											// pNext
			renderpass,											// renderPass 
			subpass,											// subpass 
			framebuffer,										// framebuffer
			VK_FALSE,											// occlusionQueryEnable
			0,													// queryFlags 
			0,													// pipelineStatistics 
		};

		VkCommandBufferBeginInfo begin_info{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,		// sType 
			nullptr,											// pNext 
			flags,												// flags 
			&inheritance_info,									// pInheritanceInfo 
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
			VK_STRUCTURE_TYPE_SUBMIT_INFO,						// sType 
			nullptr,											// pNext  
			static_cast<uint32_t>(wait_semaphores.size()),		// waitSemaphoreCount  
			wait_semaphores.data(),								// pWaitSemaphores  
			&flags,												// pWaitDstStageMask  
			static_cast<uint32_t>(buffers.size()),				// commandBufferCount  
			buffers.data(),										// pCommandBuffers 
			static_cast<uint32_t>(signal_semaphores.size()),	// signalSemaphoreCount
			signal_semaphores.data(),							// pSignalSemaphores
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
	void * pNext) :
		device(std::move(device))
{
	VkMemoryAllocateInfo allocate_info{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,		// sType 
		pNext,										// pNext 
		size,										// allocationSize 
		memory_type_index,							// memoryTypeIndex 
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
	NO_COPY_OR_ASSIGNMENT(MemoryMap)

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
	NO_COPY_OR_ASSIGNMENT(VulkanImageView)
		VulkanImageView() = delete;

	VulkanImageView(
		std::shared_ptr<VulkanDevice> device,
		VkImage image,
		VkFormat format,
		VkImageViewType view_type,
		VkComponentMapping components,
		VkImageSubresourceRange subresource_range) :
		device(std::move(device))
	{
		VkImageViewCreateInfo create_info{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,	// sType 
			nullptr,									// pNext 
			0,											// flags (reserved for future use)
			image,										// image 
			view_type,									// viewType 
			format,										// format 
			components,									// components 
			subresource_range,							// subresourceRange 
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
	NO_COPY_OR_ASSIGNMENT(VulkanSampler)
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
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,			// sType
			nullptr,										// pNext
			0,												// flags (reserved for future use)
			magFilter,										// magFilter
			minFilter,										// minFilter
			mipmapMode,										// mipmapMode
			addressModeU,									// addressModeU
			addressModeV,									// addressModeV
			addressModeW,									// addressModeW
			mipLodBias,										// mipLodBias
			anisotropyEnable,								// anisotropyEnable
			maxAnisotropy,									// maxAnisotropy
			compareEnable,									// compareEnable
			compareOp,										// compareOp
			minLod,											// minLod
			maxLod,											// maxLod
			borderColor,									// borderColor
			unnormalizedCoordinates,						// unnormalizedCoordinates
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
	NO_COPY_OR_ASSIGNMENT(VulkanShaderModule)
		VulkanShaderModule() = delete;

	VulkanShaderModule(
		std::shared_ptr<VulkanDevice> device,
		const std::vector<uint32_t>& code) :
		device(std::move(device))
	{
		VkShaderModuleCreateInfo create_info{
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,			// sType
			nullptr,											// pNext
			0,													// flags (reserved for future use)
			code.size() * sizeof(uint32_t),						// codeSize
			code.data(),										// pCode
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
	NO_COPY_OR_ASSIGNMENT(VulkanPipelineCache)
		VulkanPipelineCache() = delete;

	explicit VulkanPipelineCache(std::shared_ptr<VulkanDevice> device) :
		device(std::move(device))
	{
		VkPipelineCacheCreateInfo create_info{
			VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,	// sType
			nullptr,										// pNext
			0,												// flags (reserved for future use)
			0,												// initialDataSize
			nullptr,										// pInitialData
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
	NO_COPY_OR_ASSIGNMENT(VulkanRenderpass)
		VulkanRenderpass() = delete;

	VulkanRenderpass(
		std::shared_ptr<VulkanDevice> device,
		const std::vector<VkAttachmentDescription>& attachments,
		const std::vector<VkSubpassDescription>& subpasses,
		const std::vector<VkSubpassDependency>& dependencies = std::vector<VkSubpassDependency>()) :
		device(std::move(device))
	{
		VkRenderPassCreateInfo create_info{
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,			// sType
			nullptr,											// pNext
			0,													// flags (reserved for future use)
			static_cast<uint32_t>(attachments.size()),			// attachmentCount
			attachments.data(),									// pAttachments
			static_cast<uint32_t>(subpasses.size()),			// subpassCount
			subpasses.data(),									// pSubpasses
			static_cast<uint32_t>(dependencies.size()),			// dependencyCount
			dependencies.data(),								// pDependencies
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
	NO_COPY_OR_ASSIGNMENT(VulkanRenderPassScope)
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
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,			// sType
			nullptr,											// pNext
			renderpass,											// renderPass
			framebuffer,										// framebuffer
			renderarea,											// renderArea
			static_cast<uint32_t>(clearvalues.size()),			// clearValueCount
			clearvalues.data()									// pClearValues
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
	NO_COPY_OR_ASSIGNMENT(VulkanFramebuffer)
		VulkanFramebuffer() = delete;

	VulkanFramebuffer(
		std::shared_ptr<VulkanDevice> device,
		std::shared_ptr<VulkanRenderpass> renderpass,
		std::vector<VkImageView> attachments,
		VkExtent2D extent,
		uint32_t layers) :
		device(std::move(device)),
		renderpass(std::move(renderpass))
	{
		VkFramebufferCreateInfo create_info{
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,		// sType
			nullptr,										// pNext
			0,												// flags (reserved for future use)
			this->renderpass->renderpass,					// renderPass
			static_cast<uint32_t>(attachments.size()),		// attachmentCount
			attachments.data(),								// pAttachments
			extent.width,									// width
			extent.height,									// height
			layers,											// layers
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
	NO_COPY_OR_ASSIGNMENT(VulkanPipelineLayout)
		VulkanPipelineLayout() = delete;

	VulkanPipelineLayout(
		std::shared_ptr<VulkanDevice> device,
		const std::vector<VkDescriptorSetLayout>& setlayouts,
		const std::vector<VkPushConstantRange>& pushconstantranges) :
		device(std::move(device))
	{
		VkPipelineLayoutCreateInfo create_info{
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,		// sType
			nullptr,											// pNext
			0,													// flags (reserved for future use)
			static_cast<uint32_t>(setlayouts.size()),			// setLayoutCount
			setlayouts.data(),									// pSetLayouts
			static_cast<uint32_t>(pushconstantranges.size()),	// pushConstantRangeCount
			pushconstantranges.data(),							// pPushConstantRanges
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
	NO_COPY_OR_ASSIGNMENT(VulkanComputePipeline)
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
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,		// sType
			nullptr,											// pNext
			0,													// flags
			stage,												// stage
			layout,												// layout
			basePipelineHandle,									// basePipelineHandle
			basePipelineIndex,									// basePipelineIndex
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

class VulkanGraphicsPipeline {
public:
	NO_COPY_OR_ASSIGNMENT(VulkanGraphicsPipeline)
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
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,		// sType
			nullptr,														// pNext
			0,																// flags (reserved for future use)
			static_cast<uint32_t>(binding_descriptions.size()),				// vertexBindingDescriptionCount
			binding_descriptions.data(),									// pVertexBindingDescriptions
			static_cast<uint32_t>(attribute_descriptions.size()),			// vertexAttributeDescriptionCount
			attribute_descriptions.data(),									// pVertexAttributeDescriptions
		};

		VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,	// sType
			nullptr,														// pNext
			0,																// flags (reserved for future use)
			primitive_topology,												// topology
			VK_FALSE,														// primitiveRestartEnable
		};

		VkPipelineColorBlendAttachmentState blend_attachment_state{
			VK_FALSE,																									// blendEnable
			VK_BLEND_FACTOR_SRC_COLOR,																					// srcColorBlendFactor
			VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,																		// dstColorBlendFactor
			VK_BLEND_OP_ADD,																							// colorBlendOp
			VK_BLEND_FACTOR_SRC_ALPHA,																					// srcAlphaBlendFactor
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,																		// dstAlphaBlendFactor
			VK_BLEND_OP_ADD,																							// alphaBlendOp
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,	// colorWriteMask
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state{
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,		// sType 
			nullptr,														// pNext
			0,																// flags (reserved for future use)
			VK_FALSE,														// logicOpEnable
			VK_LOGIC_OP_MAX_ENUM,											// logicOp
			1,																// attachmentCount
			&blend_attachment_state,										// pAttachments  
			{0},															// blendConstants[4]
		};

		VkPipelineDynamicStateCreateInfo dynamic_state{
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,			// sType
			nullptr,														// pNext
			0,																// flags (reserved for future use)
			static_cast<uint32_t>(dynamic_states.size()),					// dynamicStateCount
			dynamic_states.data(),											// pDynamicStates
		};

		VkPipelineViewportStateCreateInfo viewport_state{
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,			// sType
			nullptr,														// pNext
			0,																// flags (reserved for future use)
			1,																// viewportCount
			nullptr,														// pViewports
			1,																// scissorCount
			nullptr,														// pScissors
		};

		const VkStencilOpState stencil_op_state{
			VK_STENCIL_OP_KEEP,												// failOp
			VK_STENCIL_OP_KEEP,												// passOp
			VK_STENCIL_OP_KEEP,												// depthFailOp
			VK_COMPARE_OP_ALWAYS,											// compareOp
			0,																// compareMask
			0,																// writeMask
			0,																// reference
		};

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,		// sType
			nullptr,														// pNext
			0,																// flags (reserved for future use)
			VK_TRUE,														// depthTestEnable
			VK_TRUE,														// depthWriteEnable
			VK_COMPARE_OP_LESS_OR_EQUAL,									// depthCompareOp
			VK_FALSE,														// depthBoundsTestEnable
			VK_FALSE,														// stencilTestEnable
			stencil_op_state,												// front  
			stencil_op_state,												// back
			0,																// minDepthBounds
			0,																// maxDepthBounds
		};

		VkPipelineMultisampleStateCreateInfo multi_sample_state{
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,		// sType 
			nullptr,														// pNext
			0,																// flags (reserved for future use)
			VK_SAMPLE_COUNT_1_BIT,											// rasterizationSamples
			VK_FALSE,														// sampleShadingEnable
			0,																// minSampleShading
			nullptr,														// pSampleMask
			VK_FALSE,														// alphaToCoverageEnable
			VK_FALSE,														// alphaToOneEnable
		};

		VkGraphicsPipelineCreateInfo create_info{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,				// sType
			nullptr,														// pNext
			0,																// flags
			static_cast<uint32_t>(shaderstages.size()),						// stageCount
			shaderstages.data(),											// pStages
			&vertex_input_state,											// pVertexInputState
			&input_assembly_state,											// pInputAssemblyState
			nullptr,														// pTessellationState
			&viewport_state,												// pViewportState
			&rasterization_state,											// pRasterizationState
			&multi_sample_state,											// pMultisampleState
			&depth_stencil_state,											// pDepthStencilState
			&color_blend_state,												// pColorBlendState
			&dynamic_state,													// pDynamicState
			pipeline_layout,												// layout
			render_pass,													// renderPass
			0,																// subpass
			0,																// basePipelineHandle
			0,																// basePipelineIndex
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

#ifdef VK_USE_PLATFORM_WIN32_KHR
class VulkanAccelerationStructureGeometry {
public:
	VulkanAccelerationStructureGeometry(
		VkGeometryTypeKHR geometryType,
		uint32_t maxPrimitiveCount,
		VkIndexType	indexType,
		uint32_t maxVertexCount,
		VkFormat vertexFormat,
		VkBool32 allowsTransforms)
	{
		VkAccelerationStructureCreateGeometryTypeInfoKHR create_info{
			VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR,
			nullptr,
			geometryType,
			maxPrimitiveCount,
			indexType,
			maxVertexCount,
			vertexFormat,
			allowsTransforms
		};

	}
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
			VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			nullptr,
			compactedSize,
			this->type,
			flags,
			static_cast<uint32_t>(geometryInfos.size()),
			geometryInfos.data(),
			deviceAddress
		};

		THROW_ON_ERROR(this->vulkan->vkCreateAccelerationStructureKHR(this->device->device, &create_info, nullptr, &this->as));

		VkMemoryRequirements memory_requirements = this->getMemoryRequirements(
			VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR,	// type
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR);				// buildType

		this->buffer = std::make_shared<VulkanBuffer>(
			this->vulkan,
			this->device,
			0,
			memory_requirements.size,
			VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_SHARING_MODE_EXCLUSIVE);

		uint32_t memory_type_index = this->device->physical_device.getMemoryTypeIndex(
			memory_requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkMemoryAllocateFlagsInfo allocate_info{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR,
			nullptr,
			VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,	// flags
			0,											// deviceMask
		};

		this->memory = std::make_shared<VulkanMemory>(
			this->device,
			memory_requirements.size,
			memory_type_index,
			&allocate_info);

		this->device->bindBufferMemory(
			buffer->buffer,
			memory->memory,
			0);

		VkBindAccelerationStructureMemoryInfoKHR memory_info = {
			VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR,
			nullptr,
			this->as,			// accelerationStructure
			memory->memory,		// memory
			0,					// memoryOffset
			0,					// deviceIndexCount
			nullptr,			// pDeviceIndices;
		};

		this->vulkan->vkBindAccelerationStructureMemoryKHR(this->device->device, 1, &memory_info);
	}

	~VulkanAccelerationStructure()
	{
		this->vulkan->vkDestroyAccelerationStructureKHR(this->device->device, this->as, nullptr);
	}

	VkMemoryRequirements getMemoryRequirements(
		VkAccelerationStructureMemoryRequirementsTypeKHR type, 
		VkAccelerationStructureBuildTypeKHR buildType)
	{
		VkAccelerationStructureMemoryRequirementsInfoKHR memory_requirements_info{
			VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR,
			nullptr,
			type,											// type
			buildType,										// buildType
			this->as										// accelerationStructure
		};

		VkMemoryRequirements2 memory_requirements2{
			VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
		};

		this->vulkan->vkGetAccelerationStructureMemoryRequirementsKHR(this->device->device, &memory_requirements_info, &memory_requirements2);

		return memory_requirements2.memoryRequirements;
	}

	void build(
		VkCommandBuffer command,
		std::vector<VkAccelerationStructureGeometryKHR>& geometries,
		std::vector<VkAccelerationStructureBuildOffsetInfoKHR>& build_offset_infos)
	{
		VkDeviceAddress address = this->buffer->getDeviceAddress();

		VkAccelerationStructureGeometryKHR* pGeometries = geometries.data();

		VkAccelerationStructureBuildGeometryInfoKHR build_geometry_info{
			VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			nullptr,
			this->type,													// type
			VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,	// flags
			VK_FALSE,													// update
			VK_NULL_HANDLE,												// srcAccelerationStructure
			this->as,													// dstAccelerationStructure
			VK_FALSE,													// geometryArrayOfPointers
			static_cast<uint32_t>(geometries.size()),					// geometryCount
			&pGeometries,												// ppGeometries
			address														// scratchData
		};

		VkAccelerationStructureBuildOffsetInfoKHR* pOffsetInfos = build_offset_infos.data();
		this->vulkan->vkCmdBuildAccelerationStructureKHR(
			command,
			static_cast<uint32_t>(build_offset_infos.size()),
			&build_geometry_info,
			&pOffsetInfos);

		VkMemoryBarrier barrier{
			VK_STRUCTURE_TYPE_MEMORY_BARRIER,
			nullptr,
			VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,				// srcAccessMask
			VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,				// dstAccessMask
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
	std::shared_ptr<VulkanBuffer> buffer;
	std::shared_ptr<VulkanMemory> memory;
};
#endif
