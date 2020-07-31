#pragma once

#include <Innovator/Wrapper.h>

#include <glm/glm.hpp>
#include <vector>

struct State {
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
	  VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,	// sType
	  nullptr,														// pNext
	  0,															// flags;
	  VK_FALSE,														// depthClampEnable
	  VK_FALSE,														// rasterizerDiscardEnable
	  VK_POLYGON_MODE_FILL,											// polygonMode
	  VK_CULL_MODE_BACK_BIT,										// cullMode
	  VK_FRONT_FACE_COUNTER_CLOCKWISE,								// frontFace
	  VK_FALSE,														// depthBiasEnable
	  0.0f,															// depthBiasConstantFactor
	  0.0f,															// depthBiasClamp
	  0.0f,															// depthBiasSlopeFactor
	  1.0f,															// lineWidth
	};

	VkImage image{ 0 };
	VkImage swapchain_source{ 0 };
	VkFormat swapchain_format{ VK_FORMAT_UNDEFINED };
	VkImageView imageView{ 0 };
	VkImageLayout imageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
	VkSampler sampler{ 0 };
	VulkanCommandBuffers* command{ 0 };

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

	VkIndexType index_buffer_type;
	VkBuffer index_buffer{ 0 };
	uint32_t index_count;

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
		this->statecpy.swapchain_source = stateptr->swapchain_source;
		this->statecpy.swapchain_format = stateptr->swapchain_format;
		*stateptr = this->statecpy;
	}

	State* stateptr;
	State statecpy;
};
