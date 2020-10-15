(begin
   (import "main-renderpass.scm")

   (window 
      (main-renderpass 
         (group
            (viewmatrix 
               (dvec3 0 0 3)
               (dvec3 0 0 0)
               (dvec3 0 1 0))

            (projmatrix 1000 0.1 1.0 0.7)

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

               layout(location = 0) in vec2 texCoord;
               layout(location = 0) out vec4 FragColor;

               void main() {
                  FragColor = vec4(texCoord, 0, 1);
               }
            ]])

         (bufferdata-float -1 -1 0  1 -1 0  1 1 0)
         (cpumemorybuffer
            (bufferusageflags VK_BUFFER_USAGE_VERTEX_BUFFER_BIT))

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
               (uint32 3)
               (uint32 1)
               (uint32 0)
               (uint32 0)
               VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)))))
