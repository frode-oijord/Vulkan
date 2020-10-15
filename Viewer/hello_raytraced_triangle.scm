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
            const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
            const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeEXT.xy);
            vec2 d = inUV * 2.0 - 1.0;

            vec4 origin = inverse(ModelViewMatrix) * vec4(0,0,0,1);
            vec4 target = inverse(ProjectionMatrix) * vec4(d.x, d.y, 1, 1) ;
            vec4 direction = inverse(ModelViewMatrix) * vec4(normalize(target.xyz), 0) ;

            hitValue = vec3(0.0);

            uint rayFlags = 0xff;
            uint cullMask = 0;
            uint sbtRecordOffset = 0;
            uint sbtRecordStride = 0;
            uint missIndex = 0;
            float Tmin = 0.0;
            float Tmax = 10.0;
            const int hitValueLocation = 0;

            traceRayEXT(
               topLevelAS,
               gl_RayFlagsOpaqueEXT,
               rayFlags,
               sbtRecordOffset,
               sbtRecordStride,
               missIndex,
               origin.xyz,
               Tmin,
               direction.xyz,
               Tmax,
               hitValueLocation);

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
            hitValue = vec3(0.5, 0.5, 0);
         }
      ]])

      (bufferdata-uint32 0 1 2)
      (cpumemorybuffer 
         (bufferusageflags 
         VK_BUFFER_USAGE_INDEX_BUFFER_BIT 
         VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT))

      (indexbufferdescription VK_INDEX_TYPE_UINT32)

      (bufferdata-float -1 -1 0.5  1 -1 0.5  1 1 0.5)
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

      (transformbuffer)
      (descriptorsetlayoutbinding 
         (uint32 0)
         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
         VK_SHADER_STAGE_RAYGEN_BIT)

      (rtximage
         VK_IMAGE_TYPE_2D
         VK_FORMAT_B8G8R8A8_UNORM
         (extent3 256 256 1)
         (uint32 1)
         (uint32 1)
         VK_SAMPLE_COUNT_1_BIT
         VK_IMAGE_TILING_OPTIMAL
         (imageusageflags VK_IMAGE_USAGE_TRANSFER_SRC_BIT VK_IMAGE_USAGE_STORAGE_BIT)
         VK_SHARING_MODE_EXCLUSIVE
         (imagecreateflags)
         (memorypropertyflags VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
         VK_IMAGE_LAYOUT_GENERAL)

      (descriptorsetlayoutbinding
         (uint32 1)
         VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
         VK_SHADER_STAGE_RAYGEN_BIT)

      (acceleration-structure)
      (raytracecommand)))
