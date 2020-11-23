(begin
   (import "main-renderpass.scm")
   (import "indexed-shape.scm")
   
   (window
      (extent2 1920 1080)
      (main-renderpass
         (group
            (viewmatrix 
               (dvec3 10 2 6) 
               (dvec3 10 0 0) 
               (dvec3  0 1 0))

            (modelmatrix 
               (dvec3 10 0 0) 
               (dvec3 1 1 1))

            (projmatrix 1000 0.01 1.0 0.7)

            (textureimage 
               (uint32 1)
               (shaderstageflags VK_SHADER_STAGE_FRAGMENT_BIT)
               VK_FILTER_LINEAR
               VK_SAMPLER_MIPMAP_MODE_LINEAR
               VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
               "world.ktx")
               
            (shader VK_SHADER_STAGE_VERTEX_BIT [[
               #version 450

               layout(std140, binding = 0) uniform Transform {
                  mat4 ModelViewMatrix;
                  mat4 ProjectionMatrix;
                  mat4 TextureMatrix;
               };

               layout(location = 0) in vec3 Position;
               layout(location = 0) out vec2 texCoord;

               out gl_PerVertex {
                  vec4 gl_Position;
               };

               void main() 
               {
                  texCoord = Position.xy * 0.5 + 0.5;
                  gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(Position, 1.0);
               }
            ]])
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
               (bufferdata-float -1 -1 0  1 -1 0  1 1 0  -1 1 0))))))
