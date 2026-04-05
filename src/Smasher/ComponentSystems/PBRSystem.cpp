#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL.h>
#include "Smasher/Engine.h"
#include "Smasher/ComponentSystems/PBRSystem.h"
#include "Smasher/ComponentSystems/EngineSystem.h"
#include "Smasher/Base.h"
#include "Manifest.h"

namespace Smasher {
	namespace PBRSystem {
		struct StaticMeshInstance {
			glm::mat4 transform;
		};

		ErrorCode Initialize(entt::registry& registry, std::shared_ptr<SDL_GPUDeviceWrapper> gpu) {
			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			assert(registry.ctx().contains<EngineSystem::Context>() && "EngineSystem must be initialized before PBRSystem");
			assert(registry.ctx().contains<SDLSystem::Context>() && "SDLSystem must be initialized before PBRSystem");

			SDL_GPUDevice* device = gpu->Get();

			auto &ctx = registry.ctx().emplace<Context>();
			auto &sdlSystemCtx = registry.ctx().get<SDLSystem::Context>();

			// TODO: Save composition shader code to EngineConfig.h
			{
				auto& resourceManager = registry.ctx().get<EngineSystem::Context>().engineRef.get().GetResourceManager();
				ctx.depthPassFragShader = resourceManager.GetOrLoadResource<Manifest::Shaders::depth_pass_frag_shader, Smasher::SDLGraphicShaderResource>(gpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT);
				ctx.depthPassVertShader = resourceManager.GetOrLoadResource<Manifest::Shaders::depth_pass_vert_shader, Smasher::SDLGraphicShaderResource>(gpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX);
				ctx.testTeapotMeshResource = resourceManager.GetOrLoadResource<Manifest::Models::teapot, Smasher::StaticMeshResource>(gpu);
			}

			StaticMeshInstance teapotInstances[3] = {
				glm::mat4(1.0f),
				glm::mat4(1.0f),
				glm::mat4(1.0f),
			};
			//teapotInstances[0].transform = glm::translate(glm::mat4(0.3f), glm::vec3(0.f, 0.f, 0.4f));


			SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
			SDL_GPUTransferBuffer* transferBuffer = NULL;
			// Create Depth Texture
			{
				SDL_GPUTextureCreateInfo depthStencilTextureInfo{};
				int width, height;
				SDL_GetWindowSizeInPixels(sdlSystemCtx.window, &width, &height);
				depthStencilTextureInfo.format = SDLSystem::GetGPUDepthStencilFormat(device);
				depthStencilTextureInfo.height = height;
				depthStencilTextureInfo.width = width;
				depthStencilTextureInfo.layer_count_or_depth = 1;
				depthStencilTextureInfo.num_levels = 1;
				depthStencilTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
				depthStencilTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
				depthStencilTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
				ctx.gDepthPrePass = SDL_CreateGPUTexture(device, &depthStencilTextureInfo);
				SDL_SetGPUTextureName(device, ctx.gDepthPrePass, "Depth Pre Pass Texture");
			}

			// Create Instance buffer
			{
				size_t NUM_INSTANCES = 1;
				SDL_GPUBufferCreateInfo instanceDataInfo = {};
				instanceDataInfo.size = NUM_INSTANCES * sizeof(StaticMeshInstance);
				instanceDataInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
				ctx.testInstanceBuffer = SDL_CreateGPUBuffer(device, &instanceDataInfo);

				SDL_GPUTransferBufferCreateInfo transferBufferInfo{};
				transferBufferInfo.size = NUM_INSTANCES * sizeof(StaticMeshInstance);
				transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
				transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferInfo);

				SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

				void* data = SDL_MapGPUTransferBuffer(device, transferBuffer, false);

				// Upload Vertex Positions to Transfer Buffer
				SDL_memcpy(data, teapotInstances, NUM_INSTANCES * sizeof(StaticMeshInstance));

				SDL_GPUTransferBufferLocation location{};
				location.transfer_buffer = transferBuffer;
				location.offset = 0;

				SDL_GPUBufferRegion region{};
				region.buffer = ctx.testInstanceBuffer;
				region.size = NUM_INSTANCES * sizeof(StaticMeshInstance);
				region.offset = 0;

				SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
				SDL_UnmapGPUTransferBuffer(device, transferBuffer);

				SDL_EndGPUCopyPass(copyPass);

			}

			SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
			SDL_WaitForGPUFences(device, true, &fence, 1);
			SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

			// Create depth Prepass Pipeline
			{
				SDL_GPUGraphicsPipelineCreateInfo depthPassPipelineInfo{};

				// Vertex Buffers
				// Instance Data Buffer (slot 0)
				//		model matrix location(0-3)
				// Position (slot 1, location 4)
				// Normal	(slot 2, location 5)
				// UV Coord (slot 3, location 6)

				SDL_GPUVertexBufferDescription vertexBufferDesc[3];
				// Instance Buffer
				vertexBufferDesc[0] = {};
				vertexBufferDesc[0].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
				vertexBufferDesc[0].slot = 0;
				vertexBufferDesc[0].pitch = sizeof(StaticMeshInstance);
				vertexBufferDesc[0].instance_step_rate = 0;

				// Vertex Position Buffer
				vertexBufferDesc[1] = {};
				vertexBufferDesc[1].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
				vertexBufferDesc[1].slot = 1;
				vertexBufferDesc[1].pitch = sizeof(float) * 3;
				vertexBufferDesc[1].instance_step_rate = 0;

				// Vertex Normal Buffer
				vertexBufferDesc[2] = {};
				vertexBufferDesc[2].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
				vertexBufferDesc[2].slot = 2;
				vertexBufferDesc[2].pitch = sizeof(float) * 3;
				vertexBufferDesc[2].instance_step_rate = 0;


				SDL_GPUVertexAttribute vertexAttributes[6];
				// Model Transform
				vertexAttributes[0].buffer_slot = 0;
				vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
				vertexAttributes[0].location = 0;
				vertexAttributes[0].offset = 0 * sizeof(glm::vec4);
				vertexAttributes[1].buffer_slot = 0;
				vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
				vertexAttributes[1].location = 1;
				vertexAttributes[1].offset = 1 * sizeof(glm::vec4);
				vertexAttributes[2].buffer_slot = 0;
				vertexAttributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
				vertexAttributes[2].location = 2;
				vertexAttributes[2].offset = 2 * sizeof(glm::vec4);
				vertexAttributes[3].buffer_slot = 0;
				vertexAttributes[3].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
				vertexAttributes[3].location = 3;
				vertexAttributes[3].offset = 3 * sizeof(glm::vec4);

				// Vertex Position
				vertexAttributes[4].buffer_slot = 1;
				vertexAttributes[4].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
				vertexAttributes[4].location = 4;
				vertexAttributes[4].offset = 0;

				// Vertex Normal
				vertexAttributes[5].buffer_slot = 1;
				vertexAttributes[5].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
				vertexAttributes[5].location = 5;
				vertexAttributes[5].offset = 0;

				depthPassPipelineInfo.fragment_shader = ctx.depthPassFragShader->GetShader();
				depthPassPipelineInfo.vertex_shader = ctx.depthPassVertShader->GetShader();
				depthPassPipelineInfo.target_info.color_target_descriptions = NULL;
				depthPassPipelineInfo.target_info.num_color_targets = 0;
				depthPassPipelineInfo.target_info.has_depth_stencil_target = true;
				depthPassPipelineInfo.target_info.depth_stencil_format = SDLSystem::GetGPUDepthStencilFormat(device);
				depthPassPipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;
				depthPassPipelineInfo.vertex_input_state.num_vertex_attributes = 6;
				depthPassPipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDesc;
				depthPassPipelineInfo.vertex_input_state.num_vertex_buffers = 3;
				depthPassPipelineInfo.depth_stencil_state.enable_depth_test = true;
				depthPassPipelineInfo.depth_stencil_state.enable_depth_write = true;
				depthPassPipelineInfo.depth_stencil_state.enable_stencil_test = true;
				depthPassPipelineInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
				depthPassPipelineInfo.depth_stencil_state.write_mask = 0xFF;
				depthPassPipelineInfo.depth_stencil_state.front_stencil_state.compare_op = SDL_GPU_COMPAREOP_ALWAYS;
				depthPassPipelineInfo.depth_stencil_state.front_stencil_state.depth_fail_op = SDL_GPU_STENCILOP_KEEP;
				depthPassPipelineInfo.depth_stencil_state.front_stencil_state.pass_op = SDL_GPU_STENCILOP_REPLACE;
				depthPassPipelineInfo.depth_stencil_state.front_stencil_state.fail_op = SDL_GPU_STENCILOP_REPLACE;
				depthPassPipelineInfo.depth_stencil_state.back_stencil_state.compare_op = SDL_GPU_COMPAREOP_ALWAYS;
				depthPassPipelineInfo.depth_stencil_state.back_stencil_state.depth_fail_op = SDL_GPU_STENCILOP_KEEP;
				depthPassPipelineInfo.depth_stencil_state.back_stencil_state.pass_op = SDL_GPU_STENCILOP_REPLACE;
				depthPassPipelineInfo.depth_stencil_state.back_stencil_state.fail_op = SDL_GPU_STENCILOP_REPLACE;

				ctx.depthPassPipeline = SDL_CreateGPUGraphicsPipeline(device, &depthPassPipelineInfo);
			}

			return ERROR_NoError;
		};

		ErrorCode Teardown(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_NoError;
			}

			auto& ctx = registry.ctx().get<Context>();
			return ERROR_NoError;
		};

		ErrorCode OnWindowResize(Context& ctx, SDL_GPUDevice* device) {

			return ERROR_NoError;
		};

		struct DepthPassUBO
		{
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
		};

		ErrorCode DepthPrePass(Context& ctx, SDL_GPUDevice* device, const CameraSystem::Component& camera) {
			SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
			
			SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = {};
			depthStencilTargetInfo.clear_depth = 1.f;
			depthStencilTargetInfo.clear_stencil = 0;
			depthStencilTargetInfo.cycle = true;
			depthStencilTargetInfo.layer = 0;
			depthStencilTargetInfo.mip_level = 0;
			depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
			depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
			depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
			depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;
			depthStencilTargetInfo.texture = ctx.gDepthPrePass;

			DepthPassUBO ubo = {
				.projectionMatrix = CameraSystem::GetProjectionMatrix(camera),
				.viewMatrix = CameraSystem::GetViewMatrix(camera)
			};

			SDL_PushGPUVertexUniformData(commandBuffer, 0, &ubo, sizeof(ubo));
			Uint8 materialID = 1;
			for (int i = 0; i < 1; ++i) {
				SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, NULL, 0, &depthStencilTargetInfo);
				SDL_BindGPUGraphicsPipeline(renderPass, ctx.depthPassPipeline);
				SDL_SetGPUStencilReference(renderPass, 64);
				
				SDL_GPUBuffer *instanceBuffer = ctx.testInstanceBuffer;

				SDL_GPUBufferBinding indexBuffer = {};
				indexBuffer.buffer = ctx.testTeapotMeshResource->GetIndexBuffer();
				indexBuffer.offset = 0;

				SDL_GPUBufferBinding vertexBuffers[2];
				vertexBuffers[0].buffer = instanceBuffer;
				vertexBuffers[0].offset = 0;
				vertexBuffers[1].buffer = ctx.testTeapotMeshResource->GetVertexPositionBuffer();
				vertexBuffers[1].offset = 0;

				Uint32 numIndices = ctx.testTeapotMeshResource->GetNumIndices();
				Uint32 numInstances = 1;
				SDL_BindGPUIndexBuffer(renderPass, &indexBuffer, SDL_GPUIndexElementSize::SDL_GPU_INDEXELEMENTSIZE_32BIT);

				SDL_BindGPUVertexBuffers(renderPass, 0, vertexBuffers, 2);
				for (int j = 0; j < 1; j++) {
					SDL_DrawGPUIndexedPrimitives(renderPass, numIndices, numInstances, 0, 0, 0);
				}
				materialID++;
				SDL_EndGPURenderPass(renderPass);
			}

			SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
			SDL_WaitForGPUFences(device, true, &fence, 1);
			return ERROR_NoError;
		};

		ErrorCode LightingPass(Context& ctx, SDL_GPUDevice* device) {

			return ERROR_NoError;
		};

		ErrorCode ShadowPass(Context& ctx, SDL_GPUDevice* device, const CameraSystem::Component& camera) {

			return ERROR_NoError;
		};

		ErrorCode MaterialsPass(Context& ctx, SDL_GPUDevice* device) {

			return ERROR_NoError;
		};
	}
}
