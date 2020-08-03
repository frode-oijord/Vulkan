#pragma once

#include <Innovator/Timer.h>
#include <Innovator/VulkanSurface.h>
#include <Innovator/Wrapper.h>
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
#include <filesystem>
namespace fs = std::filesystem;


class Node {
public:
	Node() = default;
	virtual ~Node() = default;
	virtual void visit(Visitor* visitor) = 0;
};


class Group : public Node {
public:
	NO_COPY_OR_ASSIGNMENT(Group)
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
	NO_COPY_OR_ASSIGNMENT(Separator)
	Separator() = default;
	virtual ~Separator() = default;

	explicit Separator(std::vector<std::shared_ptr<Node>> children) :
		Group(std::move(children))
	{}

	void visit(Visitor* visitor) override
	{
		StateScope scope(&visitor->state);
		Group::visit(visitor);
	}
};


class ProjMatrix : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(ProjMatrix)
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
			static_cast<double>(context->state.extent.width) /
			static_cast<double>(context->state.extent.height);

		this->mat = glm::perspective(
			this->fieldofview,
			this->aspectratio,
			this->nearplane,
			this->farplane);
	}

	void render(CommandVisitor* context)
	{
		context->state.ProjectionMatrix = this->mat;
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
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(ViewMatrix)
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
		context->state.ViewMatrix = glm::dmat4(glm::transpose(this->rot));
		context->state.ViewMatrix = glm::translate(context->state.ViewMatrix, -this->eye);
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
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(ModelMatrix)
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
		context->state.ModelMatrix *= this->mat;
	}

	glm::dmat4 mat{ 1.0 };
};


class TextureMatrix : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(TextureMatrix)
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
		context->state.TextureMatrix *= this->mat;
	}

	glm::dmat4 mat{ 1.0 };
};


class BufferData : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(BufferData)
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
		context->state.bufferdata = this;
	}
};


template <typename T>
class InlineBufferData : public BufferData {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(InlineBufferData)
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


class TextureImage : public BufferData {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(TextureImage)
	TextureImage() = delete;
	virtual ~TextureImage() = default;

	explicit TextureImage(const std::string& filename) :
		texture(VulkanImageFactory::Create(filename))
	{
		REGISTER_VISITOR(allocvisitor, TextureImage, update);
		REGISTER_VISITOR(pipelinevisitor, TextureImage, update);
		REGISTER_VISITOR(recordvisitor, TextureImage, update);
		REGISTER_VISITOR(rendervisitor, TextureImage, update);
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
		context->state.bufferdata = this;
		context->state.texture = this->texture.get();
	}

	void update(CommandVisitor* context)
	{
		context->state.bufferdata = this;
		context->state.texture = this->texture.get();
	}

private:
	std::shared_ptr<VulkanTextureImage> texture;
};


class CpuMemoryBuffer : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(CpuMemoryBuffer)
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
			context->vulkan,
			context->device,
			this->create_flags,
			context->state.bufferdata->size(),
			this->usage_flags,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		MemoryMap memmap(this->bufferobject->memory.get());
		context->state.bufferdata->copy(memmap.mem);

		context->state.buffer = this->bufferobject->buffer->buffer;
	}

	void update(Visitor* context)
	{
		context->state.buffer = this->bufferobject->buffer->buffer;
	}

private:
	VkBufferUsageFlags usage_flags;
	VkBufferCreateFlags create_flags;
	std::shared_ptr<VulkanBufferObject> bufferobject;
};


class GpuMemoryBuffer : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(GpuMemoryBuffer)
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
			context->vulkan,
			context->device,
			this->create_flags,
			context->state.bufferdata->size(),
			this->usage_flags,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		std::vector<VkBufferCopy> regions = { {
			.srcOffset = 0,
			.dstOffset = 0,
			.size = context->state.bufferdata->size(),
		} };

		vk.CmdCopyBuffer(
			context->command->buffer(),
			context->state.buffer,
			this->bufferobject->buffer->buffer,
			static_cast<uint32_t>(regions.size()),
			regions.data());
	}

	void update(Visitor* context)
	{
		context->state.buffer = this->bufferobject->buffer->buffer;
	}

private:
	VkBufferUsageFlags usage_flags;
	VkBufferCreateFlags create_flags;
	std::shared_ptr<VulkanBufferObject> bufferobject;
};


class TransformBuffer : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(TransformBuffer)
	virtual ~TransformBuffer() = default;

	TransformBuffer()
	{
		REGISTER_VISITOR(allocvisitor, TransformBuffer, alloc);
		REGISTER_VISITOR(pipelinevisitor, TransformBuffer, pipeline);
		REGISTER_VISITOR(rendervisitor, TransformBuffer, render);
	}

	void alloc(CommandVisitor* context)
	{
		this->buffer = std::make_shared<VulkanBufferObject>(
			context->vulkan,
			context->device,
			0,
			sizeof(glm::mat4) * 3,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}

	void pipeline(Visitor* context)
	{
		context->state.buffer = this->buffer->buffer->buffer;
	}

	void render(CommandVisitor* context)
	{
		std::array<glm::mat4, 3> data = {
		  glm::mat4(context->state.ViewMatrix * context->state.ModelMatrix),
		  glm::mat4(context->state.ProjectionMatrix),
		  glm::mat4(context->state.TextureMatrix)
		};

		MemoryMap map(this->buffer->memory.get());
		std::copy(data.begin(), data.end(), reinterpret_cast<glm::mat4*>(map.mem));
	}

private:
	std::shared_ptr<VulkanBufferObject> buffer;
};

class IndexBufferDescription : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(IndexBufferDescription)
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
		context->state.index_buffer = context->state.buffer;
		context->state.index_buffer_type = this->type;
		context->state.index_count = context->state.bufferdata->count();
	}

private:
	VkIndexType type;
};


class VertexInputAttributeDescription : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(VertexInputAttributeDescription)
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
		context->state.vertex_attributes.push_back(this->vertex_input_attribute_description);
		context->state.vertex_attribute_buffers.push_back(context->state.buffer);
		context->state.vertex_attribute_buffer_offsets.push_back(0);
		context->state.vertex_counts.push_back(context->state.bufferdata->count());
	}

private:
	VkVertexInputAttributeDescription vertex_input_attribute_description;
};


class VertexInputBindingDescription : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(VertexInputBindingDescription)
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
		context->state.vertex_input_bindings.push_back({
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
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(DescriptorSetLayoutBinding)
	DescriptorSetLayoutBinding() = delete;
	virtual ~DescriptorSetLayoutBinding() = default;

	DescriptorSetLayoutBinding(
		uint32_t binding,
		VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags) :
		binding(binding),
		descriptorType(descriptorType),
		stageFlags(stageFlags)
	{
		REGISTER_VISITOR(pipelinevisitor, DescriptorSetLayoutBinding, pipeline);
	}

	void pipeline(Visitor* context)
	{
		context->state.descriptor_pool_sizes.push_back({
		  .type = this->descriptorType,
		  .descriptorCount = 1,
		});

		context->state.descriptor_set_layout_bindings.push_back({
		  .binding = this->binding,
		  .descriptorType = this->descriptorType,
		  .descriptorCount = 1,
		  .stageFlags = this->stageFlags,
		  .pImmutableSamplers = nullptr,
		});

		this->descriptor_image_info = {
		  .sampler = context->state.sampler,
		  .imageView = context->state.imageView,
		  .imageLayout = context->state.imageLayout
		};

		this->descriptor_buffer_info = {
		  .buffer = context->state.buffer,
		  .offset = 0,
		  .range = VK_WHOLE_SIZE
		};

		VkWriteDescriptorSet set{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = 0,
			.dstSet = 0,
			.dstBinding = this->binding,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = this->descriptorType,
			.pImageInfo = &this->descriptor_image_info,
			.pBufferInfo = &this->descriptor_buffer_info,
			.pTexelBufferView = 0,
		};
		context->state.write_descriptor_sets.push_back(set);
	}

private:
	uint32_t binding;
	VkDescriptorType descriptorType;
	VkShaderStageFlags stageFlags;
	VkDescriptorImageInfo descriptor_image_info{};
	VkDescriptorBufferInfo descriptor_buffer_info{};
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
class Shader : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(Shader)
	Shader() = delete;
	virtual ~Shader() = default;

	explicit Shader(const VkShaderStageFlagBits stage, std::string glsl) :
		stage(stage)
	{
		REGISTER_VISITOR(devicevisitor, Shader, device);
		REGISTER_VISITOR(allocvisitor, Shader, alloc);
		REGISTER_VISITOR(pipelinevisitor, Shader, pipeline);

		//std::ifstream input(filename, std::ios::in);
		//std::string glsl(std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{});

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
		shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(glsl, kind, "", options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
			throw std::runtime_error(module.GetErrorMessage());
		}
		this->spv = { module.cbegin(), module.cend() };
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
		this->shader = std::make_unique<VulkanShaderModule>(context->device, this->spv);
	}

	void pipeline(Visitor* context)
	{
		context->state.shader_stage_infos.push_back({
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = this->stage,
			.module = this->shader->module,
			.pName = "main",
			.pSpecializationInfo = nullptr,
		});
	}

public:
	std::vector<uint32_t> spv;
	VkShaderStageFlagBits stage;
	std::unique_ptr<VulkanShaderModule> shader;
};
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR

class AccelerationStructure : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(AccelerationStructure)
	virtual ~AccelerationStructure() = default;

	explicit AccelerationStructure()
	{
		REGISTER_VISITOR(devicevisitor, AccelerationStructure, device);
		REGISTER_VISITOR(allocvisitor, AccelerationStructure, alloc);
	}

	void device(DeviceVisitor* visitor)
	{
		visitor->instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		visitor->device_address_features.bufferDeviceAddress = VK_TRUE;
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
		VkBufferDeviceAddressInfo index_buffer_address_info{
			VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			nullptr,
			context->state.index_buffer
		};
		VkDeviceAddress index_address = context->vulkan->vkGetBufferDeviceAddressKHR(context->device->device, &index_buffer_address_info);

		VkIndexType indexType = context->state.index_buffer_type;
		uint32_t maxPrimitiveCount = context->state.index_count / 3;

		std::vector<VkAccelerationStructureGeometryKHR> geometries;
		std::vector<VkAccelerationStructureBuildOffsetInfoKHR> build_offset_infos;
		std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> create_geometry_infos;

		for (size_t i = 0; i < context->state.vertex_attributes.size(); i++)
		{
			VkFormat vertexFormat = context->state.vertex_attributes[i].format;
			uint32_t maxVertexCount = context->state.vertex_counts[i];

			create_geometry_infos.push_back({
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR,
				.pNext = nullptr,
				.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
				.maxPrimitiveCount = maxPrimitiveCount,
				.indexType = indexType,
				.maxVertexCount = maxVertexCount,
				.vertexFormat = vertexFormat,
				.allowsTransforms = VK_FALSE
			});

			VkBuffer vertex_buffer = context->state.vertex_attribute_buffers[i];

			VkBufferDeviceAddressInfo vertex_buffer_address_info{
				.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
				.pNext = nullptr,
				.buffer = vertex_buffer,
			};

			VkDeviceAddress vertex_address = context->vulkan->vkGetBufferDeviceAddressKHR(context->device->device, &vertex_buffer_address_info);
			VkDeviceSize vertexStride = context->state.vertex_input_bindings[i].stride;

			VkAccelerationStructureGeometryTrianglesDataKHR geometry_triangles_data{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
				.pNext = nullptr,
				.vertexFormat = vertexFormat,
				.vertexData = vertex_address,
				.vertexStride = vertexStride,
				.indexType = indexType,
				.indexData = index_address,
				.transformData = {}
			};

			VkAccelerationStructureGeometryDataKHR geometry_data{};
			geometry_data.triangles = geometry_triangles_data;

			VkAccelerationStructureGeometryKHR geometry{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
				.pNext = nullptr,
				.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
				.geometry = geometry_data,
				.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
			};

			geometries.push_back(geometry);

			VkAccelerationStructureBuildOffsetInfoKHR build_offset_info{
				.primitiveCount = maxPrimitiveCount,
				.primitiveOffset = 0,
				.firstVertex = 0,
				.transformOffset = 0,
			};

			build_offset_infos.push_back(build_offset_info);
		}

		auto blas = std::make_shared<VulkanAccelerationStructure>(
			context->vulkan,
			context->device,
			0,															// compactedSize
			VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,			// type
			VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,	// flags
			create_geometry_infos,										// geometryInfos
			0);															// deviceAddress

		blas->build(
			context->command->buffer(),
			geometries,
			build_offset_infos);

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

			auto tlas = std::make_shared<VulkanAccelerationStructure>(
				context->vulkan,
				context->device,
				0,															// compactedSize
				VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,				// type
				VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,	// flags
				create_geometry_infos,										// geometryInfos
				0);															// deviceAddress

			VkTransformMatrixKHR transform{ {
				{ 1, 0, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 1, 0 }
			} };

			VkAccelerationStructureDeviceAddressInfoKHR device_address_info{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
				.pNext = nullptr,
				.accelerationStructure = blas->as,
			};

			VkDeviceOrHostAddressConstKHR data;
			data.deviceAddress = context->vulkan->vkGetAccelerationStructureDeviceAddressKHR(context->device->device, &device_address_info);

			VkAccelerationStructureInstanceKHR instance{
				.transform = transform,
				.instanceCustomIndex = 0,
				.mask = 1,
				.instanceShaderBindingTableRecordOffset = 0,
				.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
				.accelerationStructureReference = data.deviceAddress,
			};

			VkAccelerationStructureGeometryInstancesDataKHR geometry_instances_data{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
				.pNext = nullptr,
				.arrayOfPointers = VK_FALSE,
				.data = data,
			};

			VkAccelerationStructureGeometryDataKHR geometry_data{};
			geometry_data.instances = geometry_instances_data;

			std::vector<VkAccelerationStructureGeometryKHR> geometries{ {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
				.pNext = nullptr,
				.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
				.geometry = geometry_data,
				.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
			} };

			std::vector<VkAccelerationStructureBuildOffsetInfoKHR> build_offset_infos{ {
				.primitiveCount = 1,
				.primitiveOffset = 0,
				.firstVertex = 0,
				.transformOffset = 0,
			} };

			tlas->build(
				context->command->buffer(),
				geometries,
				build_offset_infos);
		}
	}
};
#endif

class Sampler : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(Sampler)
	virtual ~Sampler() = default;

	Sampler(
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
			magFilter(magFilter),
			minFilter(minFilter),
			mipmapMode(mipmapMode),
			addressModeU(addressModeU),
			addressModeV(addressModeV),
			addressModeW(addressModeW),
			mipLodBias(mipLodBias),
			anisotropyEnable(anisotropyEnable),
			maxAnisotropy(maxAnisotropy),
			compareEnable(compareEnable),
			compareOp(compareOp),
			minLod(minLod),
			maxLod(maxLod),
			borderColor(borderColor),
			unnormalizedCoordinates(unnormalizedCoordinates)
	{
		REGISTER_VISITOR(allocvisitor, Sampler, alloc);
		REGISTER_VISITOR(pipelinevisitor, Sampler, pipeline);
	}

	void alloc(CommandVisitor* context)
	{
		this->sampler = std::make_unique<VulkanSampler>(
			context->device,
			this->magFilter,
			this->minFilter,
			this->mipmapMode,
			this->addressModeU,
			this->addressModeV,
			this->addressModeW,
			this->mipLodBias,
			this->anisotropyEnable,
			this->maxAnisotropy,
			this->compareEnable,
			this->compareOp,
			this->minLod,
			this->maxLod,
			this->borderColor,
			this->unnormalizedCoordinates);
	}

	void pipeline(Visitor* context)
	{
		context->state.sampler = this->sampler->sampler;
	}

private:
	std::unique_ptr<VulkanSampler> sampler;
	VkFilter magFilter;
	VkFilter minFilter;
	VkSamplerMipmapMode mipmapMode;
	VkSamplerAddressMode addressModeU;
	VkSamplerAddressMode addressModeV;
	VkSamplerAddressMode addressModeW;
	float mipLodBias;
	VkBool32 anisotropyEnable;
	float maxAnisotropy;
	VkBool32 compareEnable;
	VkCompareOp compareOp;
	float minLod;
	float maxLod;
	VkBorderColor borderColor;
	VkBool32 unnormalizedCoordinates;
};


class Image : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(Image)
	virtual ~Image() = default;
	Image(
		VkSampleCountFlagBits sample_count,
		VkImageTiling tiling,
		VkImageUsageFlags usage_flags,
		VkSharingMode sharing_mode,
		VkImageCreateFlags create_flags,
		VkImageLayout layout) :
			sample_count(sample_count),
			tiling(tiling),
			usage_flags(usage_flags),
			sharing_mode(sharing_mode),
			create_flags(create_flags),
			layout(layout)
	{
		REGISTER_VISITOR(allocvisitor, Image, alloc);
		REGISTER_VISITOR(pipelinevisitor, Image, pipeline);
	}

	void alloc(CommandVisitor* context)
	{
		this->image = std::make_shared<VulkanImageObject>(
			context->device,
			context->state.texture->image_type(),
			context->state.texture->format(),
			context->state.texture->extent(0),
			context->state.texture->levels(),
			context->state.texture->layers(),
			this->sample_count,
			this->tiling,
			this->usage_flags,
			this->sharing_mode,
			this->create_flags,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VulkanTextureImage* texture = context->state.texture;

		std::vector<VkBufferImageCopy> regions;

		VkDeviceSize bufferOffset = 0;
		for (uint32_t mip_level = 0; mip_level < texture->levels(); mip_level++) {

			const VkImageSubresourceLayers imageSubresource{
				.aspectMask = texture->subresource_range().aspectMask,
				.mipLevel = mip_level,
				.baseArrayLayer = texture->subresource_range().baseArrayLayer,
				.layerCount = texture->subresource_range().layerCount,
			};

			VkBufferImageCopy region{
			  .bufferOffset = bufferOffset,
			  .bufferRowLength = 0,
			  .bufferImageHeight = 0,
			  .imageSubresource = imageSubresource,
			  .imageOffset = { 0, 0, 0 },
			  .imageExtent = texture->extent(mip_level),
			};

			regions.push_back(region);
			bufferOffset += texture->size(mip_level);
		}

		context->command->pipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,				// don't wait for anything
			VK_PIPELINE_STAGE_TRANSFER_BIT,					// but block transfer 
			{
			  VulkanImage::MemoryBarrier(
				this->image->image->image,
				0,
				0,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				context->state.texture->subresource_range())
			});

		vk.CmdCopyBufferToImage(
			context->command->buffer(),
			context->state.buffer,
			this->image->image->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(regions.size()),
			regions.data());

		context->command->pipelineBarrier(
			VK_PIPELINE_STAGE_TRANSFER_BIT,					// wait for transfer
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			// don't block anything
			{
				VulkanImage::MemoryBarrier(
				this->image->image->image,
				0,
				0,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				this->layout,
				context->state.texture->subresource_range())
			});

		context->state.image = this->image->image->image;
	}

	void pipeline(Visitor* context)
	{
		context->state.imageLayout = this->layout;
	}

private:
	std::shared_ptr<VulkanImageObject> image;

	VkSampleCountFlagBits sample_count;
	VkImageTiling tiling;
	VkImageUsageFlags usage_flags;
	VkSharingMode sharing_mode;
	VkImageCreateFlags create_flags;
	VkImageLayout layout;
};


class ImageView : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(ImageView)
	virtual ~ImageView() = default;
	ImageView(
		VkComponentSwizzle r,
		VkComponentSwizzle g,
		VkComponentSwizzle b,
		VkComponentSwizzle a) :
		component_mapping({ r, g, b, a })
	{
		REGISTER_VISITOR(allocvisitor, ImageView, alloc);
		REGISTER_VISITOR(pipelinevisitor, ImageView, pipeline);
	}

	void alloc(CommandVisitor* context)
	{
		this->view = std::make_unique<VulkanImageView>(
			context->device,
			context->state.image,
			context->state.texture->format(),
			context->state.texture->image_view_type(),
			this->component_mapping,
			context->state.texture->subresource_range());
	}

	void pipeline(Visitor* context)
	{
		context->state.imageView = this->view->view;
	}

private:
	std::unique_ptr<VulkanImageView> view;
	VkComponentMapping component_mapping;
};


class Extent : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(Extent)
	Extent() = delete;
	virtual ~Extent() = default;

	Extent(uint32_t width, uint32_t height) :
		extent{ width, height }
	{
		REGISTER_VISITOR(allocvisitor, Extent, update);
		REGISTER_VISITOR(pipelinevisitor, Extent, update);
		REGISTER_VISITOR(recordvisitor, Extent, update);
		REGISTER_VISITOR(resizevisitor, Extent, update);
		REGISTER_VISITOR(rendervisitor, Extent, update);
	}

	void update(Visitor* context)
	{
		context->state.extent = this->extent;
	}

private:
	VkExtent2D extent;
};


class CullMode : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(CullMode)
	CullMode() = delete;
	virtual ~CullMode() = default;

	explicit CullMode(VkCullModeFlags cullmode) :
		cullmode(cullmode)
	{
		REGISTER_VISITOR(pipelinevisitor, CullMode, pipeline);
	}

	void pipeline(Visitor* context)
	{
		context->state.rasterization_state.cullMode = this->cullmode;
	}

private:
	VkCullModeFlags cullmode;
};


class ComputeCommand : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(ComputeCommand)
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

	void pipeline(PipelineVisitor* context)
	{
		this->descriptor_set_layout = std::make_unique<VulkanDescriptorSetLayout>(
			context->device,
			context->state.descriptor_set_layout_bindings);

		this->descriptor_set = std::make_unique<VulkanDescriptorSets>(
			context->device,
			this->descriptor_pool,
			std::vector<VkDescriptorSetLayout>{ this->descriptor_set_layout->layout });

		this->pipeline_layout = std::make_unique<VulkanPipelineLayout>(
			context->device,
			std::vector<VkDescriptorSetLayout>{ this->descriptor_set_layout->layout },
			context->state.push_constant_ranges);

		for (auto& write_descriptor_set : context->state.write_descriptor_sets) {
			write_descriptor_set.dstSet = this->descriptor_set->descriptor_sets[0];
		}
		this->descriptor_set->update(context->state.write_descriptor_sets);

		this->compute_pipeline = std::make_unique<VulkanComputePipeline>(
			context->device,
			context->pipelinecache->cache,
			context->state.shader_stage_infos[0],
			this->pipeline_layout->layout);
	}

	void record(Visitor* context)
	{
		vk.CmdBindDescriptorSets(this->command->buffer(),
			VK_PIPELINE_BIND_POINT_COMPUTE,
			this->pipeline_layout->layout,
			0,
			static_cast<uint32_t>(this->descriptor_set->descriptor_sets.size()),
			this->descriptor_set->descriptor_sets.data(),
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
	std::shared_ptr<VulkanDescriptorSets> descriptor_set;
	std::shared_ptr<VulkanPipelineLayout> pipeline_layout;
	std::shared_ptr<VulkanDescriptorPool> descriptor_pool;
};


class DrawCommandBase : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(DrawCommandBase)
	DrawCommandBase() = delete;
	virtual ~DrawCommandBase() = default;

	explicit DrawCommandBase(VkPrimitiveTopology topology) :
		topology(topology)
	{}

	void alloc(Visitor* context)
	{
		this->command = std::make_unique<VulkanCommandBuffers>(
			context->device,
			1,
			VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	}

	virtual void execute(VkCommandBuffer command, Visitor*) = 0;

	void pipeline(PipelineVisitor* context)
	{
		auto descriptor_pool = std::make_shared<VulkanDescriptorPool>(
			context->device,
			context->state.descriptor_pool_sizes);

		this->descriptor_set_layout = std::make_unique<VulkanDescriptorSetLayout>(
			context->device,
			context->state.descriptor_set_layout_bindings);

		std::vector<VkDescriptorSetLayout> descriptor_set_layouts{
			this->descriptor_set_layout->layout
		};

		this->pipeline_layout = std::make_unique<VulkanPipelineLayout>(
			context->device,
			descriptor_set_layouts,
			context->state.push_constant_ranges);

		this->descriptor_sets = std::make_unique<VulkanDescriptorSets>(
			context->device,
			descriptor_pool,
			descriptor_set_layouts);

		for (auto& write_descriptor_set : context->state.write_descriptor_sets) {
			write_descriptor_set.dstSet = this->descriptor_sets->descriptor_sets[0];
		}
		this->descriptor_sets->update(context->state.write_descriptor_sets);

		this->graphics_pipeline = std::make_unique<VulkanGraphicsPipeline>(
			context->device,
			context->state.renderpass->renderpass,
			context->pipelinecache->cache,
			this->pipeline_layout->layout,
			this->topology,
			context->state.rasterization_state,
			this->dynamic_states,
			context->state.shader_stage_infos,
			context->state.vertex_input_bindings,
			context->state.vertex_attributes);
	}

	void record(Visitor* context)
	{
		this->command->begin(
			0,
			context->state.renderpass->renderpass,
			0,
			VK_NULL_HANDLE,
			VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);

		vk.CmdBindDescriptorSets(this->command->buffer(),
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipeline_layout->layout,
			0,
			static_cast<uint32_t>(this->descriptor_sets->descriptor_sets.size()),
			this->descriptor_sets->descriptor_sets.data(),
			0,
			nullptr);

		vk.CmdBindPipeline(this->command->buffer(),
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->graphics_pipeline->pipeline);

		std::vector<VkRect2D> scissors{ {
			{ 0, 0 },
			context->state.extent
		} };

		std::vector<VkViewport> viewports{ {
		  .x = 0.0f,
		  .y = 0.0f,
		  .width = static_cast<float>(context->state.extent.width),
		  .height = static_cast<float>(context->state.extent.height),
		  .minDepth = 0.0f,
		  .maxDepth = 1.0f
		} };

		vk.CmdSetScissor(this->command->buffer(),
			0,
			static_cast<uint32_t>(scissors.size()),
			scissors.data());

		vk.CmdSetViewport(this->command->buffer(),
			0,
			static_cast<uint32_t>(viewports.size()),
			viewports.data());

		vk.CmdBindVertexBuffers(this->command->buffer(),
			0,
			static_cast<uint32_t>(context->state.vertex_attribute_buffers.size()),
			context->state.vertex_attribute_buffers.data(),
			context->state.vertex_attribute_buffer_offsets.data());

		this->execute(this->command->buffer(), context);
		this->command->end();
	}

	void render(CommandVisitor* context)
	{
		vk.CmdExecuteCommands(
			context->state.command->buffer(),
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
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(DrawCommand)
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
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(IndexedDrawCommand)
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
			context->state.index_buffer,
			this->offset,
			context->state.index_buffer_type);

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
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(FramebufferAttachment)
	virtual ~FramebufferAttachment() = default;

	FramebufferAttachment(
		VkFormat format,
		VkImageUsageFlags usage,
		VkImageAspectFlags aspectMask) :
		format(format),
		usage(usage)
	{
		this->subresource_range = {
			aspectMask, 0, 1, 0, 1
		};

		REGISTER_VISITOR(allocvisitor, FramebufferAttachment, alloc);
		REGISTER_VISITOR(resizevisitor, FramebufferAttachment, alloc);
		REGISTER_VISITOR(recordvisitor, FramebufferAttachment, record);
	}

	void alloc(CommandVisitor* context)
	{
		VkExtent3D extent = {
			context->state.extent.width, context->state.extent.height, 1
		};

		this->image = std::make_shared<VulkanImageObject>(
			context->device,
			VK_IMAGE_TYPE_2D,
			this->format,
			extent,
			this->subresource_range.levelCount,
			this->subresource_range.layerCount,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			this->usage,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		this->imageview = std::make_shared<VulkanImageView>(
			context->device,
			this->image->image->image,
			this->format,
			VK_IMAGE_VIEW_TYPE_2D,
			this->component_mapping,
			this->subresource_range);

		context->state.framebuffer_attachments.push_back(this->imageview->view);

		if (this->usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
			context->state.swapchain_format = this->format;
		}
	}

	void record(Visitor* context)
	{
		if (this->usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
			context->state.swapchain_source = this->image->image->image;
		}
	}

public:
	VkFormat format;
	VkImageUsageFlags usage;
	VkImageSubresourceRange subresource_range;

	VkComponentMapping component_mapping {
		.r = VK_COMPONENT_SWIZZLE_R,
		.g = VK_COMPONENT_SWIZZLE_G,
		.b = VK_COMPONENT_SWIZZLE_B,
		.a = VK_COMPONENT_SWIZZLE_A,
	};

public:
	std::shared_ptr<VulkanImageObject> image;
	std::shared_ptr<VulkanImageView> imageview;
};

class Framebuffer : public Group {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(Framebuffer)
	virtual ~Framebuffer() = default;

	explicit Framebuffer(std::vector<std::shared_ptr<Node>> children) :
		Group(std::move(children))
	{
		REGISTER_VISITOR(allocvisitor, Framebuffer, alloc);
		REGISTER_VISITOR(resizevisitor, Framebuffer, alloc);
		REGISTER_VISITOR(recordvisitor, Framebuffer, record);
	}

	void do_alloc(CommandVisitor* context)
	{
		this->framebuffer = std::make_unique<VulkanFramebuffer>(
			context->device,
			context->state.renderpass,
			context->state.framebuffer_attachments,
			context->state.extent,
			1);
		context->state.framebuffer = this->framebuffer;
	}

	void alloc(CommandVisitor* context)
	{
		Group::visit(context);
		this->do_alloc(context);
	}

	void record(Visitor* context)
	{
		Group::visit(context);
	}

public:
	std::shared_ptr<VulkanFramebuffer> framebuffer;
};


class InputAttachment : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(InputAttachment)
	virtual ~InputAttachment() = default;
	explicit InputAttachment(uint32_t attachment, VkImageLayout layout) :
		attachment({ attachment, layout })
	{
		REGISTER_VISITOR(allocvisitor, InputAttachment, alloc);
	}

	void alloc(CommandVisitor* context)
	{
		context->state.input_attachments.push_back(this->attachment);
	}

	VkAttachmentReference attachment;
};


class ColorAttachment : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(ColorAttachment)
	virtual ~ColorAttachment() = default;
	explicit ColorAttachment(uint32_t attachment, VkImageLayout layout) :
		attachment({ attachment, layout })
	{
		REGISTER_VISITOR(allocvisitor, ColorAttachment, alloc);
	}

	void alloc(CommandVisitor* context)
	{
		context->state.color_attachments.push_back(this->attachment);
	}

private:
	VkAttachmentReference attachment;
};


class ResolveAttachment : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(ResolveAttachment)
	virtual ~ResolveAttachment() = default;
	explicit ResolveAttachment(uint32_t attachment, VkImageLayout layout) :
		attachment({ attachment, layout })
	{
		REGISTER_VISITOR(allocvisitor, ResolveAttachment, alloc);
	}

	void alloc(CommandVisitor* context)
	{
		context->state.resolve_attachments.push_back(this->attachment);
	}

private:
	VkAttachmentReference attachment;
};


class DepthStencilAttachment : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(DepthStencilAttachment)
	virtual ~DepthStencilAttachment() = default;
	explicit DepthStencilAttachment(uint32_t attachment, VkImageLayout layout) :
		attachment({ attachment, layout })
	{
		REGISTER_VISITOR(allocvisitor, DepthStencilAttachment, alloc);
	}

	void alloc(CommandVisitor* context)
	{
		context->state.depth_stencil_attachment = this->attachment;
	}

private:
	VkAttachmentReference attachment;
};

class PreserveAttachment : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(PreserveAttachment)
	virtual ~PreserveAttachment() = default;
	explicit PreserveAttachment(uint32_t attachment) :
		attachment(attachment)
	{
		REGISTER_VISITOR(allocvisitor, PreserveAttachment, alloc);
	}

	void alloc(CommandVisitor* context)
	{
		context->state.preserve_attachments.push_back(this->attachment);
	}

private:
	uint32_t attachment;
};


class PipelineBindpoint : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(PipelineBindpoint)
	virtual ~PipelineBindpoint() = default;
	explicit PipelineBindpoint(VkPipelineBindPoint bind_point) :
		bind_point(bind_point)
	{
		REGISTER_VISITOR(allocvisitor, PipelineBindpoint, alloc);
	}

	void alloc(CommandVisitor* context)
	{
		context->state.bind_point = this->bind_point;
	}

private:
	VkPipelineBindPoint bind_point;
};


class SubpassDescription : public Group {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(SubpassDescription)
	SubpassDescription() = delete;
	virtual ~SubpassDescription() = default;

	SubpassDescription(std::vector<std::shared_ptr<Node>> children) :
		Group(std::move(children))
	{
		REGISTER_VISITOR(allocvisitor, SubpassDescription, alloc);
	}

	void alloc(CommandVisitor* context)
	{
		Group::visit(context);

		context->state.subpass_descriptions.push_back({
			.flags = 0,
			.pipelineBindPoint = context->state.bind_point,
			.inputAttachmentCount = static_cast<uint32_t>(context->state.input_attachments.size()),
			.pInputAttachments = context->state.input_attachments.data(),
			.colorAttachmentCount = static_cast<uint32_t>(context->state.color_attachments.size()),
			.pColorAttachments = context->state.color_attachments.data(),
			.pResolveAttachments = context->state.resolve_attachments.data(),
			.pDepthStencilAttachment = &context->state.depth_stencil_attachment,
			.preserveAttachmentCount = static_cast<uint32_t>(context->state.preserve_attachments.size()),
			.pPreserveAttachments = context->state.preserve_attachments.data()
		});
	}
};


class RenderpassAttachment : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(RenderpassAttachment)
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

	void alloc(CommandVisitor* context)
	{
		context->state.attachment_descriptions.push_back(this->description);
	}

private:
	VkAttachmentDescription description;
};


class Renderpass : public Group {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(Renderpass)
	virtual ~Renderpass() = default;

	Renderpass(std::vector<std::shared_ptr<Node>> children) :
		Group(std::move(children))
	{
		REGISTER_VISITOR(eventvisitor, Renderpass, events);
		REGISTER_VISITOR(devicevisitor, Renderpass, device);
		REGISTER_VISITOR(allocvisitor, Renderpass, alloc);
		REGISTER_VISITOR(resizevisitor, Renderpass, resize);
		REGISTER_VISITOR(rendervisitor, Renderpass, render);
		REGISTER_VISITOR(pipelinevisitor, Renderpass, pipeline);
		REGISTER_VISITOR(recordvisitor, Renderpass, record);
	}

	void events(EventVisitor* context)
	{
		Group::visit(context);
	}

	void device(Visitor* context)
	{
		Group::visit(context);
	}

	void pipeline(Visitor* context)
	{
		Group::visit(context);
	}

	void record(Visitor* context)
	{
		Group::visit(context);
	}

	void alloc(CommandVisitor* context)
	{
		Group::visit(context);

		this->render_command = std::make_unique<VulkanCommandBuffers>(context->device);
		this->render_queue = context->device->getQueue(VK_QUEUE_GRAPHICS_BIT);

		this->renderpass = context->state.renderpass;
		this->framebuffer = context->state.framebuffer;
	}

	void resize(CommandVisitor* context)
	{
		Group::visit(context);
		this->framebuffer = context->state.framebuffer;
	}

	void render(CommandVisitor* context)
	{
		const VkRect2D renderarea{
			{ 0, 0 },						// offset
			context->state.extent			// extent
		};

		const std::vector<VkClearValue> clearvalues{
			{ .color = { 1.0f, 1.0f, 0.0f, 0.0f } },
			{ .depthStencil = { 1.0f, 0 } }
		};

		this->render_command->begin();
		{
			VulkanRenderPassScope renderpass_scope(
				this->renderpass->renderpass,
				this->framebuffer->framebuffer,
				renderarea,
				clearvalues,
				this->render_command->buffer());

			context->state.command = this->render_command.get();

			Group::visit(context);
		}
		this->render_command->end();

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
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(RenderpassDescription)
	virtual ~RenderpassDescription() = default;
	RenderpassDescription(std::vector<std::shared_ptr<Node>> children) :
		Group(std::move(children))
	{
		REGISTER_VISITOR(allocvisitor, RenderpassDescription, alloc);
		REGISTER_VISITOR(resizevisitor, RenderpassDescription, resize);
		REGISTER_VISITOR(pipelinevisitor, RenderpassDescription, pipeline);
		REGISTER_VISITOR(recordvisitor, RenderpassDescription, record);
	}

	void alloc(CommandVisitor* context)
	{
		Group::visit(context);

		this->renderpass = std::make_shared<VulkanRenderpass>(
			context->device,
			context->state.attachment_descriptions,
			context->state.subpass_descriptions);

		context->state.renderpass = this->renderpass;
	}

	void resize(CommandVisitor* context)
	{
		Group::visit(context);
		context->state.renderpass = this->renderpass;
	}

	void pipeline(Visitor* context)
	{
		Group::visit(context);
		context->state.renderpass = this->renderpass;
	}

	void record(Visitor* context)
	{
		Group::visit(context);
		context->state.renderpass = this->renderpass;
	}

public:
	std::shared_ptr<VulkanRenderpass> renderpass;
};


class SwapchainObject : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(SwapchainObject)
	virtual ~SwapchainObject() = default;

	SwapchainObject(
		std::shared_ptr<VulkanSurface> surface,
		VkPresentModeKHR present_mode) :
			surface(std::move(surface)),
			present_mode(present_mode),
			present_queue(nullptr)
	{
		REGISTER_VISITOR(allocvisitor, SwapchainObject, alloc);
		REGISTER_VISITOR(resizevisitor, SwapchainObject, resize);
		REGISTER_VISITOR(recordvisitor, SwapchainObject, record);
		REGISTER_VISITOR(presentvisitor, SwapchainObject, present);
	}

	void alloc(CommandVisitor* context)
	{
		this->surface->checkPresentModeSupport(context->device, this->present_mode);
		this->present_queue = context->device->getQueue(0, this->surface->surface);
		this->swapchain_image_ready = std::make_unique<VulkanSemaphore>(context->device);
		this->swap_buffers_finished = std::make_unique<VulkanSemaphore>(context->device);

		this->resize(context);
	}

	void resize(CommandVisitor* context)
	{
		VkSurfaceFormatKHR surface_format = 
			this->surface->getSupportedSurfaceFormat(context->device, context->state.swapchain_format);

		VkSwapchainKHR prevswapchain = (this->swapchain) ? this->swapchain->swapchain : 0;

		this->swapchain = std::make_unique<VulkanSwapchain>(
			context->device,
			this->surface->surface,
			3,
			surface_format.format,
			surface_format.colorSpace,
			context->state.extent,
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
			context->device->device,
			this->swapchain->swapchain,
			&count,
			nullptr));

		this->swapchain_images.resize(count);
		THROW_ON_ERROR(vk.GetSwapchainImagesKHR(
			context->device->device,
			this->swapchain->swapchain,
			&count,
			this->swapchain_images.data()));

		for (uint32_t i = 0; i < count; i++) {
			context->command->pipelineBarrier(
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {
					VulkanImage::MemoryBarrier(
						this->swapchain_images[i],
						0,
						0,
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						this->subresource_range) });
		}
	}

	void record(Visitor* context)
	{
		this->swap_buffers_command = std::make_unique<VulkanCommandBuffers>(
			context->device,
			this->swapchain_images.size(),
			VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		for (size_t i = 0; i < this->swapchain_images.size(); i++) {
			const VkImageSubresourceLayers subresource_layers{
				this->subresource_range.aspectMask,			// aspectMask
				this->subresource_range.baseMipLevel,		// mipLevel
				this->subresource_range.baseArrayLayer,		// baseArrayLayer
				this->subresource_range.layerCount			// layerCount;
			};

			const VkOffset3D offset = {
				0, 0, 0
			};

			VkExtent3D extent3d = {
				context->state.extent.width, context->state.extent.height, 1
			};

			std::vector<VkImageCopy> regions{ {
				subresource_layers,					// srcSubresource
				offset,								// srcOffset
				subresource_layers,					// dstSubresource
				offset,								// dstOffset
				extent3d							// extent
			} };

			VulkanCommandBuffers::Scope command_scope(this->swap_buffers_command.get(), i);

			VkImage srcImage = context->state.swapchain_source;
			VkImage dstImage = this->swapchain_images[i];

			this->swap_buffers_command->pipelineBarrier(
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,		// wait until color attachment is written
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,				// no later stages to block, we're done
				{
				  VulkanImage::MemoryBarrier(
					srcImage,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,			// srcAccessMask
					0,												// dstAccessMask 
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,		// oldLayout
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,			// newLayout
					this->subresource_range),
				  VulkanImage::MemoryBarrier(
					dstImage,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					0,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					this->subresource_range)
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
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					this->subresource_range),
				  VulkanImage::MemoryBarrier(
					dstImage,
					0,												// srcAccessMask
					0,												// dstAccessMask 
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					this->subresource_range)
				}, i);
		}
	}

	void present(Visitor* context)
	{
		THROW_ON_ERROR(vk.AcquireNextImageKHR(
			context->device->device,
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

	const VkImageSubresourceRange subresource_range{
	  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	  .baseMipLevel = 0,
	  .levelCount = 1,
	  .baseArrayLayer = 0,
	  .layerCount = 1,
	};

	uint32_t image_index{ 0 };
};


class OffscreenImage : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(OffscreenImage)
	virtual ~OffscreenImage() = default;

	OffscreenImage(std::shared_ptr<FramebufferAttachment> color_attachment) :
		color_attachment(std::move(color_attachment))
	{
		REGISTER_VISITOR(allocvisitor, OffscreenImage, alloc);
		REGISTER_VISITOR(resizevisitor, OffscreenImage, alloc);
		REGISTER_VISITOR(recordvisitor, OffscreenImage, record);
		REGISTER_VISITOR(rendervisitor, OffscreenImage, render);
	}

	void alloc(CommandVisitor* context)
	{
		this->fence = std::make_unique<VulkanFence>(context->device);
		this->get_image_command = std::make_unique<VulkanCommandBuffers>(context->device);

		VkExtent3D extent = {
		  context->state.extent.width, context->state.extent.height, 1
		};

		this->image = std::make_shared<VulkanImageObject>(
			context->device,
			VK_IMAGE_TYPE_2D,
			this->color_attachment->format,
			extent,
			this->subresource_range.levelCount,
			this->subresource_range.layerCount,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_LINEAR,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		context->command->pipelineBarrier(
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

		VkExtent3D extent3d = {
		  context->state.extent.width, context->state.extent.height, 1
		};

		VkImageCopy image_copy{
			.srcSubresource = subresource_layers,
			.srcOffset = offset,
			.dstSubresource = subresource_layers,
			.dstOffset = offset,
			.extent = extent3d,
		};

		VulkanCommandBuffers::Scope command_scope(this->get_image_command.get());

		this->get_image_command->pipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,				// don't wait for anything, the color attachment was rendered to in preceding render pass
			VK_PIPELINE_STAGE_TRANSFER_BIT,					// block transfer stage (copy)
			{
			  this->color_attachment->image->image->memoryBarrier(
				0,
				0,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				this->subresource_range),
			  this->image->image->memoryBarrier(
				0,
				0,
				VK_IMAGE_LAYOUT_GENERAL,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				this->subresource_range) });

		vk.CmdCopyImage(
			this->get_image_command->buffer(),
			this->color_attachment->image->image->image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			this->image->image->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &image_copy);

		this->get_image_command->pipelineBarrier(
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			{
			  this->color_attachment->image->image->memoryBarrier(
				0,
				0,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				this->subresource_range),
			  this->image->image->memoryBarrier(
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
			context->transferqueue,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			this->fence->fence);

		this->fence->wait();

		const uint8_t* data = reinterpret_cast<uint8_t*>(this->image->memory->map(VK_WHOLE_SIZE, 0, 0));
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
	std::shared_ptr<FramebufferAttachment> color_attachment;
	std::unique_ptr<VulkanCommandBuffers> get_image_command;
	const VkImageSubresourceRange subresource_range{
		VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
	};
	std::shared_ptr<VulkanImageObject> image;
	std::shared_ptr<VulkanFence> fence{ nullptr };

	VkDeviceSize dataOffset;
};


class MemoryPage {
public:
	MemoryPage(
		VkDeviceMemory memory,
		VulkanTextureImage* texture,
		VkExtent3D imageExtent,
		VkDeviceSize memoryOffset) :
			memory(memory),
			texture(texture),
			imageExtent(imageExtent),
			memoryOffset(memoryOffset)
	{}


	void bind(uint32_t key)
	{
		uint32_t i = key >> 24 & 0xFF;
		uint32_t j = key >> 16 & 0xFF;
		uint32_t k = key >> 8 & 0xFF;
		uint32_t mipLevel = key & 0xFF;

		if (mipLevel >= this->texture->levels()) {
			throw std::runtime_error("invalid mip level");
		}

		VkOffset3D imageOffset = {
		  int32_t(i * this->imageExtent.width),
		  int32_t(j * this->imageExtent.height),
		  int32_t(k * this->imageExtent.depth)
		};

		const VkImageSubresource subresource{
		  this->texture->subresource_range().aspectMask,
		  mipLevel,
		  0
		};

		this->image_memory_bind = {
		  .subresource = subresource,
		  .offset = imageOffset,
		  .extent = this->imageExtent,
		  .memory = this->memory,
		  .memoryOffset = this->memoryOffset,
		  .flags = 0,
		};

		VkDeviceSize mipOffset = 0;
		for (uint32_t m = 0; m < mipLevel; m++) {
			mipOffset += this->texture->size(m);
		}

		VkExtent3D extent = this->texture->extent(mipLevel);
		VkDeviceSize elementSize = this->texture->element_size();

		VkExtent3D brickExtent = this->texture->brick_size();
		VkDeviceSize brickSize =
			static_cast<VkDeviceSize>(brickExtent.width)*
			static_cast<VkDeviceSize>(brickExtent.height)*
			static_cast<VkDeviceSize>(brickExtent.depth);

		const VkImageSubresourceLayers imageSubresource{
			.aspectMask = this->texture->subresource_range().aspectMask,
			.mipLevel = mipLevel,
			.baseArrayLayer = this->texture->subresource_range().baseArrayLayer,
			.layerCount = this->texture->subresource_range().layerCount,
		};

		if (false) {
			VkDeviceSize width = extent.width / brickExtent.width;
			VkDeviceSize height = extent.height / brickExtent.height;

			this->buffer_image_copy = {
				.bufferOffset = mipOffset + (((k * width) + j) * height + i) * brickSize * elementSize,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = imageSubresource,
				.imageOffset = imageOffset,
				.imageExtent = this->imageExtent,
			};
		}
		else {
			VkDeviceSize i = imageOffset.x;
			VkDeviceSize j = imageOffset.y;
			VkDeviceSize k = imageOffset.z;

			VkDeviceSize width = extent.width;
			VkDeviceSize height = extent.height;

			this->buffer_image_copy = {
				.bufferOffset = mipOffset + (((k * width) + j) * height + i) * brickSize * elementSize,
				.bufferRowLength = extent.width,
				.bufferImageHeight = extent.height,
				.imageSubresource = imageSubresource,
				.imageOffset = imageOffset,
				.imageExtent = this->imageExtent
			};
		}
	}

	VkDeviceMemory memory;
	VulkanTextureImage* texture;
	VkExtent3D imageExtent;
	VkDeviceSize memoryOffset;
	VkSparseImageMemoryBind image_memory_bind;
	VkBufferImageCopy buffer_image_copy;
};

class MemoryPageManager {
public:
	MemoryPageManager(CommandVisitor* context, std::shared_ptr<VulkanImage> image) :
		image(std::move(image))
	{
		this->bind_sparse_finished = std::make_unique<VulkanSemaphore>(context->device);
		this->copy_sparse_finished = std::make_unique<VulkanSemaphore>(context->device);
		this->copy_sparse_command = std::make_unique<VulkanCommandBuffers>(context->device);

		VkMemoryRequirements memory_requirements = this->image->getMemoryRequirements();

		uint32_t memory_type_index = context->device->physical_device.getMemoryTypeIndex(
			memory_requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkSparseImageMemoryRequirements sparse_memory_requirement =
			this->image->getSparseMemoryRequirements(context->state.texture->subresource_range().aspectMask);

		uint32_t numTiles = 5000;
		VkDeviceSize pageSize = memory_requirements.alignment;

		this->image_memory = std::make_shared<VulkanMemory>(
			context->device,
			numTiles * pageSize,
			memory_type_index);

		VkExtent3D imageExtent = sparse_memory_requirement.formatProperties.imageGranularity;

		for (uint32_t i = 0; i < numTiles; i++) {
			this->reusable_pages.push_back(
				std::make_shared<MemoryPage>(
					this->image_memory->memory,
					context->state.texture,
					imageExtent,
					i * pageSize));
		}
	}

	void updateBindings(RenderVisitor* context)
	{
		std::set<uint32_t> tiles = context->image->getTiles(context);

		// gather reusable tiles
		for (auto [key, page] : this->used_image_memory_pages) {
			if (tiles.find(key) == tiles.end()) {
				if (this->unique_reusable_pages.find(key) == this->unique_reusable_pages.end()) {
					this->unique_reusable_pages.insert(key);
					this->reusable_pages.push_back(page);
				}
			}
		}

		for (auto key : this->unique_reusable_pages) {
			this->used_image_memory_pages.erase(key);
		}

		std::vector<VkBufferImageCopy> regions;
		std::vector<VkSparseImageMemoryBind> image_memory_binds;

		// bind new tiles or reuse tiles
		for (auto& key : tiles) {
			if (this->used_image_memory_pages.find(key) == this->used_image_memory_pages.end()) {
				auto page = this->reusable_pages.front();
				this->reusable_pages.pop_front();
				this->unique_reusable_pages.erase(key);

				page->bind(key);
				this->used_image_memory_pages[key] = page;
				regions.push_back(page->buffer_image_copy);
				image_memory_binds.push_back(page->image_memory_bind);
			}
		}

		if (image_memory_binds.empty()) {
			return;
		}

		std::cout << "bind count: " << image_memory_binds.size() << std::endl;

		std::vector<VkSparseImageMemoryBindInfo> image_memory_bind_info
		{ {
		  this->image->image,
		  static_cast<uint32_t>(image_memory_binds.size()),
		  image_memory_binds.data(),
		} };

		std::vector<VkSparseImageOpaqueMemoryBindInfo> image_opaque_memory_bind_infos;

		std::vector<VkSemaphore> wait_semaphores;
		std::vector<VkSemaphore> signal_semaphores{ this->bind_sparse_finished->semaphore };
		std::vector<VkSparseBufferMemoryBindInfo> buffer_memory_bind_infos;

		VkBindSparseInfo bind_sparse_info = {
			.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size()),
			.pWaitSemaphores = wait_semaphores.data(),
			.bufferBindCount = static_cast<uint32_t>(buffer_memory_bind_infos.size()),
			.pBufferBinds = buffer_memory_bind_infos.data(),
			.imageOpaqueBindCount = static_cast<uint32_t>(image_opaque_memory_bind_infos.size()),
			.pImageOpaqueBinds = image_opaque_memory_bind_infos.data(),
			.imageBindCount = static_cast<uint32_t>(image_memory_bind_info.size()),
			.pImageBinds = image_memory_bind_info.data(),
			.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size()),
			.pSignalSemaphores = signal_semaphores.data(),
		};

		vk.QueueBindSparse(context->sparsequeue, 1, &bind_sparse_info, VK_NULL_HANDLE);

		if (regions.empty()) {
			context->wait_semaphores.push_back(this->bind_sparse_finished->semaphore);
		}
		else {
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
					context->state.texture->subresource_range())
				});

			vk.CmdCopyBufferToImage(
				this->copy_sparse_command->buffer(),
				context->state.buffer,
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
					context->state.texture->subresource_range())
				});

			this->copy_sparse_command->end();

			std::vector<VkSemaphore> wait_semaphores{ this->bind_sparse_finished->semaphore };
			std::vector<VkSemaphore> signal_semaphores{ this->copy_sparse_finished->semaphore };

			this->copy_sparse_command->submit(
				context->sparsequeue,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_NULL_HANDLE,
				wait_semaphores,
				signal_semaphores);

			context->wait_semaphores.push_back(this->copy_sparse_finished->semaphore);
		}
	}

	std::shared_ptr<VulkanImage> image;
	std::shared_ptr<VulkanMemory> image_memory;
	std::map<uint32_t, std::shared_ptr<MemoryPage>> used_image_memory_pages;
	std::deque<std::shared_ptr<MemoryPage>> reusable_pages;
	std::set<uint32_t> unique_reusable_pages;

	std::unique_ptr<VulkanSemaphore> bind_sparse_finished;
	std::unique_ptr<VulkanSemaphore> copy_sparse_finished;
	std::unique_ptr<VulkanCommandBuffers> copy_sparse_command;
};



class SparseImage : public Node {
public:
	IMPLEMENT_VISITABLE
	NO_COPY_OR_ASSIGNMENT(SparseImage)
	SparseImage() = delete;
	virtual ~SparseImage() = default;
	SparseImage(
		VkSampleCountFlagBits sample_count,
		VkImageTiling tiling,
		VkImageUsageFlags usage_flags,
		VkSharingMode sharing_mode,
		VkImageCreateFlags create_flags,
		VkImageLayout layout) :
			sample_count(sample_count),
			tiling(tiling),
			usage_flags(usage_flags),
			sharing_mode(sharing_mode),
			create_flags(create_flags),
			layout(layout)
	{
		REGISTER_VISITOR(devicevisitor, SparseImage, device);
		REGISTER_VISITOR(allocvisitor, SparseImage, alloc);
		REGISTER_VISITOR(pipelinevisitor, SparseImage, pipeline);
		REGISTER_VISITOR(rendervisitor, SparseImage, render);
		REGISTER_VISITOR(devicevisitor, SparseImage, device);
	}

	void device(DeviceVisitor* context)
	{
		context->device_features.sparseBinding = VK_TRUE;
		context->device_features.sparseResidencyImage2D = VK_TRUE;
		context->device_features.sparseResidencyImage3D = VK_TRUE;
	}

	void alloc(CommandVisitor* context)
	{
		VulkanTextureImage* texture = context->state.texture;

		this->image = std::make_shared<VulkanImage>(
			context->device,
			texture->image_type(),
			texture->format(),
			texture->extent(0),
			texture->levels(),
			texture->layers(),
			this->sample_count,
			this->tiling,
			this->usage_flags,
			this->sharing_mode,
			this->create_flags);

		context->command->pipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {
			this->image->memoryBarrier(
			  0,
			  0,
			  VK_IMAGE_LAYOUT_UNDEFINED,
			  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			  context->state.texture->subresource_range()) });

		this->memory_manager = std::make_unique<MemoryPageManager>(context, this->image);
		context->state.image = this->image->image;
	}

	void pipeline(Visitor* context)
	{
		context->state.imageLayout = this->layout;
	}

	void render(RenderVisitor* context)
	{
		this->memory_manager->updateBindings(context);
	}

private:
	std::shared_ptr<VulkanImage> image;
	std::shared_ptr<VulkanMemory> image_memory;

	VkSampleCountFlagBits sample_count;
	VkImageTiling tiling;
	VkImageUsageFlags usage_flags;
	VkSharingMode sharing_mode;
	VkImageCreateFlags create_flags;
	VkImageLayout layout;

	std::unique_ptr<MemoryPageManager> memory_manager;
};
