#include "TextComponent.h"
#include "Entity.h"

namespace Smasher {
	void TextComponent::StaticRenderComponent(TextComponent& self, sf::RenderWindow& rWindow) {
		if (self.m_FontLoaded) {
			Transform2DComponent& rTransform = self.GetEntity().GetComponent<Transform2DComponent>();
			rWindow.draw(self.m_Text, rTransform.GetTransform());
		}
	}
}