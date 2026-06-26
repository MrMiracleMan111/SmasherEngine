#include <iostream>
#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include "Smasher/ComponentSystems/SDLSystem.h"
#include "Smasher/ComponentSystems/EngineSystem.h"
#include "Smasher/Engine.h"
#include "Smasher/ResourceManager.h"
#include "Smasher/Resources.h"
#include "Manifest.h"

namespace Smasher {
	namespace SDLSystem {

		SDL_GPUTextureFormat GetGPURGBAFormat(SDL_GPUDevice* device) {
			return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
		}

		SDL_GPUTextureFormat GetGPUDepthFormat(SDL_GPUDevice* device) {
			// Create depth texture
			SDL_GPUTextureFormat depthFormat;
			if (SDL_GPUTextureSupportsFormat(device, SDL_GPU_TEXTUREFORMAT_D32_FLOAT, SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
				depthFormat = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
			}
			else if (SDL_GPUTextureSupportsFormat(device, SDL_GPU_TEXTUREFORMAT_D24_UNORM, SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
				depthFormat = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
			}
			else {
				assert(false && "Could not find suitable depth format");
			}
			return depthFormat;
		}

		SDL_GPUTextureFormat GetGPUDepthStencilFormat(SDL_GPUDevice* device) {
			// Create depth texture
			SDL_GPUTextureFormat depthFormat;
			if (SDL_GPUTextureSupportsFormat(device, SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT, SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
				depthFormat = SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
			}
			else if (SDL_GPUTextureSupportsFormat(device, SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT, SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
				depthFormat = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
			}
			else {
				assert(false && "Could not find suitable depth format");
			}
			return depthFormat;
		}

		ErrorCode Initialize(entt::registry& registry, const WindowOptions& opts) {
			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			Context &ctx = registry.ctx().emplace<Context>();

			SDL_PropertiesID gpuProps = SDL_CreateProperties();

			/**
			 * 
			 *	VkStructureType             sType;
			 *	const void*                 pNext;
			 *	VkInstanceCreateFlags       flags;
			 *	const VkApplicationInfo*    pApplicationInfo;
			 *	uint32_t                    enabledLayerCount;
			 *	const char* const*          ppEnabledLayerNames;
			 *	uint32_t                    enabledExtensionCount;
			 *	const char* const*          ppEnabledExtensionNames;
			 * .
			 */


			VkInstance instance; 
			const VkApplicationInfo appInfo{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pNext = NULL,
				.pApplicationName = "",
				.applicationVersion = 0,
				.pEngineName = "",
				.engineVersion = 0,
				.apiVersion = VK_API_VERSION_1_3
			};
			const VkInstanceCreateInfo instanceInfo{
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pNext = NULL,
				.flags = VkInstanceCreateFlagBits::VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
				.pApplicationInfo = &appInfo,
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = NULL,
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = NULL
			};
			VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
			std::cout << "Create Instance Result: " << string_VkResult(result) << std::endl;
			VkPhysicalDevice vkDevice;
			uint32_t vkDeviceCount;

			result = vkEnumeratePhysicalDevices(instance, &vkDeviceCount, &vkDevice);

			VkPhysicalDeviceFeatures features10{}; // features of versions = Vulkan 1.0
			VkPhysicalDeviceFeatures2 features11_2_3{}; // features of versions >= Vulkan 1.1
			VkPhysicalDeviceVulkan11Features vulk11Features{};
			VkPhysicalDeviceVulkan12Features vulk12Features{};
			VkPhysicalDeviceVulkan13Features vulk13Features{};

			vulk11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
			vulk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			vulk13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
			vulk11Features.pNext = &vulk12Features;
			vulk12Features.pNext = &vulk13Features;
			vulk13Features.pNext = NULL;

			features11_2_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features11_2_3.pNext = &vulk11Features;

			vkGetPhysicalDeviceFeatures(vkDevice, &features10);
			vkGetPhysicalDeviceFeatures2(vkDevice, &features11_2_3);

			uint32_t numInstanceExtension = 0;
			uint32_t numDeviceExtension = 0;
			std::vector<const char *> instanceExtNames;
			std::vector<const char *> deviceExtNames;

			result = vkEnumerateInstanceExtensionProperties(NULL, &numInstanceExtension, NULL);
			std::vector<VkExtensionProperties> instanceExtensions(numInstanceExtension);
			result = vkEnumerateInstanceExtensionProperties(NULL, &numInstanceExtension, instanceExtensions.data());

			for (const auto& itr : instanceExtensions) {
				instanceExtNames.push_back(itr.extensionName);
			}

			vkEnumerateDeviceExtensionProperties(vkDevice, NULL, &numDeviceExtension, NULL);
			std::vector<VkExtensionProperties> deviceExtensions(numDeviceExtension);
			vkEnumerateDeviceExtensionProperties(vkDevice, NULL, &numDeviceExtension, deviceExtensions.data());

			for (const auto& itr : deviceExtensions) {
				deviceExtNames.push_back(itr.extensionName);
			}

			SDL_GPUVulkanOptions vulkanOpts
			{
				.vulkan_api_version = VK_API_VERSION_1_3, /**< The Vulkan API version to request for the instance. Use Vulkan's VK_MAKE_VERSION or VK_MAKE_API_VERSION. */
				.feature_list = &features11_2_3, /**< Pointer to the first element of a chain of Vulkan feature structs. (Requires API version 1.1 or higher.)*/
				.vulkan_10_physical_device_features = &features10, /**< Pointer to a VkPhysicalDeviceFeatures struct to enable additional Vulkan 1.0 features. */
				.device_extension_count = numDeviceExtension, /**< Number of additional device extensions to require. */
				.device_extension_names = deviceExtNames.data(), /**< Pointer to a list of additional device extensions to require. */
				.instance_extension_count = numInstanceExtension, /**< Number of additional instance extensions to require. */
				.instance_extension_names = instanceExtNames.data() /**< Pointer to a list of additional instance extensions to require. */
			};


			SDL_SetPointerProperty(gpuProps, SDL_PROP_GPU_DEVICE_CREATE_VULKAN_OPTIONS_POINTER, &vulkanOpts);
			SDL_SetBooleanProperty(gpuProps, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, true);
			SDL_SetBooleanProperty(gpuProps, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN, true);
			SDL_SetBooleanProperty(gpuProps, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_DXIL_BOOLEAN, false);
			SDL_SetBooleanProperty(gpuProps, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_DXBC_BOOLEAN, false);
			SDL_SetBooleanProperty(gpuProps, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_CLIP_DISTANCE_BOOLEAN, true);
			SDL_SetBooleanProperty(gpuProps, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_DEPTH_CLAMPING_BOOLEAN, true);
			SDL_SetBooleanProperty(gpuProps, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_INDIRECT_DRAW_FIRST_INSTANCE_BOOLEAN, true);
			SDL_SetBooleanProperty(gpuProps, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_ANISOTROPY_BOOLEAN, true);
			SDL_SetBooleanProperty(gpuProps, SDL_PROP_GPU_DEVICE_CREATE_VULKAN_REQUIRE_HARDWARE_ACCELERATION_BOOLEAN, true);

			SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
			SDL_GPUDevice* device = SDL_CreateGPUDeviceWithProperties(gpuProps);
			vkDestroyInstance(instance, NULL);
			//SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);

			if (device == NULL) {
				std::cerr << "Error Creating GPU: " << SDL_GetError() << std::endl;
				return ERROR_EngineFailedtoInitializeGPU;
			}

			bool featureSupportCheck = SDL_GPUTextureSupportsFormat(device, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_SIMULTANEOUS_READ_WRITE);
			assert(featureSupportCheck && "Your GPU does NOT support simultaneous read write on the compute shader");

			ctx.pGpu = std::make_shared<SDL_GPUDeviceWrapper>(device);
			ctx.window = SDL_CreateWindow(opts.title, opts.width, opts.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
			ctx.windowSize = { opts.width, opts.height };
			SDL_ClaimWindowForGPUDevice(ctx.pGpu->Get(), ctx.window);

			// TODO: Save composition shader code to EngineConfig.h
			{
				auto& resourceManager = registry.ctx().get<EngineSystem::Context>().engineRef.get().GetResourceManager();
				ctx.compositeFragShader = resourceManager.GetOrLoadResource<Manifest::Shaders::composition_frag_shader, Smasher::SDLGraphicShaderResource>(ctx.pGpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT);
				ctx.compositeVertShader = resourceManager.GetOrLoadResource<Manifest::Shaders::composition_vert_shader, Smasher::SDLGraphicShaderResource>(ctx.pGpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX);
				ctx.compositeCopyFragShader = resourceManager.GetOrLoadResource<Manifest::Shaders::composition_copy_frag_shader, Smasher::SDLGraphicShaderResource>(ctx.pGpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT);
				ctx.compositeCopyVertShader = resourceManager.GetOrLoadResource<Manifest::Shaders::composition_copy_vert_shader, Smasher::SDLGraphicShaderResource>(ctx.pGpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX);
			}

			// Create Vertex Buffer
			{
				SDL_GPUBufferCreateInfo bufferInfo{};
				bufferInfo.size = sizeof(SCREEN_QUAD);
				bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
				ctx.screenQuadVertBuffer = SDL_CreateGPUBuffer(device, &bufferInfo);
			}

			// Create GPU Texture Sampler for composite pipeline
			{
				SDL_GPUSamplerCreateInfo samplerInfo{};
				samplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;                 /**< The minification filter to apply to lookups. */
				samplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;                 /**< The magnification filter to apply to lookups. */
				samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;				/**< The mipmap filter to apply to lookups. */
				samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;				/**< The addressing mode for U coordinates outside [0, 1). */
				samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;				/**< The addressing mode for V coordinates outside [0, 1). */
				samplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;				/**< The addressing mode for W coordinates outside [0, 1). */
				samplerInfo.mip_lod_bias = 0.f;               /**< The bias to be added to mipmap LOD calculation. */
				samplerInfo.min_lod = 0.f;                    /**< Clamps the minimum of the computed LOD value. */
				samplerInfo.max_lod = 0.f;                    /**< Clamps the maximum of the computed LOD value. */
				samplerInfo.enable_anisotropy = false;          /**< true to enable anisotropic filtering. */
				samplerInfo.enable_compare = false;             /**< true to enable comparison against a reference value during lookups. */
				samplerInfo.props = 0;

				ctx.compositeSampler = SDL_CreateGPUSampler(device, &samplerInfo);
			}

			// Create G-Buffer Depth Texture
			{
				int width, height;
				SDL_GetWindowSizeInPixels(ctx.window, &width, &height);
				SDL_GPUTextureCreateInfo depthTextureInfo = {};
				depthTextureInfo.format = GetGPUDepthStencilFormat(device);
				depthTextureInfo.width = width;
				depthTextureInfo.height = height;
				depthTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
				depthTextureInfo.layer_count_or_depth = 1;
				depthTextureInfo.type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D;
				depthTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
				depthTextureInfo.num_levels = 1;
				ctx.gDepthTexture = SDL_CreateGPUTexture(device, &depthTextureInfo);
				ctx._gDepthIntermediateTexture = SDL_CreateGPUTexture(device, &depthTextureInfo);

				SDL_SetGPUTextureName(device, ctx.gDepthTexture, "G-Depth Texture");
				SDL_SetGPUTextureName(device, ctx._gDepthIntermediateTexture, "G-Depth Intermediate Texture");
			}

			// Create G-Buffer Color Texture
			{
				int width, height;
				SDL_GetWindowSizeInPixels(ctx.window, &width, &height);
				SDL_GPUTextureCreateInfo colorTextureInfo = {};
				colorTextureInfo.format = SDL_GetGPUSwapchainTextureFormat(device, ctx.window);
				colorTextureInfo.width = width;
				colorTextureInfo.height = height;
				colorTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
				colorTextureInfo.layer_count_or_depth = 1;
				colorTextureInfo.type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D;
				colorTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
				colorTextureInfo.num_levels = 1;
				ctx.gColorTexture = SDL_CreateGPUTexture(device, &colorTextureInfo);
				ctx._gColorIntermediateTexture = SDL_CreateGPUTexture(device, &colorTextureInfo);

				SDL_SetGPUTextureName(device, ctx.gColorTexture, "G-Color Texture");
				SDL_SetGPUTextureName(device, ctx._gColorIntermediateTexture, "G-Color Intermediate Texture");
			}

			// Fill Screen Quad Vertex Buffer
			// Clear G-Buffers
			{
				// Create Transfer Buffer
				SDL_GPUTransferBufferCreateInfo transferInfo{};
				transferInfo.size = sizeof(SCREEN_QUAD);
				transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
				SDL_GPUTransferBuffer *transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);

				SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);

				// Copy from Transfer Buffer to Vertex Buffer
				SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

				// Upload to Transfer Buffer
				QuadVertex* data = (QuadVertex*)SDL_MapGPUTransferBuffer(device, transferBuffer, false);
				SDL_memcpy(data, SCREEN_QUAD, sizeof(SCREEN_QUAD));
				SDL_UnmapGPUTransferBuffer(device, transferBuffer);

				SDL_GPUTransferBufferLocation location{};
				location.transfer_buffer = transferBuffer;
				location.offset = 0;

				SDL_GPUBufferRegion region{};
				region.buffer = ctx.screenQuadVertBuffer;
				region.size = sizeof(SCREEN_QUAD);
				region.offset = 0;

				SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
				SDL_EndGPUCopyPass(copyPass);


				SDL_GPUColorTargetInfo colorTargetInfos[2];
				colorTargetInfos[0].clear_color = { 0.f, 1.f, 0.f, 1.f };
				colorTargetInfos[0].cycle = false;
				colorTargetInfos[0].cycle_resolve_texture = false;
				colorTargetInfos[0].layer_or_depth_plane = 0;
				colorTargetInfos[0].load_op = SDL_GPU_LOADOP_CLEAR;
				colorTargetInfos[0].store_op = SDL_GPU_STOREOP_STORE;
				colorTargetInfos[0].resolve_texture = NULL;
				colorTargetInfos[0].mip_level = 0;
				colorTargetInfos[0].texture = ctx.gColorTexture;

				colorTargetInfos[1].clear_color = { 0.f, 1.f, 0.f, 1.f };
				colorTargetInfos[1].cycle = false;
				colorTargetInfos[1].cycle_resolve_texture = false;
				colorTargetInfos[1].layer_or_depth_plane = 0;
				colorTargetInfos[1].load_op = SDL_GPU_LOADOP_CLEAR;
				colorTargetInfos[1].store_op = SDL_GPU_STOREOP_STORE;
				colorTargetInfos[1].resolve_texture = NULL;
				colorTargetInfos[1].mip_level = 0;
				colorTargetInfos[1].texture = ctx._gColorIntermediateTexture;

				SDL_GPUDepthStencilTargetInfo depthTargetInfos[2];
				depthTargetInfos[0].clear_depth = 1.f;
				depthTargetInfos[0].clear_stencil = 0;
				depthTargetInfos[0].cycle = false;
				depthTargetInfos[0].layer = 0;
				depthTargetInfos[0].mip_level = 0;
				depthTargetInfos[0].load_op = SDL_GPU_LOADOP_CLEAR;
				depthTargetInfos[0].store_op = SDL_GPU_STOREOP_STORE;
				depthTargetInfos[0].stencil_load_op = SDL_GPU_LOADOP_CLEAR;
				depthTargetInfos[0].stencil_store_op = SDL_GPU_STOREOP_STORE;
				depthTargetInfos[0].texture = ctx.gDepthTexture;

				depthTargetInfos[1].clear_depth = 1.f;
				depthTargetInfos[1].clear_stencil = 0;
				depthTargetInfos[1].cycle = false;
				depthTargetInfos[1].layer = 0;
				depthTargetInfos[1].mip_level = 0;
				depthTargetInfos[1].load_op = SDL_GPU_LOADOP_CLEAR;
				depthTargetInfos[1].store_op = SDL_GPU_STOREOP_STORE;
				depthTargetInfos[1].stencil_load_op = SDL_GPU_LOADOP_CLEAR;
				depthTargetInfos[1].stencil_store_op = SDL_GPU_STOREOP_STORE;
				depthTargetInfos[1].texture = ctx._gDepthIntermediateTexture;
				SDL_GPURenderPass* renderPass1 = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfos[0], 1, &depthTargetInfos[0]);
				SDL_EndGPURenderPass(renderPass1);
				SDL_GPURenderPass* renderPass2 = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfos[1], 1, &depthTargetInfos[1]);
				SDL_EndGPURenderPass(renderPass2);


				SDL_GPUFence *fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
				SDL_WaitForGPUFences(device, true, &fence, 1);
				SDL_ReleaseGPUFence(device, fence);
			}

			// Create Composite pipeline
			{
				SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};

				// Color Target
				SDL_GPUColorTargetDescription colorTargetDescriptions[1];
				colorTargetDescriptions[0] = {};
				colorTargetDescriptions[0].blend_state.enable_blend = true;
				colorTargetDescriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
				colorTargetDescriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
				colorTargetDescriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				colorTargetDescriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				colorTargetDescriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				colorTargetDescriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(device, ctx.window);

				// Vertex Buffer Description (Slot 0)
				SDL_GPUVertexBufferDescription vertexBufferDescriptions[1];
				vertexBufferDescriptions[0].slot = 0;
				vertexBufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
				vertexBufferDescriptions[0].instance_step_rate = 0;
				vertexBufferDescriptions[0].pitch = sizeof(QuadVertex);

				SDL_GPUVertexAttribute vertexAttributeDescriptions[2];
				// a_position
				vertexAttributeDescriptions[0].buffer_slot = 0;								// fetch data from buffer at slot 0
				vertexAttributeDescriptions[0].location = 0;								// layout (location = 0)
				vertexAttributeDescriptions[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; // vec3
				vertexAttributeDescriptions[0].offset = 0;

				// a_uv coordinates
				vertexAttributeDescriptions[1].buffer_slot = 0;								// fetch data from buffer at slot 0
				vertexAttributeDescriptions[1].location = 1;								// layout (location = 0)
				vertexAttributeDescriptions[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2; // vec3
				vertexAttributeDescriptions[1].offset = sizeof(float) * 3;
				
				pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
				pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDescriptions;
				pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
				pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributeDescriptions;

				pipelineInfo.target_info.color_target_descriptions = colorTargetDescriptions;
				pipelineInfo.target_info.num_color_targets = 1;
				pipelineInfo.target_info.has_depth_stencil_target = true;
				pipelineInfo.target_info.depth_stencil_format = GetGPUDepthStencilFormat(device);
				pipelineInfo.depth_stencil_state.enable_depth_test = true;
				pipelineInfo.depth_stencil_state.enable_depth_write = true;
				pipelineInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_ALWAYS;

				pipelineInfo.fragment_shader = ctx.compositeFragShader->GetShader();
				pipelineInfo.vertex_shader = ctx.compositeVertShader->GetShader();

				ctx.compositePipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
			}

			// Create Composite Copy pipeline
			{
				SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};

				// G-Buffer Depth target
				SDL_GPUColorTargetDescription colorTargetDescriptions[1];
				colorTargetDescriptions[0] = {};
				colorTargetDescriptions[0].blend_state.enable_blend = true;
				colorTargetDescriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
				colorTargetDescriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
				colorTargetDescriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				colorTargetDescriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				colorTargetDescriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				colorTargetDescriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(device, ctx.window);

				// Vertex Buffer Description (Slot 0)
				SDL_GPUVertexBufferDescription vertexBufferDescriptions[1];
				vertexBufferDescriptions[0].slot = 0;
				vertexBufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
				vertexBufferDescriptions[0].instance_step_rate = 0;
				vertexBufferDescriptions[0].pitch = sizeof(QuadVertex);

				SDL_GPUVertexAttribute vertexAttributeDescriptions[2];
				// a_position
				vertexAttributeDescriptions[0].buffer_slot = 0;								// fetch data from buffer at slot 0
				vertexAttributeDescriptions[0].location = 0;								// layout (location = 0)
				vertexAttributeDescriptions[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; // vec3
				vertexAttributeDescriptions[0].offset = 0;

				// a_uv coordinates
				vertexAttributeDescriptions[1].buffer_slot = 0;								// fetch data from buffer at slot 0
				vertexAttributeDescriptions[1].location = 1;								// layout (location = 0)
				vertexAttributeDescriptions[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2; // vec3
				vertexAttributeDescriptions[1].offset = sizeof(float) * 3;

				pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
				pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDescriptions;
				pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
				pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributeDescriptions;

				pipelineInfo.target_info.color_target_descriptions = colorTargetDescriptions;
				pipelineInfo.target_info.num_color_targets = 1;
				pipelineInfo.target_info.has_depth_stencil_target = true;
				pipelineInfo.target_info.depth_stencil_format = GetGPUDepthStencilFormat(device);
				pipelineInfo.depth_stencil_state.enable_depth_test = true;
				pipelineInfo.depth_stencil_state.enable_depth_write = true;
				pipelineInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_ALWAYS;

				pipelineInfo.fragment_shader = ctx.compositeCopyFragShader->GetShader();
				pipelineInfo.vertex_shader = ctx.compositeCopyVertShader->GetShader();

				ctx.compositeCopyPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
			}

			
			return ERROR_NoError;
		}

		ErrorCode Teardown(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_SystemNotInitialized;
			}

			Context& ctx = registry.ctx().get<Context>();
			SDL_GPUDevice *device = *ctx.pGpu;

			SDL_ReleaseGPUBuffer(device, ctx.screenQuadVertBuffer);
			SDL_ReleaseGPUSampler(device, ctx.compositeSampler);
			SDL_ReleaseGPUTexture(device, ctx.gColorTexture);
			SDL_ReleaseGPUTexture(device, ctx.gDepthTexture);
			SDL_ReleaseGPUTexture(device, ctx._gColorIntermediateTexture);
			SDL_ReleaseGPUTexture(device, ctx._gDepthIntermediateTexture);
			SDL_ReleaseGPUGraphicsPipeline(device, ctx.compositePipeline);
			SDL_ReleaseGPUGraphicsPipeline(device, ctx.compositeCopyPipeline);
			SDL_ReleaseWindowFromGPUDevice(device, ctx.window);
			SDL_DestroyWindow(ctx.window);
			return ERROR_NoError;
		}

		ErrorCode CopyToWindow(Context &ctx, SDL_GPUTexture *texture) {
			SDL_GPUDevice* device = *ctx.pGpu;
			SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);

			SDL_GPUTexture* swapChainTexture = NULL;
			unsigned int width, height;
			if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, ctx.window, &swapChainTexture, &width, &height)) {
				std::cerr << "Error acquiring GPU swapchain texture: " << SDL_GetError() << std::endl;
				SDL_ClearError();
			}

			if (swapChainTexture == NULL) {
				SDL_SubmitGPUCommandBuffer(commandBuffer);
				return ERROR_NoError;
			}

			SDL_GPUBlitInfo blitInfo = {};

			blitInfo.cycle = true;
			blitInfo.load_op = SDL_GPU_LOADOP_LOAD;
			blitInfo.filter = SDL_GPU_FILTER_LINEAR;
			blitInfo.flip_mode = SDL_FLIP_NONE;

			blitInfo.destination.x = 0;
			blitInfo.destination.y = 0;
			blitInfo.destination.w = width;
			blitInfo.destination.h = height;
			blitInfo.destination.mip_level = 0;
			blitInfo.destination.layer_or_depth_plane = 0;
			blitInfo.destination.texture = swapChainTexture;

			blitInfo.source.x = 0;
			blitInfo.source.y = 0;
			blitInfo.source.w = width;
			blitInfo.source.h = height;
			blitInfo.source.mip_level = 0;
			blitInfo.source.layer_or_depth_plane = 0;
			blitInfo.source.texture = texture;

			SDL_BlitGPUTexture(commandBuffer, &blitInfo);

			SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
			SDL_WaitForGPUFences(device, true, &fence, 1);
			SDL_ReleaseGPUFence(device, fence);

			return ERROR_NoError;
		}

		ErrorCode CompositionPass(Context &ctx, const std::vector<RenderTexture> &sources) {
			SDL_GPUDevice *device = *ctx.pGpu;
			SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);

			int width, height;
			SDL_GetWindowSizeInPixels(ctx.window, &width, &height);

			SDL_GPUColorTargetInfo colorTargetInfos[1];
			// Color Target
			colorTargetInfos[0].clear_color = { 40.f / 255.f, 40.f / 255.f, 40.f / 255.f, 255.f / 255.f };
			colorTargetInfos[0].load_op = SDL_GPU_LOADOP_LOAD;
			colorTargetInfos[0].store_op = SDL_GPU_STOREOP_STORE;
			colorTargetInfos[0].resolve_texture = NULL;
			colorTargetInfos[0].resolve_mip_level = 0;
			colorTargetInfos[0].resolve_layer = 0;
			colorTargetInfos[0].cycle = false;
			colorTargetInfos[0].cycle_resolve_texture = false;
			colorTargetInfos[0].layer_or_depth_plane = 0;
			colorTargetInfos[0].mip_level = 0;

			SDL_GPUDepthStencilTargetInfo depthTargetInfo{};
			depthTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
			depthTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
			depthTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
			depthTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;
			depthTargetInfo.clear_depth = 0.9f;
			depthTargetInfo.clear_stencil = 0;

			bool clear = true;

			SDL_GPUTexture *lastColorTexture;
			SDL_GPUTexture *lastDepthTexture;
			for (size_t i = 0; i < sources.size(); i++) {
				clear = (i == 0 || i == 1);

				bool isEvenIndex = (i % 2 == 0);
				SDL_GPUTexture* gColorTarget = (isEvenIndex) ? ctx._gColorIntermediateTexture : ctx.gColorTexture;
				SDL_GPUTexture* gDepthTarget = (isEvenIndex) ? ctx._gDepthIntermediateTexture : ctx.gDepthTexture;
				SDL_GPUTexture* gColorSample = (isEvenIndex) ? ctx.gColorTexture : ctx._gColorIntermediateTexture;
				SDL_GPUTexture* gDepthSample = (isEvenIndex) ? ctx.gDepthTexture : ctx._gDepthIntermediateTexture;

				lastColorTexture = gColorTarget;
				lastDepthTexture = gDepthTarget;

				auto& source = sources[i];
				colorTargetInfos[0].load_op = (clear) ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
				colorTargetInfos[0].texture = gColorTarget;

				depthTargetInfo.load_op = (clear) ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
				depthTargetInfo.texture = gDepthTarget;

				SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, colorTargetInfos, 1, &depthTargetInfo);
				SDL_BindGPUGraphicsPipeline(renderPass, ctx.compositePipeline);

				SDL_GPUTextureSamplerBinding textureSamplerBindings[4];
				// Source Color 
				textureSamplerBindings[0].sampler = ctx.compositeSampler;
				textureSamplerBindings[0].texture = source.colorTarget;
				// Source Depth
				textureSamplerBindings[1].sampler = ctx.compositeSampler;
				textureSamplerBindings[1].texture = source.depthTarget;
				// Swapchain Texture
				textureSamplerBindings[2].sampler = ctx.compositeSampler;
				textureSamplerBindings[2].texture = gColorSample;
				// G-Buffer Depth Texture
				textureSamplerBindings[3].sampler = ctx.compositeSampler;
				textureSamplerBindings[3].texture = gDepthSample;
				
				SDL_BindGPUFragmentSamplers(renderPass, 0, textureSamplerBindings, 4);

				SDL_GPUBufferBinding bufferBindings[1];
				bufferBindings[0].buffer = ctx.screenQuadVertBuffer;
				bufferBindings[0].offset = 0;

				SDL_BindGPUVertexBuffers(renderPass, 0, bufferBindings, 1);

				SDL_DrawGPUPrimitives(renderPass, 6, 1, 0, 0);

				SDL_EndGPURenderPass(renderPass);
			}

			// Copy to final composition to ctx.gColorTexture and ctx.gDepthTexture
			// since only _gColorIntermediateTexture and _gDepthIntermediateTexture are
			// up to date currently.
			if ((sources.size() % 2) != 0) {
				bool isEvenIndex = false;
				SDL_GPUTexture* gColorTarget = ctx.gColorTexture;
				SDL_GPUTexture* gDepthTarget = ctx.gDepthTexture;
				SDL_GPUTexture* gColorSample = ctx._gColorIntermediateTexture;
				SDL_GPUTexture* gDepthSample = ctx._gDepthIntermediateTexture;

				colorTargetInfos[0].load_op = SDL_GPU_LOADOP_LOAD;
				colorTargetInfos[0].texture = gColorTarget;

				depthTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
				depthTargetInfo.texture = gDepthTarget;

				SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, colorTargetInfos, 1, &depthTargetInfo);
				SDL_BindGPUGraphicsPipeline(renderPass, ctx.compositeCopyPipeline);

				SDL_GPUTextureSamplerBinding textureSamplerBindings[2];
				// Source Color 
				textureSamplerBindings[0].sampler = ctx.compositeSampler;
				textureSamplerBindings[0].texture = gColorSample;
				// Source Depth
				textureSamplerBindings[1].sampler = ctx.compositeSampler;
				textureSamplerBindings[1].texture = gDepthSample;

				SDL_BindGPUFragmentSamplers(renderPass, 0, textureSamplerBindings, 2);

				SDL_GPUBufferBinding bufferBindings[1];
				bufferBindings[0].buffer = ctx.screenQuadVertBuffer;
				bufferBindings[0].offset = 0;

				SDL_BindGPUVertexBuffers(renderPass, 0, bufferBindings, 1);

				SDL_DrawGPUPrimitives(renderPass, 6, 1, 0, 0);

				SDL_EndGPURenderPass(renderPass);
			}

			SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
			SDL_WaitForGPUFences(device, true, &fence, 1);
			SDL_ReleaseGPUFence(device, fence);
			return ERROR_NoError;
		}
	}
}
