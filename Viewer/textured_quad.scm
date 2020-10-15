(begin
   (import "main-renderpass.scm")
   (import "indexed-shape.scm")
   
   (define texture2d (filename)
      (group
         (sampler 
            VK_FILTER_LINEAR
            VK_FILTER_LINEAR
            VK_SAMPLER_MIPMAP_MODE_NEAREST
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

         (texturedata filename)
         (cpumemorybuffer (bufferusageflags VK_BUFFER_USAGE_TRANSFER_SRC_BIT))

         (textureimage
            VK_SAMPLE_COUNT_1_BIT
            VK_IMAGE_TILING_OPTIMAL
            (imageusageflags VK_IMAGE_USAGE_TRANSFER_DST_BIT VK_IMAGE_USAGE_SAMPLED_BIT)
            VK_SHARING_MODE_EXCLUSIVE
            (imagecreateflags)
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

   (window
      (main-renderpass
         (group
            (viewmatrix 
               (dvec3 10 2 6) 
               (dvec3 10 0 0) 
               (dvec3  0 1 0))

            (modelmatrix 
               (dvec3 10 0 0) 
               (dvec3 1 1 1))

            (projmatrix 1000 0.1 1.0 0.7)

            (texture2d "mipmap_7of7_levels.ktx")
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
