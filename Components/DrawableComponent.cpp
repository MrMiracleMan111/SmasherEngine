#include "DrawableComponent.h"
#include "Entity.h"

namespace Smasher {
	void DrawableComponent::StaticRenderComponent(DrawableComponent& self, sf::RenderWindow& rWindow) {
		if (self.m_TextureLoaded) {
			Transform2DComponent& rTransform = self.GetEntity().GetComponent<Transform2DComponent>();
			rWindow.draw(self.m_Sprite, rTransform.GetTransform());
		}
	}
}