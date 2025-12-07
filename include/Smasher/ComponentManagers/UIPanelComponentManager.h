#pragma once

#include "Smasher/Base.h"
#include "Smasher/ComponentManagers/BaseComponentManager.h"
#include "Smasher/Events.h"
#include "Smasher/Components/UIPanelComponent.h"

namespace Smasher {
	class Entity;
	class UIPanelComponent;
	class UIPanelComponentManager : public BaseComponentManager<UIPanelComponent> {
	public:
		UIPanelComponentManager() = delete;
		UIPanelComponentManager(Layer &state);
		UIPanelComponentManager(const UIPanelComponentManager&) = default;
		~UIPanelComponentManager() = default;

		template<typename... Args>
		UIPanelComponent& AddComponent(Entity &entity, Args&&... args);

		void RenderComponents(sf::RenderWindow &window) override;

		void OnMouseMove(Events::MouseMoveEvent &event);
		void OnMouseButton(Events::MouseButtonEvent &event);
		void SetShaderResource(std::shared_ptr<ShaderResource> pShaderResource) { m_ShaderResourcePtr = pShaderResource; }

	private:
		EventSubscriptionHandle m_MouseMoveSubscription;
		EventSubscriptionHandle m_MouseButtonSubscription;
		std::shared_ptr<ShaderResource> m_ShaderResourcePtr;
	};
}

#include "Smasher/ComponentManagers/UIPanelComponentManager.inl"