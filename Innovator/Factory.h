#pragma once

#include <Innovator/Defines.h>

#include <boost/iostreams/device/mapped_file.hpp>

#include <memory>
#include <fstream>
#include <functional>

class VulkanTextureImage {
public:
	NO_COPY_OR_ASSIGNMENT(VulkanTextureImage)
	VulkanTextureImage() = default;
	virtual ~VulkanTextureImage() = default;

	virtual VkExtent3D extent(size_t mip_level) const = 0;
	virtual uint32_t element_size() const = 0;
	virtual uint32_t base_level() const = 0;
	virtual uint32_t levels() const = 0;
	virtual uint32_t base_layer() const = 0;
	virtual uint32_t layers() const = 0;
	virtual size_t size() const = 0;
	virtual size_t size(size_t level) const = 0;
	virtual const unsigned char* data() const = 0;
	virtual VkFormat format() const = 0;
	virtual VkImageType image_type() const = 0;
	virtual VkImageViewType image_view_type() const = 0;
	virtual VkImageSubresourceRange subresource_range() const = 0;
};

class VulkanImageFactory {
public:
	typedef std::function<std::shared_ptr<VulkanTextureImage>(const std::string&)> ImageFunc;

	static std::shared_ptr<VulkanTextureImage> Create(const std::string& filename)
	{
		return create_image(filename);
	}

	template <typename ImageType>
	static void Register()
	{
		create_image = [](const std::string& filename)
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

	uint32_t element_size() const override
	{
		return 4;
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

#include <cmath>
#include <fstream>
#include <filesystem>

class DebugTextureImage : public VulkanTextureImage {
public:
	NO_COPY_OR_ASSIGNMENT(DebugTextureImage)

		explicit DebugTextureImage(const std::string&)
	{
		this->lod0_size = 4096;
		this->num_lods = log2(lod0_size) + 1;

		std::string filename = "sparse3d/checkerboard" + std::to_string(lod0_size) + ".bin";

		if (!std::filesystem::exists(filename)) {
			std::fstream out(filename, std::ios_base::out | std::ios_base::binary);
			for (size_t lod = 0; lod < num_lods; lod++) {
				size_t lod_size = lod0_size >> lod;
				glm::u8vec1* data = new glm::u8vec1[lod_size * lod_size * lod_size];
				for (size_t i = 0; i < lod_size; i++) {
					std::cout << "i = " << i << std::endl;
					for (size_t j = 0; j < lod_size; j++) {
						for (size_t k = 0; k < lod_size; k++) {
							int value = (((i & 64) == 0) ^ ((j & 32) == 0) ^ ((k & 32) == 0)) * 254 + 1;
							data[((i * lod_size) + j) * lod_size + k] = glm::u8vec1(value);
						}
					}
				}
				out.write(reinterpret_cast<char*>(data), lod_size* lod_size* lod_size);
			}
		}

		this->mapped_file.open(filename, boost::iostreams::mapped_file_base::mapmode::readonly);
	}

	virtual ~DebugTextureImage() = default;

	VkExtent3D extent(size_t level) const override
	{
		return {
		  static_cast<uint32_t>(this->lod0_size >> level),
		  static_cast<uint32_t>(this->lod0_size >> level),
		  static_cast<uint32_t>(this->lod0_size >> level),
		};
	}

	uint32_t element_size() const override
	{
		return 1;
	}

	uint32_t base_level() const override
	{
		return 0;
	}

	uint32_t levels() const override
	{
		return static_cast<uint32_t>(this->num_lods);
	}

	uint32_t base_layer() const override
	{
		return 0;
	}

	uint32_t layers() const override
	{
		return 1;
	}

	size_t size() const override
	{
		return this->mapped_file.size();
	}

	size_t size(size_t level) const override
	{
		size_t extent = this->lod0_size >> level;
		size_t num_pixels = extent * extent * extent;
		return num_pixels * this->element_size();
	}

	const unsigned char* data() const override
	{
		return reinterpret_cast<const unsigned char*>(this->mapped_file.const_data());
	}

	VkFormat format() const override
	{
		return VK_FORMAT_R8_UNORM;
	}

	VkImageType image_type() const override
	{
		return VK_IMAGE_TYPE_3D;
	}

	VkImageViewType image_view_type() const override
	{
		return VK_IMAGE_VIEW_TYPE_3D;
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

	boost::iostreams::mapped_file mapped_file;

	size_t lod0_size;
	size_t num_lods;
};
