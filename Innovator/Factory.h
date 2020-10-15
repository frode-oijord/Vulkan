#pragma once

#include <Innovator/Defines.h>
#include <boost/iostreams/device/mapped_file.hpp>

#include <memory>
#include <fstream>
#include <functional>

class VulkanTextureImage : public NonCopyable {
public:
	VulkanTextureImage() = default;
	virtual ~VulkanTextureImage() = default;

	virtual VkExtent3D extent(size_t mip_level) const = 0;
	virtual uint32_t element_size() const = 0;
	virtual VkExtent3D brick_size() const = 0;
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
	virtual VkImageAspectFlags aspect_mask() const = 0;
	VkImageSubresourceRange subresource_range() const
	{
		return {
		  this->aspect_mask(),        // aspectMask 
		  this->base_level(),         // baseMipLevel 
		  this->levels(),             // levelCount 
		  this->base_layer(),         // baseArrayLayer 
		  this->layers()              // layerCount 
		};
	}

	std::vector<VkBufferImageCopy> get_regions()
	{
		std::vector<VkBufferImageCopy> regions(this->levels());

		VkDeviceSize bufferOffset = 0;
		for (uint32_t mip_level = 0; mip_level < this->levels(); mip_level++) {

			const VkImageSubresourceLayers imageSubresource{
				.aspectMask = this->aspect_mask(),
				.mipLevel = mip_level,
				.baseArrayLayer = this->base_layer(),
				.layerCount = this->layers()
			};

			regions[mip_level] = {
				.bufferOffset = bufferOffset,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = imageSubresource,
				.imageOffset = { 0, 0, 0 },
				.imageExtent = this->extent(mip_level),
			};

			bufferOffset += this->size(mip_level);
		}
		return regions;
	}
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

	VkExtent3D brick_size() const override {
		return VkExtent3D{ 1, 1, 1 };
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

	VkImageAspectFlags aspect_mask() const override
	{
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}

	gli::texture2d texture;
};


class DebugTextureImage : public VulkanTextureImage {
public:
	explicit DebugTextureImage(const std::string& filename)
	{
		this->lod0_size = 256;
		this->num_lods = 4;// log2(lod0_size) + 1;

		std::vector<glm::u8vec4> texels;
		for (size_t lod = 0; lod < num_lods; lod++) {
			size_t lod_size = lod0_size >> lod;
			for (size_t i = 0; i < lod_size; i++) {
				for (size_t j = 0; j < lod_size; j++) {
					for (size_t k = 0; k < lod_size; k++) {
						texels.push_back(glm::u8vec4(i, j, k, 255.0));
					}
				}
			}
		}
		{
			std::fstream out(filename, std::ios_base::out | std::ios_base::binary);
			out.write(reinterpret_cast<char*>(texels.data()), texels.size() * 4);
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

	VkExtent3D brick_size() const override {
		return VkExtent3D{
		  1, 1, 1
		};
	}

	uint32_t element_size() const override
	{
		return 4;
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
		return VK_FORMAT_R8G8B8A8_UNORM;
	}

	VkImageType image_type() const override
	{
		return VK_IMAGE_TYPE_3D;
	}

	VkImageViewType image_view_type() const override
	{
		return VK_IMAGE_VIEW_TYPE_3D;
	}

	VkImageAspectFlags aspect_mask() const override
	{
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}


	boost::iostreams::mapped_file mapped_file;

	size_t lod0_size;
	size_t num_lods;
};


static void write_brick(size_t start_i, size_t start_j, size_t start_k, std::vector<glm::u8vec4>& texels, glm::u8vec4 texel)
{
	for (size_t i = 0; i < 32; i++) {
		for (size_t j = 0; j < 32; j++) {
			for (size_t k = 0; k < 16; k++) {
				texels.push_back(texel);
			}
		}
	}
}


class DebugTextureImageBricked : public VulkanTextureImage {
public:
	explicit DebugTextureImageBricked(const std::string& filename)
	{
		this->lod0_size = 256;
		this->num_lods = 4;// log2(lod0_size) + 1;

		std::vector<glm::u8vec4> texels;
		for (size_t lod = 0; lod < num_lods; lod++) {
			size_t lod_size = lod0_size >> lod;
			std::cout << std::endl << "writing lod " << lod;
			for (size_t start_k = 0; start_k < lod_size; start_k += 16) {
				std::cout << ".";
				for (size_t start_j = 0; start_j < lod_size; start_j += 32) {
					for (size_t start_i = 0; start_i < lod_size; start_i += 32) {
						glm::u8vec4 texel(start_i, start_j, start_k, 255);
						write_brick(start_i, start_j, start_k, texels, texel);
					}
				}
			}
		}
		{
			std::fstream out(filename, std::ios_base::out | std::ios_base::binary);
			out.write(reinterpret_cast<char*>(texels.data()), texels.size() * 4);
		}
		this->mapped_file.open(filename, boost::iostreams::mapped_file_base::mapmode::readonly);
	}

	virtual ~DebugTextureImageBricked() = default;

	VkExtent3D extent(size_t level) const override
	{
		return {
		  static_cast<uint32_t>(this->lod0_size >> level),
		  static_cast<uint32_t>(this->lod0_size >> level),
		  static_cast<uint32_t>(this->lod0_size >> level),
		};
	}

	VkExtent3D brick_size() const override {
		return VkExtent3D{
		  32, 32, 16
		};
	}

	uint32_t element_size() const override
	{
		return 4;
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
		return VK_FORMAT_R8G8B8A8_UNORM;
	}

	VkImageType image_type() const override
	{
		return VK_IMAGE_TYPE_3D;
	}

	VkImageViewType image_view_type() const override
	{
		return VK_IMAGE_VIEW_TYPE_3D;
	}

	VkImageAspectFlags aspect_mask() const override
	{
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}

	boost::iostreams::mapped_file mapped_file;

	size_t lod0_size;
	size_t num_lods;
};
