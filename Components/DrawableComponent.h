#pragma once
#include "Base.h"
#include "IComponent.h"
#include "Transform2DComponent.h"
#include "ResourceManager.h"

namespace Smasher {
	class SMASHER_API DrawableComponent : public IComponent
	{
	public:
		DrawableComponent() : IComponent() {}

		template <class T>
		void SetTextureAsset() {
			auto& rResourceManager = GetEntity().GetEngine().GetResourceManager();
			m_ResourceHandle = rResourceManager.GetResourceHandle<T>(ResourceType::TEXTURE);
			m_Texture = rResourceManager.GetResource<TextureResource>(m_ResourceHandle);
			m_Sprite.setTexture(m_Texture->GetTexture());
			m_TextureLoaded = true;
		}

		static void StaticRenderComponent(DrawableComponent& self, sf::RenderWindow& rWindow);

	private:
		std::shared_ptr<TextureResource> m_Texture;
		ResourceHandle m_ResourceHandle;
		sf::Sprite m_Sprite;
		bool m_TextureLoaded = false;
	};
}
