#pragma once

#include <Innovator/Context.h>
#include <Innovator/Timer.h>
#include <Innovator/VulkanSurface.h>
#include <Innovator/Wrapper.h>
#include <Innovator/Visitor.h>
#include <Innovator/Defines.h>
#include <Innovator/Factory.h>

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include <queue>
#include <memory>
#include <utility>
#include <vector>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

class Node {
public:
	Node() = default;
	virtual ~Node() = default;
	virtual void visit(Visitor* visitor, Context* context) = 0;
};

class Group : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
	NO_COPY_OR_ASSIGNMENT(Group)
	Group() = default;
	virtual ~Group() = default;

	explicit Group(std::vector<std::shared_ptr<Node>> children)
		: children(std::move(children)) {}

	std::vector<std::shared_ptr<Node>> children;
};

class Separator : public Group {
public:
	IMPLEMENT_VISITABLE_INLINE
	NO_COPY_OR_ASSIGNMENT(Separator)
	Separator() = default;
	virtual ~Separator() = default;

	explicit Separator(std::vector<std::shared_ptr<Node>> children)
		: Group(std::move(children))
	{}
};

class ViewMatrix : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
	NO_COPY_OR_ASSIGNMENT(ViewMatrix)
	ViewMatrix() = delete;
	virtual ~ViewMatrix() = default;

	ViewMatrix(glm::dvec3 eye, glm::dvec3 target, glm::dvec3 up)
		: mat(glm::lookAt(eye, target, up))
	{
		REGISTER_VISITOR_CALLBACK(rendervisitor, ViewMatrix, render);
	}

	ViewMatrix(double m0, double m1, double m2,
						 double m3, double m4, double m5,
						 double m6, double m7, double m8)
		: ViewMatrix(glm::dvec3(m0, m1, m2),
								 glm::dvec3(m3, m4, m5),
								 glm::dvec3(m6, m7, m8))
	{}

	void zoom(double dy)
	{
		this->mat = glm::translate(this->mat, glm::dvec3(0.0, 0.0, dy));
	}

	void pan(const glm::dvec2& dx)
	{
		this->mat = glm::translate(this->mat, glm::dvec3(dx, 0.0));
	}

	void render(Context* context)
	{
		context->state.ViewMatrix = this->mat;
	}

private:
	glm::dmat4 mat{ 1.0 };
};


class ProjMatrix : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(ProjMatrix)
  ProjMatrix() = delete;
  virtual ~ProjMatrix() = default;

  ProjMatrix(double farplane, double nearplane, double aspectratio, double fieldofview)
    : farplane(farplane),
      nearplane(nearplane),
      aspectratio(aspectratio),
      fieldofview(fieldofview)
  {
		REGISTER_VISITOR_CALLBACK(resizevisitor, ProjMatrix, resize);
		REGISTER_VISITOR_CALLBACK(rendervisitor, ProjMatrix, render);
	}

  void resize(Context* context) 
  {
    this->aspectratio = static_cast<double>(context->state.extent.width) /
                        static_cast<double>(context->state.extent.height);

    this->mat = glm::perspective(this->fieldofview,
                                 this->aspectratio,
                                 this->nearplane,
                                 this->farplane);
  }

  void render(Context* context)
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

class ModelMatrix : public Node {
public:
  IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(ModelMatrix)
  ModelMatrix() = default;
  virtual ~ModelMatrix() = default;

  ModelMatrix(const glm::dvec3& t, const glm::dvec3& s)
  {
    REGISTER_VISITOR_CALLBACK(rendervisitor, ModelMatrix, render);

    this->matrix = glm::scale(this->matrix, s);
    this->matrix = glm::translate(this->matrix, t);
  }

  ModelMatrix(double t0, double t1, double t2,
              double s0, double s1, double s2)
    : ModelMatrix(glm::dvec3(t0, t1, t2), glm::dvec3(s0, s1, s2))
  {}

  virtual void render(Context* context)
  {
    context->state.ModelMatrix *= this->matrix;
  }

public:
  glm::dmat4 matrix{ 1.0 };
};


class TextureMatrix : public Node {
public:
  IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(TextureMatrix)
  TextureMatrix() = default;
  virtual ~TextureMatrix() = default;

  TextureMatrix(const glm::dvec3& t, const glm::dvec3& s)
  {
    REGISTER_VISITOR_CALLBACK(rendervisitor, TextureMatrix, render);

    this->matrix = glm::scale(this->matrix, s);
    this->matrix = glm::translate(this->matrix, t);
  }

  TextureMatrix(double t0, double t1, double t2,
                double s0, double s1, double s2)
    : TextureMatrix(glm::dvec3(t0, t1, t2), glm::dvec3(s0, s1, s2))
  {}

  virtual void render(Context* context)
  {
    context->state.TextureMatrix *= this->matrix;
  }

public:
  glm::dmat4 matrix{ 1.0 };
};


class BufferData : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(BufferData)
  BufferData() = default;
  virtual ~BufferData() = default;

  virtual void copy(char * dst) const = 0;
  virtual size_t size() const = 0;
  virtual size_t stride() const = 0;
  size_t count() const 
  {
    return this->size() / this->stride();
  }

	void update(Context* context) 
  {
    context->state.bufferdata = this;
  }
};

template <typename T>
class InlineBufferData : public BufferData {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(InlineBufferData)
  InlineBufferData() = default;
  virtual ~InlineBufferData() = default;

  explicit InlineBufferData(std::vector<T> values) :
    values(std::move(values))
  {
    REGISTER_VISITOR_CALLBACK(allocvisitor, InlineBufferData<T>, update);
    REGISTER_VISITOR_CALLBACK(pipelinevisitor, InlineBufferData<T>, update);
    REGISTER_VISITOR_CALLBACK(recordvisitor, InlineBufferData<T>, update);
	}

  void copy(char * dst) const override
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
  IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(TextureImage)
  TextureImage() = delete;
  virtual ~TextureImage() = default;

  explicit TextureImage(const std::string& filename) :
    texture(VulkanImageFactory::Create(filename))
  {
    REGISTER_VISITOR_CALLBACK(allocvisitor, TextureImage, update);
    REGISTER_VISITOR_CALLBACK(pipelinevisitor, TextureImage, update);
    REGISTER_VISITOR_CALLBACK(recordvisitor, TextureImage, update);
    REGISTER_VISITOR_CALLBACK(rendervisitor, TextureImage, update);
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

  void update(Context* context)
  {
    context->state.bufferdata = this;
    context->state.texture = this->texture.get();
  }

private:
  std::shared_ptr<VulkanTextureImage> texture;
};


class CpuMemoryBuffer : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(CpuMemoryBuffer)
  CpuMemoryBuffer() = delete;
  virtual ~CpuMemoryBuffer() = default;
	
	explicit CpuMemoryBuffer(VkBufferUsageFlags usage_flags,
                           VkBufferCreateFlags create_flags = 0) :
      usage_flags(usage_flags),
      create_flags(create_flags)
  {    
		REGISTER_VISITOR_CALLBACK(allocvisitor, CpuMemoryBuffer, alloc);
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, CpuMemoryBuffer, update);
		REGISTER_VISITOR_CALLBACK(recordvisitor, CpuMemoryBuffer, update);
		REGISTER_VISITOR_CALLBACK(rendervisitor, CpuMemoryBuffer, update);
	}

	void alloc(Context* context)
  {
    this->bufferobject = std::make_shared<VulkanBufferObject>(
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

  void update(Context* context) 
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
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(GpuMemoryBuffer)
  GpuMemoryBuffer() = delete;
  virtual ~GpuMemoryBuffer() = default;

  explicit GpuMemoryBuffer(VkBufferUsageFlags usage_flags, VkBufferCreateFlags create_flags = 0) : 
    usage_flags(usage_flags), 
    create_flags(create_flags)
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, GpuMemoryBuffer, alloc);
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, GpuMemoryBuffer, update);
		REGISTER_VISITOR_CALLBACK(recordvisitor, GpuMemoryBuffer, update);
		REGISTER_VISITOR_CALLBACK(rendervisitor, GpuMemoryBuffer, update);
  }

	void alloc(Context* context)
  {
    this->bufferobject = std::make_shared<VulkanBufferObject>(
      context->device,
      this->create_flags,
      context->state.bufferdata->size(),
      this->usage_flags,
      VK_SHARING_MODE_EXCLUSIVE,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    std::vector<VkBufferCopy> regions = { {
        0,                                                   // srcOffset
        0,                                                   // dstOffset
        context->state.bufferdata->size(),                   // size
    } };

    vkCmdCopyBuffer(context->command->buffer(),
                    context->state.buffer,
                    this->bufferobject->buffer->buffer,
                    static_cast<uint32_t>(regions.size()),
                    regions.data());
  }

  void update(Context* context) 
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
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(TransformBuffer)
  virtual ~TransformBuffer() = default;

  TransformBuffer()
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, TransformBuffer, alloc);
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, TransformBuffer, pipeline);
		REGISTER_VISITOR_CALLBACK(rendervisitor, TransformBuffer, render);
	}

	void alloc(Context* context)
  {
    this->buffer = std::make_shared<VulkanBufferObject>(
      context->device,
      0,
      sizeof(glm::mat4) * 3,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_SHARING_MODE_EXCLUSIVE,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  }

  void pipeline(Context* creator) 
  {
    creator->state.buffer = this->buffer->buffer->buffer;
  }

  void render(Context* context)
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
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(IndexBufferDescription)
  IndexBufferDescription() = delete;
  virtual ~IndexBufferDescription() = default;

  explicit IndexBufferDescription(VkIndexType type) : 
    type(type)
  {
		REGISTER_VISITOR_CALLBACK(recordvisitor, IndexBufferDescription, record);
	}

  void record(Context* context) 
  {
    context->state.index_buffer_description = {
      this->type,
      context->state.buffer
    };
  }

private:
  VkIndexType type;
};

class VertexInputAttributeDescription : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(VertexInputAttributeDescription)
  VertexInputAttributeDescription() = delete;
  virtual ~VertexInputAttributeDescription() = default;

  explicit VertexInputAttributeDescription(uint32_t location,
                                           uint32_t binding,
                                           VkFormat format,
                                           uint32_t offset) :
    vertex_input_attribute_description({ 
      location, binding, format, offset 
    })
  {
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, VertexInputAttributeDescription, pipeline);
		REGISTER_VISITOR_CALLBACK(recordvisitor, VertexInputAttributeDescription, record);
	}

  void pipeline(Context* creator) 
  {
    creator->state.vertex_attributes.push_back(this->vertex_input_attribute_description);
  }

  void record(Context* recorder) 
  {
    recorder->state.vertex_attribute_buffers.push_back(recorder->state.buffer);
    recorder->state.vertex_attribute_buffer_offsets.push_back(0);
  }

private:
  VkVertexInputAttributeDescription vertex_input_attribute_description;
};

class VertexInputBindingDescription : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(VertexInputBindingDescription)
  VertexInputBindingDescription() = delete;
  virtual ~VertexInputBindingDescription() = default;

  explicit VertexInputBindingDescription(uint32_t binding,
                                         uint32_t stride,
                                         VkVertexInputRate inputRate) :
    binding(binding),
    stride(stride),
    inputRate(inputRate)
  {
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, VertexInputBindingDescription, pipeline);
	}

  void pipeline(Context* creator) 
  {
    creator->state.vertex_input_bindings.push_back({
      this->binding,
      this->stride,
      this->inputRate,
    });
  }
  
private:
  uint32_t binding;
  uint32_t stride;
  VkVertexInputRate inputRate;
};

class DescriptorSetLayoutBinding : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(DescriptorSetLayoutBinding)
  DescriptorSetLayoutBinding() = delete;
  virtual ~DescriptorSetLayoutBinding() = default;

  DescriptorSetLayoutBinding(uint32_t binding, 
                             VkDescriptorType descriptorType, 
                             VkShaderStageFlags stageFlags) :
    binding(binding),
    descriptorType(descriptorType),
    stageFlags(stageFlags)
  {
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, DescriptorSetLayoutBinding, pipeline);
	}

  void pipeline(Context* creator) 
  {
    creator->state.descriptor_pool_sizes.push_back({
      this->descriptorType,                                       // type 
      1,                                                          // descriptorCount
    });

    creator->state.descriptor_set_layout_bindings.push_back({
      this->binding,
      this->descriptorType,
      1,
      this->stageFlags,
      nullptr,
    });

    this->descriptor_image_info = {
      creator->state.sampler,
      creator->state.imageView,
      creator->state.imageLayout
    };

    this->descriptor_buffer_info = {
      creator->state.buffer,                                      // buffer
      0,                                                          // offset
      VK_WHOLE_SIZE                                               // range
    };
   
    creator->state.write_descriptor_sets.push_back({
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                      // sType
      nullptr,                                                     // pNext
      nullptr,                                                     // dstSet
      this->binding,                                               // dstBinding
      0,                                                           // dstArrayElement
      1,                                                           // descriptorCount
      this->descriptorType,                                        // descriptorType
      &this->descriptor_image_info,                                // pImageInfo
      &this->descriptor_buffer_info,                               // pBufferInfo
      nullptr,                                                     // pTexelBufferView
    });
  }

private:
  uint32_t binding;
  VkDescriptorType descriptorType;
  VkShaderStageFlags stageFlags;
  VkDescriptorImageInfo descriptor_image_info{};
  VkDescriptorBufferInfo descriptor_buffer_info{};
};

class Shader : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(Shader)
  Shader() = delete;
  virtual ~Shader() = default;

  explicit Shader(const VkShaderStageFlagBits stage, std::string glsl):
    stage(stage)
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, Shader, alloc);
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, Shader, pipeline);

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

	void alloc(Context* context) 
  {
    this->shader = std::make_unique<VulkanShaderModule>(context->device, this->spv);
  }

  void pipeline(Context* context) 
  {
    context->state.shader_stage_infos.push_back({
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType 
      nullptr,                                             // pNext
      0,                                                   // flags (reserved for future use)
      this->stage,                                         // stage
      this->shader->module,                                // module 
      "main",                                              // pName 
      nullptr,                                             // pSpecializationInfo
    });
  }

protected:
  std::vector<uint32_t> spv;
  VkShaderStageFlagBits stage;
  std::unique_ptr<VulkanShaderModule> shader;
};

class Sampler : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(Sampler)
  virtual ~Sampler() = default;
  
  Sampler(VkFilter magFilter,
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
		REGISTER_VISITOR_CALLBACK(allocvisitor, Sampler, alloc);
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, Sampler, pipeline);
	}

	void alloc(Context* context)
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

  void pipeline(Context* context) 
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
  IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(Image)
  virtual ~Image() = default;
  Image(VkSampleCountFlagBits sample_count,
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
    REGISTER_VISITOR_CALLBACK(allocvisitor, Image, alloc);
    REGISTER_VISITOR_CALLBACK(pipelinevisitor, Image, pipeline);
  }

  void alloc(Context* context)
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
        texture->subresource_range().aspectMask,               // aspectMask
        mip_level,                                             // mipLevel
        texture->subresource_range().baseArrayLayer,           // baseArrayLayer
        texture->subresource_range().layerCount,               // layerCount
      };

      uint32_t bufferRowLength = 0;
      uint32_t bufferImageHeight = 0;
      VkOffset3D imageOffset = { 0, 0, 0 };
      VkExtent3D imageExtent = texture->extent(mip_level);

      VkBufferImageCopy region{
        bufferOffset,
        bufferRowLength,
        bufferImageHeight,
        imageSubresource,
        imageOffset,
        imageExtent
      };

      regions.push_back(region);
      bufferOffset += texture->size(mip_level);
    }

    context->command->pipelineBarrier(
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,          // don't wait for anything
      VK_PIPELINE_STAGE_TRANSFER_BIT,             // but block transfer 
      { 
        VulkanImage::MemoryBarrier(
          this->image->image->image,
          0,
          0,
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          context->state.texture->subresource_range()) 
      });

    vkCmdCopyBufferToImage(
      context->command->buffer(),
      context->state.buffer,
      this->image->image->image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      static_cast<uint32_t>(regions.size()),
      regions.data());

    context->command->pipelineBarrier(
      VK_PIPELINE_STAGE_TRANSFER_BIT,               // wait for transfer
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,         // don't block anything
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

  void pipeline(Context* context)
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
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(ImageView)
  virtual ~ImageView() = default;
  ImageView(VkComponentSwizzle r,
            VkComponentSwizzle g,
            VkComponentSwizzle b,
            VkComponentSwizzle a) :
    component_mapping({ r, g, b, a })
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, ImageView, alloc);
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, ImageView, pipeline);
	}

  void alloc(Context* context) 
  {
		this->view = std::make_unique<VulkanImageView>(
			context->device,
			context->state.image,
			context->state.texture->format(),
			context->state.texture->image_view_type(),
			this->component_mapping,
			context->state.texture->subresource_range());
  }

  void pipeline(Context* context) 
  {
    context->state.imageView = this->view->view;
  }

private:
  std::unique_ptr<VulkanImageView> view;
  VkComponentMapping component_mapping;
};


class Extent : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
	NO_COPY_OR_ASSIGNMENT(Extent)
	Extent() = delete;
	virtual ~Extent() = default;

	Extent(uint32_t width, uint32_t height) :
		extent{ width, height }
	{
		REGISTER_VISITOR_CALLBACK(allocvisitor, Extent, update);
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, Extent, update);
		REGISTER_VISITOR_CALLBACK(recordvisitor, Extent, update);
		REGISTER_VISITOR_CALLBACK(resizevisitor, Extent, update);
		REGISTER_VISITOR_CALLBACK(rendervisitor, Extent, update);
	}

	void update(Context* context)
	{
		context->state.extent = this->extent;
	}

private:
	VkExtent2D extent;
};


class CullMode : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(CullMode)
  CullMode() = delete;
  virtual ~CullMode() = default;

  explicit CullMode(VkCullModeFlags cullmode) :
    cullmode(cullmode)
  {
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, CullMode, pipeline);
	}
  
  void pipeline(Context* context) 
  {
    context->state.rasterization_state.cullMode = this->cullmode;
  }

private:
  VkCullModeFlags cullmode;
};

class ComputeCommand : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(ComputeCommand)
  ComputeCommand() = delete;
  virtual ~ComputeCommand() = default;

  explicit ComputeCommand(uint32_t group_count_x, 
                          uint32_t group_count_y, 
                          uint32_t group_count_z) : 
    group_count_x(group_count_x), 
    group_count_y(group_count_y), 
    group_count_z(group_count_z) 
  {
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, ComputeCommand, pipeline);
	}

  void pipeline(Context* context) 
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

    for (auto & write_descriptor_set : context->state.write_descriptor_sets) {
      write_descriptor_set.dstSet = this->descriptor_set->descriptor_sets[0];
    }
    this->descriptor_set->update(context->state.write_descriptor_sets);

    this->compute_pipeline = std::make_unique<VulkanComputePipeline>(
      context->device,
      context->pipelinecache->cache,
      context->state.shader_stage_infos[0],
      this->pipeline_layout->layout);
  }

  void record(Context* context) 
  {
    vkCmdBindDescriptorSets(this->command->buffer(),
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            this->pipeline_layout->layout,
                            0,
                            static_cast<uint32_t>(this->descriptor_set->descriptor_sets.size()),
                            this->descriptor_set->descriptor_sets.data(),
                            0,
                            nullptr);

    vkCmdBindPipeline(this->command->buffer(),
                      VK_PIPELINE_BIND_POINT_COMPUTE,
                      this->compute_pipeline->pipeline);

    vkCmdDispatch(this->command->buffer(),
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
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(DrawCommandBase)
  DrawCommandBase() = delete;
  virtual ~DrawCommandBase() = default;

  explicit DrawCommandBase(VkPrimitiveTopology topology) : 
    topology(topology)
  {}

	void alloc(Context* context) 
  {
    this->command = std::make_unique<VulkanCommandBuffers>(
      context->device,
      1,
      VK_COMMAND_BUFFER_LEVEL_SECONDARY);
  }

  virtual void execute(VkCommandBuffer command, Context* context) = 0;

  void pipeline(Context* context) 
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

    for (auto & write_descriptor_set : context->state.write_descriptor_sets) {
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

  void record(Context* context) 
  {
    this->command->begin(
      0,
      context->state.renderpass->renderpass,
      0,
      VK_NULL_HANDLE,
      VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);

    vkCmdBindDescriptorSets(this->command->buffer(), 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            this->pipeline_layout->layout, 
                            0, 
                            static_cast<uint32_t>(this->descriptor_sets->descriptor_sets.size()), 
                            this->descriptor_sets->descriptor_sets.data(), 
                            0, 
                            nullptr);

    vkCmdBindPipeline(this->command->buffer(), 
                      VK_PIPELINE_BIND_POINT_GRAPHICS, 
                      this->graphics_pipeline->pipeline);

    std::vector<VkRect2D> scissors{ {
      { 0, 0 },
      context->state.extent
    } };

    std::vector<VkViewport> viewports{ {
      0.0f,																								// x
      0.0f,																								// y
      static_cast<float>(context->state.extent.width),		// width
      static_cast<float>(context->state.extent.height),		// height
      0.0f,																								// minDepth
      1.0f																								// maxDepth
    } };

    vkCmdSetScissor(this->command->buffer(), 
                    0, 
                    static_cast<uint32_t>(scissors.size()), 
                    scissors.data());

    vkCmdSetViewport(this->command->buffer(), 
                     0, 
                     static_cast<uint32_t>(viewports.size()),
                     viewports.data());

    vkCmdBindVertexBuffers(this->command->buffer(), 
                           0, 
                           static_cast<uint32_t>(context->state.vertex_attribute_buffers.size()),
                           context->state.vertex_attribute_buffers.data(),
                           context->state.vertex_attribute_buffer_offsets.data());
    
    this->execute(this->command->buffer(), context);
    this->command->end();
  }

  void render(Context* context) 
  {
    vkCmdExecuteCommands(context->state.command->buffer(),
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
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(DrawCommand)
  virtual ~DrawCommand() = default;

  explicit DrawCommand(uint32_t vertexcount,
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
		REGISTER_VISITOR_CALLBACK(allocvisitor, DrawCommand, alloc);
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, DrawCommand, pipeline);
		REGISTER_VISITOR_CALLBACK(recordvisitor, DrawCommand, record);
		REGISTER_VISITOR_CALLBACK(rendervisitor, DrawCommand, render);
	}

private:
  void execute(VkCommandBuffer command, Context*) override
  {
    vkCmdDraw(command, this->vertexcount, this->instancecount, this->firstvertex, this->firstinstance);
  }

  uint32_t vertexcount;
  uint32_t instancecount;
  uint32_t firstvertex;
  uint32_t firstinstance;
};

class IndexedDrawCommand : public DrawCommandBase {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(IndexedDrawCommand)
  virtual ~IndexedDrawCommand() = default;

  explicit IndexedDrawCommand(uint32_t indexcount,
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
		REGISTER_VISITOR_CALLBACK(allocvisitor, IndexedDrawCommand, alloc);
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, IndexedDrawCommand, pipeline);
		REGISTER_VISITOR_CALLBACK(recordvisitor, IndexedDrawCommand, record);
		REGISTER_VISITOR_CALLBACK(rendervisitor, IndexedDrawCommand, render);
	}

private:
  void execute(VkCommandBuffer command, Context* context) override
  {
    vkCmdBindIndexBuffer(command, 
                         context->state.index_buffer_description.buffer, 
                         this->offset, 
                         context->state.index_buffer_description.type);

    vkCmdDrawIndexed(command, 
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
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(FramebufferAttachment)
  virtual ~FramebufferAttachment() = default;

  FramebufferAttachment(VkFormat format,
                        VkImageUsageFlags usage,
                        VkImageAspectFlags aspectMask) :
    format(format),
    usage(usage)
  {
    this->subresource_range = {
      aspectMask, 0, 1, 0, 1
    };

		REGISTER_VISITOR_CALLBACK(allocvisitor, FramebufferAttachment, alloc);
		REGISTER_VISITOR_CALLBACK(resizevisitor, FramebufferAttachment, alloc);
  }

	void alloc(Context* context) 
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
  }

public:
  VkFormat format;
  VkImageUsageFlags usage;
  VkImageSubresourceRange subresource_range;

  VkComponentMapping component_mapping{
    VK_COMPONENT_SWIZZLE_R,
    VK_COMPONENT_SWIZZLE_G,
    VK_COMPONENT_SWIZZLE_B,
    VK_COMPONENT_SWIZZLE_A
  };

public:
  std::shared_ptr<VulkanImageObject> image;
  std::shared_ptr<VulkanImageView> imageview;
};

class Framebuffer : public Group {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(Framebuffer)
  virtual ~Framebuffer() = default;

  explicit Framebuffer(std::vector<std::shared_ptr<Node>> children) :
    Group(std::move(children))
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, Framebuffer, alloc);
		REGISTER_VISITOR_CALLBACK(resizevisitor, Framebuffer, resize);
	}

	void do_alloc(Context* context)
	{
		this->framebuffer = std::make_unique<VulkanFramebuffer>(
			context->device,
			context->state.renderpass,
			context->state.framebuffer_attachments,
			context->state.extent,
			1);
		context->state.framebuffer = this->framebuffer;
	}

	void alloc(Context* context) 
	{
		allocvisitor.visit_group(this, context);
		this->do_alloc(context);
	}

	void resize(Context* context)
	{
		resizevisitor.visit_group(this, context);
		this->do_alloc(context);
	}

public:
  std::shared_ptr<VulkanFramebuffer> framebuffer;
};

class InputAttachment : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(InputAttachment)
  virtual ~InputAttachment() = default;
  explicit InputAttachment(uint32_t attachment, VkImageLayout layout) :
    attachment({ attachment, layout })
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, InputAttachment, alloc);
	}

	void alloc(Context* context) 
	{
    context->state.input_attachments.push_back(this->attachment);
  }

  VkAttachmentReference attachment;
};


class ColorAttachment : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(ColorAttachment)
  virtual ~ColorAttachment() = default;
  explicit ColorAttachment(uint32_t attachment, VkImageLayout layout) :
    attachment({ attachment, layout })
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, ColorAttachment, alloc);
	}

	void alloc(Context* context) 
  {
    context->state.color_attachments.push_back(this->attachment);
  }

private:
  VkAttachmentReference attachment;
};


class ResolveAttachment : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(ResolveAttachment)
  virtual ~ResolveAttachment() = default;
  explicit ResolveAttachment(uint32_t attachment, VkImageLayout layout) :
    attachment({ attachment, layout })
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, ResolveAttachment, alloc);
	}

	void alloc(Context* context) 
  {
    context->state.resolve_attachments.push_back(this->attachment);
  }

private:
  VkAttachmentReference attachment;
};

class DepthStencilAttachment : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(DepthStencilAttachment)
  virtual ~DepthStencilAttachment() = default;
  explicit DepthStencilAttachment(uint32_t attachment, VkImageLayout layout) :
    attachment({ attachment, layout })
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, DepthStencilAttachment, alloc);
	}

	void alloc(Context* context) 
  {
    context->state.depth_stencil_attachment = this->attachment;
  }

private:
  VkAttachmentReference attachment;
};

class PreserveAttachment : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(PreserveAttachment)
  virtual ~PreserveAttachment() = default;
  explicit PreserveAttachment(uint32_t attachment) :
    attachment(attachment)
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, PreserveAttachment, alloc);
	}

	void alloc(Context* context) 
  {
    context->state.preserve_attachments.push_back(this->attachment);
  }

private:
  uint32_t attachment;
};

class PipelineBindpoint : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(PipelineBindpoint)
  virtual ~PipelineBindpoint() = default;
  explicit PipelineBindpoint(VkPipelineBindPoint bind_point) :
    bind_point(bind_point)
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, PipelineBindpoint, alloc);
	}

	void alloc(Context* context) 
  {
    context->state.bind_point = this->bind_point;
  }

private:
  VkPipelineBindPoint bind_point;
};


class SubpassDescription : public Group {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(SubpassDescription)
  SubpassDescription() = delete;
  virtual ~SubpassDescription() = default;

  SubpassDescription(std::vector<std::shared_ptr<Node>> children) :
    Group(std::move(children))
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, SubpassDescription, alloc);
	}

	void alloc(Context* context) 
  {
		allocvisitor.visit_group(this, context);

    context->state.subpass_descriptions.push_back({
      0,                                                                 // flags
      context->state.bind_point,                                         // pipelineBindPoint
      static_cast<uint32_t>(context->state.input_attachments.size()),    // inputAttachmentCount
      context->state.input_attachments.data(),                           // pInputAttachments
      static_cast<uint32_t>(context->state.color_attachments.size()),    // colorAttachmentCount
      context->state.color_attachments.data(),                           // pColorAttachments
      context->state.resolve_attachments.data(),                         // pResolveAttachments
      &context->state.depth_stencil_attachment,                          // pDepthStencilAttachment
      static_cast<uint32_t>(context->state.preserve_attachments.size()), // preserveAttachmentCount
      context->state.preserve_attachments.data()                         // pPreserveAttachments
    });
  }
};

class RenderpassAttachment: public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(RenderpassAttachment)
  virtual ~RenderpassAttachment() = default;

  RenderpassAttachment(VkFormat format,
                       VkSampleCountFlagBits samples,
                       VkAttachmentLoadOp loadOp,
                       VkAttachmentStoreOp storeOp,
                       VkAttachmentLoadOp stencilLoadOp,
                       VkAttachmentStoreOp stencilStoreOp,
                       VkImageLayout initialLayout,
                       VkImageLayout finalLayout)
  {
    this->description = {
      0, 
      format, 
      samples, 
      loadOp, 
      storeOp, 
      stencilLoadOp, 
      stencilStoreOp, 
      initialLayout, 
      finalLayout
    };

		REGISTER_VISITOR_CALLBACK(allocvisitor, RenderpassAttachment, alloc);
	}

	void alloc(Context* context) 
  {
    context->state.attachment_descriptions.push_back(this->description);
  }

private:
  VkAttachmentDescription description;
};

class Renderpass : public Group {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(Renderpass)
  virtual ~Renderpass() = default;

  Renderpass(std::vector<std::shared_ptr<Node>> children) :
    Group(std::move(children))
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, Renderpass, alloc);
		REGISTER_VISITOR_CALLBACK(resizevisitor, Renderpass, resize);
		REGISTER_VISITOR_CALLBACK(rendervisitor, Renderpass, render);
	}

  void alloc(Context* context) 
  {
		allocvisitor.visit_group(this, context);

    this->render_command = std::make_unique<VulkanCommandBuffers>(context->device);
    this->render_queue = context->device->getQueue(VK_QUEUE_GRAPHICS_BIT);

    this->renderpass = context->state.renderpass;
    this->framebuffer = context->state.framebuffer;
  }

  void resize(Context* context)
  {
		resizevisitor.visit_group(this, context);
    this->framebuffer = context->state.framebuffer;
  }

  void render(Context* context) 
  {
    const VkRect2D renderarea{
      { 0, 0 },												// offset
      context->state.extent           // extent
    };

    const std::vector<VkClearValue> clearvalues{
      { { { 0.0f, 0.0f, 0.0f, 0.0f } } },
      { { { 1.0f, 0 } } }
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
			rendervisitor.visit_group(this, context);
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
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(RenderpassDescription)
  virtual ~RenderpassDescription() = default;
  RenderpassDescription(std::vector<std::shared_ptr<Node>> children) :
    Group(std::move(children))
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, RenderpassDescription, alloc);
		REGISTER_VISITOR_CALLBACK(resizevisitor, RenderpassDescription, resize);
		REGISTER_VISITOR_CALLBACK(pipelinevisitor, RenderpassDescription, pipeline);
		REGISTER_VISITOR_CALLBACK(recordvisitor, RenderpassDescription, record);
	}

	void alloc(Context* context) 
	{
		allocvisitor.visit_group(this, context);

    this->renderpass = std::make_shared<VulkanRenderpass>(context->device,
                                                          context->state.attachment_descriptions,
                                                          context->state.subpass_descriptions);
		context->state.renderpass = this->renderpass;
  }

	void resize(Context* context)
	{
		resizevisitor.visit_group(this, context);
		context->state.renderpass = this->renderpass;
	}

	void pipeline(Context* context)
	{
		pipelinevisitor.visit_group(this, context);
		context->state.renderpass = this->renderpass;
	}

	void record(Context* context) 
	{
		recordvisitor.visit_group(this, context);
		context->state.renderpass = this->renderpass;
	}

public:
  std::shared_ptr<VulkanRenderpass> renderpass;
};

class SwapchainObject : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(SwapchainObject)
  virtual ~SwapchainObject() = default;

  SwapchainObject(std::shared_ptr<FramebufferAttachment> color_attachment,
                  std::shared_ptr<VulkanSurface> surface,
                  VkSurfaceFormatKHR surface_format,
                  VkPresentModeKHR present_mode) :
      color_attachment(std::move(color_attachment)),
      surface(std::move(surface)),
      surface_format(surface_format),
      present_mode(present_mode),
      present_queue(nullptr)
  {
		REGISTER_VISITOR_CALLBACK(allocvisitor, SwapchainObject, alloc);
		REGISTER_VISITOR_CALLBACK(resizevisitor, SwapchainObject, resize);
		REGISTER_VISITOR_CALLBACK(recordvisitor, SwapchainObject, record);
		REGISTER_VISITOR_CALLBACK(presentvisitor, SwapchainObject, present);
	}

	void alloc(Context* context) 
  {
    this->present_queue = context->device->getQueue(0, this->surface->surface);
    this->swapchain_image_ready = std::make_unique<VulkanSemaphore>(context->device);
    this->swap_buffers_finished = std::make_unique<VulkanSemaphore>(context->device);

    this->resize(context);
  }

  void resize(Context* context) 
  {
    VkSwapchainKHR prevswapchain = (this->swapchain) ? this->swapchain->swapchain : nullptr;

    this->swapchain = std::make_unique<VulkanSwapchain>(
      context->device,
      context->vulkan,
      this->surface->surface,
      3,
      this->surface_format.format,
      this->surface_format.colorSpace,
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
    THROW_ON_ERROR(context->vulkan->vkGetSwapchainImages(context->device->device, 
                                                        this->swapchain->swapchain, 
                                                        &count, 
                                                        nullptr));
    this->swapchain_images.resize(count);
    THROW_ON_ERROR(context->vulkan->vkGetSwapchainImages(context->device->device, 
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

  void record(Context* context) 
  {
    this->swap_buffers_command = std::make_unique<VulkanCommandBuffers>(context->device,
                                                                        this->swapchain_images.size(),
                                                                        VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    for (size_t i = 0; i < this->swapchain_images.size(); i++) {
      const VkImageSubresourceLayers subresource_layers{
        this->subresource_range.aspectMask,     // aspectMask
        this->subresource_range.baseMipLevel,   // mipLevel
        this->subresource_range.baseArrayLayer, // baseArrayLayer
        this->subresource_range.layerCount      // layerCount;
      };

      const VkOffset3D offset = {
        0, 0, 0
      };

      VkExtent3D extent3d = {
        context->state.extent.width, context->state.extent.height, 1
      };

      std::vector<VkImageCopy> regions{ {
        subresource_layers,             // srcSubresource
        offset,                         // srcOffset
        subresource_layers,             // dstSubresource
        offset,                         // dstOffset
        extent3d                        // extent
      } };

      VulkanCommandBuffers::Scope command_scope(this->swap_buffers_command.get(), i);

      VkImage srcImage = this->color_attachment->image->image->image;
      VkImage dstImage = this->swapchain_images[i];

      this->swap_buffers_command->pipelineBarrier(
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // wait until color attachment is written
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // no later stages to block, we're done
        {
          VulkanImage::MemoryBarrier(
            srcImage,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,       // srcAccessMask
            0,                                          // dstAccessMask 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,   // oldLayout
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,       // newLayout
            this->subresource_range),
          VulkanImage::MemoryBarrier(
            dstImage,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            this->subresource_range)
          }, i);

      vkCmdCopyImage(this->swap_buffers_command->buffer(i),
                     srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     static_cast<uint32_t>(regions.size()), regions.data());

      this->swap_buffers_command->pipelineBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,                 // wait until copy is done
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // nothing to block
        {
          VulkanImage::MemoryBarrier(
            srcImage,
            0,                                          // srcAccessMask
            0,                                          // dstAccessMask 
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            this->subresource_range),
          VulkanImage::MemoryBarrier(
            dstImage,
            0,                                          // srcAccessMask
            0,                                          // dstAccessMask 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            this->subresource_range)
        }, i);
    }
  }

  void present(Context* context)
  {
    THROW_ON_ERROR(context->vulkan->vkAcquireNextImage(
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
      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,              // sType
      nullptr,                                         // pNext
      static_cast<uint32_t>(signal_semaphores.size()), // waitSemaphoreCount
      signal_semaphores.data(),                        // pWaitSemaphores
      1,                                               // swapchainCount
      &this->swapchain->swapchain,                     // pSwapchains
      &this->image_index,                              // pImageIndices
      nullptr                                          // pResults
    };

    THROW_ON_ERROR(context->vulkan->vkQueuePresent(this->present_queue, &present_info));
  }

private:
  std::shared_ptr<FramebufferAttachment> color_attachment;
  std::shared_ptr<VulkanSurface> surface;
  VkPresentModeKHR present_mode;
  VkSurfaceFormatKHR surface_format;
  VkQueue present_queue;

  std::unique_ptr<VulkanSwapchain> swapchain;
  std::vector<VkImage> swapchain_images;
  std::unique_ptr<VulkanCommandBuffers> swap_buffers_command;
  std::unique_ptr<VulkanSemaphore> swapchain_image_ready;
  std::unique_ptr<VulkanSemaphore> swap_buffers_finished;

  const VkImageSubresourceRange subresource_range{
    VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
  };

  uint32_t image_index{ 0 };
};


class OffscreenImage : public Node {
public:
	IMPLEMENT_VISITABLE_INLINE
		NO_COPY_OR_ASSIGNMENT(OffscreenImage)
		virtual ~OffscreenImage() = default;

	OffscreenImage(std::shared_ptr<FramebufferAttachment> color_attachment)
		: color_attachment(std::move(color_attachment))
	{
		REGISTER_VISITOR_CALLBACK(allocvisitor, OffscreenImage, alloc);
		REGISTER_VISITOR_CALLBACK(resizevisitor, OffscreenImage, alloc);
		REGISTER_VISITOR_CALLBACK(recordvisitor, OffscreenImage, record);
		REGISTER_VISITOR_CALLBACK(rendervisitor, OffscreenImage, render);
	}

  void alloc(Context* context)
  {
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

  void record(Context* context)
  {
    const VkImageSubresourceLayers subresource_layers{
      this->subresource_range.aspectMask,                             // aspectMask
      this->subresource_range.baseMipLevel,                           // mipLevel
      this->subresource_range.baseArrayLayer,                         // baseArrayLayer
      this->subresource_range.layerCount                              // layerCount;
    };

    const VkOffset3D offset = {
      0, 0, 0
    };

    VkExtent3D extent3d = {
      context->state.extent.width, context->state.extent.height, 1
    };

    VkImageCopy image_copy{
      subresource_layers,                                             // srcSubresource
      offset,                                                         // srcOffset
      subresource_layers,                                             // dstSubresource
      offset,                                                         // dstOffset
      extent3d                                                        // extent
    };

    VulkanCommandBuffers::Scope command_scope(this->get_image_command.get());

    this->get_image_command->pipelineBarrier(
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,          // don't wait for anything, the color attachment was rendered to in preceding render pass
      VK_PIPELINE_STAGE_TRANSFER_BIT,             // block transfer stage (copy)
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

    vkCmdCopyImage(
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

  std::set<uint32_t> getTiles(class Context* context)
	{
    //Timer timer("offscreen render");

		this->get_image_command->submit(
			context->queue,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

		const uint8_t* data = reinterpret_cast<uint8_t*>(this->image->memory->map(VK_WHOLE_SIZE, 0, 0));
    data += this->dataOffset;

    std::set<uint32_t> tiles;

		for (VkDeviceSize p = 0; p < this->image->memory_requirements.size; p+=4) {
      uint8_t i = data[p + 0];
      uint8_t j = data[p + 1];
      uint8_t k = data[p + 2];
      uint8_t m = data[p + 3];

      if (i != 0 || j != 0 || k != 0 || m != 0) {
        uint32_t key = (i >> 0) << 24 | (j >> 0) << 16 | (k >> 0) << 8 | m + 0;
        if (tiles.find(key) == tiles.end()) {
          tiles.insert(key);
          tiles.insert((i >> 1) << 24 | (j >> 1) << 16 | (k >> 1) << 8 | m + 1);
        }
      }
		}

		this->image->memory->unmap();

    return tiles;
	}

  void render(Context* context)
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

	VkDeviceSize dataOffset;
};


class SparseImage : public Node {
public:
  IMPLEMENT_VISITABLE_INLINE
  NO_COPY_OR_ASSIGNMENT(SparseImage)
  virtual ~SparseImage() = default;
  SparseImage(VkSampleCountFlagBits sample_count,
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
    REGISTER_VISITOR_CALLBACK(allocvisitor, SparseImage, alloc);
    REGISTER_VISITOR_CALLBACK(pipelinevisitor, SparseImage, pipeline);
    REGISTER_VISITOR_CALLBACK(rendervisitor, SparseImage, render);
  }

  void alloc(Context* context)
  {
    this->bind_sparse_finished = std::make_unique<VulkanSemaphore>(context->device);
    this->copy_sparse_finished = std::make_unique<VulkanSemaphore>(context->device);
    this->copy_sparse_command = std::make_unique<VulkanCommandBuffers>(context->device);

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

    VkSparseImageMemoryRequirements sparse_memory_requirement =
      image->getSparseMemoryRequirements(texture->subresource_range().aspectMask);

    VkMemoryRequirements memory_requirements = this->image->getMemoryRequirements();

    this->numPages = 10000;
    this->pageSize = memory_requirements.alignment;
    this->imageExtent = sparse_memory_requirement.formatProperties.imageGranularity;

    uint32_t memory_type_index = context->device->physical_device.getMemoryTypeIndex(
      memory_requirements.memoryTypeBits,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    this->image_memory = std::make_shared<VulkanMemory>(
      context->device,
      numPages * pageSize,
      memory_type_index);

    this->image_buffer = std::make_shared<VulkanBufferObject>(
      context->device,
      0,
      this->numPages * this->pageSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_SHARING_MODE_EXCLUSIVE,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    this->buffer_memory_map = std::make_shared<MemoryMap>(this->image_buffer->memory.get());

    for (uint32_t i = 0; i < this->numPages; i++) {
      this->free_memory_offsets.push(i * this->pageSize);
    }
    context->state.image = this->image->image;
  }

  void pipeline(Context* context)
  {
    context->state.imageLayout = this->layout;
  }

  void render(Context* context)
  {
    VulkanTextureImage* texture = context->state.texture;

    std::vector<VkSparseImageMemoryBind> image_memory_binds;
    std::vector<VkBufferImageCopy> regions;

    std::set<uint32_t> tiles = context->image->getTiles(context);

    // try to free some pages
    std::vector<uint32_t> erase_these_keys;
    for (auto& [key, image_memory_bind] : this->bound_memory_pages) {
      if (tiles.find(key) == tiles.end()) { // we can free this page
        image_memory_bind.memory = nullptr;
        image_memory_binds.push_back(image_memory_bind);
        this->free_memory_offsets.push(image_memory_bind.memoryOffset);
        erase_these_keys.push_back(key);
      }
    }

    for (auto key : erase_these_keys) {
      this->bound_memory_pages.erase(key);
    }

    for (uint32_t key : tiles) {
      if (this->bound_memory_pages.find(key) == this->bound_memory_pages.end()) {
        if (this->free_memory_offsets.empty()) {
          throw std::runtime_error("no free memory pages");
        }
        VkDeviceSize memoryOffset = this->free_memory_offsets.front();
        this->free_memory_offsets.pop();

        uint32_t i = key >> 24 & 0xFF;
        uint32_t j = key >> 16 & 0xFF;
        uint32_t k = key >> 8 & 0xFF;
        uint32_t mipLevel = key & 0xFF;

        if (mipLevel >= texture->levels()) {
          throw std::runtime_error("invalid mip level");
        }

        VkOffset3D imageOffset = {
          int32_t(i * imageExtent.width),
          int32_t(j * imageExtent.height),
          int32_t(k * imageExtent.depth)
        };

        VkImageSubresourceRange subresource_range = texture->subresource_range();

        const VkImageSubresource subresource{
          subresource_range.aspectMask, mipLevel, 0
        };

        VkSparseImageMemoryBind image_memory_bind {
          subresource,                                            // subresource
          imageOffset,                                            // offset
          imageExtent,                                            // extent
          image_memory->memory,                                   // memory
          memoryOffset,                                           // memoryOffset
          0                                                       // flags
        };

        this->bound_memory_pages.insert({ key, image_memory_bind });
        image_memory_binds.push_back(image_memory_bind);

        const VkImageSubresourceLayers imageSubresource{
          subresource_range.aspectMask,                           // aspectMask
          mipLevel,                                               // mipLevel
          subresource_range.baseArrayLayer,                       // baseArrayLayer
          subresource_range.layerCount,                           // layerCount
        };

        regions.push_back({
          memoryOffset,                                           // bufferOffset 
          0,                                                      // bufferRowLength
          0,                                                      // bufferImageHeight
          imageSubresource,                                       // imageSubresource
          imageOffset,                                            // imageOffset
          imageExtent,                                            // imageExtent
        });

        VkExtent3D extent = texture->extent(mipLevel);

        VkDeviceSize mipOffset = 0;
        for (uint32_t m = 0; m < mipLevel; m++) {
          mipOffset += texture->size(m);
        }

        uint32_t test = ((imageOffset.z * extent.height) + imageOffset.y) * extent.width + imageOffset.x;

        VkDeviceSize bufferOffset = mipOffset + 
          static_cast<VkDeviceSize>(((imageOffset.z * extent.height) + imageOffset.y) * extent.width + imageOffset.x) * texture->element_size();

        auto first = texture->data() + bufferOffset;
        auto last = first + pageSize;
        auto dest = buffer_memory_map->mem + memoryOffset;

        std::copy(first, last, dest);
      }
    }

    if (image_memory_binds.empty()) {
      return;
    }

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
      VK_STRUCTURE_TYPE_BIND_SPARSE_INFO,                                   // sType
      nullptr,                                                              // pNext
      static_cast<uint32_t>(wait_semaphores.size()),                        // waitSemaphoreCount
      wait_semaphores.data(),                                               // pWaitSemaphores
      static_cast<uint32_t>(buffer_memory_bind_infos.size()),               // bufferBindCount
      buffer_memory_bind_infos.data(),                                      // pBufferBinds
      static_cast<uint32_t>(image_opaque_memory_bind_infos.size()),         // imageOpaqueBindCount
      image_opaque_memory_bind_infos.data(),                                // pImageOpaqueBinds
      static_cast<uint32_t>(image_memory_bind_info.size()),                 // imageBindCount
      image_memory_bind_info.data(),                                        // pImageBinds
      static_cast<uint32_t>(signal_semaphores.size()),                      // signalSemaphoreCount
      signal_semaphores.data(),                                             // pSignalSemaphores
    };

    vkQueueBindSparse(context->queue, 1, &bind_sparse_info, VK_NULL_HANDLE);

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

      vkCmdCopyBufferToImage(
        this->copy_sparse_command->buffer(),
        this->image_buffer->buffer->buffer,
        this->image->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        static_cast<uint32_t>(regions.size()),
        regions.data());

      this->copy_sparse_command->pipelineBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,                     // wait for transfer operation (copy)
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,              // block fragment shader where texture is sampled
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
        context->queue,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_NULL_HANDLE,
        wait_semaphores,
        signal_semaphores);

      context->wait_semaphores.push_back(this->copy_sparse_finished->semaphore);
    }
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

  std::shared_ptr<VulkanBufferObject> image_buffer;
  std::shared_ptr<MemoryMap> buffer_memory_map;
  VkDeviceSize pageSize;
  uint32_t numPages;
  VkExtent3D imageExtent;

  std::queue<VkDeviceSize> free_memory_offsets;
  std::map<uint32_t, VkSparseImageMemoryBind> bound_memory_pages;

  std::unique_ptr<VulkanSemaphore> bind_sparse_finished;
  std::unique_ptr<VulkanSemaphore> copy_sparse_finished;
  std::unique_ptr<VulkanCommandBuffers> copy_sparse_command;
};
