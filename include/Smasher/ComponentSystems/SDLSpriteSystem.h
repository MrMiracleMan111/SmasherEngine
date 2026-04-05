#include <SDL3/SDL.h>
#include <entt/entity/registry.hpp>
#include "Smasher/Base.h"
#include "Smasher/ErrorCodes.h"
#include "Smasher/Events.h"
#include "Smasher/Resources.h"
namespace Smasher {
	namespace SDLSpriteSystem {

		// the vertex input layout
		struct Vertex
		{
			float x, y, z;      //vec3 position
			float r, g, b, a;   //vec4 color
		};

		// a list of vertices
		static Vertex TEST_vertices[]
		{
			{0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},     // top vertex
			{-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f},   // bottom left vertex
			{0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f}     // bottom right vertex
		};

		struct SMASHER_API Context {
			SDL_GPUTexture							 *renderTexture;
			SDL_GPUTexture							 *depthTexture;
			SDL_GPUGraphicsPipeline					 *renderPipeline;
			SDL_Window								 *window;
			SDL_GPUBuffer							 *vertexBuffer;
			SDL_GPUTransferBuffer					 *transferBuffer;
			std::shared_ptr<SDL_GPUDeviceWrapper>	  pGPU;
			std::shared_ptr<SDLGraphicShaderResource> pFragShaderResource;
			std::shared_ptr<SDLGraphicShaderResource> pVertShaderResource;
		};

		SMASHER_API ErrorCode Initialize(
			entt::registry& registry,
			std::shared_ptr<SDL_GPUDeviceWrapper> pGPU,
			std::shared_ptr<SDLGraphicShaderResource> pFrag,
			std::shared_ptr<SDLGraphicShaderResource> pVert,
			SDL_Window* window);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
		SMASHER_API void OnWindowResize(const Smasher::Events::WindowResizeEvent &event, entt::registry& registry);
	
		SMASHER_API ErrorCode DrawTriangle(Context& ctx);
	}
}
