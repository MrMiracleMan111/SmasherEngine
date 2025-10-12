#pragma once
#include "Base.h"
#include "IComponent.h"
#include "ResourceManager.h"
#include "EngineConfig.h"
#include "Transform2DWrapper.h"
#include "EventManager.h"

namespace Smasher {
	class SMASHER_API UIPanelComponent : public IComponent, public Transform2DWrapper<UIPanelComponent> {
	public:
		UIPanelComponent() : IComponent(), Transform2DWrapper(*this) {}
		UIPanelComponent(const UIPanelComponent&) = default;
		UIPanelComponent& operator=(const UIPanelComponent&) = default;


		static void StaticRenderComponent(UIPanelComponent& self, sf::RenderWindow& rWindow);
		
	private:
		EventSubscriptionHandle m_MousePressSubscription;
		EventSubscriptionHandle m_MouseReleaseSubscription;
	};
}