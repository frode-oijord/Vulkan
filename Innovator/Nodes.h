#pragma once

#include <Innovator/Timer.h>
#include <Innovator/Visitor.h>
#include <Innovator/Defines.h>
#include <Innovator/Factory.h>

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <shaderc/shaderc.hpp>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include <set>
#include <deque>
#include <memory>
#include <vector>
#include <utility>
#include <fstream>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;


class Node : public NonCopyable {
public:
	Node() = default;
	virtual ~Node() = default;
	virtual void visit(Visitor* visitor) = 0;
};


class Group : public Node {
public:
	Group() = default;
	virtual ~Group() = default;

	explicit Group(std::vector<std::shared_ptr<Node>> children) :
		children(std::move(children))
	{}

	void visit(Visitor* visitor) override
	{
		for (auto child : this->children) {
			child->visit(visitor);
		}
	}

	std::vector<std::shared_ptr<Node>> children;
};


class Separator : public Group {
public:
	Separator() = default;
	virtual ~Separator() = default;

	explicit Separator(std::vector<std::shared_ptr<Node>> children) :
		Group(std::move(children))
	{}

	void visit(Visitor* visitor) override
	{
		StateScope scope(visitor->state.get());
		Group::visit(visitor);
	}
};


class ProjMatrix : public Node {
public:
	IMPLEMENT_VISITABLE;
	ProjMatrix() = delete;
	virtual ~ProjMatrix() = default;

	ProjMatrix(double farplane, double nearplane, double aspectratio, double fieldofview) :
		farplane(farplane),
		nearplane(nearplane),
		aspectratio(aspectratio),
		fieldofview(fieldofview)
	{
		REGISTER_VISITOR(resizevisitor, ProjMatrix, resize);
		REGISTER_VISITOR(rendervisitor, ProjMatrix, render);
	}

	void resize(CommandVisitor* context)
	{
		this->aspectratio =
			static_cast<double>(context->state->extent.width) /
			static_cast<double>(context->state->extent.height);

		this->mat = glm::perspective(
			this->fieldofview,
			this->aspectratio,
			this->nearplane,
			this->farplane);
	}

	void render(CommandVisitor* context)
	{
		context->state->ProjectionMatrix = this->mat;
	}

private:
	glm::dmat4 mat{ 1.0 };

	double farplane;
	double nearplane;
	double aspectratio;
	double fieldofview;
};


class ViewMatrix : public Node {
public:
	IMPLEMENT_VISITABLE;
	ViewMatrix() = delete;
	virtual ~ViewMatrix() = default;

	ViewMatrix(glm::dvec3 eye, glm::dvec3 target, glm::dvec3 up) :
		eye(eye), target(target)
	{
		REGISTER_VISITOR(rendervisitor, ViewMatrix, render);

		this->rot[1] = up;
		this->updateOrientation();
	}

	void orbit(glm::dvec2 dx)
	{
		glm::dvec3 eye = this->eye + this->rot[0] * dx.x + this->rot[1] * dx.y;
		glm::dvec3 newdir = eye - this->target;
		this->eye = this->target + glm::normalize(newdir) * glm::length(this->eye - this->target);
		this->updateOrientation();
	}

	void pan(glm::dvec2 dx)
	{
		glm::dvec3 offset = this->rot[0] * dx.x + this->rot[1] * dx.y;
		this->eye += offset;
		this->target += offset;
		this->updateOrientation();
	}

	void zoom(double dz)
	{
		double focaldist = glm::length(this->eye - this->target);
		this->eye += this->rot[2] * dz * focaldist;
		this->updateOrientation();
	}

	void render(CommandVisitor* context)
	{
		context->state->ViewMatrix = glm::dmat4(glm::transpose(this->rot));
		context->state->ViewMatrix = glm::translate(context->state->ViewMatrix, -this->eye);
	}

	void updateOrientation()
	{
		this->rot[2] = glm::normalize(this->target - this->eye);
		this->rot[0] = glm::normalize(glm::cross(this->rot[1], this->rot[2]));
		this->rot[1] = glm::normalize(glm::cross(this->rot[2], this->rot[0]));
	}


	glm::dmat3 rot;
	glm::dvec3 eye;
	glm::dvec3 target;
};


class ModelMatrix : public Node {
public:
	IMPLEMENT_VISITABLE;
	ModelMatrix() = default;
	virtual ~ModelMatrix() = default;

	ModelMatrix(const glm::dvec3& t, const glm::dvec3& s)
	{
		REGISTER_VISITOR(rendervisitor, ModelMatrix, render);

		this->mat = glm::scale(this->mat, s);
		this->mat = glm::translate(this->mat, t);
	}

	void render(CommandVisitor* context)
	{
		context->state->ModelMatrix *= this->mat;
	}

	glm::dmat4 mat{ 1.0 };
};


class Translate : public Node {
public:
	IMPLEMENT_VISITABLE;
	Translate() = default;
	virtual ~Translate() = default;

	Translate(const glm::dvec3& t, const glm::dvec3& s)
	{
		REGISTER_VISITOR(rendervisitor, ModelMatrix, render);

		this->mat = glm::scale(this->mat, s);
		this->mat = glm::translate(this->mat, t);
	}

	void render(CommandVisitor* context)
	{
		context->state->ModelMatrix *= this->mat;
	}

	glm::dmat4 mat{ 1.0 };
};


class TextureMatrix : public Node {
public:
	IMPLEMENT_VISITABLE;
	TextureMatrix() = default;
	virtual ~TextureMatrix() = default;

	TextureMatrix(const glm::dvec3& t, const glm::dvec3& s)
	{
		REGISTER_VISITOR(rendervisitor, TextureMatrix, render);

		this->mat = glm::scale(this->mat, s);
		this->mat = glm::translate(this->mat, t);
	}

	void render(CommandVisitor* context)
	{
		context->state->TextureMatrix *= this->mat;
	}

	glm::dmat4 mat{ 1.0 };
};


class BufferData : public Node {
public:
	IMPLEMENT_VISITABLE;
	BufferData() = default;
	virtual ~BufferData() = default;

	virtual void copy(char* dst) const = 0;
	virtual size_t size() const = 0;
	virtual size_t stride() const = 0;
	size_t count() const
	{
		return this->size() / this->stride();
	}

	void update(Visitor* context)
	{
		context->state->bufferdata = this;
	}
};


template <typename T>
class InlineBufferData : public BufferData {
public:
	IMPLEMENT_VISITABLE;
	InlineBufferData() = default;
	virtual ~InlineBufferData() = default;

	explicit InlineBufferData(std::vector<T> values) :
		values(std::move(values))
	{
		REGISTER_VISITOR(allocvisitor, InlineBufferData<T>, update);
		REGISTER_VISITOR(pipelinevisitor, InlineBufferData<T>, update);
		REGISTER_VISITOR(recordvisitor, InlineBufferData<T>, update);
	}

	void copy(char* dst) const override
	{
		std::copy(this->values.begin(), this->values.end(), reinterpret_cast<T*>(dst));
	}

	size_t size() const override
	{
		return this->values.size() * sizeof(T);
	}

	size_t stride() const override
	{
		return sizeof(T);
	}

	std::vector<T> values;
};


class TextureData : public BufferData {
public:
	IMPLEMENT_VISITABLE;
	TextureData() = delete;
	virtual ~TextureData() = default;

	explicit TextureData(const std::string& filename) :
		texture(VulkanImageFactory::Create(filename))
	{
		REGISTER_VISITOR(allocvisitor, TextureData, update);
		REGISTER_VISITOR(pipelinevisitor, TextureData, update);
		REGISTER_VISITOR(recordvisitor, TextureData, update);
		REGISTER_VISITOR(rendervisitor, TextureData, update);
	}

	void copy(char* dst) const override
	{
		std::copy(this->texture->data(), this->texture->data() + this->texture->size(), dst);
	}

	size_t size() const override
	{
		return this->texture->size();
	}

	size_t stride() const override
	{
		return this->texture->element_size();
	}

	void update(Visitor* context)
	{
		context->state->bufferdata = this;
		context->state->texture = this->texture.get();
		context->state->subresourceRange = this->texture->subresourceRange();
	}

private:
	std::shared_ptr<VulkanTextureImage> texture;
};


class CpuMemoryBuffer : public Node {
public:
	IMPLEMENT_VISITABLE;
	CpuMemoryBuffer() = delete;
	virtual ~CpuMemoryBuffer() = default;

	explicit CpuMemoryBuffer(
		VkBufferUsageFlags usage_flags,
		VkBufferCreateFlags create_flags = 0) :
		usage_flags(usage_flags),
		create_flags(create_flags)
	{
		REGISTER_VISITOR(allocvisitor, CpuMemoryBuffer, alloc);
		REGISTER_VISITOR(pipelinevisitor, CpuMemoryBuffer, update);
		REGISTER_VISITOR(recordvisitor, CpuMemoryBuffer, update);
		REGISTER_VISITOR(rendervisitor, CpuMemoryBuffer, update);
	}

	void alloc(CommandVisitor* context)
	{
		this->bufferobject = std::make_shared<VulkanBufferObject>(
			context->state->device,
			this->create_flags,
			context->state->bufferdata->size(),
			this->usage_flags,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		MemoryMap memmap(this->bufferobject->memory.get());
		context->state->bufferdata->copy(memmap.mem);

		context->state->buffer = this->bufferobject->buffer->buffer;
	}

	void update(Visitor* context)
	{
		context->state->buffer = this->bufferobject->buffer->buffer;
	}

private:
	VkBufferUsageFlags usage_flags;
	VkBufferCreateFlags create_flags;
	std::shared_ptr<VulkanBufferObject> bufferobject;
};


class GpuMemoryBuffer : public Node {
public:
	IMPLEMENT_VISITABLE;
	GpuMemoryBuffer() = delete;
	virtual ~GpuMemoryBuffer() = default;

	explicit GpuMemoryBuffer(VkBufferUsageFlags usage_flags, VkBufferCreateFlags create_flags = 0) :
		usage_flags(usage_flags),
		create_flags(create_flags)
	{
		REGISTER_VISITOR(allocvisitor, GpuMemoryBuffer, alloc);
		REGISTER_VISITOR(pipelinevisitor, GpuMemoryBuffer, update);
		REGISTER_VISITOR(recordvisitor, GpuMemoryBuffer, update);
		REGISTER_VISITOR(rendervisitor, GpuMemoryBuffer, update);
	}

	void alloc(CommandVisitor* context)
	{
		this->bufferobject = std::make_shared<VulkanBufferObject>(
			context->state->device,
			this->create_flags,
			context->state->bufferdata->size(),
			this->usage_flags,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		std::vector<VkBufferCopy> regions = { {
			.srcOffset = 0,
			.dstOffset = 0,
			.size = context->state->bufferdata->size(),
		} };

		vk.CmdCopyBuffer(
			context->state->default_command->buffer(),
			context->state->buffer,
			this->bufferobject->buffer->buffer,
			static_cast<uint32_t>(regions.size()),
			regions.data());
	}

	void update(Visitor* context)
	{
		context->state->buffer = this->bufferobject->buffer->buffer;
	}

private:
	VkBufferUsageFlags usage_flags;
	VkBufferCreateFlags create_flags;
	std::shared_ptr<VulkanBufferObject> bufferobject;
};


class TransformBuffer : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~TransformBuffer() = default;

	TransformBuffer()
	{
		REGISTER_VISITOR(allocvisitor, TransformBuffer, alloc);
		REGISTER_VISITOR(pipelinevisitor, TransformBuffer, pipeline);
		REGISTER_VISITOR(rendervisitor, TransformBuffer, render);
	}

	void alloc(Visitor* context)
	{
		this->buffer = std::make_shared<VulkanBufferObject>(
			context->state->device,
			0,
			sizeof(glm::mat4) * 3,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}

	void pipeline(Visitor* context)
	{
		context->state->buffer = this->buffer->buffer->buffer;
	}

	void render(Visitor* context)
	{
		std::array<glm::mat4, 3> data = {
		  glm::mat4(context->state->ViewMatrix * context->state->ModelMatrix),
		  glm::mat4(context->state->ProjectionMatrix),
		  glm::mat4(context->state->TextureMatrix)
		};

		MemoryMap map(this->buffer->memory.get());
		std::copy(data.begin(), data.end(), reinterpret_cast<glm::mat4*>(map.mem));
	}

private:
	std::shared_ptr<VulkanBufferObject> buffer;
	VkDescriptorBufferInfo descriptor_buffer_info{};
};


class IndexBufferDescription : public Node {
public:
	IMPLEMENT_VISITABLE;
	IndexBufferDescription() = delete;
	virtual ~IndexBufferDescription() = default;

	explicit IndexBufferDescription(VkIndexType type) :
		type(type)
	{
		REGISTER_VISITOR(allocvisitor, IndexBufferDescription, update);
		REGISTER_VISITOR(recordvisitor, IndexBufferDescription, update);
	}

	void update(Visitor* context)
	{
		context->state->index_buffer = context->state->buffer;
		context->state->index_buffer_type = this->type;
		context->state->index_count = context->state->bufferdata->count();
	}

private:
	VkIndexType type;
};


class VertexInputAttributeDescription : public Node {
public:
	IMPLEMENT_VISITABLE;
	VertexInputAttributeDescription() = delete;
	virtual ~VertexInputAttributeDescription() = default;

	explicit VertexInputAttributeDescription(
		uint32_t location,
		uint32_t binding,
		VkFormat format,
		uint32_t offset) :
		vertex_input_attribute_description({
			.location = location,
			.binding = binding,
			.format = format,
			.offset = offset,
			})
	{
		REGISTER_VISITOR(allocvisitor, VertexInputAttributeDescription, update);
		REGISTER_VISITOR(pipelinevisitor, VertexInputAttributeDescription, update);
		REGISTER_VISITOR(recordvisitor, VertexInputAttributeDescription, update);
	}

	void update(Visitor* context)
	{
		context->state->vertex_attributes.push_back(this->vertex_input_attribute_description);
		context->state->vertex_attribute_buffers.push_back(context->state->buffer);
		context->state->vertex_attribute_buffer_offsets.push_back(0);
		context->state->vertex_counts.push_back(context->state->bufferdata->count());
	}

private:
	VkVertexInputAttributeDescription vertex_input_attribute_description;
};


class VertexInputBindingDescription : public Node {
public:
	IMPLEMENT_VISITABLE;
	VertexInputBindingDescription() = delete;
	virtual ~VertexInputBindingDescription() = default;

	explicit VertexInputBindingDescription(
		uint32_t binding,
		uint32_t stride,
		VkVertexInputRate inputRate) :
		binding(binding),
		stride(stride),
		inputRate(inputRate)
	{
		REGISTER_VISITOR(allocvisitor, VertexInputBindingDescription, update);
		REGISTER_VISITOR(pipelinevisitor, VertexInputBindingDescription, update);
	}

	void update(Visitor* context)
	{
		context->state->vertex_input_bindings.push_back({
			.binding = this->binding,
			.stride = this->stride,
			.inputRate = this->inputRate,
			});
	}

private:
	uint32_t binding;
	uint32_t stride;
	VkVertexInputRate inputRate;
};


class DescriptorSetLayoutBinding : public Node {
public:
	IMPLEMENT_VISITABLE;
	DescriptorSetLayoutBinding() = delete;
	virtual ~DescriptorSetLayoutBinding() = default;

	DescriptorSetLayoutBinding(
		uint32_t binding,
		VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags)
	{
		REGISTER_VISITOR(pipelinevisitor, DescriptorSetLayoutBinding, pipeline);

		this->info.binding = binding;
		this->info.descriptorType = descriptorType;
		this->info.stageFlags = stageFlags;
	}

	void pipeline(Visitor* context)
	{
		if (info.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
			info.descriptor_set_acceleration_structure = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
				.pNext = nullptr,
				.accelerationStructureCount = static_cast<uint32_t>(context->state->top_level_acceleration_structures.size()),
				.pAccelerationStructures = context->state->top_level_acceleration_structures.data(),
			};
		}
		else {
			info.descriptor_buffer_info = {
				.buffer = context->state->buffer,
				.offset = 0,
				.range = VK_WHOLE_SIZE
			};

			info.descriptor_image_info = {
				.sampler = context->state->sampler,
				.imageView = context->state->imageView,
				.imageLayout = context->state->imageLayout
			};
		}
		context->state->descriptor_set_infos.push_back(this->info);
	}

private:
	DescriptorSetInfo info;
};


class Shader : public Node {
public:
	IMPLEMENT_VISITABLE;
	Shader() = delete;
	virtual ~Shader() = default;

	explicit Shader(const VkShaderStageFlagBits stage, std::string glsl) :
		stage(stage)
	{
		REGISTER_VISITOR(devicevisitor, Shader, device);
		REGISTER_VISITOR(allocvisitor, Shader, alloc);
		REGISTER_VISITOR(pipelinevisitor, Shader, pipeline);
		REGISTER_VISITOR(recordvisitor, Shader, record);

		shaderc_shader_kind kind = [stage]()
		{
			switch (stage) {
			case VK_SHADER_STAGE_VERTEX_BIT:
				return shaderc_vertex_shader;
			case VK_SHADER_STAGE_COMPUTE_BIT:
				return shaderc_compute_shader;
			case VK_SHADER_STAGE_GEOMETRY_BIT:
				return shaderc_geometry_shader;
			case VK_SHADER_STAGE_FRAGMENT_BIT:
				return shaderc_fragment_shader;
			case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
				return shaderc_tess_control_shader;
			case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
				return shaderc_tess_evaluation_shader;
			case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
				return shaderc_raygen_shader;
			case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
				return shaderc_anyhit_shader;
			case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
				return shaderc_closesthit_shader;
			case VK_SHADER_STAGE_MISS_BIT_KHR:
				return shaderc_miss_shader;
			case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
				return shaderc_intersection_shader;
			default:
				throw std::runtime_error("Unknown shader stage");
			}
		}();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		shaderc::SpvCompilationResult spv = compiler.CompileGlslToSpv(glsl, kind, "", options);

		if (spv.GetCompilationStatus() != shaderc_compilation_status_success) {
			throw std::runtime_error(spv.GetErrorMessage());
		}
		this->spv = { spv.cbegin(), spv.cend() };
	}

	void device(DeviceVisitor* context)
	{
		switch (this->stage) {
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			context->device_features.tessellationShader = VK_TRUE;
			break;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			context->device_features.tessellationShader = VK_TRUE;
			break;
		case VK_SHADER_STAGE_GEOMETRY_BIT:
			context->device_features.geometryShader = VK_TRUE;
			break;
		default: break;
		}
	}

	void alloc(CommandVisitor* context)
	{
		this->shader = std::make_unique<VulkanShaderModule>(context->state->device, this->spv);
	}

	void updateState(Visitor* context)
	{
		context->state->shader_stage_infos.push_back({
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = this->stage,
			.module = this->shader->shadermodule,
			.pName = "main",
			.pSpecializationInfo = nullptr,
			});
	}

	void pipeline(Visitor* context)
	{
		this->updateState(context);
	}

	void record(Visitor* context)
	{
		this->updateState(context);
	}

public:
	std::vector<uint32_t> spv;
	VkShaderStageFlagBits stage;
	std::unique_ptr<VulkanShaderModule> shader;
};


#ifdef VK_KHR_ray_tracing

class BottomLevelAccelerationStructure : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~BottomLevelAccelerationStructure() = default;

	explicit BottomLevelAccelerationStructure()
	{
		REGISTER_VISITOR(allocvisitor, BottomLevelAccelerationStructure, alloc);
		REGISTER_VISITOR(devicevisitor, BottomLevelAccelerationStructure, device);
	}

	void device(DeviceVisitor* context)
	{
		context->getFeatures<VkPhysicalDeviceBufferDeviceAddressFeatures>().bufferDeviceAddress = VK_TRUE;
	}

	void alloc(CommandVisitor* context)
	{
		VkBufferDeviceAddressInfo index_buffer_address_info{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.pNext = nullptr,
			.buffer = context->state->index_buffer,
		};

		VkDeviceOrHostAddressConstKHR indexData{
			.deviceAddress = context->state->vulkan->vkGetBufferDeviceAddressKHR(
				context->state->device->device, &index_buffer_address_info)
		};

		VkIndexType indexType = context->state->index_buffer_type;
		uint32_t maxPrimitiveCount = context->state->index_count / 3;

		std::vector<VkAccelerationStructureGeometryKHR> geometries;
		std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> create_geometry_type_infos;

		for (size_t i = 0; i < context->state->vertex_attributes.size(); i++)
		{
			VkFormat vertexFormat = context->state->vertex_attributes[i].format;
			uint32_t maxVertexCount = context->state->vertex_counts[i] / 3;

			VkBufferDeviceAddressInfo vertex_buffer_address_info{
				.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
				.pNext = nullptr,
				.buffer = context->state->vertex_attribute_buffers[i],
			};

			VkDeviceOrHostAddressConstKHR vertexData{
				.deviceAddress = context->state->vulkan->vkGetBufferDeviceAddressKHR(
					context->state->device->device, &vertex_buffer_address_info)
			};

			VkDeviceSize vertexStride = context->state->vertex_input_bindings[i].stride;

			VkAccelerationStructureGeometryTrianglesDataKHR geometry_triangles_data{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
				.pNext = nullptr,
				.vertexFormat = vertexFormat,
				.vertexData = vertexData,
				.vertexStride = vertexStride,
				.indexType = indexType,
				.indexData = indexData,
				.transformData = {}
			};

			VkAccelerationStructureGeometryDataKHR geometry_data{
				.triangles = geometry_triangles_data
			};

			VkAccelerationStructureGeometryKHR geometry{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
				.pNext = nullptr,
				.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
				.geometry = geometry_data,
				.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
			};

			geometries.push_back(geometry);

			create_geometry_type_infos.push_back({
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR,
				.pNext = nullptr,
				.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
				.maxPrimitiveCount = maxPrimitiveCount,
				.indexType = indexType,
				.maxVertexCount = maxVertexCount,
				.vertexFormat = vertexFormat,
				.allowsTransforms = VK_FALSE
				});
		}

		this->as = std::make_shared<VulkanAccelerationStructure>(
			context->state->vulkan,
			context->state->device,
			0,															// compactedSize
			VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,			// type
			VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,	// flags
			create_geometry_type_infos,									// geometryInfos
			0);															// deviceAddress

		VkAccelerationStructureBuildOffsetInfoKHR build_offset_info{
			.primitiveCount = maxPrimitiveCount,
			.primitiveOffset = 0,
			.firstVertex = 0,
			.transformOffset = 0,
		};

		std::vector<VkAccelerationStructureBuildOffsetInfoKHR> build_offset_infos{
			build_offset_info
		};

		this->as->build(
			context->state->default_command->buffer(),
			geometries,
			build_offset_infos);

		context->state->bottom_level_acceleration_structures.push_back(this->as->as);
	}

	std::shared_ptr<VulkanAccelerationStructure> as{ nullptr };
};


class TopLevelAccelerationStructure : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~TopLevelAccelerationStructure() = default;

	explicit TopLevelAccelerationStructure()
	{
		REGISTER_VISITOR(allocvisitor, TopLevelAccelerationStructure, alloc);
		REGISTER_VISITOR(pipelinevisitor, TopLevelAccelerationStructure, pipeline);
	}

	void alloc(CommandVisitor* context)
	{
		std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> create_geometry_infos{ {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR,
			.pNext = nullptr,
			.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
			.maxPrimitiveCount = 1,
			.indexType = VK_INDEX_TYPE_NONE_KHR,
			.maxVertexCount = 0,
			.vertexFormat = VK_FORMAT_UNDEFINED,
			.allowsTransforms = VK_FALSE,
		} };

		this->tlas = std::make_shared<VulkanAccelerationStructure>(
			context->state->vulkan,
			context->state->device,
			0,															// compactedSize
			VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,				// type
			VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,	// flags
			create_geometry_infos,										// geometryInfos
			0);															// deviceAddress

		std::vector<VkAccelerationStructureGeometryKHR> acceleration_structure_geometries;
		std::vector<VkAccelerationStructureBuildOffsetInfoKHR> build_offset_infos;

		for (auto& blas : context->state->bottom_level_acceleration_structures) {

			VkTransformMatrixKHR transform{ {
				{ 1, 0, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 1, 0 }
			} };

			VkAccelerationStructureInstanceKHR instance{
				.transform = transform,
				.instanceCustomIndex = 0,
				.mask = 0xFF,
				.instanceShaderBindingTableRecordOffset = 0,
				.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
				.accelerationStructureReference = context->state->device->getDeviceAddress(blas)
			};

			auto instance_buffer = std::make_shared<VulkanBufferObject>(
				context->state->device,
				0,
				sizeof(instance),
				VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			instance_buffer->memory->memcpy(&instance, sizeof(instance));

			VkDeviceOrHostAddressConstKHR instance_data_device_address{
				.deviceAddress = context->state->device->getDeviceAddress(instance_buffer->buffer->buffer)
			};

			VkAccelerationStructureGeometryInstancesDataKHR acceleration_structure_geometry_instances_data{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
				.pNext = nullptr,
				.arrayOfPointers = VK_FALSE,
				.data = instance_data_device_address,
			};

			VkAccelerationStructureGeometryDataKHR acceleration_structure_geometry_data{
				.instances = acceleration_structure_geometry_instances_data
			};

			VkAccelerationStructureGeometryKHR acceleration_structure_geometry{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
				.pNext = nullptr,
				.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
				.geometry = acceleration_structure_geometry_data,
				.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
			};

			acceleration_structure_geometries.push_back(acceleration_structure_geometry);

			VkAccelerationStructureBuildOffsetInfoKHR build_offset_info{
				.primitiveCount = 1,
				.primitiveOffset = 0,
				.firstVertex = 0,
				.transformOffset = 0,
			};

			build_offset_infos.push_back(build_offset_info);
		}

		this->tlas->build(
			context->state->default_command->buffer(),
			acceleration_structure_geometries,
			build_offset_infos);
	}

	void pipeline(Visitor* context)
	{
		context->state->top_level_acceleration_structures.push_back(this->tlas->as);
	}

	std::shared_ptr<VulkanAccelerationStructure> tlas{ nullptr };
};


class RayTraceCommand : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~RayTraceCommand() = default;

	explicit RayTraceCommand()
	{
		REGISTER_VISITOR(devicevisitor, RayTraceCommand, device);
		REGISTER_VISITOR(allocvisitor, RayTraceCommand, alloc);
		REGISTER_VISITOR(pipelinevisitor, RayTraceCommand, pipeline);
		REGISTER_VISITOR(recordvisitor, RayTraceCommand, record);
		REGISTER_VISITOR(rendervisitor, RayTraceCommand, render);
	}

	void device(DeviceVisitor* visitor)
	{
		visitor->getFeatures<VkPhysicalDeviceRayTracingFeaturesKHR>().rayTracing = VK_TRUE;
		visitor->getFeatures<VkPhysicalDeviceBufferDeviceAddressFeatures>().bufferDeviceAddress = VK_TRUE;

		visitor->device_extensions.push_back(VK_KHR_RAY_TRACING_EXTENSION_NAME);
		visitor->device_extensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
		visitor->device_extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		visitor->device_extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		visitor->device_extensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
		visitor->device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
		visitor->device_extensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
	}

	void alloc(CommandVisitor* context)
	{
		this->command = std::make_unique<VulkanCommandBuffers>(context->state->device);
		this->queue = context->state->device->getQueue(VK_QUEUE_GRAPHICS_BIT);
	}

	void pipeline(Visitor* context)
	{
		std::vector<VkDescriptorPoolSize> descriptor_pool_sizes;
		std::vector<VkWriteDescriptorSet> write_descriptor_sets;
		std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;

		for (auto& info : context->state->descriptor_set_infos) {
			VkWriteDescriptorSet write_descriptor_set{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = nullptr,
				.dstBinding = info.binding,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = info.descriptorType,
				.pImageInfo = nullptr,
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr,
			};

			if (info.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
				write_descriptor_set.pNext = &info.descriptor_set_acceleration_structure;
			}

			write_descriptor_set.pBufferInfo = &info.descriptor_buffer_info;
			write_descriptor_set.pImageInfo = &info.descriptor_image_info;

			descriptor_pool_sizes.push_back({
				.type = info.descriptorType,
				.descriptorCount = 1,
				});

			descriptor_set_layout_bindings.push_back({
				.binding = info.binding,
				.descriptorType = info.descriptorType,
				.descriptorCount = 1,
				.stageFlags = info.stageFlags,
				.pImmutableSamplers = nullptr,
				});

			write_descriptor_sets.push_back(write_descriptor_set);
		}

		auto descriptor_pool = std::make_shared<VulkanDescriptorPool>(
			context->state->device,
			descriptor_pool_sizes);

		this->descriptor_set_layout = std::make_unique<VulkanDescriptorSetLayout>(
			context->state->device,
			descriptor_set_layout_bindings);

		std::vector<VkDescriptorSetLayout> descriptor_set_layouts{
			this->descriptor_set_layout->layout
		};

		this->pipeline_layout = std::make_unique<VulkanPipelineLayout>(
			context->state->device,
			descriptor_set_layouts,
			context->state->pushConstantRanges);

		this->descriptor_sets = std::make_unique<VulkanDescriptorSets>(
			context->state->device,
			descriptor_pool,
			descriptor_set_layouts);

		for (auto& write_descriptor_set : write_descriptor_sets) {
			write_descriptor_set.dstSet = this->descriptor_sets->descriptor_sets[0];
		}
		this->descriptor_sets->update(write_descriptor_sets);

		for (size_t i = 0; i < context->state->shader_stage_infos.size(); i++) {
			VkRayTracingShaderGroupCreateInfoKHR rtx_group_info{
				.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
				.pNext = nullptr,
				.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
				.generalShader = VK_SHADER_UNUSED_KHR,
				.closestHitShader = VK_SHADER_UNUSED_KHR,
				.anyHitShader = VK_SHADER_UNUSED_KHR,
				.intersectionShader = VK_SHADER_UNUSED_KHR,
			};

			switch (context->state->shader_stage_infos[i].stage) {
			case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
			case VK_SHADER_STAGE_MISS_BIT_KHR:
				rtx_group_info.generalShader = i;
				break;
			case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
				rtx_group_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
				rtx_group_info.closestHitShader = i;
				break;
			default:
				throw std::runtime_error("unsupported rtx shader stage");
			}

			this->shader_groups.push_back(rtx_group_info);
		}

		this->rtx_pipeline = std::make_shared<VulkanRayTracingPipeline>(
			context->state->vulkan,
			context->state->device,
			context->state->shader_stage_infos,
			this->shader_groups,
			this->pipeline_layout.get());
	}

	void record(Visitor* context)
	{
		VkPhysicalDeviceRayTracingPropertiesKHR ray_tracing_properties = context->state->device->physical_device.ray_tracing_properties;

		const VkDeviceSize shader_binding_table_count = context->state->shader_stage_infos.size();
		const VkDeviceSize shader_binding_table_stride = ray_tracing_properties.shaderGroupBaseAlignment;
		const VkDeviceSize shader_binding_table_size = shader_binding_table_stride * shader_binding_table_count;
		const VkDeviceSize shader_binding_table_handle_size = ray_tracing_properties.shaderGroupHandleSize;

		this->shader_binding_table = std::make_shared<VulkanBufferObject>(
			context->state->device,
			0,
			shader_binding_table_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		std::vector<uint8_t> shader_handle_storage(shader_binding_table_size);
		THROW_ON_ERROR(context->state->vulkan->vkGetRayTracingShaderGroupHandlesKHR(
			context->state->device->device,
			this->rtx_pipeline->pipeline,
			0,
			shader_binding_table_count,
			shader_binding_table_size,
			shader_handle_storage.data()));

		for (uint32_t i = 0; i < shader_binding_table_count; i++) {
			this->shader_binding_table->memory->memcpy(
				&shader_handle_storage[shader_binding_table_handle_size * i],
				shader_binding_table_handle_size,
				shader_binding_table_stride * i);
		}

		VkStridedBufferRegionKHR shader_sbt_entry{
			.buffer = this->shader_binding_table->buffer->buffer,
			.offset = 0,
			.stride = shader_binding_table_stride,
			.size = shader_binding_table_size,
		};

		VkStridedBufferRegionKHR raygen_shader_sbt_entry{};
		VkStridedBufferRegionKHR miss_shader_sbt_entry{};
		VkStridedBufferRegionKHR hit_shader_sbt_entry{};
		VkStridedBufferRegionKHR callable_shader_sbt_entry{};

		for (size_t i = 0; i < context->state->shader_stage_infos.size(); i++) {
			switch (context->state->shader_stage_infos[i].stage) {
			case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
				raygen_shader_sbt_entry = shader_sbt_entry;
				raygen_shader_sbt_entry.offset = shader_sbt_entry.stride * i;
				break;
			case VK_SHADER_STAGE_MISS_BIT_KHR:
				miss_shader_sbt_entry = shader_sbt_entry;
				miss_shader_sbt_entry.offset = shader_sbt_entry.stride * i;
				break;
			case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
				hit_shader_sbt_entry = shader_sbt_entry;
				hit_shader_sbt_entry.offset = shader_sbt_entry.stride * i;
				break;
			case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
				callable_shader_sbt_entry = shader_sbt_entry;
				callable_shader_sbt_entry.offset = shader_sbt_entry.stride * i;
				break;
			default:
				throw std::runtime_error("unsupported rtx shader stage");
				break;
			}
		}

		VulkanCommandBuffers::Scope scope(this->command.get());

		vk.CmdBindPipeline(this->command->buffer(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, this->rtx_pipeline->pipeline);

		vk.CmdBindDescriptorSets(
			this->command->buffer(),
			VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
			this->pipeline_layout->layout,
			0,
			static_cast<uint32_t>(this->descriptor_sets->descriptor_sets.size()),
			this->descriptor_sets->descriptor_sets.data(),
			0,
			nullptr);

		context->state->vulkan->vkCmdTraceRaysKHR(
			this->command->buffer(),
			&raygen_shader_sbt_entry,
			&miss_shader_sbt_entry,
			&hit_shader_sbt_entry,
			&callable_shader_sbt_entry,
			context->state->extent.width,
			context->state->extent.height,
			1);
	}

	void render(Visitor* context)
	{
		this->command->submit(this->queue, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
	}

	std::shared_ptr<VulkanDescriptorSetLayout> descriptor_set_layout{ nullptr };
	std::shared_ptr<VulkanDescriptorSets> descriptor_sets{ nullptr };
	std::shared_ptr<VulkanPipelineLayout> pipeline_layout{ nullptr };
	std::shared_ptr<VulkanBufferObject> shader_binding_table{ nullptr };
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups;
	std::shared_ptr<VulkanRayTracingPipeline> rtx_pipeline;
	std::unique_ptr<VulkanCommandBuffers> command;
	VkQueue queue{ nullptr };

};
#endif


class Extent : public Node {
public:
	IMPLEMENT_VISITABLE;
	Extent() = delete;
	virtual ~Extent() = default;

	Extent(uint32_t width, uint32_t height, uint32_t depth = 1) :
		extent{ width, height, depth }
	{
		REGISTER_VISITOR(allocvisitor, Extent, update);
		REGISTER_VISITOR(pipelinevisitor, Extent, update);
		REGISTER_VISITOR(recordvisitor, Extent, update);
		REGISTER_VISITOR(resizevisitor, Extent, update);
		REGISTER_VISITOR(rendervisitor, Extent, update);
	}

	void update(Visitor* context)
	{
		context->state->extent = this->extent;
	}

private:
	VkExtent3D extent;
};


class CullMode : public Node {
public:
	IMPLEMENT_VISITABLE;
	CullMode() = delete;
	virtual ~CullMode() = default;

	explicit CullMode(VkCullModeFlags cullmode) :
		cullmode(cullmode)
	{
		REGISTER_VISITOR(pipelinevisitor, CullMode, pipeline);
	}

	void pipeline(Visitor* context)
	{
		context->state->rasterization_state.cullMode = this->cullmode;
	}

private:
	VkCullModeFlags cullmode;
};


class ComputeCommand : public Node {
public:
	IMPLEMENT_VISITABLE;
	ComputeCommand() = delete;
	virtual ~ComputeCommand() = default;

	explicit ComputeCommand(
		uint32_t group_count_x,
		uint32_t group_count_y,
		uint32_t group_count_z) :
		group_count_x(group_count_x),
		group_count_y(group_count_y),
		group_count_z(group_count_z)
	{
		REGISTER_VISITOR(pipelinevisitor, ComputeCommand, pipeline);
	}

	void pipeline(Visitor* context)
	{
		std::vector<VkDescriptorPoolSize> descriptor_pool_sizes;
		std::vector<VkWriteDescriptorSet> write_descriptor_sets;
		std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;

		for (auto& info : context->state->descriptor_set_infos) {
			VkWriteDescriptorSet write_descriptor_set{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = nullptr,
				.dstBinding = info.binding,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = info.descriptorType,
				.pImageInfo = nullptr,
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr,
			};

			if (info.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
				write_descriptor_set.pNext = &info.descriptor_set_acceleration_structure;
			}

			write_descriptor_set.pBufferInfo = &info.descriptor_buffer_info;
			write_descriptor_set.pImageInfo = &info.descriptor_image_info;

			descriptor_pool_sizes.push_back({
				.type = info.descriptorType,
				.descriptorCount = 1,
				});

			descriptor_set_layout_bindings.push_back({
				.binding = info.binding,
				.descriptorType = info.descriptorType,
				.descriptorCount = 1,
				.stageFlags = info.stageFlags,
				.pImmutableSamplers = nullptr,
				});

			write_descriptor_sets.push_back(write_descriptor_set);
		}

		auto descriptor_pool = std::make_shared<VulkanDescriptorPool>(
			context->state->device,
			descriptor_pool_sizes);

		this->descriptor_set_layout = std::make_unique<VulkanDescriptorSetLayout>(
			context->state->device,
			descriptor_set_layout_bindings);

		std::vector<VkDescriptorSetLayout> descriptor_set_layouts{
			this->descriptor_set_layout->layout
		};

		this->pipeline_layout = std::make_unique<VulkanPipelineLayout>(
			context->state->device,
			descriptor_set_layouts,
			context->state->pushConstantRanges);

		this->descriptor_sets = std::make_unique<VulkanDescriptorSets>(
			context->state->device,
			descriptor_pool,
			descriptor_set_layouts);

		for (auto& write_descriptor_set : write_descriptor_sets) {
			write_descriptor_set.dstSet = this->descriptor_sets->descriptor_sets[0];
		}
		this->descriptor_sets->update(write_descriptor_sets);

		this->compute_pipeline = std::make_unique<VulkanComputePipeline>(
			context->state->device,
			context->state->pipelinecache->cache,
			context->state->shader_stage_infos[0],
			this->pipeline_layout->layout);
	}

	void record(Visitor* context)
	{
		vk.CmdBindDescriptorSets(this->command->buffer(),
			VK_PIPELINE_BIND_POINT_COMPUTE,
			this->pipeline_layout->layout,
			0,
			static_cast<uint32_t>(this->descriptor_sets->descriptor_sets.size()),
			this->descriptor_sets->descriptor_sets.data(),
			0,
			nullptr);

		vk.CmdBindPipeline(this->command->buffer(),
			VK_PIPELINE_BIND_POINT_COMPUTE,
			this->compute_pipeline->pipeline);

		vk.CmdDispatch(this->command->buffer(),
			this->group_count_x,
			this->group_count_y,
			this->group_count_z);
	}

private:
	uint32_t group_count_x;
	uint32_t group_count_y;
	uint32_t group_count_z;

	std::unique_ptr<VulkanComputePipeline> compute_pipeline;
	std::unique_ptr<VulkanCommandBuffers> command;

	std::shared_ptr<VulkanDescriptorSetLayout> descriptor_set_layout;
	std::shared_ptr<VulkanDescriptorSets> descriptor_sets;
	std::shared_ptr<VulkanPipelineLayout> pipeline_layout;
	std::shared_ptr<VulkanDescriptorPool> descriptor_pool;
};


class DrawCommandBase : public Node {
public:
	IMPLEMENT_VISITABLE;
	DrawCommandBase() = delete;
	virtual ~DrawCommandBase() = default;

	explicit DrawCommandBase(VkPrimitiveTopology topology) :
		topology(topology)
	{}

	void alloc(Visitor* context)
	{
		this->command = std::make_unique<VulkanCommandBuffers>(
			context->state->device,
			1,
			VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	}

	virtual void execute(VkCommandBuffer command, Visitor*) = 0;

	void pipeline(Visitor* context)
	{
		std::vector<VkDescriptorPoolSize> descriptor_pool_sizes;
		std::vector<VkWriteDescriptorSet> write_descriptor_sets;
		std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;

		for (auto& info : context->state->descriptor_set_infos) {
			VkWriteDescriptorSet write_descriptor_set{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = nullptr,
				.dstBinding = info.binding,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = info.descriptorType,
				.pImageInfo = nullptr,
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr,
			};

			if (info.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
				write_descriptor_set.pNext = &info.descriptor_set_acceleration_structure;
			}

			write_descriptor_set.pBufferInfo = &info.descriptor_buffer_info;
			write_descriptor_set.pImageInfo = &info.descriptor_image_info;

			descriptor_pool_sizes.push_back({
				.type = info.descriptorType,
				.descriptorCount = 1,
				});

			descriptor_set_layout_bindings.push_back({
				.binding = info.binding,
				.descriptorType = info.descriptorType,
				.descriptorCount = 1,
				.stageFlags = info.stageFlags,
				.pImmutableSamplers = nullptr,
				});

			write_descriptor_sets.push_back(write_descriptor_set);
		}

		auto descriptor_pool = std::make_shared<VulkanDescriptorPool>(
			context->state->device,
			descriptor_pool_sizes);

		this->descriptor_set_layout = std::make_unique<VulkanDescriptorSetLayout>(
			context->state->device,
			descriptor_set_layout_bindings);

		std::vector<VkDescriptorSetLayout> descriptor_set_layouts{
			this->descriptor_set_layout->layout
		};

		//context->state->pushConstantRanges.push_back({
		//	VK_SHADER_STAGE_FRAGMENT_BIT,
		//	0,
		//	sizeof(glm::vec3) * 1,
		//});

		this->pipeline_layout = std::make_unique<VulkanPipelineLayout>(
			context->state->device,
			descriptor_set_layouts,
			context->state->pushConstantRanges);

		this->descriptor_sets = std::make_unique<VulkanDescriptorSets>(
			context->state->device,
			descriptor_pool,
			descriptor_set_layouts);

		for (auto& write_descriptor_set : write_descriptor_sets) {
			write_descriptor_set.dstSet = this->descriptor_sets->descriptor_sets[0];
		}
		this->descriptor_sets->update(write_descriptor_sets);

		this->graphics_pipeline = std::make_unique<VulkanGraphicsPipeline>(
			context->state->device,
			context->state->renderpass->renderpass,
			context->state->pipelinecache->cache,
			this->pipeline_layout->layout,
			this->topology,
			context->state->rasterization_state,
			this->dynamic_states,
			context->state->shader_stage_infos,
			context->state->vertex_input_bindings,
			context->state->vertex_attributes);
	}

	void record(Visitor* context)
	{
		this->command->begin(
			0,
			context->state->renderpass->renderpass,
			0,
			VK_NULL_HANDLE,
			VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);

		this->descriptor_sets->bind(this->command->buffer(), this->pipeline_layout->layout);
		this->graphics_pipeline->bind(this->command->buffer());

		std::vector<VkRect2D> scissors{ {
			{ 0, 0 },
			VkExtent2D{ context->state->extent.width, context->state->extent.width }
		} };

		vk.CmdSetScissor(this->command->buffer(),
			0,
			static_cast<uint32_t>(scissors.size()),
			scissors.data());

		std::vector<VkViewport> viewports{ {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(context->state->extent.width),
			.height = static_cast<float>(context->state->extent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		} };

		vk.CmdSetViewport(this->command->buffer(),
			0,
			static_cast<uint32_t>(viewports.size()),
			viewports.data());

		vk.CmdBindVertexBuffers(this->command->buffer(),
			0,
			static_cast<uint32_t>(context->state->vertex_attribute_buffers.size()),
			context->state->vertex_attribute_buffers.data(),
			context->state->vertex_attribute_buffer_offsets.data());

		this->execute(this->command->buffer(), context);
		this->command->end();
	}

	void render(Visitor* context)
	{
		//std::vector<glm::vec3> push_constants;
		//push_constants.push_back(glm::vec3(2048, 8192, 1024));

		//vk.CmdPushConstants(
		//	context->state->command->buffer(),
		//	this->pipeline_layout->layout,
		//	VK_SHADER_STAGE_FRAGMENT_BIT,
		//	0,
		//	push_constants.size() * sizeof(glm::vec3),
		//	push_constants.data());

		vk.CmdExecuteCommands(
			context->state->command->buffer(),
			static_cast<uint32_t>(this->command->buffers.size()),
			this->command->buffers.data());
	}

private:
	VkPrimitiveTopology topology;
	std::unique_ptr<VulkanCommandBuffers> command;
	std::unique_ptr<VulkanGraphicsPipeline> graphics_pipeline;
	std::vector<VkDynamicState> dynamic_states{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	std::shared_ptr<VulkanDescriptorSetLayout> descriptor_set_layout;
	std::shared_ptr<VulkanDescriptorSets> descriptor_sets;
	std::shared_ptr<VulkanPipelineLayout> pipeline_layout;
};


class DrawCommand : public DrawCommandBase {
public:
	IMPLEMENT_VISITABLE;
	virtual ~DrawCommand() = default;

	explicit DrawCommand(
		uint32_t vertexcount,
		uint32_t instancecount,
		uint32_t firstvertex,
		uint32_t firstinstance,
		VkPrimitiveTopology topology) :
		DrawCommandBase(topology),
		vertexcount(vertexcount),
		instancecount(instancecount),
		firstvertex(firstvertex),
		firstinstance(firstinstance)
	{
		REGISTER_VISITOR(allocvisitor, DrawCommand, alloc);
		REGISTER_VISITOR(pipelinevisitor, DrawCommand, pipeline);
		REGISTER_VISITOR(recordvisitor, DrawCommand, record);
		REGISTER_VISITOR(rendervisitor, DrawCommand, render);
	}

private:
	void execute(VkCommandBuffer command, Visitor*) override
	{
		vk.CmdDraw(
			command,
			this->vertexcount,
			this->instancecount,
			this->firstvertex,
			this->firstinstance);
	}

	uint32_t vertexcount;
	uint32_t instancecount;
	uint32_t firstvertex;
	uint32_t firstinstance;
};

class IndexedDrawCommand : public DrawCommandBase {
public:
	IMPLEMENT_VISITABLE;
	virtual ~IndexedDrawCommand() = default;

	explicit IndexedDrawCommand(
		uint32_t indexcount,
		uint32_t instancecount,
		uint32_t firstindex,
		int32_t vertexoffset,
		uint32_t firstinstance,
		VkPrimitiveTopology topology) :
		DrawCommandBase(topology),
		indexcount(indexcount),
		instancecount(instancecount),
		firstindex(firstindex),
		vertexoffset(vertexoffset),
		firstinstance(firstinstance),
		offset(0)
	{
		REGISTER_VISITOR(allocvisitor, IndexedDrawCommand, alloc);
		REGISTER_VISITOR(pipelinevisitor, IndexedDrawCommand, pipeline);
		REGISTER_VISITOR(recordvisitor, IndexedDrawCommand, record);
		REGISTER_VISITOR(rendervisitor, IndexedDrawCommand, render);
	}

private:
	void execute(VkCommandBuffer command, Visitor* context) override
	{
		vk.CmdBindIndexBuffer(
			command,
			context->state->index_buffer,
			this->offset,
			context->state->index_buffer_type);

		vk.CmdDrawIndexed(
			command,
			this->indexcount,
			this->instancecount,
			this->firstindex,
			this->vertexoffset,
			this->firstinstance);
	}

	uint32_t indexcount;
	uint32_t instancecount;
	uint32_t firstindex;
	int32_t vertexoffset;
	uint32_t firstinstance;
	VkDeviceSize offset;
};


class FramebufferAttachment : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~FramebufferAttachment() = default;

	explicit FramebufferAttachment(
		VkFormat format,
		VkImageLayout layout,
		VkImageUsageFlags usage,
		VkImageAspectFlags aspectMask) :
		format(format),
		layout(layout),
		usage(usage),
		subresourceRange({ aspectMask, 0, 1, 0, 1 })
	{
		REGISTER_VISITOR(allocvisitor, FramebufferAttachment, alloc);
		REGISTER_VISITOR(resizevisitor, FramebufferAttachment, alloc);
	}

	void alloc(Visitor* context)
	{
		this->image = std::make_shared<VulkanImageObject>(
			context->state->device,
			VK_IMAGE_TYPE_2D,
			this->format,
			context->state->extent,
			1,
			1,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			this->usage,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkComponentMapping componentMapping{
			.r = VK_COMPONENT_SWIZZLE_R,
			.g = VK_COMPONENT_SWIZZLE_G,
			.b = VK_COMPONENT_SWIZZLE_B,
			.a = VK_COMPONENT_SWIZZLE_A,
		};

		this->view = std::make_shared<VulkanImageView>(
			context->state->device,
			this->image->image->image,
			VK_IMAGE_VIEW_TYPE_2D,
			this->format,
			componentMapping,
			this->subresourceRange);

		context->state->default_command->pipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {
			VulkanImage::MemoryBarrier(
				this->image->image->image,
				0,
				0,
				VK_IMAGE_LAYOUT_UNDEFINED,
				this->layout,
				this->subresourceRange) });
	}

public:
	VkFormat format;
	VkImageLayout layout;
	VkImageUsageFlags usage;
	VkImageSubresourceRange subresourceRange;

	std::shared_ptr<VulkanImageObject> image;
	std::shared_ptr<VulkanImageView> view;
};


class RTXbuffer : public Group {
public:
	IMPLEMENT_VISITABLE;
	virtual ~RTXbuffer() = default;

	explicit RTXbuffer(std::vector<std::shared_ptr<Node>> children) :
		Group(std::move(children))
	{
		REGISTER_VISITOR(allocvisitor, RTXbuffer, alloc);
		REGISTER_VISITOR(resizevisitor, RTXbuffer, alloc);
		REGISTER_VISITOR(pipelinevisitor, RTXbuffer, update);
		REGISTER_VISITOR(recordvisitor, RTXbuffer, update);
	}

	void alloc(Visitor* context)
	{
		for (auto child : this->children) {
			child->visit(context);
		}
		this->update(context);
	}

	void update(Visitor* context)
	{
		auto attachment0 = static_pointer_cast<FramebufferAttachment>(this->children[0]);

		context->state->renderTarget = {
			.image = attachment0->image->image->image,
			.format = attachment0->format,
			.layout = attachment0->layout,
			.subresourceRange = attachment0->subresourceRange
		};

		context->state->imageView = attachment0->view->view;
		context->state->imageLayout = attachment0->layout;
	}

	VkExtent3D extent{ 1920, 1080, 1 };
};


class Framebuffer : public Group {
public:
	IMPLEMENT_VISITABLE;
	virtual ~Framebuffer() = default;

	explicit Framebuffer(std::vector<std::shared_ptr<Node>> children) :
		Group(std::move(children))
	{
		REGISTER_VISITOR(allocvisitor, Framebuffer, alloc);
		REGISTER_VISITOR(resizevisitor, Framebuffer, alloc);
		REGISTER_VISITOR(recordvisitor, Framebuffer, update);
	}

	void update(Visitor* context)
	{
		auto attachment0 = static_pointer_cast<FramebufferAttachment>(this->children[0]);

		context->state->renderTarget = {
			.image = attachment0->image->image->image,
			.format = attachment0->format,
			.layout = attachment0->layout,
			.subresourceRange = attachment0->subresourceRange
		};
	}

	void alloc(Visitor* context)
	{
		std::vector<VkImageView> framebuffer_attachments;

		for (auto child : this->children) {
			child->visit(context);
			auto attachment = static_pointer_cast<FramebufferAttachment>(child);
			framebuffer_attachments.push_back(attachment->view->view);
		}

		context->state->framebuffer = std::make_unique<VulkanFramebuffer>(
			context->state->device,
			context->state->renderpass,
			framebuffer_attachments,
			context->state->extent.width,
			context->state->extent.height,
			1);

		this->update(context);
	}
};


class InputAttachment : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~InputAttachment() = default;
	explicit InputAttachment(uint32_t attachment, VkImageLayout layout) :
		attachment({ attachment, layout })
	{
		REGISTER_VISITOR(allocvisitor, InputAttachment, alloc);
	}

	void alloc(Visitor* context)
	{
		context->state->input_attachments.push_back(this->attachment);
	}

	VkAttachmentReference attachment;
};


class ColorAttachment : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~ColorAttachment() = default;
	explicit ColorAttachment(uint32_t attachment, VkImageLayout layout) :
		attachment({ attachment, layout })
	{
		REGISTER_VISITOR(allocvisitor, ColorAttachment, alloc);
	}

	void alloc(Visitor* context)
	{
		context->state->color_attachments.push_back(this->attachment);
	}

private:
	VkAttachmentReference attachment;
};


class ResolveAttachment : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~ResolveAttachment() = default;
	explicit ResolveAttachment(uint32_t attachment, VkImageLayout layout) :
		attachment({ attachment, layout })
	{
		REGISTER_VISITOR(allocvisitor, ResolveAttachment, alloc);
	}

	void alloc(Visitor* context)
	{
		context->state->resolve_attachments.push_back(this->attachment);
	}

private:
	VkAttachmentReference attachment;
};


class DepthStencilAttachment : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~DepthStencilAttachment() = default;
	explicit DepthStencilAttachment(uint32_t attachment, VkImageLayout layout) :
		attachment({ attachment, layout })
	{
		REGISTER_VISITOR(allocvisitor, DepthStencilAttachment, alloc);
	}

	void alloc(Visitor* context)
	{
		context->state->depth_stencil_attachment = this->attachment;
	}

private:
	VkAttachmentReference attachment;
};

class PreserveAttachment : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~PreserveAttachment() = default;
	explicit PreserveAttachment(uint32_t attachment) :
		attachment(attachment)
	{
		REGISTER_VISITOR(allocvisitor, PreserveAttachment, alloc);
	}

	void alloc(Visitor* context)
	{
		context->state->preserve_attachments.push_back(this->attachment);
	}

private:
	uint32_t attachment;
};


class PipelineBindpoint : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~PipelineBindpoint() = default;
	explicit PipelineBindpoint(VkPipelineBindPoint bind_point) :
		bind_point(bind_point)
	{
		REGISTER_VISITOR(allocvisitor, PipelineBindpoint, alloc);
	}

	void alloc(Visitor* context)
	{
		context->state->bind_point = this->bind_point;
	}

private:
	VkPipelineBindPoint bind_point;
};


class SubpassDescription : public Group {
public:
	IMPLEMENT_VISITABLE;
	SubpassDescription() = delete;
	virtual ~SubpassDescription() = default;

	SubpassDescription(std::vector<std::shared_ptr<Node>> children) :
		Group(std::move(children))
	{
		REGISTER_VISITOR(allocvisitor, SubpassDescription, alloc);
	}

	void alloc(Visitor* context)
	{
		Group::visit(context);
		context->state->subpass_descriptions.push_back({
			.flags = 0,
			.pipelineBindPoint = context->state->bind_point,
			.inputAttachmentCount = static_cast<uint32_t>(context->state->input_attachments.size()),
			.pInputAttachments = context->state->input_attachments.data(),
			.colorAttachmentCount = static_cast<uint32_t>(context->state->color_attachments.size()),
			.pColorAttachments = context->state->color_attachments.data(),
			.pResolveAttachments = context->state->resolve_attachments.data(),
			.pDepthStencilAttachment = &context->state->depth_stencil_attachment,
			.preserveAttachmentCount = static_cast<uint32_t>(context->state->preserve_attachments.size()),
			.pPreserveAttachments = context->state->preserve_attachments.data()
			});
	}
};


class RenderpassAttachment : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~RenderpassAttachment() = default;

	RenderpassAttachment(
		VkFormat format,
		VkSampleCountFlagBits samples,
		VkAttachmentLoadOp loadOp,
		VkAttachmentStoreOp storeOp,
		VkAttachmentLoadOp stencilLoadOp,
		VkAttachmentStoreOp stencilStoreOp,
		VkImageLayout initialLayout,
		VkImageLayout finalLayout)
	{
		this->description = {
			.flags = 0,
			.format = format,
			.samples = samples,
			.loadOp = loadOp,
			.storeOp = storeOp,
			.stencilLoadOp = stencilLoadOp,
			.stencilStoreOp = stencilStoreOp,
			.initialLayout = initialLayout,
			.finalLayout = finalLayout
		};

		REGISTER_VISITOR(allocvisitor, RenderpassAttachment, alloc);
	}

	void alloc(Visitor* context)
	{
		context->state->attachment_descriptions.push_back(this->description);
	}

private:
	VkAttachmentDescription description;
};


class Renderpass : public Group {
public:
	IMPLEMENT_VISITABLE;
	virtual ~Renderpass() = default;

	Renderpass(std::vector<std::shared_ptr<Node>> children) :
		Group(std::move(children))
	{
		REGISTER_VISITOR(allocvisitor, Renderpass, alloc);
		REGISTER_VISITOR(resizevisitor, Renderpass, resize);
		REGISTER_VISITOR(rendervisitor, Renderpass, render);
		REGISTER_VISITOR(eventvisitor, Renderpass, visitChildren);
		REGISTER_VISITOR(devicevisitor, Renderpass, visitChildren);
		REGISTER_VISITOR(pipelinevisitor, Renderpass, visitChildren);
		REGISTER_VISITOR(recordvisitor, Renderpass, visitChildren);
	}

	void visitChildren(Visitor* context)
	{
		Group::visit(context);
	}

	void alloc(Visitor* context)
	{
		Group::visit(context);

		this->render_command = std::make_unique<VulkanCommandBuffers>(context->state->device);
		this->render_queue = context->state->device->getQueue(VK_QUEUE_GRAPHICS_BIT);

		this->renderpass = context->state->renderpass;
		this->framebuffer = context->state->framebuffer;
	}

	void resize(Visitor* context)
	{
		Group::visit(context);
		this->framebuffer = context->state->framebuffer;
	}

	void render(Visitor* context)
	{
		const VkRect2D renderarea{
			.offset = { 0, 0 },
			.extent = VkExtent2D{ context->state->extent.width, context->state->extent.height }
		};

		const std::vector<VkClearValue> clearvalues{
			{.color = { 0.0f, 0.0f, 0.0f, 0.0f } },
			{.depthStencil = { 1.0f, 0 } }
		};

		{
			VulkanCommandBuffers::Scope render_command_scope(this->render_command.get());

			VulkanRenderPassScope renderpass_scope(
				this->renderpass->renderpass,
				this->framebuffer->framebuffer,
				renderarea,
				clearvalues,
				this->render_command->buffer());

			context->state->command = this->render_command.get();

			Group::visit(context);
		}

		this->render_command->submit(
			this->render_queue,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
	}

public:
	VkQueue render_queue{ nullptr };
	std::unique_ptr<VulkanCommandBuffers> render_command;
	std::shared_ptr<VulkanRenderpass> renderpass;
	std::shared_ptr<VulkanFramebuffer> framebuffer;
};


class RenderpassDescription : public Group {
public:
	IMPLEMENT_VISITABLE;
	virtual ~RenderpassDescription() = default;
	RenderpassDescription(std::vector<std::shared_ptr<Node>> children) :
		Group(std::move(children))
	{
		REGISTER_VISITOR(allocvisitor, RenderpassDescription, alloc);
		REGISTER_VISITOR(resizevisitor, RenderpassDescription, visitChildren);
		REGISTER_VISITOR(pipelinevisitor, RenderpassDescription, visitChildren);
		REGISTER_VISITOR(recordvisitor, RenderpassDescription, visitChildren);
	}

	void alloc(Visitor* context)
	{
		Group::visit(context);

		this->renderpass = std::make_shared<VulkanRenderpass>(
			context->state->device,
			context->state->attachment_descriptions,
			context->state->subpass_descriptions);

		context->state->renderpass = this->renderpass;
	}

	void visitChildren(Visitor* context)
	{
		Group::visit(context);
		context->state->renderpass = this->renderpass;
	}

public:
	std::shared_ptr<VulkanRenderpass> renderpass;
};


class Swapchain : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~Swapchain() = default;

	Swapchain(std::shared_ptr<VulkanSurface> surface, VkPresentModeKHR present_mode) :
		surface(surface),
		present_mode(present_mode),
		present_queue(nullptr)
	{
		REGISTER_VISITOR(allocvisitor, Swapchain, alloc);
		REGISTER_VISITOR(resizevisitor, Swapchain, resize);
		REGISTER_VISITOR(recordvisitor, Swapchain, record);
		REGISTER_VISITOR(presentvisitor, Swapchain, present);
	}

	void alloc(Visitor* context)
	{
		this->surface->checkPresentModeSupport(context->state->device, this->present_mode);
		this->present_queue = context->state->device->getQueue(0, this->surface->surface);
		this->swapchain_image_ready = std::make_unique<VulkanSemaphore>(context->state->device);
		this->swap_buffers_finished = std::make_unique<VulkanSemaphore>(context->state->device);

		this->resize(context);
	}

	void resize(Visitor* context)
	{
		VkSurfaceFormatKHR surface_format =
			this->surface->getSupportedSurfaceFormat(context->state->device, context->state->renderTarget.format);

		VkSwapchainKHR prevswapchain = (this->swapchain) ? this->swapchain->swapchain : 0;

		this->swapchain = std::make_unique<VulkanSwapchain>(
			context->state->device,
			this->surface->surface,
			3,
			surface_format.format,
			surface_format.colorSpace,
			VkExtent2D{ context->state->extent.width, context->state->extent.height },
			1,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			std::vector<uint32_t>{ 0, },
			VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			this->present_mode,
			VK_FALSE,
			prevswapchain);

		uint32_t count;
		THROW_ON_ERROR(vk.GetSwapchainImagesKHR(
			context->state->device->device,
			this->swapchain->swapchain,
			&count,
			nullptr));

		this->swapchain_images.resize(count);
		THROW_ON_ERROR(vk.GetSwapchainImagesKHR(
			context->state->device->device,
			this->swapchain->swapchain,
			&count,
			this->swapchain_images.data()));

		for (uint32_t i = 0; i < count; i++) {
			context->state->default_command->pipelineBarrier(
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {
					VulkanImage::MemoryBarrier(
						this->swapchain_images[i],
						0,
						0,
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						context->state->renderTarget.subresourceRange) });
		}
	}

	void record(Visitor* context)
	{
		this->swap_buffers_command = std::make_unique<VulkanCommandBuffers>(
			context->state->device,
			this->swapchain_images.size(),
			VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		for (size_t i = 0; i < this->swapchain_images.size(); i++) {
			const VkImageSubresourceLayers subresource_layers{
				.aspectMask = context->state->renderTarget.subresourceRange.aspectMask,
				.mipLevel = context->state->renderTarget.subresourceRange.baseMipLevel,
				.baseArrayLayer = context->state->renderTarget.subresourceRange.baseArrayLayer,
				.layerCount = context->state->renderTarget.subresourceRange.layerCount,
			};

			VkOffset3D offset = {
				.x = 0,
				.y = 0,
				.z = 0
			};

			std::vector<VkImageCopy> regions{ {
				.srcSubresource = subresource_layers,
				.srcOffset = offset,
				.dstSubresource = subresource_layers,
				.dstOffset = offset,
				.extent = {
					.width = context->state->extent.width,
					.height = context->state->extent.height,
					.depth = 1
				}
			} };

			VulkanCommandBuffers::Scope command_scope(this->swap_buffers_command.get(), i);

			VkImage srcImage = context->state->renderTarget.image;
			VkImage dstImage = this->swapchain_images[i];

			this->swap_buffers_command->pipelineBarrier(
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,		// wait until color attachment is written
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,				// no later stages to block, we're done
				{
				  VulkanImage::MemoryBarrier(
					srcImage,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,			// srcAccessMask
					0,												// dstAccessMask 
					context->state->renderTarget.layout,			// oldLayout
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,			// newLayout
					context->state->renderTarget.subresourceRange),
				  VulkanImage::MemoryBarrier(
					dstImage,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					0,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					context->state->renderTarget.subresourceRange)
				}, i);

			vk.CmdCopyImage(this->swap_buffers_command->buffer(i),
				srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				static_cast<uint32_t>(regions.size()), regions.data());

			this->swap_buffers_command->pipelineBarrier(
				VK_PIPELINE_STAGE_TRANSFER_BIT,						// wait until copy is done
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,				// nothing to block
				{
				  VulkanImage::MemoryBarrier(
					srcImage,
					0,												// srcAccessMask
					0,												// dstAccessMask
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					context->state->renderTarget.layout,
					context->state->renderTarget.subresourceRange),
				  VulkanImage::MemoryBarrier(
					dstImage,
					0,												// srcAccessMask
					0,												// dstAccessMask 
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					context->state->renderTarget.subresourceRange)
				}, i);
		}
	}

	void present(Visitor* context)
	{
		THROW_ON_ERROR(vk.AcquireNextImageKHR(
			context->state->device->device,
			this->swapchain->swapchain,
			UINT64_MAX,
			this->swapchain_image_ready->semaphore,
			VK_NULL_HANDLE,
			&this->image_index));

		std::vector<VkSemaphore> wait_semaphores = { this->swapchain_image_ready->semaphore };
		std::vector<VkSemaphore> signal_semaphores = { this->swap_buffers_finished->semaphore };

		this->swap_buffers_command->submit(
			this->present_queue,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			this->image_index,
			VK_NULL_HANDLE,
			wait_semaphores,
			signal_semaphores);

		VkPresentInfoKHR present_info{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size()),
			.pWaitSemaphores = signal_semaphores.data(),
			.swapchainCount = 1,
			.pSwapchains = &this->swapchain->swapchain,
			.pImageIndices = &this->image_index,
			.pResults = nullptr,
		};

		THROW_ON_ERROR(vk.QueuePresentKHR(this->present_queue, &present_info));
	}

private:
	std::shared_ptr<VulkanSurface> surface;
	VkPresentModeKHR present_mode;
	VkQueue present_queue;

	std::unique_ptr<VulkanSwapchain> swapchain;
	std::vector<VkImage> swapchain_images;
	std::unique_ptr<VulkanCommandBuffers> swap_buffers_command;
	std::unique_ptr<VulkanSemaphore> swapchain_image_ready;
	std::unique_ptr<VulkanSemaphore> swap_buffers_finished;

	uint32_t image_index{ 0 };
};


class OffscreenImage : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~OffscreenImage() = default;

	OffscreenImage()
	{
		REGISTER_VISITOR(allocvisitor, OffscreenImage, alloc);
		REGISTER_VISITOR(resizevisitor, OffscreenImage, alloc);
		REGISTER_VISITOR(recordvisitor, OffscreenImage, record);
		REGISTER_VISITOR(rendervisitor, OffscreenImage, render);
	}

	void alloc(Visitor* context)
	{
		this->fence = std::make_unique<VulkanFence>(context->state->device);
		this->get_image_command = std::make_unique<VulkanCommandBuffers>(context->state->device);
		// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkQueueFlagBits.html
		// All commands that are allowed on a queue that supports transfer operations are also allowed on a 
		// queue that supports either graphics or compute operations. Thus, if the capabilities of a queue 
		// family include VK_QUEUE_GRAPHICS_BIT or VK_QUEUE_COMPUTE_BIT, then reporting the 
		// VK_QUEUE_TRANSFER_BIT capability separately for that queue family is optional.
		this->get_image_queue = context->state->device->getQueue(VK_QUEUE_COMPUTE_BIT);

		this->image = std::make_shared<VulkanImageObject>(
			context->state->device,
			VK_IMAGE_TYPE_2D,
			context->state->renderTarget.format,
			context->state->extent,
			this->subresource_range.levelCount,
			this->subresource_range.layerCount,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_LINEAR,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		context->state->default_command->pipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {
			this->image->image->memoryBarrier(
				0,
				0,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_GENERAL,
				this->subresource_range) });

		VkImageSubresource image_subresource{
			VK_IMAGE_ASPECT_COLOR_BIT, 0, 0
		};
		VkSubresourceLayout subresource_layout = this->image->image->getSubresourceLayout(image_subresource);
		this->dataOffset = subresource_layout.offset;
	}

	void record(Visitor* context)
	{
		const VkImageSubresourceLayers subresource_layers{
			.aspectMask = this->subresource_range.aspectMask,
			.mipLevel = this->subresource_range.baseMipLevel,
			.baseArrayLayer = this->subresource_range.baseArrayLayer,
			.layerCount = this->subresource_range.layerCount,
		};

		const VkOffset3D offset = {
			0, 0, 0
		};

		VkImageCopy image_copy{
			.srcSubresource = subresource_layers,
			.srcOffset = offset,
			.dstSubresource = subresource_layers,
			.dstOffset = offset,
			.extent = context->state->extent,
		};

		VulkanCommandBuffers::Scope command_scope(this->get_image_command.get());

		this->get_image_command->pipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,				// don't wait for anything, the color attachment was rendered to in preceding render pass
			VK_PIPELINE_STAGE_TRANSFER_BIT,					// block transfer stage (copy)
			{
				VulkanImage::MemoryBarrier(
					context->state->renderTarget.image,
					0,
					0,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					this->subresource_range),

				VulkanImage::MemoryBarrier(
					this->image->image->image,
					0,
					0,
					VK_IMAGE_LAYOUT_GENERAL,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					this->subresource_range) });

		vk.CmdCopyImage(
			this->get_image_command->buffer(),
			context->state->renderTarget.image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			this->image->image->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &image_copy);

		this->get_image_command->pipelineBarrier(
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			{
				VulkanImage::MemoryBarrier(
					context->state->renderTarget.image,
					0,
					0,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					this->subresource_range),

				VulkanImage::MemoryBarrier(
					this->image->image->image,
					0,
					0,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_GENERAL,
					this->subresource_range) });
	}

	std::set<uint32_t> getTiles(CommandVisitor* context)
	{
		this->fence->reset();
		this->get_image_command->submit(
			this->get_image_queue,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			this->fence->fence);

		this->fence->wait();

		auto data = reinterpret_cast<uint8_t*>(this->image->memory->map(VK_WHOLE_SIZE, 0, 0));
		data += this->dataOffset;

		std::set<uint32_t> tiles;

		for (VkDeviceSize p = 0; p < this->image->memory_requirements.size; p += 4) {
			uint8_t i = data[p + 0];
			uint8_t j = data[p + 1];
			uint8_t k = data[p + 2];
			uint8_t m = data[p + 3];

			if (m > 0 && m < 6) {
				uint32_t key = (i >> 0) << 24 | (j >> 0) << 16 | (k >> 0) << 8 | m - 1;
				if (tiles.find(key) == tiles.end()) {
					tiles.insert(key);
					tiles.insert((i >> 1) << 24 | (j >> 1) << 16 | (k >> 1) << 8 | m + 0);
				}
			}
		}

		this->image->memory->unmap();

		return tiles;
	}

	void render(RenderVisitor* context)
	{
		context->image = this;
	}

private:
	std::unique_ptr<VulkanCommandBuffers> get_image_command;
	VkQueue get_image_queue{ nullptr };
	const VkImageSubresourceRange subresource_range{
		VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
	};
	std::shared_ptr<VulkanImageObject> image;
	std::shared_ptr<VulkanFence> fence{ nullptr };

	VkDeviceSize dataOffset{ 0 };
};


class TextureImage : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~TextureImage() = default;

	explicit TextureImage(
		uint32_t binding,
		VkShaderStageFlags stageFlags,
		VkFilter filter,
		VkSamplerMipmapMode mipmapMode,
		VkSamplerAddressMode addressMode,
		const std::string& filename) :
		binding(binding),
		stageFlags(stageFlags),
		filter(filter),
		mipmapMode(mipmapMode),
		addressMode(addressMode),
		texture(VulkanImageFactory::Create(filename))
	{
		REGISTER_VISITOR(allocvisitor, TextureImage, alloc);
		REGISTER_VISITOR(pipelinevisitor, TextureImage, updateState);
		REGISTER_VISITOR(recordvisitor, TextureImage, updateState);
	}

	void alloc(Visitor* context)
	{
		this->sampler = std::make_unique<VulkanSampler>(
			context->state->device,
			this->filter,
			this->filter,
			this->mipmapMode,
			this->addressMode,
			this->addressMode,
			this->addressMode,
			0.0f,
			VK_FALSE,
			0.0f,
			VK_FALSE,
			VK_COMPARE_OP_NEVER,
			0.0f,
			static_cast<float>(this->texture->levels()),
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			VK_FALSE);

		this->image = std::make_unique<VulkanImageObject>(
			context->state->device,
			this->texture->image_type(),
			this->texture->format(),
			this->texture->extent(0),
			this->texture->levels(),
			this->texture->layers(),
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED);

		this->buffer = std::make_unique<VulkanBufferObject>(
			context->state->device,
			0,
			this->texture->size(),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		MemoryMap memmap(this->buffer->memory.get());
		std::copy(this->texture->data(), this->texture->data() + this->texture->size(), memmap.mem);

		VkComponentMapping componentMapping{
			.r = VK_COMPONENT_SWIZZLE_R,
			.g = VK_COMPONENT_SWIZZLE_G,
			.b = VK_COMPONENT_SWIZZLE_B,
			.a = VK_COMPONENT_SWIZZLE_A,
		};

		VkImageSubresourceRange subresourceRange = this->texture->subresourceRange();

		this->view = std::make_unique<VulkanImageView>(
			context->state->device,
			this->image->image->image,
			this->texture->image_view_type(),
			this->texture->format(),
			componentMapping,
			subresourceRange);

		std::vector<VkBufferImageCopy> regions = this->texture->getRegions();

		context->state->default_command->pipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,					// don't wait for anything
			VK_PIPELINE_STAGE_TRANSFER_BIT,						// but block transfer
			{
				VulkanImage::MemoryBarrier(
					this->image->image->image,
					0,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,					// old layout
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,		// new layout
					subresourceRange)
			});

		vk.CmdCopyBufferToImage(
			context->state->default_command->buffer(),
			this->buffer->buffer->buffer,
			this->image->image->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(regions.size()),
			regions.data());

		context->state->default_command->pipelineBarrier(
			VK_PIPELINE_STAGE_TRANSFER_BIT,						// wait for transfer
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,				// don't block anything
			{
				VulkanImage::MemoryBarrier(
					this->image->image->image,
					0,
					0,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,		// old layout
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,	// new layout
					subresourceRange)
			});

		this->updateState(context);
	}

	void updateState(Visitor* context)
	{
		VkDescriptorImageInfo descriptorImageInfo{
			.sampler = this->sampler->sampler,
			.imageView = this->view->view,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};

		DescriptorSetInfo descriptorSetInfo{
			.binding = this->binding,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.stageFlags = this->stageFlags,
			.descriptor_image_info = descriptorImageInfo,
		};

		context->state->descriptor_set_infos.push_back(descriptorSetInfo);
	}

	uint32_t binding;
	VkShaderStageFlags stageFlags;
	VkFilter filter;
	VkSamplerMipmapMode mipmapMode;
	VkSamplerAddressMode addressMode;

	std::shared_ptr<VulkanTextureImage> texture;
	std::unique_ptr<VulkanSampler> sampler;
	std::unique_ptr<VulkanImageObject> image;
	std::unique_ptr<VulkanBufferObject> buffer;
	std::unique_ptr<VulkanImageView> view;
};


class SharedMemoryPageData {
public:
	SharedMemoryPageData(
		std::shared_ptr<VulkanMemory> imagememory,
		std::shared_ptr<VulkanTextureImage> texture,
		VkExtent3D tileExtent,
		VkDeviceSize tileSize,
		std::shared_ptr<VulkanBufferObject> buffer) :
		imagememory(std::move(imagememory)),
		texture(std::move(texture)),
		tileExtent(tileExtent),
		tileSize(tileSize),
		buffer(std::move(buffer)),
		buffermemory(std::make_shared<MemoryMap>(this->buffer->memory.get()))
	{
		mipOffsets.push_back(0);
		for (uint32_t m = 0; m < this->texture->levels() - 1; m++) {
			mipOffsets.push_back(mipOffsets[m] + this->texture->size(m));
		}
	}

	~SharedMemoryPageData() = default;

	std::shared_ptr<VulkanMemory> imagememory;
	std::shared_ptr<VulkanTextureImage> texture;
	VkExtent3D tileExtent;
	VkDeviceSize tileSize;
	std::shared_ptr<VulkanBufferObject> buffer;
	std::shared_ptr<MemoryMap> buffermemory;
	std::vector<VkDeviceSize> mipOffsets;
};


class MemoryPage {
public:
	MemoryPage(std::shared_ptr<SharedMemoryPageData> self, VkDeviceSize memoryOffset) :
		self(std::move(self)), memoryOffset(memoryOffset)
	{}


	void bind(uint32_t key)
	{
		uint32_t i = key >> 24 & 0xFF;
		uint32_t j = key >> 16 & 0xFF;
		uint32_t k = key >> 8 & 0xFF;
		uint32_t level = key & 0xFF;

		level = std::clamp(level, self->texture->base_level(), self->texture->levels() - 1);
		VkExtent3D extent = self->texture->extent(level);

		VkDeviceSize width = extent.width / self->tileExtent.width;
		VkDeviceSize height = extent.height / self->tileExtent.height;
		VkDeviceSize depth = extent.depth / self->tileExtent.depth;

		i = std::clamp(i, 0u, (uint32_t)width - 1);
		j = std::clamp(j, 0u, (uint32_t)height - 1);
		k = std::clamp(k, 0u, (uint32_t)depth - 1);

		assert(i < width && j < height && k < depth);
		assert(level < self->texture->levels());

		VkDeviceSize bufferOffset = self->mipOffsets[level] + (((k * height) + j) * width + i) * self->tileSize;
		assert(bufferOffset + self->tileSize <= self->texture->size());

		VkOffset3D imageOffset = {
			int32_t(i * self->tileExtent.width),
			int32_t(j * self->tileExtent.height),
			int32_t(k * self->tileExtent.depth)
		};

		VkImageSubresourceRange subresourceRange = self->texture->subresourceRange();

		const VkImageSubresource subresource{
			subresourceRange.aspectMask,
			level,
			0
		};

		this->image_memory_bind = {
			.subresource = subresource,
			.offset = imageOffset,
			.extent = self->tileExtent,
			.memory = self->imagememory->memory,
			.memoryOffset = this->memoryOffset,
			.flags = 0,
		};

		const VkImageSubresourceLayers imageSubresource{
			.aspectMask = subresourceRange.aspectMask,
			.mipLevel = level,
			.baseArrayLayer = subresourceRange.baseArrayLayer,
			.layerCount = subresourceRange.layerCount,
		};

		this->buffer_image_copy = {
			.bufferOffset = this->memoryOffset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = imageSubresource,
			.imageOffset = imageOffset,
			.imageExtent = self->tileExtent,
		};

		std::copy(
			self->texture->const_data() + bufferOffset,
			self->texture->const_data() + bufferOffset + self->tileSize,
			self->buffermemory->mem + this->memoryOffset);

		//VkDeviceSize i = imageOffset.x;
		//VkDeviceSize j = imageOffset.y;
		//VkDeviceSize k = imageOffset.z;

		//VkDeviceSize width = extent.width;
		//VkDeviceSize height = extent.height;

		//VkDeviceSize elementSize = self->texture->element_size();

		//this->buffer_image_copy = {
		//	.bufferOffset = mipOffset + (((k * width) + j) * height + i) * elementSize,
		//	.bufferRowLength = extent.width,
		//	.bufferImageHeight = extent.height,
		//	.imageSubresource = imageSubresource,
		//	.imageOffset = imageOffset,
		//	.imageExtent = self->tileExtent
		//};
	}

	std::shared_ptr<SharedMemoryPageData> self{ nullptr };
	VkDeviceSize memoryOffset{ 0 };
	VkSparseImageMemoryBind image_memory_bind{};
	VkBufferImageCopy buffer_image_copy{};
};


class SparseTextureImage : public Node {
public:
	IMPLEMENT_VISITABLE;
	virtual ~SparseTextureImage() = default;

	explicit SparseTextureImage(
		uint32_t binding,
		VkShaderStageFlags stageFlags,
		VkFilter filter,
		VkSamplerMipmapMode mipmapMode,
		VkSamplerAddressMode addressMode,
		const std::string& filename) :
		binding(binding),
		stageFlags(stageFlags),
		filter(filter),
		mipmapMode(mipmapMode),
		addressMode(addressMode),
		texture(std::make_shared<DebugTextureImageBricked>(filename))
	{
		REGISTER_VISITOR(devicevisitor, SparseTextureImage, device);
		REGISTER_VISITOR(allocvisitor, SparseTextureImage, alloc);
		REGISTER_VISITOR(pipelinevisitor, SparseTextureImage, updateState);
		REGISTER_VISITOR(recordvisitor, SparseTextureImage, updateState);
		REGISTER_VISITOR(rendervisitor, SparseTextureImage, render);
	}

	void device(DeviceVisitor* context)
	{
		context->device_features.sparseBinding = VK_TRUE;
		context->device_features.sparseResidencyImage2D = VK_TRUE;
		context->device_features.sparseResidencyImage3D = VK_TRUE;
	}

	void alloc(Visitor* context)
	{
		this->sampler = std::make_unique<VulkanSampler>(
			context->state->device,
			this->filter,
			this->filter,
			this->mipmapMode,
			this->addressMode,
			this->addressMode,
			this->addressMode,
			0.0f,
			VK_FALSE,
			0.0f,
			VK_FALSE,
			VK_COMPARE_OP_NEVER,
			0.0f,
			static_cast<float>(this->texture->levels()),
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			VK_FALSE);

		this->image = std::make_unique<VulkanImage>(
			context->state->device,
			this->texture->image_type(),
			this->texture->format(),
			this->texture->extent(0),
			this->texture->levels(),
			this->texture->layers(),
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED);

		VkComponentMapping componentMapping{
			.r = VK_COMPONENT_SWIZZLE_R,
			.g = VK_COMPONENT_SWIZZLE_G,
			.b = VK_COMPONENT_SWIZZLE_B,
			.a = VK_COMPONENT_SWIZZLE_A,
		};

		VkImageSubresourceRange subresourceRange = this->texture->subresourceRange();

		this->view = std::make_unique<VulkanImageView>(
			context->state->device,
			this->image->image,
			this->texture->image_view_type(),
			this->texture->format(),
			componentMapping,
			subresourceRange);

		context->state->default_command->pipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {
			this->image->memoryBarrier(
				0,
				0,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				subresourceRange) });

		this->updateState(context);

		this->bind_sparse_finished = std::make_unique<VulkanSemaphore>(context->state->device);
		this->copy_sparse_finished = std::make_unique<VulkanSemaphore>(context->state->device);
		this->copy_sparse_command = std::make_unique<VulkanCommandBuffers>(context->state->device);
		this->copy_sparse_queue = context->state->device->getQueue(VK_QUEUE_SPARSE_BINDING_BIT);

		VkMemoryRequirements memory_requirements = this->image->getMemoryRequirements();

		uint32_t memory_type_index = context->state->device->physical_device.getMemoryTypeIndex(
			memory_requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkSparseImageMemoryRequirements sparse_memory_requirement =
			this->image->getSparseMemoryRequirements(subresourceRange.aspectMask);

		VkDeviceSize pageSize = memory_requirements.alignment;

		this->buffer = std::make_shared<VulkanBufferObject>(
			context->state->device,
			0,
			this->numTiles * pageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		auto image_memory = std::make_shared<VulkanMemory>(
			context->state->device,
			this->numTiles * pageSize,
			memory_type_index);

		auto shared_data = std::make_shared<SharedMemoryPageData>(
			image_memory,
			this->texture,
			sparse_memory_requirement.formatProperties.imageGranularity,
			pageSize,
			this->buffer);

		for (uint32_t i = 0; i < this->numTiles; i++) {
			this->free_pages.push_back(std::make_shared<MemoryPage>(shared_data, i * pageSize));
		}
	}

	void updateState(Visitor* context)
	{
		VkDescriptorImageInfo descriptorImageInfo{
			.sampler = this->sampler->sampler,
			.imageView = this->view->view,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};

		DescriptorSetInfo descriptorSetInfo{
			.binding = this->binding,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.stageFlags = this->stageFlags,
			.descriptor_image_info = descriptorImageInfo,
		};

		context->state->descriptor_set_infos.push_back(descriptorSetInfo);
	}

	void render(RenderVisitor* context)
	{
		Timer timer("getTiles");
		std::set<uint32_t> tiles = context->image->getTiles(context);

		std::deque<uint32_t> reusable_keys;
		for (auto [key, page] : this->used_pages) {
			if (!tiles.contains(key)) {
				reusable_keys.push_back(key);
			}
		}

		auto get_page = [&]() {
			if (this->free_pages.empty()) {
				auto key = reusable_keys.front();
				reusable_keys.pop_front();
				auto page = this->used_pages.at(key);
				this->used_pages.erase(key);
				return page;
			}
			else {
				auto page = this->free_pages.front();
				this->free_pages.pop_front();
				return page;
			}
		};

		std::vector<VkBufferImageCopy> regions;
		std::vector<VkSparseImageMemoryBind> image_memory_binds;

		for (auto key : tiles) {
			if (!this->used_pages.contains(key)) {
				auto page = get_page();

				page->bind(key);
				this->used_pages[key] = page;

				regions.push_back(page->buffer_image_copy);
				image_memory_binds.push_back(page->image_memory_bind);
			}
		}

		std::cout << "tile count: " << tiles.size();
		std::cout << " used count: " << this->used_pages.size();
		std::cout << " bind count: " << image_memory_binds.size() << std::endl;

		if (image_memory_binds.empty()) {
			return;
		}

		std::vector<VkSparseImageMemoryBindInfo> image_memory_bind_info
		{ {
			this->image->image,
			static_cast<uint32_t>(image_memory_binds.size()),
			image_memory_binds.data(),
		} };

		std::vector<VkSparseImageOpaqueMemoryBindInfo> image_opaque_memory_bind_infos{};

		std::vector<VkSemaphore> bind_sparse_wait_semaphores{};
		std::vector<VkSemaphore> bind_sparse_signal_semaphores{ this->bind_sparse_finished->semaphore };
		std::vector<VkSparseBufferMemoryBindInfo> buffer_memory_bind_infos;

		VkBindSparseInfo bind_sparse_info = {
			.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<uint32_t>(bind_sparse_wait_semaphores.size()),
			.pWaitSemaphores = bind_sparse_wait_semaphores.data(),
			.bufferBindCount = static_cast<uint32_t>(buffer_memory_bind_infos.size()),
			.pBufferBinds = buffer_memory_bind_infos.data(),
			.imageOpaqueBindCount = static_cast<uint32_t>(image_opaque_memory_bind_infos.size()),
			.pImageOpaqueBinds = image_opaque_memory_bind_infos.data(),
			.imageBindCount = static_cast<uint32_t>(image_memory_bind_info.size()),
			.pImageBinds = image_memory_bind_info.data(),
			.signalSemaphoreCount = static_cast<uint32_t>(bind_sparse_signal_semaphores.size()),
			.pSignalSemaphores = bind_sparse_signal_semaphores.data(),
		};

		vk.QueueBindSparse(this->copy_sparse_queue, 1, &bind_sparse_info, VK_NULL_HANDLE);

		VkImageSubresourceRange subresourceRange = this->texture->subresourceRange();

		this->copy_sparse_command->begin();

		this->copy_sparse_command->pipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			{
			  VulkanImage::MemoryBarrier(
				this->image->image,
				0,
				0,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange)
			});

		vk.CmdCopyBufferToImage(
			this->copy_sparse_command->buffer(),
			this->buffer->buffer->buffer,
			this->image->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(regions.size()),
			regions.data());

		this->copy_sparse_command->pipelineBarrier(
			VK_PIPELINE_STAGE_TRANSFER_BIT,							// wait for transfer operation (copy)
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,					// block fragment shader where texture is sampled
			{
			  VulkanImage::MemoryBarrier(
				this->image->image,
				0,
				0,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				subresourceRange)
			});

		this->copy_sparse_command->end();

		std::vector<VkSemaphore> copy_sparse_signal_semaphores{};

		this->copy_sparse_command->submit(
			this->copy_sparse_queue,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_NULL_HANDLE,
			bind_sparse_signal_semaphores,			// wait semaphores
			copy_sparse_signal_semaphores);			// signal semaphores
	}


	uint32_t binding;
	VkShaderStageFlags stageFlags;
	VkFilter filter;
	VkSamplerMipmapMode mipmapMode;
	VkSamplerAddressMode addressMode;

	uint32_t numTiles{ 2048 }; // 128 mb cache size
	std::shared_ptr<VulkanTextureImage> texture;
	std::unique_ptr<VulkanSampler> sampler;
	std::unique_ptr<VulkanImage> image;
	std::shared_ptr<VulkanBufferObject> buffer;
	std::unique_ptr<VulkanImageView> view;


	std::map<uint32_t, std::shared_ptr<MemoryPage>> used_pages;
	std::deque<std::shared_ptr<MemoryPage>> free_pages;

	std::unique_ptr<VulkanSemaphore> bind_sparse_finished;
	std::unique_ptr<VulkanSemaphore> copy_sparse_finished;
	std::unique_ptr<VulkanCommandBuffers> copy_sparse_command;
	VkQueue copy_sparse_queue{ nullptr };
};
