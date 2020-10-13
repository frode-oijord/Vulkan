(define vertex-shape (vertices)
    (group
        vertices
        (cpumemorybuffer
        (bufferusageflags
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT))

        (vertexinputattributedescription
            (uint32 0)
            (uint32 0)
            VK_FORMAT_R32G32B32_SFLOAT 
            (uint32 0))

        (vertexinputbindingdescription
            (uint32 0)
            (uint32 12)
            VK_VERTEX_INPUT_RATE_VERTEX)

        (transformbuffer)
        (descriptorsetlayoutbinding 
            (uint32 0) 
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER 
            VK_SHADER_STAGE_VERTEX_BIT)

        (drawcommand 
            (count vertices)
            (uint32 1)
            (uint32 0)
            (uint32 0)
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)))
