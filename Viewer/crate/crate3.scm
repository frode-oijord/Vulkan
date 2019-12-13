(begin
   (define texture2d (filename)
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

         (textureimage filename)
         (cpumemorybuffer (bufferusageflags VK_BUFFER_USAGE_TRANSFER_SRC_BIT))

         (image 
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

   (define lod-color-attachment 
      (framebuffer-attachment 
         VK_FORMAT_A8B8G8R8_UINT_PACK32
         (imageusageflags 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT 
            VK_IMAGE_USAGE_SAMPLED_BIT)
         (imageaspectflags VK_IMAGE_ASPECT_COLOR_BIT)))

   (define main-color-attachment 
      (framebuffer-attachment 
         VK_FORMAT_B8G8R8A8_UNORM
         (imageusageflags 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT 
            VK_IMAGE_USAGE_SAMPLED_BIT)
         (imageaspectflags VK_IMAGE_ASPECT_COLOR_BIT)))

   (define lod-renderpass (renderpass
      (renderpass-description
         (renderpass-attachment
            VK_FORMAT_A8B8G8R8_UINT_PACK32
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
         lod-color-attachment
         (framebuffer-attachment
            VK_FORMAT_D32_SFLOAT
            (imageusageflags VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            (imageaspectflags VK_IMAGE_ASPECT_DEPTH_BIT)))

      (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
         #version 450

         layout(binding = 1) uniform sampler2D Texture;
         layout(location = 0) in vec2 texCoord;
         layout(location = 0) out uvec4 FragColor;

         void main() 
         {
            uint mip = uint(textureQueryLod(Texture, texCoord).x);

            uint factor = (32 << mip);

            uint i = uint(gl_FragCoord.x) / factor;
            uint j = uint(gl_FragCoord.y) / factor;

            FragColor = uvec4(i, j, 0, mip);
         }
      ]])

      (indexed-shape 
         (bufferdata-uint32 0 1 2 2 3 0)
         (bufferdata-float 0 0 0.5  1 0 0.5  1 1 0.5  0 1 0.5))

      (offscreen-image lod-color-attachment)))


   (define main-renderpass (renderpass
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
         main-color-attachment
         (framebuffer-attachment
            VK_FORMAT_D32_SFLOAT
            (imageusageflags VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            (imageaspectflags VK_IMAGE_ASPECT_DEPTH_BIT)))

      (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
         #version 450

         layout(binding = 1) uniform sampler2D Texture;
         layout(location = 0) in vec2 texCoord;
         layout(location = 0) out vec4 FragColor;

         void main() {
            FragColor = texture(Texture, texCoord);
         }
      ]])

      (indexed-shape 
         (bufferdata-uint32 0 1 2 2 3 0)
         (bufferdata-float 0 0 0.5  1 0 0.5  1 1 0.5  0 1 0.5))))

   (define scene 
      (group
         (viewmatrix 
            0 2 6 
            0 0 0 
            0 1 0)

         (projmatrix 1000 0.1 1.0 0.7)

         (shader VK_SHADER_STAGE_VERTEX_BIT [[
            #version 450

            layout(std140, binding = 0) uniform Transform {
               mat4 ModelViewMatrix;
               mat4 ProjectionMatrix;
            };

            layout(location = 0) in vec3 Position;
            layout(location = 0) out vec2 texCoord;

            out gl_PerVertex {
               vec4 gl_Position;
            };

            void main() 
            {
               texCoord = Position.xy;
               gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(Position, 1.0);
            }
         ]])

         (texture2d "crate/texture.ktx")

         (separator 
            (extent (uint32 240) (uint32 135))
            (projmatrix 1000 0.1 1.0 0.7)
            lod-renderpass)
         (separator 
            (projmatrix 1000 0.1 1.0 0.7)
            main-renderpass)
         ))

   (define vulkan-window (window scene main-color-attachment))
      vulkan-window)
