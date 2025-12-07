#pragma once

namespace Smasher {
	template <class T>
	DrawableComponent& DrawableComponent::SetTextureAsset(const TextureOptions& opts) {
		auto& rResourceManager = GetEntity().GetEngine().GetResourceManager();
		auto& rCompManager = dynamic_cast<DrawableComponentManager&>(GetManager());
		// Load Resource
		auto pTextureResource = rResourceManager.template GetOrLoadResource<T, TextureResource>();
		// Update Render Batch
		rCompManager.OnComponentSetTexture(*this, T::Id, opts.transluscent);
		// Assign texture to component
		m_TextureResource = pTextureResource;

		sf::Texture& texture = m_TextureResource->GetTexture();
		m_TextureLoaded = true;
		SetClipRect(sf::IntRect{ 0, 0, (int)(texture.getSize().x), (int)(texture.getSize().y) });
		GetClipTransform(); // Cache the clip transform
		return *this;
	}
}