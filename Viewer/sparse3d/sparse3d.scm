(begin
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

         (textureimage filename)

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

   (define slice (z) 
      (indexed-shape 
         (bufferdata-uint32 0 1 2 2 3 0)
         (bufferdata-float 0 0 z  1 0 z  1 1 z  0 1 z)))


   (define main-color-attachment 
      (framebuffer-attachment 
         VK_FORMAT_B8G8R8A8_UNORM
         (imageusageflags 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT 
            VK_IMAGE_USAGE_SAMPLED_BIT)
         (imageaspectflags VK_IMAGE_ASPECT_COLOR_BIT)))

   (define framebuffer-object (attachment scene)
      (renderpass
         (renderpass-description
            (renderpass-attachment
               (format attachment)
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
            attachment
            (framebuffer-attachment
               VK_FORMAT_D32_SFLOAT
               (imageusageflags VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
               (imageaspectflags VK_IMAGE_ASPECT_DEPTH_BIT)))
               
         scene))

   (define lod-renderpass (framebuffer-object
      (define lod-color-attachment 
         (framebuffer-attachment 
            VK_FORMAT_R8G8B8A8_UINT
            (imageusageflags 
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
               VK_IMAGE_USAGE_TRANSFER_SRC_BIT 
               VK_IMAGE_USAGE_SAMPLED_BIT)
            (imageaspectflags VK_IMAGE_ASPECT_COLOR_BIT)))
      (group
         (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
            #version 450

            layout(location = 0) in vec3 texCoord;
            layout(location = 0) out uvec4 FragColor;

            const vec3 textureSize = vec3(4096.0);
            const vec3 tileSize = vec3(64.0, 32.0, 32.0);

            float mipmapLevel(vec3 uv)
            {
               vec3 dx = dFdx(uv);
               vec3 dy = dFdy(uv);
               float d = max(dot(dx, dx), dot(dy, dy));
               return 0.5 * log2(d);
            }

            void main() 
            {
               float lod = mipmapLevel(texCoord * textureSize);

               uint mip = uint(lod);
               mip = uint(max(int(mip) - 3, 0));
               uvec3 ijk = uvec3(texCoord * (textureSize / tileSize));

               FragColor = uvec4(ijk.x >> mip, ijk.y >> mip, ijk.z >> mip, mip);
            }
         ]])

         (slice 0.0)
         (offscreen-image lod-color-attachment))))


   (define main-renderpass (framebuffer-object
      main-color-attachment
      (group
         (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
            #version 450

            layout(binding = 1) uniform sampler3D Texture;
            layout(location = 0) in vec3 texCoord;
            layout(location = 0) out vec4 FragColor;

            void main() {
               float r = texture(Texture, texCoord).r;
               FragColor = vec4(r, r, r, 1.0);
            }
         ]])

         (sparse-texture "")
         (slice 0.0))))

   (define scene 
      (group
         (viewmatrix 
            .5 .5 2 
            .2 .2 0 
            0 1 0)

         (projmatrix 1000 0.001 1.0 0.7)

         (texturematrix 0 0 0 1 1 1)
         (modelmatrix 0 0 0 1 1 1)

         (shader VK_SHADER_STAGE_VERTEX_BIT [[
            #version 450

            layout(std140, binding = 0) uniform Transform {
               mat4 ModelViewMatrix;
               mat4 ProjectionMatrix;
               mat4 TextureMatrix;
            };

            layout(location = 0) in vec3 Position;
            layout(location = 0) out vec3 texCoord;

            out gl_PerVertex {
               vec4 gl_Position;
            };

            void main() 
            {
               texCoord = vec3(TextureMatrix * vec4(Position, 1.0));
               gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(Position, 1.0);
            }
         ]])

         (separator 
            (extent (uint32 240) (uint32 135))
            lod-renderpass)
         (separator main-renderpass)))

   (define vulkan-window (window scene main-color-attachment))
      vulkan-window)
