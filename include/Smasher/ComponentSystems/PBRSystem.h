#pragma once
#include "Smasher/ComponentSystems/StaticMeshSystem.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <queue>
#include "Smasher/Base.h"
#include "Smasher/Resources.h"
#include "Smasher/ErrorCodes.h"
#include "Smasher/Exceptions.h"
#include "Smasher/Util/GraphicsUtil.h"
#include "Smasher/Util/GPUBVHTree.h"
#include "Smasher/ComponentSystems/CameraSystem.h"
#include "Smasher/ComponentSystems/SDLSystem.h"

// Physical Based Rendering System
// From this guide: https://www.3dgep.com/forward-plus/#Deferred_Shading
//  ========= RENDER PIPELINE ========= 
//
// 
//   OPQAUE STAGE
// 
//	 ++++BUFFERS++++
//   Depth/Stencil		(D24_UNORM_S8_UINT)  Stencil Buffer will store material ID
//	 UV Position		(R16G16_UNORM)
//	 Light Accumulation (R8G8B8A8_UNORM  Back Buffer)
//   Albedo				(R8G8B8A8_UNORM)
//   Specular			(R8G8B8A8_UNORM)
//   Normals			(R32G32B32A32_FLOAT)
//
//   ++++PASSES++++
//   G-Buffer Depth Pre-Pass
//			- Writes Material ID to stencil buffer
//			- Writes UV Position to UV Position buffer
//			- Writes Normals to normals buffer
//	 Transluscent Pass
//
namespace Smasher {
	namespace PBRSystem {
		// Maximum number of different types of static meshe models
		static const int MAX_LIGHT_INSTANCES = 128;
		static const int MAX_STATIC_MESH_MESHES = 128;
		static const int MAX_STATIC_MESH_MATERIALS = 128;
		static const int MAX_STATIC_MESH_INSTANCE_COUNT = 1024 * 64;
		static const int MICRO_TEXTURE_DIMENSION = 128;
		static const int SMALL_TEXTURE_DIMENSION = 256;
		static const int MEDIUM_TEXTURE_DIMENSION = 512;
		static const int LARGE_TEXTURE_DIMENSION = 1024;
		static const int SHADOWMAP_TEXTURE_DIMENSION = 1024;

		struct StaticMeshProps {
			glm::vec3 aabbMin;
			uint32_t resourceId;
			glm::vec3 aabbMax;
			uint32_t padding;
		};

		struct BVHInternalNode
		{
			glm::vec3 minAABB;
			glm::vec3 maxAABB;
			int children[32];
			int childCount;
			int parentNode;
		};

		struct BVHLeafNode
		{
			glm::vec3 minAABB;
			glm::vec3 maxAABB;
			int parentNode;
			int instanceIndex;
		};

		struct MaterialProps {
			glm::vec4 albedo;		// float 16 align
			glm::vec4 specular;		// float 16 align
			int albedoTextureIndex; // -1 = invalid
			uint32_t resourceId;	// Id of Resource in ResourceManager
			int padding1;
			int padding2;
		};

		constexpr static MaterialProps DEFAULT_MATERIAL_PROPS {
			.albedo = glm::vec4{ 1.f, 0.f, 1.f, 1.f },
			.specular = glm::vec4{ 0.f, 0.f, 0.f, 0.f },
			.albedoTextureIndex = 0
		};

		constexpr static StaticMeshProps DEFAULT_STATIC_MESH_PROPS {
			.aabbMin = glm::vec3{0.f, 0.f, 0.f},
			.resourceId = 0,
			.aabbMax = glm::vec3{0.f, 0.f, 0.f},
			.padding = 0
		};

		SDL_GPUTextureFormat SHADOWMAP_TEX_FORMAT = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
		SDL_GPUTextureFormat ALBEDO_TEX_FORMAT = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
		SDL_GPUTextureFormat LIGHTING_TEX_FORMAT = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
		SDL_GPUTextureFormat NORMALS_TEX_FORMAT = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
		SDL_GPUTextureFormat MATERIAL_IDS_TEX_FORMAT = SDL_GPU_TEXTUREFORMAT_R32_INT;
		SDL_GPUTextureFormat UV_TEX_FORMAT = SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT;// SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT;

		enum LightType : int {
			DIRECTIONAL,
			POINT,
			SPOT
		};

		struct LightData {
			glm::vec3 direction;
			LightType type;
			glm::vec3 position;
			float spreadAngle;
			glm::vec3 color;
			float falloff;
		};

		// the vertex input layout
		struct QuadVertex
		{
			float x, y, z, u, v;      //vec3 position, vec2 uv coodrinates
		};

		struct DebugVertex
		{
			float x, y, z;
		};

		// Fullscreen quad
		static const QuadVertex SCREEN_QUAD[]
		{
			{-1.0f,	-1.0f, 0.0f,		0.0f, 1.0f},    // top left vertex
			{-1.0f,  1.0f, 0.0f,		0.0f, 0.0f},	// bottom left vertex
			{ 1.0f,	 1.0f, 0.0f,		1.0f, 0.0f},    // bottom right vertex

			{ 1.0f,  1.0f, 0.0f,		1.0f, 0.0f},	// bottom right vertex
			{ 1.0f,	-1.0f, 0.0f,		1.0f, 1.0f},	// top right vertex
			{-1.0f, -1.0f, 0.0f,		0.0f, 1.0f}     // top left vertex
		};

		static const DebugVertex DEBUG_CUBE_VERTICES[]
		{
			-1.f, -1.f, -1.f, // 0: Bottom-Back-Left
			 1.f, -1.f, -1.f, // 1: Bottom-Back-Right
			 1.f,  1.f, -1.f, // 2: Top-Back-Right
			-1.f,  1.f, -1.f, // 3: Top-Back-Left
			-1.f, -1.f,  1.f, // 4: Bottom-Front-Left
			 1.f, -1.f,  1.f, // 5: Bottom-Front-Right
			 1.f,  1.f,  1.f, // 6: Top-Front-Right
			-1.f,  1.f,  1.f  // 7: Top-Front-Left
		};

		static const uint32_t DEBUG_CUBE_INDICES[]
		{
			// Back face
			0, 1, 1, 2, 2, 3, 3, 0,
			// Front face
			4, 5, 5, 6, 6, 7, 7, 4,
			// Edges
			0, 4, 1, 5, 2, 6, 3, 7
		};

		struct Context {
			GraphicsUtil::GPUBlockPool<StaticMeshSystem::StaticMeshData> staticMeshBatchPool;
			GraphicsUtil::GPURadixSortPool radixSortPool;
			GraphicsUtil::GPUBVHTreeMorton bvhTree;
			std::array<std::shared_ptr<MaterialResource>, MAX_STATIC_MESH_MATERIALS> staticMeshMaterialBindings;
			std::array<std::shared_ptr<StaticMeshResource>, MAX_STATIC_MESH_MATERIALS> staticMeshBindings;
			glm::vec2 viewSize;
			SDL_GPUTransferBuffer *staticMeshTransferBuffer;
			SDL_GPUTransferBuffer *materialPropsTransferBuffer;
			SDL_GPUTransferBuffer *meshPropsTransferBuffer;

			SDL_GPUBuffer *screenQuadBuffer;
			SDL_GPUBuffer *gMaterialProps;
			SDL_GPUBuffer *gMeshProps;
			SDL_GPUBuffer* gLightInstances;
			SDL_GPUBuffer *BVHInternalNodes;
			SDL_GPUBuffer* BVHLeafNodes;

			SDL_GPUBuffer* debugCubeVertexBuffer;
			SDL_GPUBuffer* debugCubeIndexBuffer;


			// Static Mesh Materials Texture Array of 512 x 512 textures
			// 256 materials
			SDL_GPUTexture* staticMeshAlbedos512; 

			// G-Buffer Objects
			SDL_GPUTexture *gDepthPrePass;
			SDL_GPUTexture *gMaterialIds;
			SDL_GPUTexture *gTriangleNormals;
			SDL_GPUTexture *gComputedNormals;
			SDL_GPUTexture *gUV;
			SDL_GPUTexture *gAlbedo;
			SDL_GPUTexture *gSpecular;
			SDL_GPUTexture *gLighting;

			SDL_GPUTexture *gShadowMap;

			SDL_GPUSampler* gDepthPrePassSampler;
			SDL_GPUSampler* gNormalsSampler;
			SDL_GPUSampler* gUVSampler;
			SDL_GPUSampler* gAlbedoSampler;
			SDL_GPUSampler* gSpecularSampler;
			SDL_GPUSampler* gLightingSampler;

			SDL_GPUGraphicsPipeline *depthPassPipeline;
			SDL_GPUGraphicsPipeline *materialPassPipeline;
			SDL_GPUGraphicsPipeline *shadowPassPipeline;
			SDL_GPUGraphicsPipeline *debugBVHPipeline;

			std::shared_ptr<SDLGraphicShaderResource> depthPassFragShader;
			std::shared_ptr<SDLGraphicShaderResource> depthPassVertShader;
			std::shared_ptr<SDLGraphicShaderResource> materialPassFragShader;
			std::shared_ptr<SDLGraphicShaderResource> materialPassVertShader;
			std::shared_ptr<SDLGraphicShaderResource> shadowPassFragShader;
			std::shared_ptr<SDLGraphicShaderResource> shadowPassVertShader;
			std::shared_ptr<SDLGraphicShaderResource> debugBVHFragShader;
			std::shared_ptr<SDLGraphicShaderResource> debugBVHVertShader;


			std::shared_ptr<SDLComputeShaderResource> lightingShader;
			std::shared_ptr<SDLComputeShaderResource> radixInputShader;
			std::shared_ptr<SDLComputeShaderResource> initializeBVHShader;
			std::shared_ptr<SDLComputeShaderResource> constructBVHShader;

			std::shared_ptr<SDL_GPUDeviceWrapper> gpu;
		};

		SMASHER_API ErrorCode Initialize(entt::registry& registry, std::shared_ptr<SDL_GPUDeviceWrapper> gpu);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
		SMASHER_API Expected<MaterialBinding> BindMaterial(Context& ctx, std::shared_ptr<MaterialResource> material);
		SMASHER_API Expected<StaticMeshBinding> BindStaticMesh(Context& ctx, std::shared_ptr<StaticMeshResource> mesh);
		SMASHER_API ErrorCode OnWindowResize(Context& ctx);
		SMASHER_API ErrorCode SyncStaticMeshInstances(Context& ctx, const StaticMeshSystem::Context& staticMeshCtx);
		SMASHER_API ErrorCode DepthPrePass(Context& ctx, const StaticMeshSystem::Context& staticMeshCtx, ResourceManager& resourceManager, const CameraSystem::Component& camera);
		SMASHER_API ErrorCode LightingPass(Context& ctx, const Smasher::StaticMeshSystem::Context& staticMeshCtx, ResourceManager& resourceManager, const CameraSystem::Component& camera);
		SMASHER_API ErrorCode ShadowPass(Context& ctx, const Smasher::StaticMeshSystem::Context& staticMeshCtx, ResourceManager &resourceManager, const CameraSystem::Component& camera);
		SMASHER_API ErrorCode MaterialsPass(Context& ctx, const CameraSystem::Component& camera);
		SMASHER_API ErrorCode ConstructBVH(Context& ctx, const StaticMeshSystem::Context& staticMeshCtx);
		SMASHER_API ErrorCode DebugDrawBVH(Context& ctx, SDL_GPUBuffer* bvhBuffer, SDL_GPUTexture* depthTexture, SDL_GPUTexture* outputTexture, const CameraSystem::Component& camera, int numLeaves);
	}
}