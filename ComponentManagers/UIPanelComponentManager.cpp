#include "UIPanelComponentManager.h"
#include "Layer.h"

namespace Smasher {
	UIPanelComponentManager::UIPanelComponentManager(Layer& layer) :
		BaseComponentManager<UIPanelComponent>(layer)
	{
		m_MouseMoveSubscription = layer.Subscribe<Events::MouseMoveEvent>(&UIPanelComponentManager::OnMouseMove, this);
		m_MouseButtonSubscription = layer.Subscribe<Events::MouseButtonEvent>(&UIPanelComponentManager::OnMouseButton, this);
	}

	UIPanelComponent& UIPanelComponent::SetOnHoverCallback(std::function<void(Events::MouseMoveEvent&)> callback) {
		m_MouseMoveCallback = callback;
		return *this;
	}

	UIPanelComponent& UIPanelComponent::SetOnPressCallback(std::function<void(Events::MouseButtonEvent&)> callback) {
		m_MousePressCallback = callback;
		return *this;
	}

	void UIPanelComponentManager::RenderComponents(sf::RenderWindow& rWindow)
	{
		for (auto& itr : m_Components) {
			UIPanelComponent::StaticRenderComponent(itr, rWindow);
		}
	}

	void UIPanelComponentManager::OnMouseMove(Events::MouseMoveEvent& event) {
		for (auto& itr : m_Components) {
			UIPanelState prev = itr.GetPanelState();
			if (!event.IsBlocked() && itr.IntersectsPanel(event.Position.x, event.Position.y)) {
				itr.SetPanelState(prev | UIPanelState::HOVERED);
				if ((bool)(itr.GetPanelSettings() & UIPanelSettings::BLOCK_MOUSE_MOVE)) {
					event.Block();
				}
			}
			else {
				itr.SetPanelState(prev & ~UIPanelState::HOVERED);
			}

			if (prev != itr.GetPanelState()) {
				itr.OnHoverEvent(event);
			}
		}
	}

	void UIPanelComponentManager::OnMouseButton(Events::MouseButtonEvent& event) {
		for (auto& itr : m_Components) {
			UIPanelState prev = itr.GetPanelState();
			bool pressed = (event.Type == Mouse::MouseEventType::BUTTON_PRESS && event.ButtonCode == sf::Mouse::Button::Left);
			if (itr.IntersectsPanel(event.Position.x, event.Position.y) && pressed) {
				itr.SetPanelState(prev | UIPanelState::PRESSED);
				if ((bool)(itr.GetPanelSettings() & UIPanelSettings::BLOCK_MOUSE_PRESS)) {
					event.StopPropagate();
				}
			}
			else {
				itr.SetPanelState(prev & ~UIPanelState::PRESSED);
			}

			if (prev != itr.GetPanelState()) {
				itr.OnPressEvent(event);
			}
		}
	}

}
