#pragma once
#include "Base.h"
#include "IComponent.h"
#include "Transform2DComponent.h"
#include "ResourceManager.h"

namespace Smasher {
	class DrawableComponent : public IComponent
	{
	public:
		DrawableComponent() : IComponent() {}

		template <class T>
		void SetTextureAsset() {
			auto& rResourceManager = GetEntity().GetEngine().GetResourceManager();
			m_ResourceHandle = rResourceManager.GetResourceHandle<T>();
			TextureResource& rResource = rResourceManager.GetResource<TextureResource>(m_ResourceHandle);
			m_Sprite.setTexture(rResource.GetTexture());
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
