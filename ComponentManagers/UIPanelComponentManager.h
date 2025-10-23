#pragma once

#include "Smasher/Base.h"
#include "ComponentManagers/BaseComponentManager.h"
#include "Smasher/Events.h"
#include "Smasher/Components/UIPanelComponent.h"

namespace Smasher {
	class UIPanelComponent;
	class SMASHER_API UIPanelComponentManager : public BaseComponentManager<UIPanelComponent> {
	public:
		UIPanelComponentManager() = delete;
		UIPanelComponentManager(Layer& state);
		UIPanelComponentManager(const UIPanelComponentManager&) = default;
		~UIPanelComponentManager() = default;

		void RenderComponents(sf::RenderWindow& rWindow) override;

		void OnMouseMove(Events::MouseMoveEvent& event);
		void OnMouseButton(Events::MouseButtonEvent& event);

	private:
		EventSubscriptionHandle m_MouseMoveSubscription;
		EventSubscriptionHandle m_MouseButtonSubscription;
	};
}