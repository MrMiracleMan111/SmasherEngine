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
#include "Smasher/Base.h"
#include "Smasher/ComponentManagers/BaseComponentManager.h"
#include "Smasher/ComponentManagers/RenderBatch.h"
#include "Smasher/Components/DrawableComponent.h"
#include "Smasher/Events.h"

namespace Smasher {
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Component Manager for rendering @ref DrawableComponent.
/// 
/// @details
/// @ref DrawableComponentManager utilizes render batches to improve rendering performance.
/// @ref DrawableComponent instances are grouped by texture type and then assigned to @ref RenderBatch instances
/// which can each hold up to @ref RenderBatch::MAX_MODEL_COUNT number of @ref DrawableComponent instances.
//////////////////////////////////////////////////////////////////////////////////////////////////
	class SMASHER_API DrawableComponentManager : public BaseComponentManager<DrawableComponent> {
	public:
		DrawableComponentManager() = delete;
		DrawableComponentManager(Layer& state);
		DrawableComponentManager(const DrawableComponentManager&) = default;
		~DrawableComponentManager();

		void Render(sf::RenderWindow& rWindow) override;

		template<typename... Args>
		DrawableComponent& AddComponent(Entity& entity, Args&&... args);

		// Sets component batch index and render batch pointer
		void OnComponentChangeData(DrawableComponent& rComponent);
		void OnComponentSetTexture(DrawableComponent& rComponent, ResourceID id, bool transulcent);
		void OnComponentDelete(DrawableComponent& rComponent);
		void SetShaderResource(std::shared_ptr<ShaderResource> pShaderResource) { m_ShaderResource = pShaderResource; }

		void OnWindowClose(Events::WindowCloseEvent& event);

		std::shared_ptr<ShaderResource> GetDefaultShader() const { return m_DefaultShader; }

	private:
		void DrawBatch(RenderBatch& rRenderBatch);
		std::map<ResourceID, std::list<RenderBatch>> m_OpaqueBatches; // Linked List of Batches
		std::map<ResourceID, std::list<RenderBatch>> m_TranslucentBatches;// Linked List of Batches

		std::shared_ptr<ShaderResource> m_ShaderResource; // Solely for preventing destruction of resource object
		std::shared_ptr<ShaderResource> m_DefaultShader; // Shader loaded from EngineConfig.h
	};
}

#include "DrawableComponentManager.inl"