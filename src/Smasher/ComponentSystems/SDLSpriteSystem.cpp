#include "Smasher/ComponentSystems/SDLSpriteSystem.h"
#include "Smasher/ComponentSystems/SDLSystem.h"

namespace Smasher {
	namespace SDLSpriteSystem {
		SMASHER_API ErrorCode Initialize(
			entt::registry& registry,
			std::shared_ptr<SDL_GPUDeviceWrapper> pGPU,
			std::shared_ptr<SDLGraphicShaderResource> pFrag,
			std::shared_ptr<SDLGraphicShaderResource> pVert,
			SDL_Window *window) {

			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			SDL_GPUDevice* device = *pGPU;
			auto& ctx = registry.ctx().emplace<Context>();
			ctx.pGPU = pGPU;
			ctx.pFragShaderResource = pFrag;
			ctx.pVertShaderResource = pVert;
			ctx.window = window;

			// Create render texture
			assert(SDL_GPUTextureSupportsFormat(device, SDL_GetGPUSwapchainTextureFormat(device, ctx.window), SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_READ) && "Unsupported format + usage");
			int width, height;
			SDL_GetWindowSizeInPixels(ctx.window, &width, &height);
			SDL_GPUTextureCreateInfo renderTextureInfo = {};
			renderTextureInfo.format = SDL_GetGPUSwapchainTextureFormat(device, ctx.window);
			renderTextureInfo.width = width;
			renderTextureInfo.height = height;
			renderTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
			renderTextureInfo.layer_count_or_depth = 1;
			renderTextureInfo.type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D;
			renderTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
			renderTextureInfo.num_levels = 1;
			ctx.renderTexture = SDL_CreateGPUTexture(*pGPU, &renderTextureInfo);
			SDL_SetGPUTextureName(device, ctx.renderTexture, "Sprite System Color Texture");

			// Create depth texture
			SDL_GPUTextureCreateInfo depthTextureInfo = {};
			depthTextureInfo.format = SDLSystem::GetGPUDepthFormat(device);
			depthTextureInfo.width = width;
			depthTextureInfo.height = height;
			depthTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
			depthTextureInfo.layer_count_or_depth = 1;
			depthTextureInfo.type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D;
			depthTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
			depthTextureInfo.num_levels = 1;
			ctx.depthTexture = SDL_CreateGPUTexture(device, &depthTextureInfo);
			SDL_SetGPUTextureName(device, ctx.depthTexture, "Sprite System Depth Texture");

			SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};
			pipelineInfo.vertex_shader	 = pVert->GetShader();
			pipelineInfo.fragment_shader = pFrag->GetShader();
			pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

			// Create Vertex Buffer
			SDL_GPUBufferCreateInfo bufferInfo{};
			bufferInfo.size = sizeof(TEST_vertices);
			bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
			ctx.vertexBuffer = SDL_CreateGPUBuffer(device, &bufferInfo);

			// Create Transfer Buffer
			SDL_GPUTransferBufferCreateInfo transferInfo{};
			transferInfo.size = sizeof(TEST_vertices);
			transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
			ctx.transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);

			// Vertex Buffer Description (Slot 0)
			SDL_GPUVertexBufferDescription vertexBufferDescriptions[1];
			vertexBufferDescriptions[0].slot = 0;
			vertexBufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
			vertexBufferDescriptions[0].instance_step_rate = 0;
			vertexBufferDescriptions[0].pitch = sizeof(Vertex);

			SDL_GPUVertexAttribute vertexAttributeDescriptions[2];
			// a_position
			vertexAttributeDescriptions[0].buffer_slot = 0;								// fetch data from buffer at slot 0
			vertexAttributeDescriptions[0].location = 0;								// layout (location = 0)
			vertexAttributeDescriptions[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; // vec3
			vertexAttributeDescriptions[0].offset = 0;									// 

			// a_color
			vertexAttributeDescriptions[1].buffer_slot = 0;								// fetch data from buffer at slot 1
			vertexAttributeDescriptions[1].location = 1;								// layout (location = 1)
			vertexAttributeDescriptions[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4; // vec4
			vertexAttributeDescriptions[1].offset = sizeof(float) * 3;					// 

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

			pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
			pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDescriptions;
			pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
			pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributeDescriptions;
			
			pipelineInfo.target_info.num_color_targets = 1;
			pipelineInfo.target_info.color_target_descriptions = colorTargetDescriptions;
			pipelineInfo.target_info.has_depth_stencil_target = true;
			pipelineInfo.target_info.depth_stencil_format = depthTextureInfo.format;
			
			pipelineInfo.depth_stencil_state.enable_depth_write = true;
			pipelineInfo.depth_stencil_state.enable_depth_test = true;
			pipelineInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;

			ctx.renderPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);

			return ERROR_NoError;
		}

		SMASHER_API ErrorCode Teardown(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_NoError;
			}

			auto& ctx = registry.ctx().get<Context>();
			SDL_GPUDevice* device = *ctx.pGPU;

			SDL_ReleaseGPUTexture(device, ctx.renderTexture);
			SDL_ReleaseGPUTexture(device, ctx.depthTexture);
			SDL_ReleaseGPUGraphicsPipeline(device, ctx.renderPipeline);
			SDL_ReleaseGPUBuffer(device, ctx.vertexBuffer);
			SDL_ReleaseGPUTransferBuffer(device, ctx.transferBuffer);

			return ERROR_NoError;
		}

		ErrorCode CopyPass(Context& ctx, SDL_GPUCommandBuffer *commandBuffer) {
			assert(commandBuffer != NULL);

			SDL_GPUDevice * device = *ctx.pGPU;

			// Copy from Transfer Buffer to Vertex Buffer
			SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

			// Upload to Transfer Buffer
			Vertex* data = (Vertex*)SDL_MapGPUTransferBuffer(device, ctx.transferBuffer, false);
			SDL_memcpy(data, TEST_vertices, sizeof(TEST_vertices));
			SDL_UnmapGPUTransferBuffer(device, ctx.transferBuffer);

			SDL_GPUTransferBufferLocation location{};
			location.transfer_buffer = ctx.transferBuffer;
			location.offset = 0;

			SDL_GPUBufferRegion region{};
			region.buffer = ctx.vertexBuffer;
			region.size = sizeof(TEST_vertices);
			region.offset = 0;

			SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
			SDL_EndGPUCopyPass(copyPass);

			return ERROR_NoError;
		}

		ErrorCode DrawTriangle(Context& ctx) {
			SDL_GPUDevice *device = *ctx.pGPU;
			SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);
			

			ErrorCode ret = CopyPass(ctx, commandBuffer);

			SDL_GPUColorTargetInfo colorTargetInfo{};
			colorTargetInfo.clear_color = { 240.f / 255.f, 240.f / 255.f, 240.f / 255.f, 255.f / 255.f };
			colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
			colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
			colorTargetInfo.texture = ctx.renderTexture;
			colorTargetInfo.resolve_texture = NULL;
			colorTargetInfo.cycle = false;
			colorTargetInfo.mip_level = 0;
			colorTargetInfo.layer_or_depth_plane = 0;
			colorTargetInfo.cycle_resolve_texture = false;

			SDL_GPUDepthStencilTargetInfo depthTargetInfo{};
			depthTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
			depthTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
			depthTargetInfo.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
			depthTargetInfo.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
			depthTargetInfo.clear_depth = 1.f;
			depthTargetInfo.clear_stencil = 0;
			depthTargetInfo.texture = ctx.depthTexture;
			depthTargetInfo.mip_level = 0;

			SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, &depthTargetInfo);

			SDL_BindGPUGraphicsPipeline(renderPass, ctx.renderPipeline);

			SDL_GPUBufferBinding bufferBindings[1];
			bufferBindings[0].buffer = ctx.vertexBuffer;
			bufferBindings[0].offset = 0;

			SDL_BindGPUVertexBuffers(renderPass, 0, bufferBindings, 1);

			SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

			SDL_EndGPURenderPass(renderPass);

			SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
			SDL_WaitForGPUFences(device, true, &fence, 1);

			return ERROR_NoError;
		}

		void OnWindowResize(const Smasher::Events::WindowResizeEvent& event, entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return;	// System not initialized
			}

			auto& ctx = registry.ctx().get<Context>();
			SDL_GPUDevice* device = *ctx.pGPU;

			//SDL_ReleaseGPUTexture(device, ctx.renderTexture);
			//SDL_ReleaseGPUTexture(device, ctx.depthTexture);

			//// Create render texture
			//SDL_GPUTextureCreateInfo renderTextureInfo = {};
			//renderTextureInfo.format = SDL_GetGPUSwapchainTextureFormat(device, ctx.window);
			//renderTextureInfo.width = event.WindowSize.x;
			//renderTextureInfo.height = event.WindowSize.y;
			//renderTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
			//renderTextureInfo.layer_count_or_depth = 1;
			//renderTextureInfo.type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D;
			//renderTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
			//renderTextureInfo.num_levels = 1;
			//ctx.renderTexture = SDL_CreateGPUTexture(device, &renderTextureInfo);

			//// Create depth texture
			//SDL_GPUTextureCreateInfo depthTextureInfo = {};
			//depthTextureInfo.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
			//depthTextureInfo.width = event.WindowSize.x;
			//depthTextureInfo.height = event.WindowSize.y;
			//depthTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
			//depthTextureInfo.layer_count_or_depth = 1;
			//depthTextureInfo.type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D;
			//depthTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
			//depthTextureInfo.num_levels = 1;
			//ctx.depthTexture = SDL_CreateGPUTexture(device, &depthTextureInfo);
		}
	}
}
