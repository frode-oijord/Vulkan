#pragma once

#include <Innovator/Nodes.h>
#include <Scheme/Scheme.h>

#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <utility>

using namespace scm;

template <typename BaseType, typename SubType, typename... Arg, std::size_t... i>
std::shared_ptr<BaseType> make_shared_object_impl(const List& lst, std::index_sequence<i...>)
{
	return std::make_shared<SubType>(std::any_cast<Arg>(lst[i])...);
}

template <typename BaseType, typename SubType, typename... Arg>
std::shared_ptr<BaseType> make_shared_object(const List& lst)
{
	return make_shared_object_impl<BaseType, SubType, Arg...>(lst, std::index_sequence_for<Arg...>{});
}

template <typename Type, typename... Arg, std::size_t... i>
Type make_object_impl(const List& lst, std::index_sequence<i...>)
{
	return Type(std::any_cast<Arg>(lst[i])...);
}

template <typename Type, typename... Arg>
Type make_object(const List& lst)
{
	return make_object_impl<Type, Arg...>(lst, std::index_sequence_for<Arg...>{});
}

template <typename NodeType, typename... Arg>
std::shared_ptr<Node> node(const List& lst)
{
	return make_shared_object<Node, NodeType, Arg...>(lst);
}

template <typename Type, typename ItemType>
std::shared_ptr<Node> shared_from_node_list(const List& lst)
{
	return std::make_shared<Type>(scm::any_cast<ItemType>(lst));
}

template <typename T>
std::shared_ptr<Node> bufferdata(const List& lst)
{
	return std::make_shared<InlineBufferData<T>>(scm::num_cast<T>(lst));
}

uint32_t count(const List& list)
{
	if (list.empty()) {
		throw std::runtime_error("count needs at least 1 argument");
	}
	auto node = std::any_cast<std::shared_ptr<Node>>(list[0]);
	auto bufferdata = std::dynamic_pointer_cast<BufferData>(node);
	if (!bufferdata) {
		throw std::invalid_argument("count only works on BufferData nodes!");
	}
	return static_cast<uint32_t>(bufferdata->count());
}

std::string toString(const List& list)
{
	if (list.empty()) {
		throw std::runtime_error("print needs at least 1 argument");
	}
	auto obj = std::any_cast<std::shared_ptr<VulkanObject>>(list[0]);
	return obj->toString();
}

template <typename Flags, typename FlagBits>
Flags flags(const List& lst) {
	if (lst.empty()) {
		return 0;
	}
	std::vector<FlagBits> flagbits = any_cast<FlagBits>(lst);
	Flags flags = 0;
	for (auto bit : flagbits) {
		flags |= bit;
	}
	return flags;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
int window(const List& lst)
{
	auto scene = std::any_cast<std::shared_ptr<Node>>(lst[0]);
	auto window = std::make_shared<VulkanWindow>(scene, 1920, 1080);
	return window->show();
}
#endif

VkComponentMapping componentMapping(const List& lst)
{
	return VkComponentMapping{ 
		.r = std::any_cast<VkComponentSwizzle>(lst[0]),
		.g = std::any_cast<VkComponentSwizzle>(lst[1]),
		.b = std::any_cast<VkComponentSwizzle>(lst[2]),
		.a = std::any_cast<VkComponentSwizzle>(lst[3]),
	};
}

VkImageSubresourceRange imageSubresourceRange(const List& lst)
{
	return VkImageSubresourceRange{
		.aspectMask = std::any_cast<VkImageAspectFlags>(lst[0]),
		.baseMipLevel = std::any_cast<uint32_t>(lst[1]),
		.levelCount = std::any_cast<uint32_t>(lst[2]),
		.baseArrayLayer = std::any_cast<uint32_t>(lst[3]),
		.layerCount = std::any_cast<uint32_t>(lst[4]),
	};
}

VkExtent3D extent3(const List& lst)
{
	return VkExtent3D{
		.width = static_cast<uint32_t>(std::any_cast<Number>(lst[0])),
		.height = static_cast<uint32_t>(std::any_cast<Number>(lst[1])),
		.depth = static_cast<uint32_t>(std::any_cast<Number>(lst[2])),
	};
}


env_ptr innovator_env()
{
	env_ptr innovator_env = std::make_shared<Env>();

	innovator_env->inner.insert({ "vulkan", fun_ptr(make_shared_object<VulkanObject, VulkanInstance>) });
	innovator_env->inner.insert({ "int32", fun_ptr(make_object<int32_t, Number>) });
	innovator_env->inner.insert({ "uint32", fun_ptr(make_object<uint32_t, Number>) });
	innovator_env->inner.insert({ "float", fun_ptr(make_object<float, Number>) });
	innovator_env->inner.insert({ "dvec3", fun_ptr(make_object<glm::dvec3, Number, Number, Number>) });
	innovator_env->inner.insert({ "extent3", fun_ptr(extent3) });
	innovator_env->inner.insert({ "count", fun_ptr(count) });
	innovator_env->inner.insert({ "print", fun_ptr(toString) });
	innovator_env->inner.insert({ "memorypropertyflags", fun_ptr(flags<VkMemoryPropertyFlags, VkMemoryPropertyFlagBits>) });
	innovator_env->inner.insert({ "bufferusageflags", fun_ptr(flags<VkBufferUsageFlags, VkBufferUsageFlagBits>) });
	innovator_env->inner.insert({ "imageusageflags", fun_ptr(flags<VkImageUsageFlags, VkImageUsageFlagBits>) });
	innovator_env->inner.insert({ "imageaspectflags", fun_ptr(flags<VkImageAspectFlags, VkImageAspectFlagBits>) });
	innovator_env->inner.insert({ "imagecreateflags", fun_ptr(flags<VkImageCreateFlags, VkImageCreateFlagBits>) });
	innovator_env->inner.insert({ "imageaspectflags", fun_ptr(flags<VkImageAspectFlags, VkImageAspectFlagBits>) });
#ifdef VK_USE_PLATFORM_WIN32_KHR
	innovator_env->inner.insert({ "window", fun_ptr(window) });
	innovator_env->inner.insert({ "raytracecommand", fun_ptr(node<RayTraceCommand>) });
	innovator_env->inner.insert({ "bottom-level-acceleration-structure", fun_ptr(node<BottomLevelAccelerationStructure>) });
	innovator_env->inner.insert({ "top-level-acceleration-structure", fun_ptr(node<TopLevelAccelerationStructure>) });
#endif
	innovator_env->inner.insert({ "extent", fun_ptr(node<Extent, uint32_t, uint32_t>) });
	innovator_env->inner.insert({ "offscreen-image", fun_ptr(node<OffscreenImage>) });
	innovator_env->inner.insert({ "pipeline-bindpoint", fun_ptr(node<PipelineBindpoint, VkPipelineBindPoint>) });
	innovator_env->inner.insert({ "color-attachment", fun_ptr(node<ColorAttachment, uint32_t, VkImageLayout>) });
	innovator_env->inner.insert({ "depth-attachment", fun_ptr(node<DepthStencilAttachment, uint32_t, VkImageLayout>) });
	innovator_env->inner.insert({ "subpass", fun_ptr(shared_from_node_list<SubpassDescription, std::shared_ptr<Node>>) });
	innovator_env->inner.insert({ "renderpass-attachment", fun_ptr(node<RenderpassAttachment, VkFormat, VkSampleCountFlagBits, VkAttachmentLoadOp, VkAttachmentStoreOp, VkAttachmentLoadOp, VkAttachmentStoreOp, VkImageLayout, VkImageLayout>) });
	innovator_env->inner.insert({ "renderpass-description", fun_ptr(shared_from_node_list<RenderpassDescription, std::shared_ptr<Node>>) });
	innovator_env->inner.insert({ "renderpass", fun_ptr(shared_from_node_list<Renderpass, std::shared_ptr<Node>>) });
	innovator_env->inner.insert({ "viewmatrix", fun_ptr(node<ViewMatrix, glm::dvec3, glm::dvec3, glm::dvec3>) });
	innovator_env->inner.insert({ "projmatrix", fun_ptr(node<ProjMatrix, Number, Number, Number, Number>) });
	innovator_env->inner.insert({ "modelmatrix", fun_ptr(node<ModelMatrix, glm::dvec3, glm::dvec3>) });
	innovator_env->inner.insert({ "texturematrix", fun_ptr(node<TextureMatrix, glm::dvec3, glm::dvec3>) });
	innovator_env->inner.insert({ "framebuffer", fun_ptr(shared_from_node_list<Framebuffer, std::shared_ptr<Node>>) });
	innovator_env->inner.insert({ "framebuffer-attachment", fun_ptr(shared_from_node_list<FramebufferAttachment, std::shared_ptr<Node>>) });
	innovator_env->inner.insert({ "shader", fun_ptr(node<Shader, VkShaderStageFlagBits, std::string>) });
	innovator_env->inner.insert({ "sampler", fun_ptr(node<Sampler, VkFilter, VkFilter, VkSamplerMipmapMode, VkSamplerAddressMode, VkSamplerAddressMode, VkSamplerAddressMode, float, uint32_t, float, uint32_t, VkCompareOp, float, float, VkBorderColor, uint32_t>) });
	innovator_env->inner.insert({ "texturedata", fun_ptr(node<TextureData, std::string>) });
	innovator_env->inner.insert({ "textureimage", fun_ptr(node<TextureImage, VkSampleCountFlagBits, VkImageTiling, VkImageUsageFlags, VkSharingMode, VkImageCreateFlags, VkImageLayout>) });
	innovator_env->inner.insert({ "image", fun_ptr(node<Image, VkImageType, VkFormat, VkExtent3D, uint32_t, uint32_t, VkSampleCountFlagBits, VkImageTiling, VkImageUsageFlags, VkSharingMode, VkImageCreateFlags, VkMemoryPropertyFlags>) });
	innovator_env->inner.insert({ "currentimagerendertarget", fun_ptr(node<CurrentImageRenderTarget>) });
	innovator_env->inner.insert({ "sparse-image", fun_ptr(node<SparseImage, VkSampleCountFlagBits, VkImageTiling, VkImageUsageFlags, VkSharingMode, VkImageCreateFlags, VkImageLayout>) });
	innovator_env->inner.insert({ "imageview", fun_ptr(node<ImageView, VkImageViewType, VkFormat, VkComponentMapping, VkImageSubresourceRange>) });
	innovator_env->inner.insert({ "imagelayout", fun_ptr(node<ImageLayout, VkImageLayout, VkImageLayout, VkImageSubresourceRange>) });
	innovator_env->inner.insert({ "component-mapping", fun_ptr(componentMapping) });
	innovator_env->inner.insert({ "subresource-range", fun_ptr(imageSubresourceRange) });
	innovator_env->inner.insert({ "group", fun_ptr(shared_from_node_list<Group, std::shared_ptr<Node>>) });
	innovator_env->inner.insert({ "separator", fun_ptr(shared_from_node_list<Separator, std::shared_ptr<Node>>) });
	innovator_env->inner.insert({ "bufferdata-float", fun_ptr(bufferdata<float>) });
	innovator_env->inner.insert({ "bufferdata-uint32", fun_ptr(bufferdata<uint32_t>) });
	innovator_env->inner.insert({ "cpumemorybuffer", fun_ptr(node<CpuMemoryBuffer, VkBufferUsageFlags>) });
	innovator_env->inner.insert({ "gpumemorybuffer", fun_ptr(node<GpuMemoryBuffer, VkBufferUsageFlags>) });
	innovator_env->inner.insert({ "transformbuffer", fun_ptr(node<TransformBuffer>) });
	innovator_env->inner.insert({ "drawcommand", fun_ptr(node<DrawCommand, uint32_t, uint32_t, uint32_t, uint32_t, VkPrimitiveTopology>) });
	innovator_env->inner.insert({ "indexeddrawcommand", fun_ptr(node<IndexedDrawCommand, uint32_t, uint32_t, uint32_t, int32_t, uint32_t, VkPrimitiveTopology>) });
	innovator_env->inner.insert({ "indexbufferdescription", fun_ptr(node<IndexBufferDescription, VkIndexType>) });
	innovator_env->inner.insert({ "descriptorsetlayoutbinding", fun_ptr(node<DescriptorSetLayoutBinding, uint32_t, VkDescriptorType, VkShaderStageFlagBits>) });
	innovator_env->inner.insert({ "vertexinputbindingdescription", fun_ptr(node<VertexInputBindingDescription, uint32_t, uint32_t, VkVertexInputRate>) });
	innovator_env->inner.insert({ "vertexinputattributedescription", fun_ptr(node<VertexInputAttributeDescription, uint32_t, uint32_t, VkFormat, uint32_t>) });

	innovator_env->inner.insert({ "VK_PRESENT_MODE_IMMEDIATE_KHR", VK_PRESENT_MODE_IMMEDIATE_KHR });
	innovator_env->inner.insert({ "VK_PRESENT_MODE_MAILBOX_KHR", VK_PRESENT_MODE_MAILBOX_KHR });
	innovator_env->inner.insert({ "VK_PRESENT_MODE_FIFO_KHR", VK_PRESENT_MODE_FIFO_KHR });
	innovator_env->inner.insert({ "VK_PRESENT_MODE_FIFO_RELAXED_KHR", VK_PRESENT_MODE_FIFO_RELAXED_KHR });

	innovator_env->inner.insert({ "VK_IMAGE_CREATE_SPARSE_BINDING_BIT", VK_IMAGE_CREATE_SPARSE_BINDING_BIT });
	innovator_env->inner.insert({ "VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT", VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT });
	innovator_env->inner.insert({ "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT", VK_IMAGE_CREATE_SPARSE_ALIASED_BIT });

	innovator_env->inner.insert({ "VK_IMAGE_VIEW_TYPE_2D", VK_IMAGE_VIEW_TYPE_2D });
	innovator_env->inner.insert({ "VK_IMAGE_VIEW_TYPE_3D", VK_IMAGE_VIEW_TYPE_3D });

	innovator_env->inner.insert({ "VK_FILTER_NEAREST", VK_FILTER_NEAREST });
	innovator_env->inner.insert({ "VK_FILTER_LINEAR", VK_FILTER_LINEAR });
	innovator_env->inner.insert({ "VK_FILTER_CUBIC_IMG", VK_FILTER_CUBIC_IMG });

	innovator_env->inner.insert({ "VK_COMPARE_OP_NEVER", VK_COMPARE_OP_NEVER });
	innovator_env->inner.insert({ "VK_COMPARE_OP_LESS", VK_COMPARE_OP_LESS });
	innovator_env->inner.insert({ "VK_COMPARE_OP_EQUAL", VK_COMPARE_OP_EQUAL });
	innovator_env->inner.insert({ "VK_COMPARE_OP_LESS_OR_EQUAL", VK_COMPARE_OP_LESS_OR_EQUAL });
	innovator_env->inner.insert({ "VK_COMPARE_OP_GREATER", VK_COMPARE_OP_GREATER });

	innovator_env->inner.insert({ "VK_COMPARE_OP_NOT_EQUAL", VK_COMPARE_OP_NOT_EQUAL });
	innovator_env->inner.insert({ "VK_COMPARE_OP_GREATER_OR_EQUAL", VK_COMPARE_OP_GREATER_OR_EQUAL });
	innovator_env->inner.insert({ "VK_COMPARE_OP_ALWAYS", VK_COMPARE_OP_ALWAYS });

	innovator_env->inner.insert({ "VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK", VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK });
	innovator_env->inner.insert({ "VK_BORDER_COLOR_INT_TRANSPARENT_BLACK", VK_BORDER_COLOR_INT_TRANSPARENT_BLACK });
	innovator_env->inner.insert({ "VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK", VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK });
	innovator_env->inner.insert({ "VK_BORDER_COLOR_INT_OPAQUE_BLACK", VK_BORDER_COLOR_INT_OPAQUE_BLACK });
	innovator_env->inner.insert({ "VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE", VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE });
	innovator_env->inner.insert({ "VK_BORDER_COLOR_INT_OPAQUE_WHITE", VK_BORDER_COLOR_INT_OPAQUE_WHITE });

	innovator_env->inner.insert({ "VK_ATTACHMENT_LOAD_OP_LOAD", VK_ATTACHMENT_LOAD_OP_LOAD });
	innovator_env->inner.insert({ "VK_ATTACHMENT_LOAD_OP_CLEAR", VK_ATTACHMENT_LOAD_OP_CLEAR });
	innovator_env->inner.insert({ "VK_ATTACHMENT_LOAD_OP_DONT_CARE", VK_ATTACHMENT_LOAD_OP_DONT_CARE });

	innovator_env->inner.insert({ "VK_ATTACHMENT_STORE_OP_STORE", VK_ATTACHMENT_STORE_OP_STORE });
	innovator_env->inner.insert({ "VK_ATTACHMENT_STORE_OP_DONT_CARE", VK_ATTACHMENT_STORE_OP_DONT_CARE });

	innovator_env->inner.insert({ "VK_SAMPLER_MIPMAP_MODE_NEAREST", VK_SAMPLER_MIPMAP_MODE_NEAREST });
	innovator_env->inner.insert({ "VK_SAMPLER_MIPMAP_MODE_LINEAR", VK_SAMPLER_MIPMAP_MODE_LINEAR });

	innovator_env->inner.insert({ "VK_SAMPLER_ADDRESS_MODE_REPEAT", VK_SAMPLER_ADDRESS_MODE_REPEAT });
	innovator_env->inner.insert({ "VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT", VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT });
	innovator_env->inner.insert({ "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE", VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE });
	innovator_env->inner.insert({ "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER", VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER });
	innovator_env->inner.insert({ "VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE", VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE });

	innovator_env->inner.insert({ "VK_COMPONENT_SWIZZLE_IDENTITY", VK_COMPONENT_SWIZZLE_IDENTITY });
	innovator_env->inner.insert({ "VK_COMPONENT_SWIZZLE_ZERO", VK_COMPONENT_SWIZZLE_ZERO });
	innovator_env->inner.insert({ "VK_COMPONENT_SWIZZLE_ONE", VK_COMPONENT_SWIZZLE_ONE });
	innovator_env->inner.insert({ "VK_COMPONENT_SWIZZLE_R", VK_COMPONENT_SWIZZLE_R });
	innovator_env->inner.insert({ "VK_COMPONENT_SWIZZLE_G", VK_COMPONENT_SWIZZLE_G });
	innovator_env->inner.insert({ "VK_COMPONENT_SWIZZLE_B", VK_COMPONENT_SWIZZLE_B });
	innovator_env->inner.insert({ "VK_COMPONENT_SWIZZLE_A", VK_COMPONENT_SWIZZLE_A });
	innovator_env->inner.insert({ "VK_COMPONENT_SWIZZLE_BEGIN_RANGE", VK_COMPONENT_SWIZZLE_IDENTITY });
	innovator_env->inner.insert({ "VK_COMPONENT_SWIZZLE_END_RANGE", VK_COMPONENT_SWIZZLE_A });
	innovator_env->inner.insert({ "VK_COMPONENT_SWIZZLE_RANGE_SIZE", (VK_COMPONENT_SWIZZLE_A - VK_COMPONENT_SWIZZLE_IDENTITY + 1) });

	innovator_env->inner.insert({ "VK_SAMPLE_COUNT_1_BIT", VK_SAMPLE_COUNT_1_BIT });
	innovator_env->inner.insert({ "VK_SAMPLE_COUNT_2_BIT", VK_SAMPLE_COUNT_2_BIT });
	innovator_env->inner.insert({ "VK_SAMPLE_COUNT_4_BIT", VK_SAMPLE_COUNT_4_BIT });
	innovator_env->inner.insert({ "VK_SAMPLE_COUNT_8_BIT", VK_SAMPLE_COUNT_8_BIT });
	innovator_env->inner.insert({ "VK_SAMPLE_COUNT_16_BIT", VK_SAMPLE_COUNT_16_BIT });
	innovator_env->inner.insert({ "VK_SAMPLE_COUNT_32_BIT", VK_SAMPLE_COUNT_32_BIT });
	innovator_env->inner.insert({ "VK_SAMPLE_COUNT_64_BIT", VK_SAMPLE_COUNT_64_BIT });

	innovator_env->inner.insert({ "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT", VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT });
	innovator_env->inner.insert({ "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT", VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT });
	
	innovator_env->inner.insert({ "VK_SHARING_MODE_EXCLUSIVE", VK_SHARING_MODE_EXCLUSIVE });
	innovator_env->inner.insert({ "VK_SHARING_MODE_CONCURRENT", VK_SHARING_MODE_CONCURRENT });

	innovator_env->inner.insert({ "VK_IMAGE_TYPE_1D", VK_IMAGE_TYPE_1D });
	innovator_env->inner.insert({ "VK_IMAGE_TYPE_2D", VK_IMAGE_TYPE_2D });
	innovator_env->inner.insert({ "VK_IMAGE_TYPE_3D", VK_IMAGE_TYPE_3D });

	innovator_env->inner.insert({ "VK_IMAGE_TILING_OPTIMAL", VK_IMAGE_TILING_OPTIMAL });
	innovator_env->inner.insert({ "VK_IMAGE_TILING_LINEAR", VK_IMAGE_TILING_LINEAR });

	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_UNDEFINED", VK_IMAGE_LAYOUT_UNDEFINED });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_GENERAL", VK_IMAGE_LAYOUT_GENERAL });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL", VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL", VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL", VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL", VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_PREINITIALIZED", VK_IMAGE_LAYOUT_PREINITIALIZED });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL", VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL", VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR", VK_IMAGE_LAYOUT_PRESENT_SRC_KHR });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR", VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV", VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR", VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL });
	innovator_env->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR", VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL });

	innovator_env->inner.insert({ "VK_IMAGE_ASPECT_COLOR_BIT", VK_IMAGE_ASPECT_COLOR_BIT });
	innovator_env->inner.insert({ "VK_IMAGE_ASPECT_DEPTH_BIT", VK_IMAGE_ASPECT_DEPTH_BIT });

	innovator_env->inner.insert({ "VK_IMAGE_USAGE_TRANSFER_SRC_BIT", VK_IMAGE_USAGE_TRANSFER_SRC_BIT });
	innovator_env->inner.insert({ "VK_IMAGE_USAGE_TRANSFER_DST_BIT", VK_IMAGE_USAGE_TRANSFER_DST_BIT });
	innovator_env->inner.insert({ "VK_IMAGE_USAGE_SAMPLED_BIT", VK_IMAGE_USAGE_SAMPLED_BIT });
	innovator_env->inner.insert({ "VK_IMAGE_USAGE_STORAGE_BIT", VK_IMAGE_USAGE_STORAGE_BIT });
	innovator_env->inner.insert({ "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT", VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT });
	innovator_env->inner.insert({ "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT", VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT });
	innovator_env->inner.insert({ "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT", VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT });
	innovator_env->inner.insert({ "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT", VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT });

	innovator_env->inner.insert({ "VK_SHADER_STAGE_VERTEX_BIT", VK_SHADER_STAGE_VERTEX_BIT });
	innovator_env->inner.insert({ "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT });
	innovator_env->inner.insert({ "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT });
	innovator_env->inner.insert({ "VK_SHADER_STAGE_GEOMETRY_BIT", VK_SHADER_STAGE_GEOMETRY_BIT });
	innovator_env->inner.insert({ "VK_SHADER_STAGE_FRAGMENT_BIT", VK_SHADER_STAGE_FRAGMENT_BIT });
	innovator_env->inner.insert({ "VK_SHADER_STAGE_COMPUTE_BIT", VK_SHADER_STAGE_COMPUTE_BIT });
#ifdef VK_USE_PLATFORM_WIN32_KHR
	innovator_env->inner.insert({ "VK_SHADER_STAGE_RAYGEN_BIT", VK_SHADER_STAGE_RAYGEN_BIT_KHR });
	innovator_env->inner.insert({ "VK_SHADER_STAGE_ANY_HIT_BIT", VK_SHADER_STAGE_ANY_HIT_BIT_KHR });
	innovator_env->inner.insert({ "VK_SHADER_STAGE_CLOSEST_HIT_BIT", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR });
	innovator_env->inner.insert({ "VK_SHADER_STAGE_MISS_BIT", VK_SHADER_STAGE_MISS_BIT_KHR });
	innovator_env->inner.insert({ "VK_SHADER_STAGE_INTERSECTION_BIT", VK_SHADER_STAGE_INTERSECTION_BIT_KHR });
#endif
	innovator_env->inner.insert({ "VK_BUFFER_USAGE_TRANSFER_SRC_BIT", VK_BUFFER_USAGE_TRANSFER_SRC_BIT });
	innovator_env->inner.insert({ "VK_BUFFER_USAGE_TRANSFER_DST_BIT", VK_BUFFER_USAGE_TRANSFER_DST_BIT });
	innovator_env->inner.insert({ "VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT", VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT });
	innovator_env->inner.insert({ "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT", VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT });
	innovator_env->inner.insert({ "VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT", VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT });
	innovator_env->inner.insert({ "VK_BUFFER_USAGE_STORAGE_BUFFER_BIT", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT });
	innovator_env->inner.insert({ "VK_BUFFER_USAGE_INDEX_BUFFER_BIT", VK_BUFFER_USAGE_INDEX_BUFFER_BIT });
	innovator_env->inner.insert({ "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT", VK_BUFFER_USAGE_VERTEX_BUFFER_BIT });
	innovator_env->inner.insert({ "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT", VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT });
#ifdef VK_USE_PLATFORM_WIN32_KHR
	innovator_env->inner.insert({ "VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR", VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR });
	innovator_env->inner.insert({ "VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT", VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT });
#endif
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_SAMPLER", VK_DESCRIPTOR_TYPE_SAMPLER });
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE", VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE });
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE });
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER", VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER });
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER", VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER });
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER });
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC });
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT", VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT });
	innovator_env->inner.insert({ "VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE", VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR });

	innovator_env->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_POINT_LIST", VK_PRIMITIVE_TOPOLOGY_POINT_LIST });
	innovator_env->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_LINE_LIST", VK_PRIMITIVE_TOPOLOGY_LINE_LIST });
	innovator_env->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_LINE_STRIP", VK_PRIMITIVE_TOPOLOGY_LINE_STRIP });
	innovator_env->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST });
	innovator_env->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP });
	innovator_env->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN });
	innovator_env->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY", VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY });
	innovator_env->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY", VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY });
	innovator_env->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY });
	innovator_env->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY });
	innovator_env->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_PATCH_LIST", VK_PRIMITIVE_TOPOLOGY_PATCH_LIST });

	innovator_env->inner.insert({ "VK_PIPELINE_BIND_POINT_GRAPHICS", VK_PIPELINE_BIND_POINT_GRAPHICS });
	innovator_env->inner.insert({ "VK_PIPELINE_BIND_POINT_COMPUTE", VK_PIPELINE_BIND_POINT_COMPUTE });

	innovator_env->inner.insert({ "VK_INDEX_TYPE_UINT16", VK_INDEX_TYPE_UINT16 });
	innovator_env->inner.insert({ "VK_INDEX_TYPE_UINT32", VK_INDEX_TYPE_UINT32 });

	innovator_env->inner.insert({ "VK_VERTEX_INPUT_RATE_VERTEX", VK_VERTEX_INPUT_RATE_VERTEX });
	innovator_env->inner.insert({ "VK_VERTEX_INPUT_RATE_INSTANCE", VK_VERTEX_INPUT_RATE_INSTANCE });

	innovator_env->inner.insert({ "VK_FORMAT_R32_UINT", VK_FORMAT_R32_UINT });
	innovator_env->inner.insert({ "VK_FORMAT_R32_SINT", VK_FORMAT_R32_SINT });
	innovator_env->inner.insert({ "VK_FORMAT_R32_SFLOAT", VK_FORMAT_R32_SFLOAT });
	innovator_env->inner.insert({ "VK_FORMAT_D32_SFLOAT", VK_FORMAT_D32_SFLOAT });
	innovator_env->inner.insert({ "VK_FORMAT_R32G32_UINT", VK_FORMAT_R32G32_UINT });
	innovator_env->inner.insert({ "VK_FORMAT_R32G32_SINT", VK_FORMAT_R32G32_SINT });
	innovator_env->inner.insert({ "VK_FORMAT_R32G32_SFLOAT", VK_FORMAT_R32G32_SFLOAT });
	innovator_env->inner.insert({ "VK_FORMAT_B8G8R8A8_UNORM", VK_FORMAT_B8G8R8A8_UNORM });
	innovator_env->inner.insert({ "VK_FORMAT_R8G8B8A8_UNORM", VK_FORMAT_R8G8B8A8_UNORM });
	innovator_env->inner.insert({ "VK_FORMAT_B8G8R8A8_UINT", VK_FORMAT_B8G8R8A8_UINT });
	innovator_env->inner.insert({ "VK_FORMAT_A8B8G8R8_UINT_PACK32", VK_FORMAT_A8B8G8R8_UINT_PACK32 });
	innovator_env->inner.insert({ "VK_FORMAT_R8G8B8A8_UINT", VK_FORMAT_R8G8B8A8_UINT });
	innovator_env->inner.insert({ "VK_FORMAT_R32G32B32_SINT", VK_FORMAT_R32G32B32_SINT });
	innovator_env->inner.insert({ "VK_FORMAT_R32G32B32_SFLOAT", VK_FORMAT_R32G32B32_SFLOAT });
	innovator_env->inner.insert({ "VK_FORMAT_R16G16B16A16_UINT", VK_FORMAT_R16G16B16A16_UINT });
	innovator_env->inner.insert({ "VK_FORMAT_R32G32B32A32_UINT", VK_FORMAT_R32G32B32A32_UINT });
	innovator_env->inner.insert({ "VK_FORMAT_R32G32B32A32_SINT", VK_FORMAT_R32G32B32A32_SINT });
	innovator_env->inner.insert({ "VK_FORMAT_R32G32B32A32_SFLOAT", VK_FORMAT_R32G32B32A32_SFLOAT });

	return innovator_env;
}
