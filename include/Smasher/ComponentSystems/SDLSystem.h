#pragma once
#include <SDL3/SDL.h>
#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>
#include "Smasher/Base.h"
#include "Smasher/ErrorCodes.h"
#include "Smasher/Resources.h"
#include "Smasher/EventFeeder.h"

namespace Smasher {
	namespace SDLSystem {
		static const int MAX_COMPOSITION_TEXTURE_PAIRS = 8;

		// the vertex input layout
		struct QuadVertex
		{
			float x, y, z, u, v;      //vec3 position, vec2 uv coodrinates
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

		struct SMASHER_API RenderTexture {
			SDL_GPUTexture	*colorTarget;
			SDL_GPUTexture	*depthTarget;
		};

		struct SMASHER_API Context {
			std::shared_ptr<SDL_GPUDeviceWrapper> pGpu;
			std::shared_ptr<SDLGraphicShaderResource> compositeFragShader;
			std::shared_ptr<SDLGraphicShaderResource> compositeVertShader;
			std::shared_ptr<SDLGraphicShaderResource> compositeCopyFragShader;
			std::shared_ptr<SDLGraphicShaderResource> compositeCopyVertShader;
			std::shared_ptr<SDLComputeShaderResource> compositeCompShader;

			glm::uvec2 windowSize{ 1u, 1u };
			SDL_Window *window = nullptr;
			SDL_GPUCommandBuffer *commandBuffer = nullptr;
			SDL_GPUTexture *swapChainTexture = nullptr;
			SDL_GPUTexture *gColorTexture = nullptr;	// We swap between writing to gColorTexture and _gColorIntermediateTexture
			SDL_GPUTexture *gDepthTexture = nullptr;	// We swap between writing to gDepthTexture and _gDepthIntermediateTexture
			SDL_GPUTexture *_gColorIntermediateTexture = nullptr;	// We swap between writing to gColorTexture and _gColorIntermediateTexture
			SDL_GPUTexture *_gDepthIntermediateTexture = nullptr;	// We swap between writing to gDepthTexture and _gDepthIntermediateTexture
			SDL_GPUBuffer *screenQuadVertBuffer = nullptr;
			SDL_GPUGraphicsPipeline *compositePipeline = nullptr;
			SDL_GPUGraphicsPipeline* compositeCopyPipeline = nullptr;
			SDL_GPUSampler *compositeSampler = nullptr;
		};

		SMASHER_API ErrorCode Initialize(entt::registry& registry, const WindowOptions& opts);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
		SMASHER_API ErrorCode PollEvents(entt::registry& registry);

		SMASHER_API ErrorCode OnWindowResize(SDLSystem::Context& ctx);
		
		SMASHER_API SDL_GPUTextureFormat GetGPUDepthStencilFormat(SDL_GPUDevice* device);

		SMASHER_API SDL_GPUTextureFormat GetGPUDepthFormat(SDL_GPUDevice *device);

		SMASHER_API ErrorCode CopyToWindow(SDLSystem::Context& ctx, SDL_GPUTexture *texture);
		// Composes render targets onto swapchain texture and global depth/stencil texture
		SMASHER_API ErrorCode CompositionPass(SDLSystem::Context &ctx, const std::vector<RenderTexture>& sources);
	}
}