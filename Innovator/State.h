#pragma once

#include <Innovator/VulkanAPI.h>

#include <glm/glm.hpp>
#include <vector>

struct RenderTarget {
	VkImage image{ 0 };
	VkFormat format{ VK_FORMAT_UNDEFINED };
	VkImageLayout layout{ VK_IMAGE_LAYOUT_UNDEFINED };
	VkImageSubresourceRange subresourceRange;
};

struct State {
	std::shared_ptr<VulkanInstance> vulkan{ nullptr };
	std::shared_ptr<VulkanDevice> device{ nullptr };
	std::shared_ptr<VulkanPipelineCache> pipelinecache{ nullptr };

	VkQueue queue{ nullptr };
	std::shared_ptr<VulkanFence> fence{ nullptr };
	std::vector<VkSemaphore> wait_semaphores;
	std::shared_ptr<VulkanCommandBuffers> default_command{ nullptr };

	VkDescriptorBufferInfo descriptor_buffer_info{
		0, 0, 0
	};
	VkBuffer buffer{ 0 };
	class BufferData* bufferdata{ 0 };
	class VulkanTextureImage* texture{ 0 };
	std::shared_ptr<VulkanRenderpass> renderpass{ 0 };
	std::shared_ptr<VulkanFramebuffer> framebuffer{ 0 };

	VkExtent2D extent{ 0, 0 };

	VkPipelineRasterizationStateCreateInfo rasterization_state{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f,
	};

	RenderTarget renderTarget;

	VkImage image{ 0 };
	VkImageView imageView{ 0 };
	VkImageLayout imageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
	VkFormat imageFormat{ VK_FORMAT_UNDEFINED };
	VkImageSubresourceRange subresourceRange;
	VkSampler sampler{ 0 };
	VulkanCommandBuffers* command{ 0 };

	std::vector<VkAccelerationStructureKHR> top_level_acceleration_structures;
	std::vector<VkAccelerationStructureKHR> bottom_level_acceleration_structures;
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;
	std::vector<VkDescriptorPoolSize> descriptor_pool_sizes;
	std::vector<VkWriteDescriptorSet> write_descriptor_sets;
	std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;
	std::vector<VkPushConstantRange> push_constant_ranges;
	std::vector<VkVertexInputBindingDescription> vertex_input_bindings;
	std::vector<VkVertexInputAttributeDescription> vertex_attributes;
	
	std::vector<VkAttachmentReference> input_attachments;
	std::vector<VkAttachmentReference> color_attachments;
	std::vector<VkAttachmentReference> resolve_attachments;
	VkAttachmentReference depth_stencil_attachment;
	std::vector<uint32_t> preserve_attachments;
	VkPipelineBindPoint bind_point;

	std::vector<VkImageView> framebuffer_attachments;
	std::vector<VkSubpassDescription> subpass_descriptions;
	std::vector<VkAttachmentDescription> attachment_descriptions;

	VkIndexType index_buffer_type{ VK_INDEX_TYPE_NONE_KHR };
	VkBuffer index_buffer{ 0 };
	uint32_t index_count{ 0 };

	std::vector<VkBuffer> vertex_attribute_buffers;
	std::vector<VkDeviceSize> vertex_attribute_buffer_offsets;
	std::vector<uint32_t> vertex_counts;

	glm::dmat4 ViewMatrix{ 1.0 };
	glm::dmat4 ModelMatrix{ 1.0 };
	glm::dmat4 TextureMatrix{ 1.0 };
	glm::dmat4 ProjectionMatrix{ 1.0 };
};


class StateScope {
public:
	StateScope() = delete;

	explicit StateScope(State* state) :
		stateptr(state),
		statecpy(*state)
	{}

	~StateScope()
	{
		this->statecpy.renderTarget = stateptr->renderTarget;
		*stateptr = this->statecpy;
	}

	State* stateptr;
	State statecpy;
};
