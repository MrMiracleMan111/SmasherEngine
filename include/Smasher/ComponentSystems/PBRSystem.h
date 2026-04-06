#pragma once
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include "Smasher/Base.h"
#include "Smasher/Resources.h"
#include "Smasher/ErrorCodes.h"
#include "Smasher/Exceptions.h"
#include "Smasher/ComponentSystems/CameraSystem.h"
#include "Smasher/ComponentSystems/SDLSystem.h"
#include "Smasher/ComponentSystems/StaticMeshSystem.h"

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
		SDL_GPUTextureFormat NORMALS_TEX_FORMAT = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
		SDL_GPUTextureFormat UV_TEX_FORMAT = SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT;// SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT;

		struct Context {
			// G-Buffer Objects
			SDL_GPUTexture *gDepthPrePass;
			SDL_GPUTexture *gNormals;
			SDL_GPUTexture *gUV;
			SDL_GPUTexture *gAlbedo;
			SDL_GPUTexture *gSpecular;
			SDL_GPUTexture *gLighting;

			SDL_GPUSampler* gDepthPrePassSampler;
			SDL_GPUSampler* gNormalsSampler;
			SDL_GPUSampler* gUVSampler;
			SDL_GPUSampler* gAlbedoSampler;
			SDL_GPUSampler* gSpecularSampler;
			SDL_GPUSampler* gLightingSampler;

			SDL_GPUGraphicsPipeline *depthPassPipeline;

			std::shared_ptr<SDLGraphicShaderResource> depthPassFragShader;
			std::shared_ptr<SDLGraphicShaderResource> depthPassVertShader;

			SDL_GPUBuffer *testInstanceBuffer;
			StaticMeshSystem::Component testTeapot;
			std::shared_ptr<StaticMeshResource> testTeapotMeshResource;
		};

		enum class LightType {
			POINT,
			SPOTLIGHT
		};

		struct Light {
			glm::vec3 position;
			glm::vec3 direction;
			glm::uvec3 color;
			LightType type;
			float intensity;
		};

		SMASHER_API ErrorCode Initialize(entt::registry& registry, std::shared_ptr<SDL_GPUDeviceWrapper> gpu);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
		SMASHER_API ErrorCode OnWindowResize(Context& ctx, SDL_GPUDevice* device);
		SMASHER_API ErrorCode DepthPrePass(Context& ctx, SDL_GPUDevice* device, const CameraSystem::Component& camera);
		SMASHER_API ErrorCode LightingPass(Context& ctx, SDL_GPUDevice* device);
		SMASHER_API ErrorCode ShadowPass(Context& ctx, SDL_GPUDevice* device, const CameraSystem::Component& camera);
		SMASHER_API ErrorCode MaterialsPass(Context& ctx, SDL_GPUDevice* device);
	}
}
