(window 
   (group
      (viewmatrix 
         (dvec3 0 0 3)
         (dvec3 0 0 0)
         (dvec3 0 1 0))

      (projmatrix 1000 0.1 1.0 0.7)

      (shader VK_SHADER_STAGE_RAYGEN_BIT [[
         #version 460
         #extension GL_EXT_ray_tracing : enable

         layout(location = 0) rayPayloadEXT vec3 hitValue;

         layout(binding = 0) uniform Transform {
            mat4 ModelViewMatrix;
            mat4 ProjectionMatrix;
            mat4 TextureMatrix;
         };

         layout(binding = 1, set = 0, rgba8) uniform image2D image;
         layout(binding = 2, set = 0) uniform accelerationStructureEXT topLevelAS;

         void main() 
         {
            // uv = normalized screen pos
            const vec2 uv = (gl_LaunchIDEXT.xy + vec2(0.5)) / gl_LaunchSizeEXT.xy;
            vec2 d = uv * 2 - 1;

            vec4 origin = inverse(ModelViewMatrix) * vec4(0, 0, 0, 1);
            vec4 target = inverse(ProjectionMatrix) * vec4(d, 1, 1);
            vec4 direction = inverse(ModelViewMatrix) * vec4(normalize(target.xyz), 0);

            hitValue = vec3(0.0);

            float tmin = 0.0;
            float tmax = 1000.0;

            traceRayEXT(
               topLevelAS,
               gl_RayFlagsOpaqueEXT,      // rayFlags
               0xff,                      // cullMask
               0,                         // sbtRecordOffset
               0,                         // sbtRecordStride
               0,                         // missIndex
               origin.xyz,
               tmin,
               direction.xyz,
               tmax,
               0);                        // hitValueLocation

            imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(hitValue, 0.0));
         }
      ]])

      (shader VK_SHADER_STAGE_MISS_BIT [[
         #version 460
         #extension GL_EXT_ray_tracing : enable

         layout(location = 0) rayPayloadInEXT vec3 hitValue;

         void main()
         {
            hitValue = vec3(0.0, 0.0, 0.5);
         }     
      ]])

      (shader VK_SHADER_STAGE_CLOSEST_HIT_BIT [[
         #version 460
         #extension GL_EXT_ray_tracing : enable

         layout(location = 0) rayPayloadInEXT vec3 hitValue;

         void main()
         {
            hitValue = vec3(0.5, 0.5, 0.0);
         }
      ]])

      (bufferdata-uint32 0 1 2)
      (cpumemorybuffer 
         (bufferusageflags 
         VK_BUFFER_USAGE_INDEX_BUFFER_BIT 
         VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT))

      (indexbufferdescription VK_INDEX_TYPE_UINT32)

      (bufferdata-float -1 -1 0  1 -1 0  1 1 0)
      (cpumemorybuffer 
         (bufferusageflags 
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT 
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT))

      (vertexinputattributedescription
         (uint32 0)
         (uint32 0)
         VK_FORMAT_R32G32B32_SFLOAT 
         (uint32 0))

      (vertexinputbindingdescription
         (uint32 0)
         (uint32 12)
         VK_VERTEX_INPUT_RATE_VERTEX)

      (bottom-level-acceleration-structure)

      (transformbuffer)
      (descriptorsetlayoutbinding 
         (uint32 0)
         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
         VK_SHADER_STAGE_RAYGEN_BIT)

      (rtxbuffer
         (framebuffer-attachment
            VK_FORMAT_B8G8R8A8_UNORM
            VK_IMAGE_LAYOUT_GENERAL
            (imageusageflags 
               VK_IMAGE_USAGE_TRANSFER_SRC_BIT
               VK_IMAGE_USAGE_STORAGE_BIT)
            (imageaspectflags VK_IMAGE_ASPECT_COLOR_BIT)))
 
      (descriptorsetlayoutbinding
         (uint32 1)
         VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
         VK_SHADER_STAGE_RAYGEN_BIT)

      (top-level-acceleration-structure)

      (descriptorsetlayoutbinding
         (uint32 2)
         VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE
         VK_SHADER_STAGE_RAYGEN_BIT)

      (raytracecommand)))
