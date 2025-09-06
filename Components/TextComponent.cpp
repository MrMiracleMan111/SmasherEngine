#include "TextComponent.h"
#include "Entity.h"

namespace Smasher {
	void TextComponent::StaticRenderComponent(TextComponent& self, sf::RenderWindow& rWindow) {
		if (self.m_FontLoaded) {
			rWindow.draw(self.m_Text);
		}
	}

	void TextComponent::SetEntity(Entity& rEntity)
	{
		IComponent::SetEntity(rEntity);
	}

	TextComponent& TextComponent::SetString(const std::string& str) {
		m_Text.setString(str);
		return *this;
	}

	TextComponent& TextComponent::SetFontSize(unsigned int size) {
		m_Text.setCharacterSize(size);
		return *this;
	}
	TextComponent& TextComponent::SetFillColor(sf::Color color)
	{
		m_Text.setFillColor(color);
		return *this;
	}
	TextComponent& TextComponent::SetOutlineThickness(float thickness)
	{
		m_Text.setOutlineThickness(thickness);
		return *this;
	}
	TextComponent& TextComponent::SetOutlineColor(sf::Color color)
	{
		m_Text.setOutlineColor(color);
		return *this;
	}
}