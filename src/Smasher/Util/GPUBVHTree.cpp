#include <SDL3/SDL.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
#include "Smasher/Base.h"
#include "Smasher/Resources.h"
#include "Smasher/Util/GraphicsUtil.h"
#include "Smasher/Util/GPUBVHTree.h"
#include "Smasher/ComponentSystems/SDLSystem.h"
#include "Smasher/ComponentSystems/StaticMeshSystem.h"
#include "Manifest.h"

namespace Smasher {
	namespace GraphicsUtil {
		static const int BVH_BRANCHING_FACTOR = 4;

		// Returns sum b^0 + b^1 + b^2 ... + b^n
		static int seriesExponentSum(int b, int n)
		{
			n = std::max(0, n);
			return (1 - (pow(b, n + 1))) / (1 - b);
		}


		GPUBVHTree::GPUBVHTree(std::shared_ptr<SDL_GPUDeviceWrapper> gpu) : m_GPU(gpu) {}

		GPUBVHTreeMorton::GPUBVHTreeMorton(std::shared_ptr<SDL_GPUDeviceWrapper> gpu, ResourceManager& resourceMgr, unsigned int maxEntries) : GPUBVHTree(gpu), m_MaxEntries(maxEntries) {
			m_DebugBVHFragShader = resourceMgr.GetOrLoadResource<Manifest::Shaders::Debug::debug_bvh_frag_shader, Smasher::SDLGraphicShaderResource>(gpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT);
			m_DebugBVHVertShader = resourceMgr.GetOrLoadResource<Manifest::Shaders::Debug::debug_bvh_vert_shader, Smasher::SDLGraphicShaderResource>(gpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX);
			m_RadixInputShader = resourceMgr.GetOrLoadResource<Manifest::Shaders::GPUSort::radix_morton_code_shader, Smasher::SDLComputeShaderResource>(gpu);
			m_InitializeBVHShader = resourceMgr.GetOrLoadResource<Manifest::Shaders::GPUSort::initialize_bvh_shader, Smasher::SDLComputeShaderResource>(gpu);
			m_ConstructBVHShader = resourceMgr.GetOrLoadResource<Manifest::Shaders::GPUSort::construct_bvh_shader, Smasher::SDLComputeShaderResource>(gpu);

			SDL_GPUVertexBufferDescription vertexBufferDesc[1];
			// Vertex Position Buffer
			vertexBufferDesc[0] = {};
			vertexBufferDesc[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
			vertexBufferDesc[0].slot = 0;
			vertexBufferDesc[0].pitch = sizeof(float) * 3;
			vertexBufferDesc[0].instance_step_rate = 0;

			SDL_GPUVertexAttribute vertexAttributes[1];
			// Vertex Position
			vertexAttributes[0].buffer_slot = 0;
			vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
			vertexAttributes[0].location = 0;
			vertexAttributes[0].offset = 0;

			SDL_GPUColorTargetDescription gTargetDescriptions[1];
			// output target
			gTargetDescriptions[0] = {};
			gTargetDescriptions[0].blend_state.enable_blend = false;
			gTargetDescriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
			gTargetDescriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
			gTargetDescriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			gTargetDescriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			gTargetDescriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			gTargetDescriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			gTargetDescriptions[0].format = SDLSystem::GetGPURGBAFormat(m_GPU->Get());

			SDL_GPUGraphicsPipelineCreateInfo debugBVHPipelineInfo{};
			debugBVHPipelineInfo.fragment_shader = m_DebugBVHFragShader->GetShader();
			debugBVHPipelineInfo.vertex_shader = m_DebugBVHVertShader->GetShader();
			debugBVHPipelineInfo.target_info.color_target_descriptions = gTargetDescriptions;
			debugBVHPipelineInfo.target_info.num_color_targets = 1;
			debugBVHPipelineInfo.target_info.has_depth_stencil_target = true;
			debugBVHPipelineInfo.target_info.depth_stencil_format = SDLSystem::GetGPUDepthStencilFormat(m_GPU->Get());
			debugBVHPipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;
			debugBVHPipelineInfo.vertex_input_state.num_vertex_attributes = 1;
			debugBVHPipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDesc;
			debugBVHPipelineInfo.vertex_input_state.num_vertex_buffers = 1;
			debugBVHPipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
			debugBVHPipelineInfo.depth_stencil_state.enable_depth_test = false;
			debugBVHPipelineInfo.depth_stencil_state.enable_depth_write = false;
			debugBVHPipelineInfo.depth_stencil_state.enable_stencil_test = false;
			debugBVHPipelineInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS; // SDL_GPU_COMPAREOP_ALWAYS;
			debugBVHPipelineInfo.depth_stencil_state.write_mask = 0xFF;
			debugBVHPipelineInfo.depth_stencil_state.front_stencil_state.compare_op = SDL_GPU_COMPAREOP_ALWAYS;
			debugBVHPipelineInfo.depth_stencil_state.front_stencil_state.depth_fail_op = SDL_GPU_STENCILOP_KEEP;
			debugBVHPipelineInfo.depth_stencil_state.front_stencil_state.pass_op = SDL_GPU_STENCILOP_REPLACE;
			debugBVHPipelineInfo.depth_stencil_state.front_stencil_state.fail_op = SDL_GPU_STENCILOP_REPLACE;
			debugBVHPipelineInfo.depth_stencil_state.back_stencil_state.compare_op = SDL_GPU_COMPAREOP_ALWAYS;
			debugBVHPipelineInfo.depth_stencil_state.back_stencil_state.depth_fail_op = SDL_GPU_STENCILOP_KEEP;
			debugBVHPipelineInfo.depth_stencil_state.back_stencil_state.pass_op = SDL_GPU_STENCILOP_REPLACE;
			debugBVHPipelineInfo.depth_stencil_state.back_stencil_state.fail_op = SDL_GPU_STENCILOP_REPLACE;
			debugBVHPipelineInfo.rasterizer_state.enable_depth_clip = false;
			debugBVHPipelineInfo.rasterizer_state.enable_depth_bias = false;
			debugBVHPipelineInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;//SDL_GPU_CULLMODE_BACK;
			debugBVHPipelineInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
			debugBVHPipelineInfo.rasterizer_state.fill_mode = SDL_GPUFillMode::SDL_GPU_FILLMODE_LINE;

			m_DebugBVHPipeline = SDL_CreateGPUGraphicsPipeline(m_GPU->Get(), &debugBVHPipelineInfo);

			// Create BVH Internal Nodes and Leaf Nodes Buffers
			{
				int maxNumLayers = std::max(1.f, 1.f + std::ceil(std::log2f(GetMaxEntries()) / std::log2f(BVH_BRANCHING_FACTOR)));
				int maxNumInternalNodes = seriesExponentSum(BVH_BRANCHING_FACTOR, maxNumLayers);
				SDL_GPUBufferCreateInfo bvhInternalInfo{};
				bvhInternalInfo.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ | SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
				bvhInternalInfo.size = sizeof(BVHMortonInternalNode) * maxNumInternalNodes;
				m_BVHInternalNodes = SDL_CreateGPUBuffer(m_GPU->Get(), &bvhInternalInfo);
				SDL_SetGPUBufferName(m_GPU->Get(), m_BVHInternalNodes, "BVH Internal Nodes");

				SDL_GPUBufferCreateInfo bvhLeafInfo{};
				bvhLeafInfo.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ | SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
				bvhLeafInfo.size = sizeof(BVHMortonLeafNode) * GetMaxEntries();
				m_BVHLeafNodes = SDL_CreateGPUBuffer(m_GPU->Get(), &bvhLeafInfo);
				SDL_SetGPUBufferName(m_GPU->Get(), m_BVHLeafNodes, "BVH Leaf Nodes");

				m_RadixSortPool = std::move(GraphicsUtil::GPURadixSortPool{ gpu, resourceMgr, GetMaxEntries() });
			}

			// Create Debug Cube Vertex Buffer
			{
				SDL_GPUBufferCreateInfo cubeVertexBufferInfo{};
				cubeVertexBufferInfo.size = sizeof(GPUBVHTree::DEBUG_CUBE_VERTICES);
				cubeVertexBufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
				m_DebugCubeVertexBuffer = SDL_CreateGPUBuffer(m_GPU->Get(), &cubeVertexBufferInfo);
				SDL_SetGPUBufferName(m_GPU->Get(), m_DebugCubeVertexBuffer, "Debug Cube Vertex Buffer");
				SDL_GPUTransferBufferCreateInfo transferBufferInfo{};
				transferBufferInfo.size = sizeof(GPUBVHTree::DEBUG_CUBE_VERTICES);
				transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
				SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(m_GPU->Get(), &transferBufferInfo);

				float* data = (float*)SDL_MapGPUTransferBuffer(m_GPU->Get(), transferBuffer, false);
				SDL_memcpy(data, GPUBVHTree::DEBUG_CUBE_VERTICES, sizeof(GPUBVHTree::DEBUG_CUBE_VERTICES));
				SDL_UnmapGPUTransferBuffer(m_GPU->Get(), transferBuffer);

				SDL_GPUTransferBufferLocation source{};
				source.offset = 0;
				source.transfer_buffer = transferBuffer;

				SDL_GPUBufferRegion region{};
				region.buffer = m_DebugCubeVertexBuffer;
				region.offset = 0;
				region.size = sizeof(GPUBVHTree::DEBUG_CUBE_VERTICES);

				SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(m_GPU->Get());
				SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
				SDL_UploadToGPUBuffer(copyPass, &source, &region, false);

				SDL_EndGPUCopyPass(copyPass);
				SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
				SDL_WaitForGPUFences(m_GPU->Get(), true, &fence, 1);
				SDL_ReleaseGPUTransferBuffer(m_GPU->Get(), transferBuffer);
			}

			// Create Debug Cube Index Buffer
			{
				SDL_GPUBufferCreateInfo cubeIndexBufferInfo{};
				cubeIndexBufferInfo.size = sizeof(GPUBVHTree::DEBUG_CUBE_INDICES);
				cubeIndexBufferInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
				m_DebugCubeIndexBuffer = SDL_CreateGPUBuffer(m_GPU->Get(), &cubeIndexBufferInfo);
				SDL_SetGPUBufferName(m_GPU->Get(), m_DebugCubeIndexBuffer, "Debug Cube Index Buffer");

				SDL_GPUTransferBufferCreateInfo transferBufferInfo{};
				transferBufferInfo.size = sizeof(GPUBVHTree::DEBUG_CUBE_INDICES);
				transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
				SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(m_GPU->Get(), &transferBufferInfo);

				float* data = (float*)SDL_MapGPUTransferBuffer(m_GPU->Get(), transferBuffer, false);
				SDL_memcpy(data, GPUBVHTree::DEBUG_CUBE_INDICES, sizeof(GPUBVHTree::DEBUG_CUBE_INDICES));
				SDL_UnmapGPUTransferBuffer(m_GPU->Get(), transferBuffer);

				SDL_GPUTransferBufferLocation source{};
				source.offset = 0;
				source.transfer_buffer = transferBuffer;

				SDL_GPUBufferRegion region{};
				region.buffer = m_DebugCubeIndexBuffer;
				region.offset = 0;
				region.size = sizeof(GPUBVHTree::DEBUG_CUBE_INDICES);

				SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(m_GPU->Get());
				SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
				SDL_UploadToGPUBuffer(copyPass, &source, &region, false);

				SDL_EndGPUCopyPass(copyPass);
				SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
				SDL_WaitForGPUFences(m_GPU->Get(), true, &fence, 1);
				SDL_ReleaseGPUTransferBuffer(m_GPU->Get(), transferBuffer);
			}
		}

		GPUBVHTreeMorton::GPUBVHTreeMorton(GPUBVHTreeMorton&& other) noexcept :
			GPUBVHTree(std::move(other)),
			m_MaxEntries(other.GetMaxEntries()),
			m_RadixSortPool(std::move(other.m_RadixSortPool)),
			m_RadixInputShader(std::move(other.m_RadixInputShader)),
			m_InitializeBVHShader(std::move(other.m_InitializeBVHShader)),
			m_ConstructBVHShader(std::move(other.m_ConstructBVHShader)),
			m_DebugBVHFragShader(std::move(other.m_DebugBVHFragShader)),
			m_DebugBVHVertShader(std::move(other.m_DebugBVHVertShader)),
			m_DebugBVHPipeline(std::move(other.m_DebugBVHPipeline)),
			m_BVHInternalNodes(other.m_BVHInternalNodes),
			m_BVHLeafNodes(other.m_BVHLeafNodes)
		{
			other.m_DebugBVHPipeline = NULL;
			other.m_BVHInternalNodes = NULL;
			other.m_BVHLeafNodes = NULL;
			other.m_MaxEntries = 0;
		}

		GPUBVHTreeMorton& GPUBVHTreeMorton::operator= (GPUBVHTreeMorton&& other) noexcept {
			if (this != &other) {
				if (m_GPU) {
					SDL_ReleaseGPUBuffer(m_GPU->Get(), m_BVHInternalNodes);
					SDL_ReleaseGPUBuffer(m_GPU->Get(), m_BVHLeafNodes);
					SDL_ReleaseGPUBuffer(m_GPU->Get(), m_DebugCubeIndexBuffer);
					SDL_ReleaseGPUBuffer(m_GPU->Get(), m_DebugCubeVertexBuffer);
					SDL_ReleaseGPUGraphicsPipeline(m_GPU->Get(), m_DebugBVHPipeline);
				}
				GPUBVHTree::operator= (std::move(other));

				m_MaxEntries = other.GetMaxEntries();
				m_RadixSortPool = std::move(other.m_RadixSortPool);
				m_RadixInputShader = std::move(other.m_RadixInputShader);
				m_InitializeBVHShader = std::move(other.m_InitializeBVHShader);
				m_ConstructBVHShader = std::move(other.m_ConstructBVHShader);
				m_DebugBVHFragShader = std::move(other.m_DebugBVHFragShader);
				m_DebugBVHVertShader = std::move(other.m_DebugBVHVertShader);
				m_DebugBVHPipeline = other.m_DebugBVHPipeline;
				m_DebugCubeIndexBuffer = other.m_DebugCubeIndexBuffer;
				m_DebugCubeVertexBuffer = other.m_DebugCubeVertexBuffer;
				m_BVHInternalNodes = other.m_BVHInternalNodes;
				m_BVHLeafNodes = other.m_BVHLeafNodes;

				other.m_DebugBVHPipeline = NULL;
				other.m_BVHInternalNodes = NULL;
				other.m_BVHLeafNodes = NULL;
				other.m_DebugCubeIndexBuffer = NULL;
				other.m_DebugCubeVertexBuffer = NULL;
				other.m_MaxEntries = 0;
			}
			return *this;
		}

		GPUBVHTreeMorton::~GPUBVHTreeMorton() {
			if (m_GPU) {
				SDL_ReleaseGPUBuffer(m_GPU->Get(), m_BVHInternalNodes);
				SDL_ReleaseGPUBuffer(m_GPU->Get(), m_BVHLeafNodes);
				SDL_ReleaseGPUBuffer(m_GPU->Get(), m_DebugCubeIndexBuffer);
				SDL_ReleaseGPUBuffer(m_GPU->Get(), m_DebugCubeVertexBuffer);
				SDL_ReleaseGPUGraphicsPipeline(m_GPU->Get(), m_DebugBVHPipeline);
			}
		}

		void GPUBVHTreeMorton::Initialize(SDL_GPUCommandBuffer* commandBuffer, const StaticMeshSystem::Context& staticMeshSysCtx, SDL_GPUBuffer* meshInstancesBuffer, SDL_GPUBuffer* meshPropsBuffer) {
			ZoneScoped;
			int numEntries = staticMeshSysCtx.totalNumInstances;
			glm::vec3 globalMinAABB = staticMeshSysCtx.minAABB;
			glm::vec3 globalMaxAABB = staticMeshSysCtx.maxAABB;
			// Calculate morton codes,
			// feed RadixEntry input buffer
			{
				SDL_GPUStorageBufferReadWriteBinding rwStorageBindings[1];
				rwStorageBindings[0].buffer = m_RadixSortPool.GetInputBuffer();
				rwStorageBindings[0].cycle = false;

				SDL_GPUBuffer* readBuffers[1] = {
					meshInstancesBuffer
				};
				SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(commandBuffer, NULL, 0, rwStorageBindings, 1);
			
				struct UBO
				{
					glm::vec3 minAABB;
					int radixOffset; // Offset within radix entries list
					glm::vec3 maxAABB;
					int instanceOffset;		// Offset within static mesh entries list
					int count;		// Number of entries
				};

				UBO ubo{
					.minAABB = globalMinAABB,
					.radixOffset = 0,
					.maxAABB = globalMaxAABB,
					.instanceOffset = 0,
					.count = 0
				};

				int radixOffset = 0;
				for (const auto& itr : staticMeshSysCtx.batches) {
					const StaticMeshSystem::StaticMeshBatchList& batches = itr.second;

					for (const auto& batch : batches.batchList) {
						// Render batch
						ubo.count = batch.modelCount;
						ubo.instanceOffset = batch.block.index * StaticMeshSystem::StaticMeshBatch::MAX_MODEL_COUNT;

						SDL_BindGPUComputePipeline(computePass, m_RadixInputShader->GetShader());
						SDL_BindGPUComputeStorageBuffers(computePass, 0, readBuffers, 1);
						SDL_PushGPUComputeUniformData(commandBuffer, 0, &ubo, sizeof(UBO));
						int numGroups = std::ceil((float)batch.modelCount / 256.f);
						SDL_DispatchGPUCompute(computePass, numGroups, 1, 1);

						ubo.radixOffset += batch.modelCount;
					}
				}
				SDL_EndGPUComputePass(computePass);
			}
			// Radix sort by Morton Code
			{
				m_RadixSortPool.SortGPUBuffer(commandBuffer, numEntries);
			}

			// Initialize BVH
			{
				struct UBO
				{
					int numEntries;
				};

				UBO ubo{
					.numEntries = numEntries
				};

				SDL_GPUBuffer* readBuffers[3] = {
					m_RadixSortPool.GetOutputBuffer(),
					meshInstancesBuffer,
					meshPropsBuffer
				};

				SDL_GPUStorageBufferReadWriteBinding rwStorageBindings[2];
				rwStorageBindings[0].buffer = m_BVHInternalNodes;
				rwStorageBindings[0].cycle = false;
				rwStorageBindings[1].buffer = m_BVHLeafNodes;
				rwStorageBindings[1].cycle = false;

				SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(commandBuffer, NULL, 0, rwStorageBindings, 2);
				SDL_BindGPUComputePipeline(computePass, m_InitializeBVHShader->GetShader());
				SDL_BindGPUComputeStorageBuffers(computePass, 0, readBuffers, 3);
				SDL_PushGPUComputeUniformData(commandBuffer, 0, &ubo, sizeof(UBO));
				int numGroups = std::ceil((float)numEntries / (float)(32.f * BVH_BRANCHING_FACTOR));
				SDL_DispatchGPUCompute(computePass, numGroups, 1, 1);
				SDL_EndGPUComputePass(computePass);
			}
		}

		void GPUBVHTreeMorton::Construct(SDL_GPUCommandBuffer* commandBuffer, glm::vec3 globalMinAABB, glm::vec3 globalMaxAABB, int numEntries) {
			ZoneScoped;
			// Construct BVH using Morton Codes
			{
				struct UBO
				{
					int numEntries;
				};

				UBO ubo{
					.numEntries = numEntries
				};

				SDL_GPUStorageBufferReadWriteBinding rwStorageBindings[2];
				rwStorageBindings[0].buffer = m_BVHInternalNodes;
				rwStorageBindings[0].cycle = false;
				rwStorageBindings[1].buffer = m_BVHLeafNodes;
				rwStorageBindings[1].cycle = false;

				SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(commandBuffer, NULL, 0, rwStorageBindings, 2);
				SDL_BindGPUComputePipeline(computePass, m_ConstructBVHShader->GetShader());
				SDL_PushGPUComputeUniformData(commandBuffer, 0, &ubo, sizeof(UBO));
				SDL_DispatchGPUCompute(computePass, 1, 1, 1);
				SDL_EndGPUComputePass(computePass);
			}
		}

		SDL_GPUBuffer* GPUBVHTreeMorton::GetInternalNodes() {
			return m_BVHInternalNodes;
		}

		SDL_GPUBuffer* GPUBVHTreeMorton::GetLeafNodes() {
			return m_BVHLeafNodes;
		}

		int GPUBVHTreeMorton::GetMaxEntries() {
			return m_MaxEntries;
		}

		Expected<SDL_GPUFence*> GPUBVHTreeMorton::DebugDraw(SDL_GPUTexture* depthTexture, SDL_GPUTexture* targetTexture, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::uvec2 viewSize, int numEntries, BVHDebugDrawOpts& opts) {
			const float log_2_4 = std::log2f(BVH_BRANCHING_FACTOR);
			int numInternalLayers = std::ceil(std::log2f((float)numEntries) / log_2_4);

			struct DebugInfoUBO
			{
				glm::mat4 projectionMatrix;
				glm::mat4 viewMatrix;
				glm::mat4 cameraMatrix;
				glm::uvec2 viewSize;
				int numLeaves;
				int minLevel;
				int maxLevel;
			};

			DebugInfoUBO ubo = {
				.projectionMatrix = projectionMatrix,
				.viewMatrix = viewMatrix,
				.cameraMatrix = glm::inverse(viewMatrix),
				.viewSize = viewSize,
				.numLeaves = numEntries,
				.minLevel = std::max(0, std::min(opts.minLevel, numInternalLayers - 1)),
				.maxLevel = std::min(numInternalLayers, std::max(0, opts.maxLevel))
			};


			// Render Debug Rectangular Prisms
			{
				SDL_GPUBuffer* storageBuffers[2];
				storageBuffers[0] = m_BVHInternalNodes;
				storageBuffers[1] = m_BVHLeafNodes;

				int totalInternalNodes = seriesExponentSum(BVH_BRANCHING_FACTOR, numInternalLayers);
				int offset = seriesExponentSum(BVH_BRANCHING_FACTOR, std::min(std::max(0, numInternalLayers - 1), opts.minLevel));

				int numNodes = (totalInternalNodes + numEntries);// std::ceil((float)(numEntries) / (float)(BVH_BRANCHING_FACTOR)) + numEntries;

				// BVH Outlines
				SDL_GPUColorTargetInfo gTargetInfos[1];
				gTargetInfos[0] = {};
				gTargetInfos[0].clear_color = { 0 / 255.f, 0 / 255.f, 0 / 255.f, 0.f / 255.f };
				gTargetInfos[0].load_op = SDL_GPU_LOADOP_LOAD;
				gTargetInfos[0].store_op = SDL_GPU_STOREOP_STORE;
				gTargetInfos[0].texture = targetTexture;
				gTargetInfos[0].resolve_texture = NULL;
				gTargetInfos[0].cycle = false;
				gTargetInfos[0].mip_level = 0;
				gTargetInfos[0].layer_or_depth_plane = 0;
				gTargetInfos[0].cycle_resolve_texture = false;

				SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = {};
				depthStencilTargetInfo.clear_depth = 1.f;
				depthStencilTargetInfo.clear_stencil = 0;
				depthStencilTargetInfo.cycle = false;
				depthStencilTargetInfo.layer = 0;
				depthStencilTargetInfo.mip_level = 0;
				depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
				depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_DONT_CARE;
				depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_LOAD;
				depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
				depthStencilTargetInfo.texture = depthTexture;


				SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(m_GPU->Get());
				SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, gTargetInfos, 1, &depthStencilTargetInfo);
				SDL_BindGPUGraphicsPipeline(renderPass, m_DebugBVHPipeline);
				SDL_SetGPUStencilReference(renderPass, 0);
				SDL_PushGPUVertexUniformData(commandBuffer, 0, &ubo, sizeof(ubo));
				SDL_BindGPUVertexStorageBuffers(renderPass, 0, storageBuffers, 2);

				SDL_GPUBufferBinding indexBuffer = {};
				indexBuffer.buffer = m_DebugCubeIndexBuffer;
				indexBuffer.offset = 0;

				SDL_GPUBufferBinding vertexBuffers[1];
				vertexBuffers[0].buffer = m_DebugCubeVertexBuffer;
				vertexBuffers[0].offset = 0;
				Uint32 numIndices = sizeof(GPUBVHTree::DEBUG_CUBE_INDICES) / sizeof(uint32_t);
				SDL_BindGPUIndexBuffer(renderPass, &indexBuffer, SDL_GPUIndexElementSize::SDL_GPU_INDEXELEMENTSIZE_32BIT);
				SDL_BindGPUVertexBuffers(renderPass, 0, vertexBuffers, 1);

				SDL_DrawGPUIndexedPrimitives(renderPass, numIndices, numNodes, 0, 0, 0);

				SDL_EndGPURenderPass(renderPass);
				SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
				return fence;
			}
		}
	}
}
