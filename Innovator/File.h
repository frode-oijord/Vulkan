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
std::shared_ptr<Node> node(const List & lst)
{
  return make_shared_object<Node, NodeType, Arg...>(lst);
}

template <typename Type, typename ItemType>
std::shared_ptr<Node> shared_from_node_list(const List& lst)
{
  return std::make_shared<Type>(scm::any_cast<ItemType>(lst));
}

template <typename T>
std::shared_ptr<Node> bufferdata(const List & lst)
{
  return std::make_shared<InlineBufferData<T>>(scm::num_cast<T>(lst));
}

uint32_t count(const List & list)
{
  auto node = std::any_cast<std::shared_ptr<Node>>(list[0]);
  auto bufferdata = std::dynamic_pointer_cast<BufferData>(node);
  if (!bufferdata) {
    throw std::invalid_argument("count only works on BufferData nodes!");
  }
  return static_cast<uint32_t>(bufferdata->count());
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


std::shared_ptr<VulkanWindow> window(const List& lst)
{
	auto scene = std::any_cast<std::shared_ptr<Node>>(lst[0]);
	auto node = std::any_cast<std::shared_ptr<Node>>(lst[1]);
	auto color_attachment = std::dynamic_pointer_cast<FramebufferAttachment>(node);
	return std::make_shared<VulkanWindow>(scene, color_attachment);
}

std::shared_ptr<Node> offscreen(const List& lst)
{
	auto node = std::any_cast<std::shared_ptr<Node>>(lst[0]);
	auto color_attachment = std::dynamic_pointer_cast<FramebufferAttachment>(node);
	return std::make_shared<OffscreenImage>(color_attachment);
}


std::any eval_file(const std::string & filename)
{
  env_ptr env = scm::global_env();
	env->outer = std::make_shared<Env>();
  
  env->outer->inner.insert({ "int32", fun_ptr(make_object<int32_t, Number>) });
  env->outer->inner.insert({ "uint32", fun_ptr(make_object<uint32_t, Number>) });
  env->outer->inner.insert({ "float", fun_ptr(make_object<float, Number>) });
  env->outer->inner.insert({ "count", fun_ptr(count) });
	env->outer->inner.insert({ "window", fun_ptr(window) });
	env->outer->inner.insert({ "extent", fun_ptr(node<Extent, uint32_t, uint32_t>) });
	env->outer->inner.insert({ "offscreen-image", fun_ptr(offscreen) });
  env->outer->inner.insert({ "pipeline-bindpoint", fun_ptr(node<PipelineBindpoint, VkPipelineBindPoint> ) });
  env->outer->inner.insert({ "color-attachment", fun_ptr(node<ColorAttachment, uint32_t, VkImageLayout>) });
  env->outer->inner.insert({ "depth-attachment", fun_ptr(node<DepthStencilAttachment, uint32_t, VkImageLayout>) });
  env->outer->inner.insert({ "subpass", fun_ptr(shared_from_node_list<SubpassDescription, std::shared_ptr<Node>>) });
  env->outer->inner.insert({ "renderpass-attachment", fun_ptr(node<RenderpassAttachment, VkFormat, VkSampleCountFlagBits, VkAttachmentLoadOp, VkAttachmentStoreOp, VkAttachmentLoadOp, VkAttachmentStoreOp, VkImageLayout, VkImageLayout>) });
  env->outer->inner.insert({ "renderpass-description", fun_ptr(shared_from_node_list<RenderpassDescription, std::shared_ptr<Node>>) });
  env->outer->inner.insert({ "renderpass", fun_ptr(shared_from_node_list<Renderpass, std::shared_ptr<Node>>) });
  env->outer->inner.insert({ "viewmatrix", fun_ptr(node<ViewMatrix, Number, Number, Number, Number, Number, Number, Number, Number, Number> ) });
  env->outer->inner.insert({ "projmatrix", fun_ptr(node<ProjMatrix, Number, Number, Number, Number>) });
  env->outer->inner.insert({ "framebuffer", fun_ptr(shared_from_node_list<Framebuffer, std::shared_ptr<Node>>) });
  env->outer->inner.insert({ "framebuffer-attachment", fun_ptr(node<FramebufferAttachment, VkFormat, VkImageUsageFlags, VkImageAspectFlags>) });
  env->outer->inner.insert({ "shader", fun_ptr(node<Shader, VkShaderStageFlagBits, std::string>) });
  env->outer->inner.insert({ "sampler", fun_ptr(node<Sampler, VkFilter, VkFilter, VkSamplerMipmapMode, VkSamplerAddressMode, VkSamplerAddressMode, VkSamplerAddressMode, float, uint32_t, float, uint32_t, VkCompareOp, float, float, VkBorderColor, uint32_t>) });
  env->outer->inner.insert({ "textureimage", fun_ptr(node<TextureImage, std::string>) });
  env->outer->inner.insert({ "image", fun_ptr(node<Image, VkSampleCountFlagBits, VkImageTiling, VkImageUsageFlags, VkSharingMode, VkImageCreateFlags, VkImageLayout>) });
  env->outer->inner.insert({ "imageview", fun_ptr(node<ImageView, VkComponentSwizzle, VkComponentSwizzle, VkComponentSwizzle, VkComponentSwizzle>) });
  env->outer->inner.insert({ "group", fun_ptr(shared_from_node_list<Group, std::shared_ptr<Node>>) });
  env->outer->inner.insert({ "separator", fun_ptr(shared_from_node_list<Separator, std::shared_ptr<Node>>) });
  env->outer->inner.insert({ "bufferdata-float", fun_ptr(bufferdata<float>) });
  env->outer->inner.insert({ "bufferdata-uint32", fun_ptr(bufferdata<uint32_t>) });
  env->outer->inner.insert({ "bufferusageflags", fun_ptr(flags<VkBufferUsageFlags, VkBufferUsageFlagBits>) });
  env->outer->inner.insert({ "imageusageflags", fun_ptr(flags<VkImageUsageFlags, VkImageUsageFlagBits>) });
  env->outer->inner.insert({ "imageaspectflags", fun_ptr(flags<VkImageAspectFlags, VkImageAspectFlagBits>) });
  env->outer->inner.insert({ "imagecreateflags", fun_ptr(flags<VkImageCreateFlags, VkImageCreateFlagBits>) });
  env->outer->inner.insert({ "cpumemorybuffer", fun_ptr(node<CpuMemoryBuffer, VkBufferUsageFlags>) });
  env->outer->inner.insert({ "gpumemorybuffer", fun_ptr(node<GpuMemoryBuffer, VkBufferUsageFlags>) });
  env->outer->inner.insert({ "transformbuffer", fun_ptr(node<TransformBuffer>) });
  env->outer->inner.insert({ "indexeddrawcommand", fun_ptr(node<IndexedDrawCommand, uint32_t, uint32_t, uint32_t, int32_t, uint32_t, VkPrimitiveTopology>) });
  env->outer->inner.insert({ "indexbufferdescription", fun_ptr(node<IndexBufferDescription, VkIndexType>) });
  env->outer->inner.insert({ "descriptorsetlayoutbinding", fun_ptr(node<DescriptorSetLayoutBinding, uint32_t, VkDescriptorType, VkShaderStageFlagBits>) });
  env->outer->inner.insert({ "vertexinputbindingdescription", fun_ptr(node<VertexInputBindingDescription, uint32_t, uint32_t, VkVertexInputRate>) });
	env->outer->inner.insert({ "vertexinputattributedescription", fun_ptr(node<VertexInputAttributeDescription, uint32_t, uint32_t, VkFormat, uint32_t>) });

	env->outer->inner.insert({ "VK_IMAGE_CREATE_SPARSE_BINDING_BIT", VK_IMAGE_CREATE_SPARSE_BINDING_BIT });
	env->outer->inner.insert({ "VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT", VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT });
	env->outer->inner.insert({ "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT", VK_IMAGE_CREATE_SPARSE_ALIASED_BIT });

	env->outer->inner.insert({ "VK_FILTER_NEAREST", VK_FILTER_NEAREST });
	env->outer->inner.insert({ "VK_FILTER_LINEAR", VK_FILTER_LINEAR });
	env->outer->inner.insert({ "VK_FILTER_CUBIC_IMG", VK_FILTER_CUBIC_IMG });

	env->outer->inner.insert({ "VK_COMPARE_OP_NEVER", VK_COMPARE_OP_NEVER });
	env->outer->inner.insert({ "VK_COMPARE_OP_LESS", VK_COMPARE_OP_LESS });
	env->outer->inner.insert({ "VK_COMPARE_OP_EQUAL", VK_COMPARE_OP_EQUAL });
	env->outer->inner.insert({ "VK_COMPARE_OP_LESS_OR_EQUAL", VK_COMPARE_OP_LESS_OR_EQUAL });
	env->outer->inner.insert({ "VK_COMPARE_OP_GREATER", VK_COMPARE_OP_GREATER });
  
	env->outer->inner.insert({ "VK_COMPARE_OP_NOT_EQUAL", VK_COMPARE_OP_NOT_EQUAL });
	env->outer->inner.insert({ "VK_COMPARE_OP_GREATER_OR_EQUAL", VK_COMPARE_OP_GREATER_OR_EQUAL });
	env->outer->inner.insert({ "VK_COMPARE_OP_ALWAYS", VK_COMPARE_OP_ALWAYS });

	env->outer->inner.insert({ "VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK", VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK });
	env->outer->inner.insert({ "VK_BORDER_COLOR_INT_TRANSPARENT_BLACK", VK_BORDER_COLOR_INT_TRANSPARENT_BLACK });
	env->outer->inner.insert({ "VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK", VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK });
	env->outer->inner.insert({ "VK_BORDER_COLOR_INT_OPAQUE_BLACK", VK_BORDER_COLOR_INT_OPAQUE_BLACK });
	env->outer->inner.insert({ "VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE", VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE });
	env->outer->inner.insert({ "VK_BORDER_COLOR_INT_OPAQUE_WHITE", VK_BORDER_COLOR_INT_OPAQUE_WHITE });

	env->outer->inner.insert({ "VK_ATTACHMENT_LOAD_OP_LOAD", VK_ATTACHMENT_LOAD_OP_LOAD });
	env->outer->inner.insert({ "VK_ATTACHMENT_LOAD_OP_CLEAR", VK_ATTACHMENT_LOAD_OP_CLEAR });
	env->outer->inner.insert({ "VK_ATTACHMENT_LOAD_OP_DONT_CARE", VK_ATTACHMENT_LOAD_OP_DONT_CARE });

	env->outer->inner.insert({ "VK_ATTACHMENT_STORE_OP_STORE", VK_ATTACHMENT_STORE_OP_STORE });
	env->outer->inner.insert({ "VK_ATTACHMENT_STORE_OP_DONT_CARE", VK_ATTACHMENT_STORE_OP_DONT_CARE });

	env->outer->inner.insert({ "VK_SAMPLER_MIPMAP_MODE_NEAREST", VK_SAMPLER_MIPMAP_MODE_NEAREST });
	env->outer->inner.insert({ "VK_SAMPLER_MIPMAP_MODE_LINEAR", VK_SAMPLER_MIPMAP_MODE_LINEAR });

	env->outer->inner.insert({ "VK_SAMPLER_ADDRESS_MODE_REPEAT", VK_SAMPLER_ADDRESS_MODE_REPEAT });
	env->outer->inner.insert({ "VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT", VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT });
	env->outer->inner.insert({ "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE", VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE });
	env->outer->inner.insert({ "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER", VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER });
	env->outer->inner.insert({ "VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE", VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE });

	env->outer->inner.insert({ "VK_COMPONENT_SWIZZLE_IDENTITY", VK_COMPONENT_SWIZZLE_IDENTITY });
	env->outer->inner.insert({ "VK_COMPONENT_SWIZZLE_ZERO", VK_COMPONENT_SWIZZLE_ZERO });
	env->outer->inner.insert({ "VK_COMPONENT_SWIZZLE_ONE", VK_COMPONENT_SWIZZLE_ONE });
	env->outer->inner.insert({ "VK_COMPONENT_SWIZZLE_R", VK_COMPONENT_SWIZZLE_R });
	env->outer->inner.insert({ "VK_COMPONENT_SWIZZLE_G", VK_COMPONENT_SWIZZLE_G });
	env->outer->inner.insert({ "VK_COMPONENT_SWIZZLE_B", VK_COMPONENT_SWIZZLE_B });
	env->outer->inner.insert({ "VK_COMPONENT_SWIZZLE_A", VK_COMPONENT_SWIZZLE_A });
	env->outer->inner.insert({ "VK_COMPONENT_SWIZZLE_BEGIN_RANGE", VK_COMPONENT_SWIZZLE_IDENTITY });
	env->outer->inner.insert({ "VK_COMPONENT_SWIZZLE_END_RANGE", VK_COMPONENT_SWIZZLE_A });
	env->outer->inner.insert({ "VK_COMPONENT_SWIZZLE_RANGE_SIZE", (VK_COMPONENT_SWIZZLE_A - VK_COMPONENT_SWIZZLE_IDENTITY + 1) });

	env->outer->inner.insert({ "VK_SAMPLE_COUNT_1_BIT", VK_SAMPLE_COUNT_1_BIT });
	env->outer->inner.insert({ "VK_SAMPLE_COUNT_2_BIT", VK_SAMPLE_COUNT_2_BIT });
	env->outer->inner.insert({ "VK_SAMPLE_COUNT_4_BIT", VK_SAMPLE_COUNT_4_BIT });
	env->outer->inner.insert({ "VK_SAMPLE_COUNT_8_BIT", VK_SAMPLE_COUNT_8_BIT });
	env->outer->inner.insert({ "VK_SAMPLE_COUNT_16_BIT", VK_SAMPLE_COUNT_16_BIT });
	env->outer->inner.insert({ "VK_SAMPLE_COUNT_32_BIT", VK_SAMPLE_COUNT_32_BIT });
	env->outer->inner.insert({ "VK_SAMPLE_COUNT_64_BIT", VK_SAMPLE_COUNT_64_BIT });

	env->outer->inner.insert({ "VK_SHARING_MODE_EXCLUSIVE", VK_SHARING_MODE_EXCLUSIVE });
	env->outer->inner.insert({ "VK_SHARING_MODE_CONCURRENT", VK_SHARING_MODE_CONCURRENT });

	env->outer->inner.insert({ "VK_IMAGE_TILING_OPTIMAL", VK_IMAGE_TILING_OPTIMAL });
	env->outer->inner.insert({ "VK_IMAGE_TILING_LINEAR", VK_IMAGE_TILING_LINEAR });

	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_UNDEFINED", VK_IMAGE_LAYOUT_UNDEFINED });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_GENERAL", VK_IMAGE_LAYOUT_GENERAL });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL", VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL", VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL", VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL", VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_PREINITIALIZED", VK_IMAGE_LAYOUT_PREINITIALIZED });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL", VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL", VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR", VK_IMAGE_LAYOUT_PRESENT_SRC_KHR });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR", VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV", VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR", VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL });
	env->outer->inner.insert({ "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR", VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL });

	env->outer->inner.insert({ "VK_IMAGE_ASPECT_COLOR_BIT", VK_IMAGE_ASPECT_COLOR_BIT });
	env->outer->inner.insert({ "VK_IMAGE_ASPECT_DEPTH_BIT", VK_IMAGE_ASPECT_DEPTH_BIT });

	env->outer->inner.insert({ "VK_IMAGE_USAGE_TRANSFER_SRC_BIT", VK_IMAGE_USAGE_TRANSFER_SRC_BIT });
	env->outer->inner.insert({ "VK_IMAGE_USAGE_TRANSFER_DST_BIT", VK_IMAGE_USAGE_TRANSFER_DST_BIT });
  env->outer->inner.insert({ "VK_IMAGE_USAGE_SAMPLED_BIT", VK_IMAGE_USAGE_SAMPLED_BIT });
	env->outer->inner.insert({ "VK_IMAGE_USAGE_STORAGE_BIT", VK_IMAGE_USAGE_STORAGE_BIT });
	env->outer->inner.insert({ "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT", VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT });
	env->outer->inner.insert({ "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT", VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT });
	env->outer->inner.insert({ "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT", VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT });
	env->outer->inner.insert({ "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT", VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT });

	env->outer->inner.insert({ "VK_SHADER_STAGE_VERTEX_BIT", VK_SHADER_STAGE_VERTEX_BIT });
	env->outer->inner.insert({ "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT });
	env->outer->inner.insert({ "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT });
	env->outer->inner.insert({ "VK_SHADER_STAGE_GEOMETRY_BIT", VK_SHADER_STAGE_GEOMETRY_BIT });
  env->outer->inner.insert({ "VK_SHADER_STAGE_FRAGMENT_BIT", VK_SHADER_STAGE_FRAGMENT_BIT });
  env->outer->inner.insert({ "VK_SHADER_STAGE_COMPUTE_BIT", VK_SHADER_STAGE_COMPUTE_BIT });

  env->outer->inner.insert({ "VK_BUFFER_USAGE_TRANSFER_SRC_BIT", VK_BUFFER_USAGE_TRANSFER_SRC_BIT });
  env->outer->inner.insert({ "VK_BUFFER_USAGE_TRANSFER_DST_BIT", VK_BUFFER_USAGE_TRANSFER_DST_BIT });
  env->outer->inner.insert({ "VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT", VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT });
  env->outer->inner.insert({ "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT", VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT });
  env->outer->inner.insert({ "VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT", VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT });
  env->outer->inner.insert({ "VK_BUFFER_USAGE_STORAGE_BUFFER_BIT", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT });
  env->outer->inner.insert({ "VK_BUFFER_USAGE_INDEX_BUFFER_BIT", VK_BUFFER_USAGE_INDEX_BUFFER_BIT });
  env->outer->inner.insert({ "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT", VK_BUFFER_USAGE_VERTEX_BUFFER_BIT });
  env->outer->inner.insert({ "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT", VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT });

  env->outer->inner.insert({ "VK_DESCRIPTOR_TYPE_SAMPLER", VK_DESCRIPTOR_TYPE_SAMPLER });
  env->outer->inner.insert({ "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });
  env->outer->inner.insert({ "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE", VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE });
  env->outer->inner.insert({ "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE });
  env->outer->inner.insert({ "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER", VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER });
  env->outer->inner.insert({ "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER", VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER });
  env->outer->inner.insert({ "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
  env->outer->inner.insert({ "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER });
  env->outer->inner.insert({ "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC });
  env->outer->inner.insert({ "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC });
  env->outer->inner.insert({ "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT", VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT });

  env->outer->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_POINT_LIST", VK_PRIMITIVE_TOPOLOGY_POINT_LIST });
  env->outer->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_LINE_LIST", VK_PRIMITIVE_TOPOLOGY_LINE_LIST });
  env->outer->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_LINE_STRIP", VK_PRIMITIVE_TOPOLOGY_LINE_STRIP });
  env->outer->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST });
  env->outer->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP });
  env->outer->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN });
  env->outer->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY", VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY });
  env->outer->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY", VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY });
  env->outer->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY });
  env->outer->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY });
  env->outer->inner.insert({ "VK_PRIMITIVE_TOPOLOGY_PATCH_LIST", VK_PRIMITIVE_TOPOLOGY_PATCH_LIST });

  env->outer->inner.insert({ "VK_PIPELINE_BIND_POINT_GRAPHICS", VK_PIPELINE_BIND_POINT_GRAPHICS });
  env->outer->inner.insert({ "VK_PIPELINE_BIND_POINT_COMPUTE", VK_PIPELINE_BIND_POINT_COMPUTE });

  env->outer->inner.insert({ "VK_INDEX_TYPE_UINT16", VK_INDEX_TYPE_UINT16 });
  env->outer->inner.insert({ "VK_INDEX_TYPE_UINT32", VK_INDEX_TYPE_UINT32 });

  env->outer->inner.insert({ "VK_VERTEX_INPUT_RATE_VERTEX", VK_VERTEX_INPUT_RATE_VERTEX });
  env->outer->inner.insert({ "VK_VERTEX_INPUT_RATE_INSTANCE", VK_VERTEX_INPUT_RATE_INSTANCE });

  env->outer->inner.insert({ "VK_FORMAT_R32_UINT", VK_FORMAT_R32_UINT });
  env->outer->inner.insert({ "VK_FORMAT_R32_SINT", VK_FORMAT_R32_SINT });
  env->outer->inner.insert({ "VK_FORMAT_R32_SFLOAT", VK_FORMAT_R32_SFLOAT });
  env->outer->inner.insert({ "VK_FORMAT_D32_SFLOAT", VK_FORMAT_D32_SFLOAT });
  env->outer->inner.insert({ "VK_FORMAT_R32G32_UINT", VK_FORMAT_R32G32_UINT });
  env->outer->inner.insert({ "VK_FORMAT_R32G32_SINT", VK_FORMAT_R32G32_SINT });
  env->outer->inner.insert({ "VK_FORMAT_R32G32_SFLOAT", VK_FORMAT_R32G32_SFLOAT });
  env->outer->inner.insert({ "VK_FORMAT_B8G8R8A8_UNORM", VK_FORMAT_B8G8R8A8_UNORM });
	env->outer->inner.insert({ "VK_FORMAT_B8G8R8A8_UINT", VK_FORMAT_B8G8R8A8_UINT});
	env->outer->inner.insert({ "VK_FORMAT_A8B8G8R8_UINT_PACK32", VK_FORMAT_A8B8G8R8_UINT_PACK32 });
  env->outer->inner.insert({ "VK_FORMAT_R32G32B32_UINT", VK_FORMAT_R32G32B32_UINT });
  env->outer->inner.insert({ "VK_FORMAT_R32G32B32_SINT", VK_FORMAT_R32G32B32_SINT });
  env->outer->inner.insert({ "VK_FORMAT_R32G32B32_SFLOAT", VK_FORMAT_R32G32B32_SFLOAT });
  env->outer->inner.insert({ "VK_FORMAT_R32G32B32A32_UINT", VK_FORMAT_R32G32B32A32_UINT });
  env->outer->inner.insert({ "VK_FORMAT_R32G32B32A32_SINT", VK_FORMAT_R32G32B32A32_SINT });
	env->outer->inner.insert({ "VK_FORMAT_R32G32B32A32_SFLOAT", VK_FORMAT_R32G32B32A32_SFLOAT });


  std::ifstream input(filename, std::ios::in);
  
  const std::string code{ 
    std::istreambuf_iterator<char>(input), 
    std::istreambuf_iterator<char>() 
  };

  std::any exp = scm::read(code.begin(), code.end());
  std::any sep = scm::eval(exp, env);
	return sep;
}
