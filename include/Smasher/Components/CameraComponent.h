#pragma once
#include <SFML/System.hpp>
#include "Smasher/Base.h"
#include "Smasher/IComponent.h"
#include "Smasher/Components/Transform2DWrapper.h"
#include "Smasher/EventManager.h"

namespace Smasher {

	namespace Events {
		struct WindowResizeEvent;
	}

	class SMASHER_API CameraComponent : public IComponent {
	public:
		CameraComponent& SetPosition(sf::Vector2f position);
		CameraComponent& Move(sf::Vector2f position);
		CameraComponent& SetRotation(Degrees rotation);
		CameraComponent& Rotate(Degrees rotation);
		CameraComponent& SetSize(sf::Vector2f size);
		CameraComponent& Zoom(float factor);
		CameraComponent& SetTarget(sf::RenderTarget& target);
		CameraComponent& ApplyToTarget();
		CameraComponent& ApplyToTarget(sf::RenderTarget& target);


		void OnAddComponent() override;
		void OnWindowResize(Events::WindowResizeEvent& event);

		const sf::View& GetView() const { return m_View; };
		sf::Vector2f GetPosition() const { return m_View.getCenter(); };
		Degrees GetRotation() const { return Degrees{ m_View.getRotation() }; };
		sf::Vector2f GetSize() const { return m_View.getSize(); };
		sf::RenderTarget& GetTarget() const { return *m_Target; }
	private:
		EventSubscriptionHandle m_ResizeHandle;
		sf::View m_View;
		sf::RenderTarget* m_Target = nullptr;
	};
}