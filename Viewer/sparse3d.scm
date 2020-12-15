(begin
   (import "stl-shape.scm")
   (import "indexed-shape.scm")
   (import "create-renderpass.scm")

   (define xslice (y) 
      (indexed-shape 
         (bufferdata-uint32 0 3 2 2 1 0)
         (bufferdata-float 
            y 0 0
            y 1 0
            y 1 1
            y 0 1)
         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST))

   (define yslice (y) 
      (indexed-shape 
         (bufferdata-uint32 0 3 2 2 1 0)
         (bufferdata-float 
            0 y 0
            1 y 0
            1 y 1
            0 y 1)
         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST))

   (define zslice (z) 
      (indexed-shape 
         (bufferdata-uint32 0 3 2 2 1 0)
         (bufferdata-float 
            0 0 z
            1 0 z
            1 1 z
            0 1 z)
         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST))

   (define cube () 
      (indexed-shape 
         (bufferdata-uint32 
            0 1 1 3 3 2 2 0
            4 5 5 7 7 6 6 4
            0 4 1 5 2 6 3 7)
         (bufferdata-float 
            0 0 0
            0 0 2
            0 4 0
            0 4 2
            1 0 0
            1 0 2
            1 4 0
            1 4 2)
         VK_PRIMITIVE_TOPOLOGY_LINE_LIST))

   (window
      (extent2 1920 1080)
      (separator
         (viewmatrix 
            (dvec3 0 5 0)
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
            layout(location = 1) out vec4 viewPos;

            out gl_PerVertex {
               vec4 gl_Position;
            };

            void main() 
            {
               texCoord = vec3(TextureMatrix * vec4(Position, 1.0));
               viewPos = ModelViewMatrix * vec4(Position, 1.0);
               gl_Position = ProjectionMatrix * viewPos;
            }
         ]])

         (separator 
            (extent (uint32 240) (uint32 136))
            (modelmatrix (dvec3 25 50 0) (dvec3 .02 .02 .02))
            (texturematrix (dvec3 25 50 0) (dvec3 .02 .005 .01))

            (create-renderpass
               VK_FORMAT_R8G8B8A8_UINT
               (group
                  (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
                     #version 450

                     layout(location = 0) in vec3 texCoord;
                     layout(location = 0) out uvec4 FragColor;

                     const vec3 textureSize = vec3(1024, 4096, 2048);
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
                  (stl-shape)
                  ))
            (offscreen-image))

         (separator 
            (create-renderpass
               VK_FORMAT_B8G8R8A8_UNORM
               (group
                  (separator
                     (modelmatrix (dvec3 25 50 0) (dvec3 .02 .02 .02))
                     (texturematrix (dvec3 25 50 0) (dvec3 .02 .005 .01))

                     (sparsetextureimage 
                        (uint32 1)
                        (shaderstageflags VK_SHADER_STAGE_FRAGMENT_BIT)
                        VK_FILTER_LINEAR
                        VK_SAMPLER_MIPMAP_MODE_LINEAR
                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
                        "D:/zgy/beagle.dat")

                     (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
                        #version 450

                        layout(binding = 1) uniform sampler3D Texture;
                        layout(location = 0) in vec3 texCoord;
                        layout(location = 1) in vec4 viewPos;
                        layout(location = 0) out vec4 FragColor;

                        void main() {
                           vec3 dx = dFdx(viewPos.xyz);
                           vec3 dy = dFdy(viewPos.xyz);
                           vec3 n = normalize(cross(dx, dy));

                           vec3 color = texture(Texture, texCoord).rrr;
                           FragColor = vec4(n.z * color * 0.5 + 0.5, 1.0);
                        }
                     ]])

                     (stl-shape)
                     )

                  (separator
                     (shader VK_SHADER_STAGE_FRAGMENT_BIT [[
                        #version 450
                        layout(location = 0) in vec3 texCoord;
                        layout(location = 0) out vec4 FragColor;
                        void main() {
                           FragColor = vec4(texCoord, 1.0);
                        }
                     ]])
                     (cube))))))))
