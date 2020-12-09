(begin
   (import "indexed-shape.scm")
   (import "create-renderpass.scm")

   (define slice (y) 
      (indexed-shape 
         (bufferdata-uint32 0 3 2 2 1 0)
         (bufferdata-float 
            0 y 0
            1 y 0
            1 y 1
            0 y 1)
         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST))

   (define cube () 
      (indexed-shape 
         (bufferdata-uint32 
            0 1 1 3 3 2 2 0
            4 5 5 7 7 6 6 4
            0 4 1 5 2 6 3 7)
         (bufferdata-float 
            0 0 0
            0 0 0.125
            0 1 0
            0 1 0.125
            1 0 0
            1 0 0.125
            1 1 0
            1 1 0.125)
         VK_PRIMITIVE_TOPOLOGY_LINE_LIST))

   (window
      (extent2 1920 1080)
      (separator
         (viewmatrix 
            (dvec3 0 2 0)
            (dvec3 0 0 0)
            (dvec3 0 0 1))

         (projmatrix 1000 0.001 1.0 0.7)

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
            (extent (uint32 240) (uint32 136))
            (modelmatrix (dvec3 0 0 0) (dvec3 1 1 0.125))
            (texturematrix (dvec3 0 0 0) (dvec3 1 1 1))
            (create-renderpass
               VK_FORMAT_R8G8B8A8_UINT
               (group
                  (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
                     #version 450

                     layout(location = 0) in vec3 texCoord;
                     layout(location = 0) out uvec4 FragColor;

                     const vec3 textureSize = vec3(16384, 16384, 2048);
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

                        FragColor = uvec4(ijk.x >> mip, ijk.y >> mip, ijk.z >> mip, mip + 1);
                     }
                  ]])
               (slice 0.5)))
            (offscreen-image))

         (separator 
            (create-renderpass
               VK_FORMAT_B8G8R8A8_UNORM
               (group
                  (separator
                     (modelmatrix (dvec3 0 0 0) (dvec3 1 1 0.125))
                     (texturematrix (dvec3 0 0 0) (dvec3 1 1 1))

                     (sparsetextureimage 
                        (uint32 1)
                        (shaderstageflags VK_SHADER_STAGE_FRAGMENT_BIT)
                        VK_FILTER_LINEAR
                        VK_SAMPLER_MIPMAP_MODE_LINEAR
                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
                        "bricked16384.dat")

                     (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
                        #version 450

                        layout(binding = 1) uniform sampler3D Texture;
                        layout(location = 0) in vec3 texCoord;
                        layout(location = 0) out vec4 FragColor;

                        void main() {
                           FragColor = texture(Texture, texCoord);
                        }
                     ]])
                     (slice 0.5))

                  (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
                     #version 450
                     layout(location = 0) in vec3 texCoord;
                     layout(location = 0) out vec4 FragColor;
                     void main() {
                        FragColor = vec4(texCoord, 1.0);
                     }
                  ]])

                  (cube)))))))
