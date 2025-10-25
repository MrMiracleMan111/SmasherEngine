#define _USE_MATH_DEFINES 1
#include <cmath>
#include "Smasher/ComponentManagers/UIPanelComponentManager.h"
#include "Smasher/Components/UIPanelComponent.h"
#include "Smasher/Base.h"
#include "Smasher/Entity.h"
#include "Smasher/Events.h"

namespace Smasher {
	UIPanelComponent::UIPanelComponent() : m_DebugSprite(sf::Vector2f(1.0f, 1.0f)), IComponent(), Transform2DWrapper(*this, m_Transformable)
	{
		m_DebugSprite.setFillColor(sf::Color::White);
		m_DebugSprite.setOrigin(0.5f, 0.5f);
	}

	void UIPanelComponent::StaticRenderComponent(UIPanelComponent& self, sf::RenderWindow& rWindow)
	{
		self.m_DebugSprite.setPosition(self.m_Transformable.getPosition());
		self.m_DebugSprite.setRotation(self.m_Transformable.getRotation());
		self.m_DebugSprite.setScale(self.m_Transformable.getScale());
		rWindow.draw(self.m_DebugSprite);
	}

	bool UIPanelComponent::IntersectsPanel(int x, int y)
	{
		float width = m_Transformable.getScale().x;
		float height = m_Transformable.getScale().y;
		sf::Vector2f pos((float)x, (float)y);
		pos = m_Transformable.getPosition() - pos;
		Smasher::Radians radians = std::atan2(pos.y, pos.x);
		float dist = std::sqrt(pos.x * pos.x + pos.y * pos.y);
		Smasher::Radians inversePanelRadians = -m_Transformable.getRotation() * (M_PI / 180.f);
		Smasher::Radians finalRadians = radians + inversePanelRadians;
		pos.x = std::cos(finalRadians) * dist;
		pos.y = std::sin(finalRadians) * dist;

		return (std::abs(pos.x) <= (width / 2)) && (std::abs(pos.y) <= (height / 2));
	}

	UIPanelComponent& UIPanelComponent::SetColor(const sf::Color& color)
	{
		m_DebugSprite.setFillColor(color);
		return *this;
	}

	void UIPanelComponent::OnHoverEvent(Events::MouseMoveEvent& event)
	{
		if (m_MouseMoveCallback) {
			m_MouseMoveCallback(event);
		}
	}

	void UIPanelComponent::OnPressEvent(Events::MouseButtonEvent& event)
	{
		if (m_MousePressCallback) {
			m_MousePressCallback(event);
		}
	}
}