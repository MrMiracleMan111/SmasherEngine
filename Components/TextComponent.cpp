#include "TextComponent.h"
#include "Entity.h"

namespace Smasher {
	void TextComponent::StaticRenderComponent(TextComponent& self, sf::RenderWindow& rWindow) {
		if (self.m_FontLoaded) {
			Transform2DComponent& rTransform = self.GetEntity().GetComponent<Transform2DComponent>();
			rWindow.draw(self.m_Text, rTransform.GetTransform());
			//self.m_Text.setCharacterSize(60);
			//self.m_Text.setPosition(10.0f, 10.0f);
			//self.m_Text.setColor(sf::Color::White);
			//rWindow.draw(self.m_Text);
		}
	}

	void TextComponent::SetEntity(Entity& rEntity)
	{
		IComponent::SetEntity(rEntity);
		assert(rEntity.HasComponent<Transform2DComponent>());

		rEntity.GetComponent<Transform2DComponent>()
			.SetScale(1.0f, 1.0f);
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
		m_Text.setColor(color);
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