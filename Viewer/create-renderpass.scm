(define create-renderpass (format scene)
    (renderpass
        (renderpass-description
            (renderpass-attachment
                format
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
                format
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                (imageusageflags
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                    VK_IMAGE_USAGE_SAMPLED_BIT)
                (imageaspectflags VK_IMAGE_ASPECT_COLOR_BIT))

            (framebuffer-attachment
                VK_FORMAT_D32_SFLOAT
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                (imageusageflags VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                (imageaspectflags VK_IMAGE_ASPECT_DEPTH_BIT)))

        scene))
