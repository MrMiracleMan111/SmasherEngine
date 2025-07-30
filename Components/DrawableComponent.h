#pragma once
#include "Base.h"
#include "IComponent.h"
#include "Transform2DComponent.h"
#include "ResourceManager.h"

namespace Smasher {
	class SMASHER_API DrawableComponent : public IComponent {
	public:
		DrawableComponent() : IComponent() {}

		template <class T>
		DrawableComponent& SetTextureAsset() {
			auto& rResourceManager = GetEntity().GetEngine().GetResourceManager();
			m_Texture = rResourceManager.GetOrLoadResource<T, TextureResource>();
			m_Sprite.setTexture(m_Texture->GetTexture());
			m_TextureLoaded = true;
			return *this;
		}

		static void StaticRenderComponent(DrawableComponent& self, sf::RenderWindow& rWindow);

	private:
		std::shared_ptr<TextureResource> m_Texture;
		sf::Sprite m_Sprite;
		bool m_TextureLoaded = false;
	};
}
