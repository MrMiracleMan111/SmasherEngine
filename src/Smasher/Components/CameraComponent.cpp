#include <iostream>
#include "Smasher/Components/CameraComponent.h"
#include "Smasher/IComponentManager.h"
#include "Smasher/Layer.h"
#include "Smasher/Events.h"
#include "Smasher/EventManager.h"

namespace Smasher {
	void CameraComponent::OnAddComponent()
	{
		EventManager& rEventManager = GetManager().GetLayer().GetEngine().GetEventManager();

		m_ResizeHandle = GetEntity().GetLayer().Subscribe<Events::WindowResizeEvent>(&CameraComponent::OnWindowResize, this);
	}

	void CameraComponent::OnWindowResize(Events::WindowResizeEvent& event) {
		m_View.setSize((float)event.WindowSize.x, (float)event.WindowSize.y);
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

	CameraComponent& CameraComponent::SetTarget(sf::RenderTarget& target)
	{
		m_Target = &target;
		return *this;
	}

	CameraComponent& CameraComponent::ApplyToTarget()
	{
		if (m_Target == nullptr) {
			throw Exceptions::CameraTargetNotSet("m_Target is NULL");
		}
		m_Target->setView(m_View);
		return *this;
	}

	CameraComponent& CameraComponent::ApplyToTarget(sf::RenderTarget& target)
	{
		target.setView(m_View);
		return *this;
	}
}