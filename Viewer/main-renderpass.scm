(define main-renderpass (scene)
    (renderpass
        (renderpass-description
            (renderpass-attachment
                VK_FORMAT_B8G8R8A8_UNORM
                VK_SAMPLE_COUNT_1_BIT
                VK_ATTACHMENT_LOAD_OP_CLEAR
                VK_ATTACHMENT_STORE_OP_STORE
                VK_ATTACHMENT_LOAD_OP_DONT_CARE
                VK_ATTACHMENT_STORE_OP_DONT_CARE
                VK_IMAGE_LAYOUT_UNDEFINED
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)

            (renderpass-attachment
                VK_FORMAT_D32_SFLOAT
                VK_SAMPLE_COUNT_1_BIT
                VK_ATTACHMENT_LOAD_OP_CLEAR
                VK_ATTACHMENT_STORE_OP_STORE
                VK_ATTACHMENT_LOAD_OP_DONT_CARE
                VK_ATTACHMENT_STORE_OP_DONT_CARE
                VK_IMAGE_LAYOUT_UNDEFINED
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)

            (subpass
                (pipeline-bindpoint VK_PIPELINE_BIND_POINT_GRAPHICS)
                (color-attachment (uint32 0) VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                (depth-attachment (uint32 1) VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)))

        (framebuffer
            (framebuffer-attachment
                (image
                    VK_IMAGE_TYPE_2D
                    VK_FORMAT_B8G8R8A8_UNORM
                    (extent3 1920 1080 1)
                    (uint32 1)
                    (uint32 1)
                    VK_SAMPLE_COUNT_1_BIT
                    VK_IMAGE_TILING_OPTIMAL
                    (imageusageflags
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                        VK_IMAGE_USAGE_SAMPLED_BIT)
                    VK_SHARING_MODE_EXCLUSIVE
                    (imagecreateflags)
                    (memorypropertyflags VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))

                (imageview 
                    VK_IMAGE_VIEW_TYPE_2D
                    VK_FORMAT_B8G8R8A8_UNORM
                    (component-mapping
                        VK_COMPONENT_SWIZZLE_R
                        VK_COMPONENT_SWIZZLE_G
                        VK_COMPONENT_SWIZZLE_B
                        VK_COMPONENT_SWIZZLE_A)
                    (subresource-range
                        (imageaspectflags VK_IMAGE_ASPECT_COLOR_BIT)
                        (uint32 0)
                        (uint32 1)
                        (uint32 0)
                        (uint32 1)))

                (imagelayout
                    VK_IMAGE_LAYOUT_UNDEFINED
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    (subresource-range
                        (imageaspectflags VK_IMAGE_ASPECT_COLOR_BIT)
                        (uint32 0)
                        (uint32 1)
                        (uint32 0)
                        (uint32 1)))

                (currentimagerendertarget))

            (framebuffer-attachment
                (image
                    VK_IMAGE_TYPE_2D
                    VK_FORMAT_D32_SFLOAT
                    (extent3 1920 1080 1)
                    (uint32 1)
                    (uint32 1)
                    VK_SAMPLE_COUNT_1_BIT
                    VK_IMAGE_TILING_OPTIMAL
                    (imageusageflags VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                    VK_SHARING_MODE_EXCLUSIVE
                    (imagecreateflags)
                    (memorypropertyflags VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))

                (imageview 
                    VK_IMAGE_VIEW_TYPE_2D
                    VK_FORMAT_D32_SFLOAT
                    (component-mapping
                        VK_COMPONENT_SWIZZLE_R
                        VK_COMPONENT_SWIZZLE_G
                        VK_COMPONENT_SWIZZLE_B
                        VK_COMPONENT_SWIZZLE_A)
                    (subresource-range
                        (imageaspectflags VK_IMAGE_ASPECT_DEPTH_BIT)
                        (uint32 0)
                        (uint32 1)
                        (uint32 0)
                        (uint32 1)))

                (imagelayout
                    VK_IMAGE_LAYOUT_UNDEFINED
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    (subresource-range
                        (imageaspectflags VK_IMAGE_ASPECT_DEPTH_BIT)
                        (uint32 0)
                        (uint32 1)
                        (uint32 0)
                        (uint32 1)))))
        scene))
