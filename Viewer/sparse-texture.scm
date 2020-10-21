(define sparse-texture (filename)
    (group
        (sampler 
            VK_FILTER_LINEAR
            VK_FILTER_LINEAR
            VK_SAMPLER_MIPMAP_MODE_LINEAR
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
            (float 0)
            (uint32 0)
            (float 1)
            (uint32 0)
            VK_COMPARE_OP_NEVER
            (float 0)
            (float 10)
            VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
            (uint32 0))

        (texturedata filename)
        (cpumemorybuffer (bufferusageflags VK_BUFFER_USAGE_TRANSFER_SRC_BIT))

        (sparse-image 
            VK_SAMPLE_COUNT_1_BIT
            VK_IMAGE_TILING_OPTIMAL
            (imageusageflags VK_IMAGE_USAGE_TRANSFER_DST_BIT VK_IMAGE_USAGE_SAMPLED_BIT)
            VK_SHARING_MODE_EXCLUSIVE
            (imagecreateflags VK_IMAGE_CREATE_SPARSE_BINDING_BIT VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT)
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)

        (imageview 
            VK_COMPONENT_SWIZZLE_R
            VK_COMPONENT_SWIZZLE_G
            VK_COMPONENT_SWIZZLE_B
            VK_COMPONENT_SWIZZLE_A)

        (descriptorsetlayoutbinding 
            (uint32 1) 
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER 
            VK_SHADER_STAGE_FRAGMENT_BIT)))
