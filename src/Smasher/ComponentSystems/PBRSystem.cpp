#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
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
			ZoneScoped;
			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			assert(registry.ctx().contains<EngineSystem::Context>() && "EngineSystem must be initialized before PBRSystem");
			assert(registry.ctx().contains<SDLSystem::Context>() && "SDLSystem must be initialized before PBRSystem");

			auto &ctx = registry.ctx().emplace<Context>();
			auto &sdlSystemCtx = registry.ctx().get<SDLSystem::Context>();

			ctx.gpu = gpu;

			ctx.viewSize = sdlSystemCtx.windowSize;

			// Pool with 512 batches (MAX)
			ctx.staticMeshBatchPool = GPUBlockPool<Smasher::StaticMeshSystem::StaticMeshData>(gpu, Smasher::StaticMeshSystem::StaticMeshBatch::MAX_MODEL_COUNT, 512);

			// TODO: Save composition shader code to EngineConfig.h
			{
				auto& resourceManager = registry.ctx().get<EngineSystem::Context>().engineRef.get().GetResourceManager();
				ctx.materialPassFragShader = resourceManager.GetOrLoadResource<Manifest::Shaders::material_pass_frag_shader, Smasher::SDLGraphicShaderResource>(gpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT);
				ctx.materialPassVertShader = resourceManager.GetOrLoadResource<Manifest::Shaders::material_pass_vert_shader, Smasher::SDLGraphicShaderResource>(gpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX);
				ctx.depthPassFragShader = resourceManager.GetOrLoadResource<Manifest::Shaders::depth_pass_frag_shader, Smasher::SDLGraphicShaderResource>(gpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT);
				ctx.depthPassVertShader = resourceManager.GetOrLoadResource<Manifest::Shaders::depth_pass_vert_shader, Smasher::SDLGraphicShaderResource>(gpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX);
				ctx.testTeapotMeshResource = resourceManager.GetOrLoadResource<Manifest::Models::suzanne, Smasher::StaticMeshResource>(gpu);
			}

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

			// Create Screen Quad
			{
				SDL_GPUBufferCreateInfo screenQuadBufferInfo{};
				screenQuadBufferInfo.size = sizeof(PBRSystem::SCREEN_QUAD);
				screenQuadBufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
				ctx.screenQuadBuffer = SDL_CreateGPUBuffer(ctx.gpu->Get(), &screenQuadBufferInfo);

				SDL_GPUTransferBufferCreateInfo transferBufferInfo{};
				transferBufferInfo.size = sizeof(PBRSystem::SCREEN_QUAD);
				transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
				SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(ctx.gpu->Get(), &transferBufferInfo);

				SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(ctx.gpu->Get());
				SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

				QuadVertex* data = (QuadVertex*)SDL_MapGPUTransferBuffer(ctx.gpu->Get(), transferBuffer, false);
				SDL_memcpy(data, PBRSystem::SCREEN_QUAD, sizeof(PBRSystem::SCREEN_QUAD));
				SDL_UnmapGPUTransferBuffer(ctx.gpu->Get(), transferBuffer);

				SDL_GPUTransferBufferLocation source{};
				source.offset = 0;
				source.transfer_buffer = transferBuffer;

				SDL_GPUBufferRegion region{};
				region.buffer = ctx.screenQuadBuffer;
				region.offset = 0;
				region.size = sizeof(PBRSystem::SCREEN_QUAD);
				SDL_UploadToGPUBuffer(copyPass, &source, &region, false);

				SDL_EndGPUCopyPass(copyPass);
				SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
				SDL_WaitForGPUFences(ctx.gpu->Get(), true, &fence, 1);
				SDL_ReleaseGPUTransferBuffer(ctx.gpu->Get(), transferBuffer);
			}

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

			// Create Material Ids buffer
			{
				SDL_GPUTextureCreateInfo materialIdsTextureInfo{};
				materialIdsTextureInfo.format = MATERIAL_IDS_TEX_FORMAT;
				materialIdsTextureInfo.height = height;
				materialIdsTextureInfo.width = width;
				materialIdsTextureInfo.layer_count_or_depth = 1;
				materialIdsTextureInfo.num_levels = 1;
				materialIdsTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
				materialIdsTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
				materialIdsTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_GRAPHICS_STORAGE_READ;
				ctx.gMaterialIds = SDL_CreateGPUTexture(ctx.gpu->Get(), &materialIdsTextureInfo);
				SDL_SetGPUTextureName(ctx.gpu->Get(), ctx.gMaterialIds, "Material IDs Texture");
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
				ctx.gTriangleNormals = SDL_CreateGPUTexture(ctx.gpu->Get(), &normalsTextureInfo);
				ctx.gComputedNormals = SDL_CreateGPUTexture(ctx.gpu->Get(), &normalsTextureInfo);
				SDL_SetGPUTextureName(ctx.gpu->Get(), ctx.gTriangleNormals, "Triangle Normals Texture");
				SDL_SetGPUTextureName(ctx.gpu->Get(), ctx.gComputedNormals, "Computed Normals Texture");
			}

			// Create Albedos Buffer
			{
				SDL_GPUTextureCreateInfo normalsTextureInfo{};
				normalsTextureInfo.format = ALBEDO_TEX_FORMAT;
				normalsTextureInfo.height = height;
				normalsTextureInfo.width = width;
				normalsTextureInfo.layer_count_or_depth = 1;
				normalsTextureInfo.num_levels = 1;
				normalsTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
				normalsTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
				normalsTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
				ctx.gAlbedo = SDL_CreateGPUTexture(ctx.gpu->Get(), &normalsTextureInfo);
				SDL_SetGPUTextureName(ctx.gpu->Get(), ctx.gAlbedo, "Albedo Texture");
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

			// Create Basic Samplers
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

			SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
			SDL_WaitForGPUFences(ctx.gpu->Get(), true, &fence, 1);
			SDL_ReleaseGPUFence(ctx.gpu->Get(), fence);
			SDL_ReleaseGPUTransferBuffer(ctx.gpu->Get(), transferBuffer);

			// Create depth Prepass Pipeline
			{
				SDL_GPUGraphicsPipelineCreateInfo depthPassPipelineInfo{};

				SDL_GPUColorTargetDescription gTargetDescriptions[3];
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

				gTargetDescriptions[2] = {};
				gTargetDescriptions[2].blend_state.enable_blend = false;
				gTargetDescriptions[2].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
				gTargetDescriptions[2].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
				gTargetDescriptions[2].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				gTargetDescriptions[2].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				gTargetDescriptions[2].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				gTargetDescriptions[2].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				gTargetDescriptions[2].format = MATERIAL_IDS_TEX_FORMAT;

				// Vertex Buffers
				// Instance Data Buffer (slot 0)
				//		model matrix location(0-3)
				// Position (slot 1, location 4)
				// Normal	(slot 2, location 5)
				// UV Coord (slot 3, location 6)

				SDL_GPUVertexBufferDescription vertexBufferDesc[2];
				// Instance Buffer
				vertexBufferDesc[0] = {};
				vertexBufferDesc[0].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
				vertexBufferDesc[0].slot = 0;
				vertexBufferDesc[0].pitch = sizeof(StaticMeshSystem::StaticMeshData);
				vertexBufferDesc[0].instance_step_rate = 0;

				// Vertex Buffer
				vertexBufferDesc[1] = {};
				vertexBufferDesc[1].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
				vertexBufferDesc[1].slot = 1;
				vertexBufferDesc[1].pitch = sizeof(VertexData);
				vertexBufferDesc[1].instance_step_rate = 0;

				SDL_GPUVertexAttribute vertexAttributes[8];
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
				vertexAttributes[5].offset = sizeof(float) * 3;

				// Vertex UV
				vertexAttributes[6].buffer_slot = 1;
				vertexAttributes[6].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
				vertexAttributes[6].location = 6;
				vertexAttributes[6].offset = sizeof(float) * 3 + sizeof(float) * 3;

				// Instance Material ID
				vertexAttributes[7].buffer_slot = 0;
				vertexAttributes[7].format = SDL_GPU_VERTEXELEMENTFORMAT_INT;
				vertexAttributes[7].location = 7;
				vertexAttributes[7].offset = sizeof(StaticMeshSystem::StaticMeshData::transform);

				depthPassPipelineInfo.fragment_shader = ctx.depthPassFragShader->GetShader();
				depthPassPipelineInfo.vertex_shader = ctx.depthPassVertShader->GetShader();
				depthPassPipelineInfo.target_info.color_target_descriptions = gTargetDescriptions;
				depthPassPipelineInfo.target_info.num_color_targets = 3;
				depthPassPipelineInfo.target_info.has_depth_stencil_target = true;
				depthPassPipelineInfo.target_info.depth_stencil_format = SDLSystem::GetGPUDepthStencilFormat(ctx.gpu->Get());
				depthPassPipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;
				depthPassPipelineInfo.vertex_input_state.num_vertex_attributes = 8;
				depthPassPipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDesc;
				depthPassPipelineInfo.vertex_input_state.num_vertex_buffers = 2;
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

			// Create material pass pipeline
			{
				SDL_GPUGraphicsPipelineCreateInfo materialPassPipelineInfo{};
				SDL_GPUColorTargetDescription gTargetDescriptions[2];
				// normals target
				gTargetDescriptions[0] = {};
				gTargetDescriptions[0].blend_state.enable_blend = false;
				gTargetDescriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
				gTargetDescriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
				gTargetDescriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				gTargetDescriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				gTargetDescriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				gTargetDescriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				gTargetDescriptions[0].format = NORMALS_TEX_FORMAT;

				// albedos target
				gTargetDescriptions[1] = {};
				gTargetDescriptions[1].blend_state.enable_blend = false;
				gTargetDescriptions[1].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
				gTargetDescriptions[1].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
				gTargetDescriptions[1].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				gTargetDescriptions[1].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				gTargetDescriptions[1].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
				gTargetDescriptions[1].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
				gTargetDescriptions[1].format = ALBEDO_TEX_FORMAT;

				// Vertex Buffers
				// Position (slot 0, location 0)

				SDL_GPUVertexBufferDescription vertexBufferDesc[1];
				// Vertex Position Buffer
				vertexBufferDesc[0] = {};
				vertexBufferDesc[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
				vertexBufferDesc[0].slot = 0;
				vertexBufferDesc[0].pitch = sizeof(QuadVertex);
				vertexBufferDesc[0].instance_step_rate = 0;

				SDL_GPUVertexAttribute vertexAttributes[2];
				// Vertex Position
				vertexAttributes[0].buffer_slot = 0;
				vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
				vertexAttributes[0].location = 0;
				vertexAttributes[0].offset = 0;

				// Vertex UV
				vertexAttributes[1].buffer_slot = 0;
				vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
				vertexAttributes[1].location = 1;
				vertexAttributes[1].offset = sizeof(float) * 3;

				materialPassPipelineInfo.fragment_shader = ctx.materialPassFragShader->GetShader();
				materialPassPipelineInfo.vertex_shader = ctx.materialPassVertShader->GetShader();
				materialPassPipelineInfo.target_info.color_target_descriptions = gTargetDescriptions;
				materialPassPipelineInfo.target_info.num_color_targets = 2;
				materialPassPipelineInfo.target_info.has_depth_stencil_target = false;
				materialPassPipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;
				materialPassPipelineInfo.vertex_input_state.num_vertex_attributes = 2;
				materialPassPipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDesc;
				materialPassPipelineInfo.vertex_input_state.num_vertex_buffers = 1;
				materialPassPipelineInfo.depth_stencil_state.enable_depth_test = false;
				materialPassPipelineInfo.depth_stencil_state.enable_depth_write = false;
				materialPassPipelineInfo.depth_stencil_state.enable_stencil_test = false;
				materialPassPipelineInfo.rasterizer_state.enable_depth_clip = false;
				materialPassPipelineInfo.rasterizer_state.enable_depth_bias = false;
				materialPassPipelineInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
				materialPassPipelineInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;


				ctx.materialPassPipeline = SDL_CreateGPUGraphicsPipeline(ctx.gpu->Get(), &materialPassPipelineInfo);
			}

			return ERROR_NoError;
		};

		ErrorCode Teardown(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_NoError;
			}

			auto& ctx = registry.ctx().get<Context>();
			SDL_ReleaseGPUTransferBuffer(ctx.gpu->Get(), ctx.staticMeshTransferBuffer);
			SDL_ReleaseGPUBuffer(ctx.gpu->Get(), ctx.screenQuadBuffer);
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

		Expected<MaterialBinding> BindMaterial(Context& ctx, std::shared_ptr<MaterialResource> material) {
			// Search for empty slot
			int slot = -1;
			for (int i = 0; i < ctx.staticMeshMaterialBindings.size(); i++) {
				if (!ctx.staticMeshMaterialBindings[i]) {
					slot = i;
					break;
				}
			}
			if (slot == -1) {
				return Expected<MaterialBinding>::Error(ERROR_MaxMaterialBindings);
			}

			ctx.staticMeshMaterialBindings[slot] = material;
			MaterialBinding binding { material->GetId(), static_cast<uint32_t>(slot) };

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
			SDL_memcpy(data, material->GetAlbedo().Get(), textureSizeBytes);
			SDL_UnmapGPUTransferBuffer(ctx.gpu->Get(), transferBuffer);

			// Copy from transfer buffer to internal texture
			SDL_GPUTextureTransferInfo textureTransferInfo{};
			textureTransferInfo.transfer_buffer = transferBuffer;
			textureTransferInfo.pixels_per_row = material->GetDimensions().x;
			textureTransferInfo.rows_per_layer = material->GetDimensions().y;
			textureTransferInfo.offset = 0;

			SDL_GPUTextureRegion textureRegion{};
			textureRegion.layer = slot;
			textureRegion.texture = ctx.staticMeshAlbedos512;
			textureRegion.w = material->GetDimensions().x;
			textureRegion.h = material->GetDimensions().y;
			textureRegion.d = 1;

			SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureRegion, false);

			SDL_EndGPUCopyPass(copyPass);
			SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
			SDL_WaitForGPUFences(ctx.gpu->Get(), true, &fence, 1);
			SDL_ReleaseGPUTransferBuffer(ctx.gpu->Get(), transferBuffer);
			return binding;
		}

		struct CameraInfoUBO
		{
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
			glm::mat4 cameraMatrix;
			glm::uvec2 viewSize;
		};

		ErrorCode DepthPrePass(Context& ctx, const Smasher::StaticMeshSystem::Context& staticMeshCtx, ResourceManager &resourceManager, const CameraSystem::Component& camera) {
			ZoneScoped;
			SDL_GPUColorTargetInfo gTargetInfos[3];
			// Normals
			gTargetInfos[0] = {};
			gTargetInfos[0].clear_color = { 0 / 255.f, 0 / 255.f, 0 / 255.f, 0.f / 255.f };
			gTargetInfos[0].load_op = SDL_GPU_LOADOP_CLEAR;
			gTargetInfos[0].store_op = SDL_GPU_STOREOP_STORE;
			gTargetInfos[0].texture = ctx.gTriangleNormals;
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

			// Material IDs
			gTargetInfos[2] = {};
			gTargetInfos[2].clear_color = { 0 / 255.f };
			gTargetInfos[2].load_op = SDL_GPU_LOADOP_CLEAR;
			gTargetInfos[2].store_op = SDL_GPU_STOREOP_STORE;
			gTargetInfos[2].texture = ctx.gMaterialIds;
			gTargetInfos[2].resolve_texture = NULL;
			gTargetInfos[2].cycle = true;
			gTargetInfos[2].mip_level = 0;
			gTargetInfos[2].layer_or_depth_plane = 0;
			gTargetInfos[2].cycle_resolve_texture = false;

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
				int count = 0;
				for (const auto& itr : staticMeshCtx.batches) {
					for (const auto& batch : itr.second.batchList) {
						static const size_t BLOCK_SIZE_BYTES = sizeof(StaticMeshSystem::StaticMeshData) * StaticMeshSystem::StaticMeshBatch::MAX_MODEL_COUNT;
						// Update batch
						if (batch.dirty) {
							void* dst = SDL_MapGPUTransferBuffer(ctx.gpu->Get(), ctx.staticMeshTransferBuffer, true);
							SDL_memcpy(dst, batch.instances.data(), BLOCK_SIZE_BYTES);
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
			CameraInfoUBO ubo = {
				.projectionMatrix = CameraSystem::GetProjectionMatrix(camera),
				.viewMatrix = viewMatrix,
				.cameraMatrix = glm::inverse(viewMatrix),
				.viewSize = ctx.viewSize
			};

			Uint8 materialID = 1;
			// Render Static Meshes
			{
				SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(ctx.gpu->Get());
				SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, gTargetInfos, 3, &depthStencilTargetInfo);
				SDL_BindGPUGraphicsPipeline(renderPass, ctx.depthPassPipeline);
				SDL_SetGPUStencilReference(renderPass, 64);
				SDL_PushGPUVertexUniformData(commandBuffer, 0, &ubo, sizeof(ubo));

				SDL_GPUBuffer* instanceBuffer = ctx.staticMeshBatchPool.GetBuffer();

				for (const auto& itr : staticMeshCtx.batches) {
					std::shared_ptr<StaticMeshResource> meshResource = resourceManager.GetResource<StaticMeshResource>(itr.first);
					SDL_GPUBufferBinding indexBuffer = {};
					indexBuffer.buffer = meshResource->GetIndexBuffer();
					indexBuffer.offset = 0;

					SDL_GPUBufferBinding vertexBuffers[2];
					vertexBuffers[0].buffer = instanceBuffer;
					vertexBuffers[0].offset = 0;
					vertexBuffers[1].buffer = meshResource->GetVertexBuffer();
					vertexBuffers[1].offset = 0;

					Uint32 numIndices = meshResource->GetNumIndices();
					Uint32 numInstances = 1;
					SDL_BindGPUIndexBuffer(renderPass, &indexBuffer, SDL_GPUIndexElementSize::SDL_GPU_INDEXELEMENTSIZE_32BIT);
					SDL_BindGPUVertexBuffers(renderPass, 0, vertexBuffers, 2);

					for (const auto& batch : itr.second.batchList) {
						// Render batch
						int instanceOffset = batch.block.index * StaticMeshSystem::StaticMeshBatch::MAX_MODEL_COUNT;
						SDL_DrawGPUIndexedPrimitives(renderPass, numIndices, batch.modelCount, 0, 0, instanceOffset);
					}

					//for (const auto& range : itr.second.gpuBlockRanges) {
					//	ZoneScoped;
					//	// Render batch
					//	int instanceOffset = range.first.index * StaticMeshSystem::StaticMeshBatch::MAX_MODEL_COUNT;
					//	SDL_DrawGPUIndexedPrimitives(renderPass, numIndices, range.second * 48, 0, 0, instanceOffset);
					//}
				}
				SDL_EndGPURenderPass(renderPass);
				TracyCZone(tracyZoneCtx, true);
				TracyCZoneName(tracyZoneCtx, "Depth Pre Pass Wait for Fence", strlen("Depth Pre Pass Wait for Fence"));
				SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
				SDL_WaitForGPUFences(ctx.gpu->Get(), true, &fence, 1);
				SDL_ReleaseGPUFence(ctx.gpu->Get(), fence);
				TracyCZoneEnd(tracyZoneCtx);
			}

			return ERROR_NoError;
		};

		ErrorCode LightingPass(Context& ctx) {

			return ERROR_NoError;
		};

		ErrorCode ShadowPass(Context& ctx, const CameraSystem::Component& camera) {

			return ERROR_NoError;
		};

		ErrorCode MaterialsPass(Context& ctx, const CameraSystem::Component& camera) {
			ZoneScoped;

			glm::mat4 viewMatrix = CameraSystem::GetViewMatrix(camera);
			CameraInfoUBO ubo = {
				.projectionMatrix = CameraSystem::GetProjectionMatrix(camera),
				.viewMatrix = viewMatrix,
				.cameraMatrix = glm::inverse(viewMatrix),
				.viewSize = ctx.viewSize
			};

			SDL_GPUColorTargetInfo gTargetInfos[2];
			// Normals
			gTargetInfos[0] = {};
			gTargetInfos[0].clear_color = { 0 / 255.f, 0 / 255.f, 0 / 255.f, 0.f / 255.f };
			gTargetInfos[0].load_op = SDL_GPU_LOADOP_CLEAR;
			gTargetInfos[0].store_op = SDL_GPU_STOREOP_STORE;
			gTargetInfos[0].texture = ctx.gComputedNormals;
			gTargetInfos[0].resolve_texture = NULL;
			gTargetInfos[0].cycle = true;
			gTargetInfos[0].mip_level = 0;
			gTargetInfos[0].layer_or_depth_plane = 0;
			gTargetInfos[0].cycle_resolve_texture = false;

			// albedo
			gTargetInfos[1] = {};
			gTargetInfos[1].clear_color = { 0 / 255.f, 0 / 255.f, 0 / 255.f, 0.f / 255.f };
			gTargetInfos[1].load_op = SDL_GPU_LOADOP_CLEAR;
			gTargetInfos[1].store_op = SDL_GPU_STOREOP_STORE;
			gTargetInfos[1].texture = ctx.gAlbedo;
			gTargetInfos[1].resolve_texture = NULL;
			gTargetInfos[1].cycle = true;
			gTargetInfos[1].mip_level = 0;
			gTargetInfos[1].layer_or_depth_plane = 0;
			gTargetInfos[1].cycle_resolve_texture = false;

			// Render Pass
			{
				SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(ctx.gpu->Get());
				SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, gTargetInfos, 2, NULL);
				SDL_BindGPUGraphicsPipeline(renderPass, ctx.materialPassPipeline);
				SDL_GPUTextureSamplerBinding samplerBindings[3];
				// Albedos 
				samplerBindings[0] = {};
				samplerBindings[0].texture = ctx.staticMeshAlbedos512;
				samplerBindings[0].sampler = ctx.gAlbedoSampler;

				// Global Buffer Normals
				samplerBindings[1] = {};
				samplerBindings[1].texture = ctx.gTriangleNormals;
				samplerBindings[1].sampler = ctx.gNormalsSampler;

				// Global Buffer UVs
				samplerBindings[2] = {};
				samplerBindings[2].texture = ctx.gUV;
				samplerBindings[2].sampler = ctx.gUVSampler;

				SDL_GPUBufferBinding vertexBuffer[1];
				vertexBuffer[0].buffer = ctx.screenQuadBuffer;
				vertexBuffer[0].offset = 0;

				SDL_BindGPUFragmentSamplers(renderPass, 0, samplerBindings, 3);
				SDL_BindGPUVertexBuffers(renderPass, 0, vertexBuffer, 1);
				SDL_BindGPUFragmentStorageTextures(renderPass, 0, &ctx.gMaterialIds, 1);
				SDL_PushGPUFragmentUniformData(commandBuffer, 0, &ubo, sizeof(ubo));

				SDL_DrawGPUPrimitives(renderPass, 6, 1, 0, 0);

				SDL_EndGPURenderPass(renderPass);
				SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
				SDL_WaitForGPUFences(ctx.gpu->Get(), true, &fence, 1);
				SDL_ReleaseGPUFence(ctx.gpu->Get(), fence);
			}

			return ERROR_NoError;
		};
	}
}
