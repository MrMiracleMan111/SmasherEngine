#include "CameraComponent.h"

namespace Smasher {
	CameraComponent& CameraComponent::SetPosition(sf::Vector2f position) {
		m_View.setCenter(position);
		return *this;
	}

	CameraComponent& CameraComponent::SetRotation(Degrees rotation) {
		m_View.setRotation(rotation);
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