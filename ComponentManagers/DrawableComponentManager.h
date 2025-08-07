#pragma once
#include <GL/glew.h>
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__linux__)
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#endif
#include <memory>
#include <map>
#include <SFML/Window.hpp>
#include "Base.h"
//#include "Components/DrawableComponent.h"
#include "ComponentManagers/BaseComponentManager.h"
#include "RenderBatch.h"
#include "Components/DrawableComponent.h"
#include "Events.h"

namespace Smasher {
	class SMASHER_API DrawableComponentManager : public BaseComponentManager<DrawableComponent> {
	public:
		DrawableComponentManager(GameState& state);
		DrawableComponentManager(const DrawableComponentManager&) = default;
		~DrawableComponentManager() = default;

		void Render(sf::RenderWindow& rWindow) override;

		// Sets component batch index and render batch pointer
		void OnComponentChangeData(DrawableComponent& rComponent);
		void OnComponentSetTexture(DrawableComponent& rComponent, ResourceID id, bool transulcent);
		void OnComponentDelete(DrawableComponent& rComponent);
		void SetShaderResource(std::shared_ptr<ShaderResource> pShaderResource) { m_ShaderResource = pShaderResource; }

		void OnWindowClose(const WindowCloseEvent& event);

	private:
		void DrawBatch(RenderBatch& rRenderBatch);
		std::map<ResourceID, RenderBatch> m_OpaqueBatches;
		std::map<ResourceID, RenderBatch> m_TranslucentBatches;

		std::shared_ptr<ShaderResource> m_ShaderResource; // Solely for preventing destruction of resource object
	};
}