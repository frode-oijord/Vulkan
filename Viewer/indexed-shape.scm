(define indexed-shape (indices vertices)
    (group
        vertices
        (cpumemorybuffer (bufferusageflags VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
        (gpumemorybuffer (bufferusageflags VK_BUFFER_USAGE_TRANSFER_DST_BIT VK_BUFFER_USAGE_VERTEX_BUFFER_BIT))
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

        indices
        (cpumemorybuffer (bufferusageflags VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
        (gpumemorybuffer (bufferusageflags VK_BUFFER_USAGE_TRANSFER_DST_BIT VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
        (indexbufferdescription VK_INDEX_TYPE_UINT32)
        
        (indexeddrawcommand 
            (count indices)
            (uint32 1)
            (uint32 0)
            (int32 0)
            (uint32 0)
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)))
