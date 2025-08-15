#pragma once
#include <SFML/System.hpp>
#include "Base.h"
#include "IComponent.h"
#include "Components/Transform2DWrapper.h"
#include "EventManager.h"

namespace Smasher {

	namespace Events {
		struct WindowResizeEvent;
	}

	class SMASHER_API CameraComponent : public IComponent {
	public:
		CameraComponent& SetPosition(sf::Vector2f position);
		CameraComponent& SetRotation(Degrees rotation);
		CameraComponent& SetSize(sf::Vector2f size);
		CameraComponent& Zoom(float factor);

		void OnAddComponent() override;
		void OnWindowResize(const Events::WindowResizeEvent& event);

		const sf::View& GetView() const { return m_View; };
		sf::Vector2f GetPosition() const { return m_View.getCenter(); };
		Degrees GetRotation() const { return Degrees{ m_View.getRotation() }; };
		sf::Vector2f GetSize() const { return m_View.getSize(); };

	private:
		EventSubscriptionHandle m_ResizeHandle;
		sf::View m_View;
	};
}