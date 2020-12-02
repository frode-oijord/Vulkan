(begin
   (import "main-renderpass.scm")
   (import "indexed-shape.scm")

   (define lod-renderpass (renderpass
      (renderpass-description
         (renderpass-attachment
            VK_FORMAT_R8G8B8A8_UINT
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
            VK_FORMAT_R8G8B8A8_UINT
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

      (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
         #version 450

         layout(location = 0) in vec2 texCoord;
         layout(location = 0) out uvec4 FragColor;

         const vec2 textureSize = vec2(8192.0);
         const vec2 tileSize = vec2(128.0);

         float mipmapLevel(vec2 uv)
         {
            vec2 dx = dFdx(uv);
            vec2 dy = dFdy(uv);
            float d = max(dot(dx, dx), dot(dy, dy));
            return 0.5 * log2(d);
         }

         void main() 
         {
            float lod = mipmapLevel(texCoord * textureSize);

            uint mip = uint(lod);
            mip = uint(max(int(mip) - 4, 0));
            uvec2 ij = uvec2(texCoord * (textureSize / tileSize));

            FragColor = uvec4(ij.x >> mip, ij.y >> mip, 0, mip + 1);
         }
      ]])

      (indexed-shape 
         (bufferdata-uint32 0 1 2 2 3 0)
         (bufferdata-float 0 0 0.5  1 0 0.5  1 1 0.5  0 1 0.5))))

   (window 
      (extent2 1920 1080)
      (group
         (viewmatrix 
            (dvec3 0.5 0.5 3)
            (dvec3 0 0 0)
            (dvec3 0 1 0))

         (projmatrix 1000 0.001 1.0 0.7)

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

         (separator 
            (extent (uint32 120) (uint32 68))
            lod-renderpass
            (offscreen-image))

         (separator 
            (main-renderpass
               (group
                  (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
                     #version 450

                     layout(binding = 1) uniform sampler2D Texture;
                     layout(location = 0) in vec2 texCoord;
                     layout(location = 0) out vec4 FragColor;

                     void main() {
                        FragColor = texture(Texture, texCoord);
                     }
                  ]])

                  (sparsetextureimage 
                     (uint32 1)
                     (shaderstageflags VK_SHADER_STAGE_FRAGMENT_BIT)
                     VK_FILTER_LINEAR
                     VK_SAMPLER_MIPMAP_MODE_LINEAR
                     VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
                     "world.bin")

                  (indexed-shape 
                     (bufferdata-uint32 0 1 2 2 3 0)
                     (bufferdata-float 0 0 0.5  1 0 0.5  1 1 0.5  0 1 0.5))))))))

