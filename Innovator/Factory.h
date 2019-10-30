#pragma once

#include <Innovator/Defines.h>
#include <vulkan/vulkan.h>
#include <functional>
#include <memory>

class VulkanTextureImage {
public:
  NO_COPY_OR_ASSIGNMENT(VulkanTextureImage)
  VulkanTextureImage() = default;
  virtual ~VulkanTextureImage() = default;

  virtual VkExtent3D extent(size_t mip_level) const = 0;
  virtual uint32_t base_level() const = 0;
  virtual uint32_t levels() const = 0;
  virtual uint32_t base_layer() const = 0;
  virtual uint32_t layers() const = 0;
  virtual size_t size() const = 0;
  virtual size_t size(size_t level) const = 0;
  virtual const unsigned char * data() const = 0;
  virtual VkFormat format() const = 0;
  virtual VkImageType image_type() const = 0;
  virtual VkImageViewType image_view_type() const = 0;
  virtual VkImageSubresourceRange subresource_range() const = 0;
};

class VulkanImageFactory {
public:
  typedef std::function<std::shared_ptr<VulkanTextureImage>(const std::string &)> ImageFunc;

  static std::shared_ptr<VulkanTextureImage> Create(const std::string & filename)
  {
    return create_image(filename);
  }

  template <typename ImageType>
  static void Register()
  {
    create_image = [](const std::string & filename)
    {
      return std::make_shared<ImageType>(filename);
    };
  }

  inline static ImageFunc create_image;
};

#include <gli/gli.hpp>

class GliTextureImage : public VulkanTextureImage {
public:
  NO_COPY_OR_ASSIGNMENT(GliTextureImage)

  explicit GliTextureImage(const std::string& filename) :
    texture(gli::load(filename))
  {}

  virtual ~GliTextureImage() = default;

  VkExtent3D extent(size_t level) const override
  {
    return {
      static_cast<uint32_t>(static_cast<uint32_t>(this->texture[level].extent().x)),
      static_cast<uint32_t>(static_cast<uint32_t>(this->texture[level].extent().y)),
      static_cast<uint32_t>(static_cast<uint32_t>(this->texture[level].extent().z)),
    };
  }

  uint32_t base_level() const override
  {
    return static_cast<uint32_t>(this->texture.base_level());
  }

  uint32_t levels() const override
  {
    return static_cast<uint32_t>(this->texture.levels());
  }

  uint32_t base_layer() const override
  {
    return static_cast<uint32_t>(this->texture.base_layer());
  }

  uint32_t layers() const override
  {
    return static_cast<uint32_t>(this->texture.layers());
  }

  size_t size() const override
  {
    return this->texture.size();
  }

  size_t size(size_t level) const override
  {
    return this->texture.size(level);
  }

  const unsigned char* data() const override
  {
    return reinterpret_cast<const unsigned char*>(this->texture.data());
  }

  VkFormat format() const override
  {
    return VK_FORMAT_R8G8B8A8_UNORM;
  }

  VkImageType image_type() const override
  {
    return VK_IMAGE_TYPE_2D;
  }

  VkImageViewType image_view_type() const override
  {
    return VK_IMAGE_VIEW_TYPE_2D;
  }

  VkImageSubresourceRange subresource_range() const override
  {
    return {
      VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask 
      this->base_level(),         // baseMipLevel 
      this->levels(),             // levelCount 
      this->base_layer(),         // baseArrayLayer 
      this->layers()              // layerCount 
    };
  }

  gli::texture2d texture;
};

class DebugTextureImage : public VulkanTextureImage {
public:
  NO_COPY_OR_ASSIGNMENT(DebugTextureImage)

  explicit DebugTextureImage(const std::string& filename)
  {
    this->texture = gli::texture2d(
      gli::texture::format_type::FORMAT_RGBA8_UNORM_PACK8,
      gli::extent2d(512, 512));

      this->texture[0].clear(glm::u8vec4(0, 0, 255, 255));
      this->texture[1].clear(glm::u8vec4(0, 255, 0, 255));
      this->texture[2].clear(glm::u8vec4(0, 255, 255, 255));
      this->texture[3].clear(glm::u8vec4(255, 0, 0, 255));
      this->texture[4].clear(glm::u8vec4(255, 0, 255, 255));
      this->texture[5].clear(glm::u8vec4(255, 255, 0, 255));
      this->texture[6].clear(glm::u8vec4(255, 255, 255, 255));
      this->texture[7].clear(glm::u8vec4(255, 0, 0, 255));
      this->texture[8].clear(glm::u8vec4(255, 0, 0, 255));
      this->texture[9].clear(glm::u8vec4(255, 0, 0, 255));
  }

  virtual ~DebugTextureImage() = default;

  VkExtent3D extent(size_t level) const override
  {
    return {
      static_cast<uint32_t>(static_cast<uint32_t>(this->texture[level].extent().x)),
      static_cast<uint32_t>(static_cast<uint32_t>(this->texture[level].extent().y)),
      static_cast<uint32_t>(static_cast<uint32_t>(this->texture[level].extent().z)),
    };
  }

  uint32_t base_level() const override
  {
    return static_cast<uint32_t>(this->texture.base_level());
  }

  uint32_t levels() const override
  {
    return static_cast<uint32_t>(this->texture.levels());
  }

  uint32_t base_layer() const override
  {
    return static_cast<uint32_t>(this->texture.base_layer());
  }

  uint32_t layers() const override
  {
    return static_cast<uint32_t>(this->texture.layers());
  }

  size_t size() const override
  {
    return this->texture.size();
  }

  size_t size(size_t level) const override
  {
    return this->texture.size(level);
  }

  const unsigned char* data() const override
  {
    return reinterpret_cast<const unsigned char*>(this->texture.data());
  }

  VkFormat format() const override
  {
    return VK_FORMAT_R8G8B8A8_UNORM;
  }

  VkImageType image_type() const override
  {
    return VK_IMAGE_TYPE_2D;
  }

  VkImageViewType image_view_type() const override
  {
    return VK_IMAGE_VIEW_TYPE_2D;
  }

  VkImageSubresourceRange subresource_range() const override
  {
    return {
      VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask 
      this->base_level(),         // baseMipLevel 
      this->levels(),             // levelCount 
      this->base_layer(),         // baseArrayLayer 
      this->layers()              // layerCount 
    };
  }

  gli::texture2d texture;
};
