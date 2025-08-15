#include <iostream>
#include "CameraComponent.h"
#include "IComponentManager.h"
#include "GameState.h"
#include "Events.h"
#include "EventManager.h"

namespace Smasher {
	void CameraComponent::OnAddComponent()
	{
		EventManager& rEventManager = GetManager().GetGameState().GetEngine().GetEventManager();

		m_ResizeHandle = rEventManager.Subscribe<Events::WindowResizeEvent>(&CameraComponent::OnWindowResize, this);
	}

	void CameraComponent::OnWindowResize(const Events::WindowResizeEvent& event) {
		m_View.setSize(event.WindowSize.x, event.WindowSize.y);
	}

	CameraComponent& CameraComponent::SetPosition(sf::Vector2f position) {
		m_View.setCenter(position);
		return *this;
	}

	CameraComponent& CameraComponent::Move(sf::Vector2f position) {
		m_View.move(position);
		return *this;
	}

	CameraComponent& CameraComponent::SetRotation(Degrees rotation) {
		m_View.setRotation(rotation);
		return *this;
	}

	CameraComponent& CameraComponent::Rotate(Degrees rotation) {
		m_View.rotate(rotation);
		return *this;
	}


	CameraComponent& CameraComponent::SetSize(sf::Vector2f size) {
		m_View.setSize(size);
		return *this;
	}

	CameraComponent& CameraComponent::Zoom(float factor) {
		m_View.zoom(factor);
		return *this;
	}
}