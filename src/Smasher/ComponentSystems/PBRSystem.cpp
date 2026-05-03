#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL.h>
#include "Smasher/Engine.h"
#include "Smasher/ComponentSystems/StaticMeshSystem.h"
#include "Smasher/ComponentSystems/PBRSystem.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ComponentSystems/EngineSystem.h"
#include "Smasher/Util/GraphicsUtil.h"
#include "Smasher/Base.h"
#include "Manifest.h"

namespace Smasher {
	namespace PBRSystem {
		ErrorCode Initialize(entt::registry& registry, std::shared_ptr<SDL_GPUDeviceWrapper> gpu) {
			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			assert(registry.ctx().contains<EngineSystem::Context>() && "EngineSystem must be initialized before PBRSystem");
			assert(registry.ctx().contains<SDLSystem::Context>() && "SDLSystem must be initialized before PBRSystem");

			auto &ctx = registry.ctx().emplace<Context>();
			auto &sdlSystemCtx = registry.ctx().get<SDLSystem::Context>();
			ctx.gpu = gpu;

			// Pool with 512 batches (MAX)
			ctx.staticMeshBatchPool = GPUBlockPool<Smasher::StaticMeshSystem::StaticMeshData>(gpu, Smasher::StaticMeshSystem::StaticMeshBatch::MAX_MODEL_COUNT, 512);

			// TODO: Save composition shader code to EngineConfig.h
			{
				auto& resourceManager = registry.ctx().get<EngineSystem::Context>().engineRef.get().GetResourceManager();
				ctx.depthPassFragShader = resourceManager.GetOrLoadResource<Manifest::Shaders::depth_pass_frag_shader, Smasher::SDLGraphicShaderResource>(gpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT);
				ctx.depthPassVertShader = resourceManager.GetOrLoadResource<Manifest::Shaders::depth_pass_vert_shader, Smasher::SDLGraphicShaderResource>(gpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX);
				ctx.testTeapotMeshResource = resourceManager.GetOrLoadResource<Manifest::Models::suzanne, Smasher::StaticMeshResource>(gpu);
			}

			//StaticMeshInstance teapotInstances[3] = {
			//	glm::mat4(1.0f),
			//	glm::mat4(1.0f),
			//	glm::mat4(1.0f),
			//};
			//teapotInstances[0].transform = glm::translate(glm::mat4(0.3f), glm::vec3(0.f, 0.f, 0.4f));

			// Create 512 x 512 Texture array for static meshes
			{
				SDL_GPUTextureCreateInfo abledoTexturesInfo{};
				abledoTexturesInfo.format = ALBEDO_TEX_FORMAT;
				abledoTexturesInfo.height = 512;
				abledoTexturesInfo.width = 512;
				abledoTexturesInfo.layer_count_or_depth = MAX_STATIC_MESH_MATERIALS;
				abledoTexturesInfo.num_levels = 1;
				abledoTexturesInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
				abledoTexturesInfo.type = SDL_GPU_TEXTURETYPE_2D_ARRAY;
				abledoTexturesInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
				ctx.staticMeshAlbedos512 = SDL_CreateGPUTexture(ctx.gpu->Get(), &abledoTexturesInfo);
				SDL_SetGPUTextureName(ctx.gpu->Get(), ctx.staticMeshAlbedos512, "512x512 Albedo Texture Atlas");
			}

			int width, height;
			SDL_GetWindowSizeInPixels(sdlSystemCtx.window, &width, &height);

			SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(ctx.gpu->Get());
			SDL_GPUTransferBuffer* transferBuffer = NULL;

			// Create Static Mesh Transfer Buffer
			{
				static const size_t BLOCK_SIZE_BYTES = sizeof(StaticMeshSystem::StaticMeshData) * StaticMeshSystem::StaticMeshBatch::MAX_MODEL_COUNT;
				SDL_GPUTransferBufferCreateInfo transferBufferInfo{};
				transferBufferInfo.size = BLOCK_SIZE_BYTES;
				transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
				ctx.staticMeshTransferBuffer = SDL_CreateGPUTransferBuffer(ctx.gpu->Get(), &transferBufferInfo);
			}

			// Create Depth Texture
			{
				SDL_GPUTextureCreateInfo depthStencilTextureInfo{};
				depthStencilTextureInfo.format = SDLSystem::GetGPUDepthStencilFormat(ctx.gpu->Get());
				depthStencilTextureInfo.height = height;
				depthStencilTextureInfo.width = width;
				depthStencilTextureInfo.layer_count_or_depth = 1;
				depthStencilTextureInfo.num_levels = 1;
				depthStencilTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
				depthStencilTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
				depthStencilTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
				ctx.gDepthPrePass = SDL_CreateGPUTexture(ctx.gpu->Get(), &depthStencilTextureInfo);
				SDL_SetGPUTextureName(ctx.gpu->Get(), ctx.gDepthPrePass, "Depth Pre Pass Texture");
			}

			// Create Normals Buffer
			{
				SDL_GPUTextureCreateInfo normalsTextureInfo{};
				normalsTextureInfo.format = NORMALS_TEX_FORMAT;
				normalsTextureInfo.height = height;
				normalsTextureInfo.width = width;
				normalsTextureInfo.layer_count_or_depth = 1;
				normalsTextureInfo.num_levels = 1;
				normalsTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
				normalsTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
				normalsTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
				ctx.gNormals = SDL_CreateGPUTexture(ctx.gpu->Get(), &normalsTextureInfo);
				SDL_SetGPUTextureName(ctx.gpu->Get(), ctx.gNormals, "Normals Texture");
			}

			// Create UV Buffer
			{
				SDL_GPUTextureCreateInfo uvTextureInfo{};
				uvTextureInfo.format = UV_TEX_FORMAT;
				uvTextureInfo.height = height;
				uvTextureInfo.width = width;
				uvTextureInfo.layer_count_or_depth = 1;
				uvTextureInfo.num_levels = 1;
				uvTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
				uvTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
				uvTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
				ctx.gUV = SDL_CreateGPUTexture(ctx.gpu->Get(), &uvTextureInfo);
				SDL_SetGPUTextureName(ctx.gpu->Get(), ctx.gUV, "UV Texture");
			}

			// Create Samplers
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

				ctx.gDepthPrePassSampler = SDL_CreateGPUSampler(ctx.gpu->Get(), &samplerInfo);
				ctx.gNormalsSampler = SDL_CreateGPUSampler(ctx.gpu->Get(), &samplerInfo);
				ctx.gUVSampler = SDL_CreateGPUSampler(ctx.gpu->Get(), &samplerInfo);
				ctx.gAlbedoSampler = SDL_CreateGPUSampler(ctx.gpu->Get(), &samplerInfo);
				ctx.gSpecularSampler = SDL_CreateGPUSampler(ctx.gpu->Get(), &samplerInfo);
				ctx.gLightingSampler = SDL_CreateGPUSampler(ctx.gpu->Get(), &samplerInfo);
			}

			// Create Instance buffer
			/* {
				size_t NUM_INSTANCES = 1;
				SDL_GPUBufferCreateInfo instanceDataInfo = {};
				instanceDataInfo.size = (Uint32)(NUM_INSTANCES * (Uint32)sizeof(StaticMeshInstance));
				instanceDataInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
				ctx.testInstanceBuffer = SDL_CreateGPUBuffer(ctx.gpu->Get(), &instanceDataInfo);

				SDL_GPUTransferBufferCreateInfo transferBufferInfo{};
				transferBufferInfo.size = (Uint32)(NUM_INSTANCES * sizeof(StaticMeshInstance));
				transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
				transferBuffer = SDL_CreateGPUTransferBuffer(ctx.gpu->Get(), &transferBufferInfo);

				SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

				void* data = SDL_MapGPUTransferBuffer(ctx.gpu->Get(), transferBuffer, false);

				// Upload Vertex Positions to Transfer Buffer
				SDL_memcpy(data, teapotInstances, NUM_INSTANCES * sizeof(StaticMeshInstance));

				SDL_GPUTransferBufferLocation location{};
				location.transfer_buffer = transferBuffer;
				location.offset = 0;

				SDL_GPUBufferRegion region{};
				region.buffer = ctx.testInstanceBuffer;
				region.size = (Uint32)(NUM_INSTANCES * sizeof(StaticMeshInstance));
				region.offset = 0;

				SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
				SDL_UnmapGPUTransferBuffer(ctx.gpu->Get(), transferBuffer);

				SDL_EndGPUCopyPass(copyPass);

			}
			*/
			SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
			SDL_WaitForGPUFences(ctx.gpu->Get(), true, &fence, 1);
			SDL_ReleaseGPUFence(ctx.gpu->Get(), fence);
			SDL_ReleaseGPUTransferBuffer(ctx.gpu->Get(), transferBuffer);

			// Create depth Prepass Pipeline
			{
				SDL_GPUGraphicsPipelineCreateInfo depthPassPipelineInfo{};

				SDL_GPUColorTargetDescription gTargetDescriptions[2];
				// G-Buffer Depth target
				gTargetDescriptions[0] = {};
				gTargetDescriptions[0].blend_state.enable_blend = false;
				gTargetDescriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
				gTargetDescriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
				gTargetDescriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				gTargetDescriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				gTargetDescriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				gTargetDescriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				gTargetDescriptions[0].format = NORMALS_TEX_FORMAT;

				gTargetDescriptions[1] = {};
				gTargetDescriptions[1].blend_state.enable_blend = false;
				gTargetDescriptions[1].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
				gTargetDescriptions[1].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
				gTargetDescriptions[1].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				gTargetDescriptions[1].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				gTargetDescriptions[1].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				gTargetDescriptions[1].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				gTargetDescriptions[1].format = UV_TEX_FORMAT;

				// Vertex Buffers
				// Instance Data Buffer (slot 0)
				//		model matrix location(0-3)
				// Position (slot 1, location 4)
				// Normal	(slot 2, location 5)
				// UV Coord (slot 3, location 6)

				SDL_GPUVertexBufferDescription vertexBufferDesc[4];
				// Instance Buffer
				vertexBufferDesc[0] = {};
				vertexBufferDesc[0].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
				vertexBufferDesc[0].slot = 0;
				vertexBufferDesc[0].pitch = sizeof(StaticMeshSystem::StaticMeshData);
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

				// Vertex UV Buffer
				vertexBufferDesc[3] = {};
				vertexBufferDesc[3].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
				vertexBufferDesc[3].slot = 3;
				vertexBufferDesc[3].pitch = sizeof(float) * 2;
				vertexBufferDesc[3].instance_step_rate = 0;


				SDL_GPUVertexAttribute vertexAttributes[7];
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
				vertexAttributes[5].buffer_slot = 2;
				vertexAttributes[5].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
				vertexAttributes[5].location = 5;
				vertexAttributes[5].offset = 0;

				// Vertex UV
				vertexAttributes[6].buffer_slot = 3;
				vertexAttributes[6].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
				vertexAttributes[6].location = 6;
				vertexAttributes[6].offset = 0;

				depthPassPipelineInfo.fragment_shader = ctx.depthPassFragShader->GetShader();
				depthPassPipelineInfo.vertex_shader = ctx.depthPassVertShader->GetShader();
				depthPassPipelineInfo.target_info.color_target_descriptions = gTargetDescriptions;
				depthPassPipelineInfo.target_info.num_color_targets = 2;
				depthPassPipelineInfo.target_info.has_depth_stencil_target = true;
				depthPassPipelineInfo.target_info.depth_stencil_format = SDLSystem::GetGPUDepthStencilFormat(ctx.gpu->Get());
				depthPassPipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;
				depthPassPipelineInfo.vertex_input_state.num_vertex_attributes = 7;
				depthPassPipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDesc;
				depthPassPipelineInfo.vertex_input_state.num_vertex_buffers = 4;
				depthPassPipelineInfo.depth_stencil_state.enable_depth_test = true;
				depthPassPipelineInfo.depth_stencil_state.enable_depth_write = true;
				depthPassPipelineInfo.depth_stencil_state.enable_stencil_test = true;
				depthPassPipelineInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS; // SDL_GPU_COMPAREOP_ALWAYS;
				depthPassPipelineInfo.depth_stencil_state.write_mask = 0xFF;
				depthPassPipelineInfo.depth_stencil_state.front_stencil_state.compare_op = SDL_GPU_COMPAREOP_ALWAYS;
				depthPassPipelineInfo.depth_stencil_state.front_stencil_state.depth_fail_op = SDL_GPU_STENCILOP_KEEP;
				depthPassPipelineInfo.depth_stencil_state.front_stencil_state.pass_op = SDL_GPU_STENCILOP_REPLACE;
				depthPassPipelineInfo.depth_stencil_state.front_stencil_state.fail_op = SDL_GPU_STENCILOP_REPLACE;
				depthPassPipelineInfo.depth_stencil_state.back_stencil_state.compare_op = SDL_GPU_COMPAREOP_NEVER;
				depthPassPipelineInfo.depth_stencil_state.back_stencil_state.depth_fail_op = SDL_GPU_STENCILOP_KEEP;
				depthPassPipelineInfo.depth_stencil_state.back_stencil_state.pass_op = SDL_GPU_STENCILOP_REPLACE;
				depthPassPipelineInfo.depth_stencil_state.back_stencil_state.fail_op = SDL_GPU_STENCILOP_REPLACE;
				depthPassPipelineInfo.rasterizer_state.enable_depth_clip = false;
				depthPassPipelineInfo.rasterizer_state.enable_depth_bias = false;
				depthPassPipelineInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
				depthPassPipelineInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;


				ctx.depthPassPipeline = SDL_CreateGPUGraphicsPipeline(ctx.gpu->Get(), &depthPassPipelineInfo);
			}

			return ERROR_NoError;
		};

		ErrorCode Teardown(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_NoError;
			}

			auto& ctx = registry.ctx().get<Context>();
			SDL_ReleaseGPUTransferBuffer(ctx.gpu->Get(), ctx.staticMeshTransferBuffer);
			return ERROR_NoError;
		};

		ErrorCode OnWindowResize(Context& ctx) {

			return ERROR_NoError;
		};

		struct AlbedoPixel {
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};

		MaterialHandle RegisterMaterial(Context& ctx, std::shared_ptr<MaterialResource> material) {
			// Upload Material to slot
			SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(ctx.gpu->Get());
			SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

			Uint32 textureSizeBytes = (Uint32)sizeof(AlbedoPixel) * material->GetDimensions().x * material->GetDimensions().y;

			// Create Transfer Buffer
			SDL_GPUTransferBufferCreateInfo transferInfo{};
			SDL_GPUTransferBuffer* transferBuffer;
			transferInfo.size = textureSizeBytes;
			transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
			transferBuffer = SDL_CreateGPUTransferBuffer(ctx.gpu->Get(), &transferInfo);

			// Upload to Albedo Texture Transfer Buffer
			AlbedoPixel* data = (AlbedoPixel*)SDL_MapGPUTransferBuffer(ctx.gpu->Get(), transferBuffer, false);
			SDL_memcpy(data, material->GetAlbedo(), textureSizeBytes);
			SDL_UnmapGPUTransferBuffer(ctx.gpu->Get(), transferBuffer);

			// Copy from transfer buffer to internal texture
			SDL_GPUTextureTransferInfo textureTransferInfo{};
			textureTransferInfo.transfer_buffer = transferBuffer;
			textureTransferInfo.pixels_per_row = material->GetDimensions().x;
			textureTransferInfo.rows_per_layer = material->GetDimensions().y;
			textureTransferInfo.offset = 0;

			SDL_GPUTextureRegion textureRegion{};
			textureRegion.layer = ctx._staticMeshSlot;
			textureRegion.texture = ctx.staticMeshAlbedos512;

			SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureRegion, false);

			SDL_EndGPUCopyPass(copyPass);
			SDL_ReleaseGPUTransferBuffer(ctx.gpu->Get(), transferBuffer);
			MaterialHandle handle = {
				.id = ctx._staticMeshSlot
			};
			ctx._staticMeshSlot++;
			return handle;
		}

		struct DepthPassUBO
		{
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
			glm::mat4 cameraMatrix;
		};

		ErrorCode DepthPrePass(Context& ctx, const Smasher::StaticMeshSystem::Context& staticMeshCtx, ResourceManager &resourceManager, const CameraSystem::Component& camera) {
			SDL_GPUColorTargetInfo gTargetInfos[2];
			// Normals
			gTargetInfos[0] = {};
			gTargetInfos[0].clear_color = { 0 / 255.f, 0 / 255.f, 0 / 255.f, 0.f / 255.f };
			gTargetInfos[0].load_op = SDL_GPU_LOADOP_CLEAR;
			gTargetInfos[0].store_op = SDL_GPU_STOREOP_STORE;
			gTargetInfos[0].texture = ctx.gNormals;
			gTargetInfos[0].resolve_texture = NULL;
			gTargetInfos[0].cycle = true;
			gTargetInfos[0].mip_level = 0;
			gTargetInfos[0].layer_or_depth_plane = 0;
			gTargetInfos[0].cycle_resolve_texture = false;

			// UVs
			gTargetInfos[1] = {};
			gTargetInfos[1].clear_color = { 0 / 255.f, 0 / 255.f, 0 / 255.f, 0.f / 255.f };
			gTargetInfos[1].load_op = SDL_GPU_LOADOP_CLEAR;
			gTargetInfos[1].store_op = SDL_GPU_STOREOP_STORE;
			gTargetInfos[1].texture = ctx.gUV;
			gTargetInfos[1].resolve_texture = NULL;
			gTargetInfos[1].cycle = true;
			gTargetInfos[1].mip_level = 0;
			gTargetInfos[1].layer_or_depth_plane = 0;
			gTargetInfos[1].cycle_resolve_texture = false;

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

			// Update Vertex Buffers for Static Meshes
			{
				SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(ctx.gpu->Get());
				SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
				for (const auto& itr : staticMeshCtx.batches) {
					for (const auto& batch : itr.second) {
						static const size_t BLOCK_SIZE_BYTES = sizeof(StaticMeshSystem::StaticMeshData) * StaticMeshSystem::StaticMeshBatch::MAX_MODEL_COUNT;
						// Update batch
						if (batch.dirty) {
							void* dst = SDL_MapGPUTransferBuffer(ctx.gpu->Get(), ctx.staticMeshTransferBuffer, true);
							SDL_memcpy(dst, batch.models.data(), BLOCK_SIZE_BYTES);
							SDL_UnmapGPUTransferBuffer(ctx.gpu->Get(), ctx.staticMeshTransferBuffer);


							SDL_GPUTransferBufferLocation location{};
							location.transfer_buffer = ctx.staticMeshTransferBuffer;
							location.offset = 0;

							SDL_GPUBufferRegion region{};
							region.buffer = ctx.staticMeshBatchPool.GetBuffer();
							region.size = BLOCK_SIZE_BYTES;
							region.offset = batch.block.index * BLOCK_SIZE_BYTES;

							SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
						}
					}
				}
				SDL_EndGPUCopyPass(copyPass);
				SDL_GPUFence *fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
				SDL_WaitForGPUFences(ctx.gpu->Get(), true, &fence, 1);
				SDL_ReleaseGPUFence(ctx.gpu->Get(), fence);
			}

			glm::mat4 viewMatrix = CameraSystem::GetViewMatrix(camera);
			DepthPassUBO ubo = {
				.projectionMatrix = CameraSystem::GetProjectionMatrix(camera),
				.viewMatrix = viewMatrix,
				.cameraMatrix = glm::inverse(viewMatrix)
			};

			Uint8 materialID = 1;
			// Render Static Meshes
			{
				SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(ctx.gpu->Get());
				SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, gTargetInfos, 2, &depthStencilTargetInfo);
				SDL_BindGPUGraphicsPipeline(renderPass, ctx.depthPassPipeline);
				SDL_SetGPUStencilReference(renderPass, 64);
				SDL_PushGPUVertexUniformData(commandBuffer, 0, &ubo, sizeof(ubo));

				SDL_GPUBuffer* instanceBuffer = ctx.staticMeshBatchPool.GetBuffer();

				for (const auto& itr : staticMeshCtx.batches) {
					std::shared_ptr<StaticMeshResource> meshResource = resourceManager.GetResource<StaticMeshResource>(itr.first);
					SDL_GPUBufferBinding indexBuffer = {};
					indexBuffer.buffer = meshResource->GetIndexBuffer();
					indexBuffer.offset = 0;

					SDL_GPUBufferBinding vertexBuffers[4];
					vertexBuffers[0].buffer = instanceBuffer;
					vertexBuffers[0].offset = 0;
					vertexBuffers[1].buffer = meshResource->GetVertexPositionBuffer();
					vertexBuffers[1].offset = 0;
					vertexBuffers[2].buffer = meshResource->GetVertexNormalBuffer();
					vertexBuffers[2].offset = 0;
					vertexBuffers[3].buffer = meshResource->GetVertexUVBuffer();
					vertexBuffers[3].offset = 0;

					Uint32 numIndices = meshResource->GetNumIndices();
					Uint32 numInstances = 1;
					SDL_BindGPUIndexBuffer(renderPass, &indexBuffer, SDL_GPUIndexElementSize::SDL_GPU_INDEXELEMENTSIZE_32BIT);
					SDL_BindGPUVertexBuffers(renderPass, 0, vertexBuffers, 4);

					for (const auto& batch : itr.second) {
						// Render batch
						int instanceOffset = batch.block.index * StaticMeshSystem::StaticMeshBatch::MAX_MODEL_COUNT;
						SDL_DrawGPUIndexedPrimitives(renderPass, numIndices, batch.modelCount, 0, 0, instanceOffset);
					}
				}
				SDL_EndGPURenderPass(renderPass);
				SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
				SDL_WaitForGPUFences(ctx.gpu->Get(), true, &fence, 1);
				SDL_ReleaseGPUFence(ctx.gpu->Get(), fence);
			}

			return ERROR_NoError;
		};

		ErrorCode LightingPass(Context& ctx) {

			return ERROR_NoError;
		};

		ErrorCode ShadowPass(Context& ctx, const CameraSystem::Component& camera) {

			return ERROR_NoError;
		};

		ErrorCode MaterialsPass(Context& ctx) {

			return ERROR_NoError;
		};
	}
}
