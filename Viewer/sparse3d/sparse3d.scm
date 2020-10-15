(begin
   (import "indexed-shape.scm")
   (import "sparse-texture.scm")
   (import "main-renderpass.scm")

   (define slice (z) 
      (indexed-shape 
         (bufferdata-uint32 0 1 2 2 3 0)
         (bufferdata-float 0 0 z  1 0 z  1 1 z  0 1 z)))

   (define lod-renderpass
      (group
         (renderpass
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
               (define lod-color-attachment 
                  (framebuffer-attachment 
                     VK_FORMAT_R8G8B8A8_UINT
                     (imageusageflags 
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT 
                        VK_IMAGE_USAGE_SAMPLED_BIT)
                     (imageaspectflags VK_IMAGE_ASPECT_COLOR_BIT)))

               (framebuffer-attachment
                  VK_FORMAT_D32_SFLOAT
                  (imageusageflags VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                  (imageaspectflags VK_IMAGE_ASPECT_DEPTH_BIT)))

               (group
                  (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
                     #version 450

                     layout(location = 0) in vec3 texCoord;
                     layout(location = 0) out uvec4 FragColor;

                     const vec3 textureSize = vec3(256.0);
                     const vec3 tileSize = vec3(32.0, 32.0, 16.0);

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

                        FragColor = uvec4(ijk.x >> mip, ijk.y >> mip, ijk.z >> mip, mip + 1);
                     }
                  ]])

               (slice 0)
               (offscreen-image lod-color-attachment)))))

   (window
      (group
         (viewmatrix 
            (dvec3 .5 .5  2)
            (dvec3 .2 .2  0)
            (dvec3  0  1  0))

         (projmatrix 1000 0.001 1.0 0.7)
         (texturematrix (dvec3 0 0 0) (dvec3 1 1 1))
         (modelmatrix (dvec3 0 0 0) (dvec3 1 1 1))

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
         (separator 
            (main-renderpass
               (group
                  (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
                     #version 450

                     layout(binding = 1) uniform sampler3D Texture;
                     layout(location = 0) in vec3 texCoord;
                     layout(location = 0) out vec4 FragColor;

                     void main() {
                        FragColor = texture(Texture, texCoord);
                     }
                  ]])

                  (sparse-texture "sparse3d/noise256.bin")
                  (slice 0)))))))

