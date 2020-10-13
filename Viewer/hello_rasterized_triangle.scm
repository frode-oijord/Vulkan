(begin
   (import "vertex-shape.scm")
   (import "main-renderpass.scm")

   (window 
      (group
         (main-renderpass 
            (group
               (viewmatrix 
                  (dvec3 0 2 6)
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

            (vertex-shape
               (bufferdata-float -1 -1 0  1 -1 0  1 1 0))))
            (swapchain VK_PRESENT_MODE_FIFO_KHR))))
