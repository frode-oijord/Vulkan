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
	virtual uint32_t base_level() const = 0;
	virtual uint32_t levels() const = 0;
	virtual uint32_t base_layer() const = 0;
	virtual uint32_t layers() const = 0;
	virtual size_t size() const = 0;
	virtual size_t size(size_t level) const = 0;
	virtual const uint8_t* const_data() const = 0;
	virtual uint8_t* data() = 0;
	virtual VkFormat format() const = 0;
	virtual VkImageType image_type() const = 0;
	virtual VkImageViewType image_view_type() const = 0;
	virtual VkImageAspectFlags aspect_mask() const = 0;
	VkImageSubresourceRange subresourceRange() const
	{
		return {
		  this->aspect_mask(),        // aspectMask 
		  this->base_level(),         // baseMipLevel 
		  this->levels(),             // levelCount 
		  this->base_layer(),         // baseArrayLayer 
		  this->layers()              // layerCount 
		};
	}

	std::vector<VkBufferImageCopy> getRegions()
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

	uint8_t* data() override
	{
		return reinterpret_cast<uint8_t*>(this->texture.data());
	}

	const uint8_t* const_data() const override
	{
		return reinterpret_cast<const uint8_t*>(this->texture.data());
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


class MemoryMappedImage2D : public VulkanTextureImage {
public:
	explicit MemoryMappedImage2D(const std::string& filename)
	{
#if 0
		this->texture = gli::load(filename);
		std::vector<glm::u8vec4> tiles;
		for (int k = 0; k < 7; k++) {
			std::cout << "writing lod " << k << std::endl;
			for (int tile_start_i = 0; tile_start_i < this->texture[k].extent().x; tile_start_i += 128) {
				for (int tile_start_j = 0; tile_start_j < this->texture[k].extent().y; tile_start_j += 128) {
					std::cout << "writing tile " << tile_start_i / 128 << " " << tile_start_j / 128 << " extent: " << this->texture[k].extent().x << std::endl;
					for (int i = tile_start_i; i < tile_start_i + 128; i++) {
						for (int j = tile_start_j; j < tile_start_j + 128; j++) {
							glm::uvec4 texel = this->texture.load<gli::u8vec4>(glm::uvec2(j, i), k);
							tiles.push_back(texel);
						}
					}
				}
			}
		}

		std::fstream out("world.bin", std::ios_base::out | std::ios_base::binary);
		out.write(reinterpret_cast<char*>(tiles.data()), tiles.size() * sizeof(glm::u8vec4));

		glm::u8vec4* dest = reinterpret_cast<glm::u8vec4*>(this->texture.data());
		std::copy(tiles.data(), tiles.data() + tiles.size(), dest);
#endif
		this->num_lods = 6;
		this->lod0_size = 8192;
		this->mapped_file.open(filename, boost::iostreams::mapped_file_base::mapmode::readonly);
	}

	virtual ~MemoryMappedImage2D() = default;

	VkExtent3D extent(size_t level) const override
	{
		return {
		  static_cast<uint32_t>(this->lod0_size >> level),
		  static_cast<uint32_t>(this->lod0_size >> level),
		  1,
		};
	}

	uint32_t element_size() const override
	{
		return sizeof(gli::u8vec4);
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
		size_t num_pixels = extent * extent;
		return num_pixels * this->element_size();
	}

	uint8_t* data() override
	{
		return reinterpret_cast<uint8_t*>(this->mapped_file.data());
	}

	const uint8_t* const_data() const override
	{
		return reinterpret_cast<const uint8_t*>(this->mapped_file.const_data());
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

	size_t lod0_size;
	size_t num_lods;

	boost::iostreams::mapped_file mapped_file;
};


// 16384 x 16384 x 2048
// 8192 x 8192 x 1024
// 4096 x 4095 x 512
// 2048 x 2048 x 256
// 1024 X 1024 x 128
// 512 x 512 x 64
// 256 x 256 x 32
// 128 x 128 x 16
// 64 x 64 x 8
// 32 x 32 x 4
// 16 x 16 x 2
// 8 x 8 x 1
// 4 x 4 x 1
// 2 x 2 x 1
// 1 x 1 x 1

#include <cmath>
#include <filesystem>

class DebugTextureImageBricked : public VulkanTextureImage {
public:
	explicit DebugTextureImageBricked(const std::string& filename)
	{
		//this->num_lods = std::log2(lod0_size.width) + 1;
		//this->num_lods -= 5;
		this->num_lods = 5;

		//if (!std::filesystem::exists(filename)) {
		//	std::cout << "creating file: " << filename << std::endl;
		//	std::fstream out(filename, std::ios_base::out | std::ios_base::binary);

		//	size_t brick_size = 64 * 32 * 32;

		//	std::vector<uint8_t> black_brick(brick_size);
		//	std::vector<uint8_t> white_brick(brick_size);

		//	std::fill(black_brick.begin(), black_brick.end(), 0);
		//	std::fill(white_brick.begin(), white_brick.end(), 255);

		//	for (size_t lod = 0; lod < this->num_lods; lod++) {

		//		VkExtent3D lod_size{
		//			.width = lod0_size.width >> lod,
		//			.height = lod0_size.height >> lod,
		//			.depth = lod0_size.depth >> lod,
		//		};

		//		for (size_t k = 0; k < lod_size.depth / 32; k++) {
		//			std::cout << "writing line... " << k << std::endl;
		//			for (size_t j = 0; j < lod_size.height / 32; j++) {
		//				for (size_t i = 0; i < lod_size.width / 64; i++) {
		//					size_t brick_color = ((i & 0x1) == 0) ^ ((j & 0x1) == 0) ^ ((k & 0x1) == 0);

		//					(brick_color) ?
		//						out.write(reinterpret_cast<char*>(white_brick.data()), white_brick.size()) :
		//						out.write(reinterpret_cast<char*>(black_brick.data()), black_brick.size());
		//				}
		//			}
		//		}
		//	}
		//}
		this->mapped_file.open(filename, boost::iostreams::mapped_file_base::mapmode::readonly);
	}

	virtual ~DebugTextureImageBricked() = default;

	VkExtent3D extent(size_t level) const override
	{
		return {
		  .width = this->lod0_size.width >> level,
		  .height = this->lod0_size.height >> level,
		  .depth = this->lod0_size.depth >> level,
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
		VkExtent3D extent = this->extent(level);

		size_t num_pixels =
			static_cast<size_t>(extent.width) *
			static_cast<size_t>(extent.height) *
			static_cast<size_t>(extent.depth);

		return num_pixels * this->element_size();
	}

	uint8_t* data() override
	{
		return reinterpret_cast<uint8_t*>(this->mapped_file.data());
	}

	const uint8_t* const_data() const override
	{
		return reinterpret_cast<const uint8_t*>(this->mapped_file.const_data());
	}

	VkFormat format() const override
	{
		return VK_FORMAT_R8_SNORM;
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

	VkExtent3D lod0_size{
		2048, 8192, 1024
	};
	size_t num_lods;
};
