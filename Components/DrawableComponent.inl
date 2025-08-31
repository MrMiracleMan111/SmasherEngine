#pragma once
#include "Components/DrawableComponent.h"
#include "ResourceManager.h"
#include "ComponentManagers/DrawableComponentManager.h"

namespace Smasher {

	DrawableComponent::DrawableComponent() : IComponent(), Transform2DWrapper(*this, m_Transformable),
		m_OpaqueBatchContext(nullptr, SIZE_MAX),
		m_TranslucentBatchContext(nullptr, SIZE_MAX) {
		SetScale(sf::Vector2f(100.0f, 100.0f));
	}

	DrawableComponent::DrawableComponent(
		std::shared_ptr<TextureResource> texturePtr,
		std::shared_ptr<ShaderResource> shaderPtr) : IComponent(), Transform2DWrapper(*this, m_Transformable),
		m_ShaderResource(shaderPtr),
		m_TextureResource(texturePtr),
		m_OpaqueBatchContext(nullptr, SIZE_MAX),
		m_TranslucentBatchContext(nullptr, SIZE_MAX) {
		SetScale(sf::Vector2f(100.0f, 100.0f));
	}

	template <class T>
	DrawableComponent& DrawableComponent::SetTextureAsset() {
		return SetTextureAsset<T>(false);
	}

	template <class T>
	DrawableComponent& DrawableComponent::SetTextureAsset(bool transulcent) {
		auto& rResourceManager = GetEntity().GetEngine().GetResourceManager();
		auto& rCompManager = dynamic_cast<DrawableComponentManager&>(GetManager());
		// Load Resource
		auto pTextureResource = rResourceManager.template GetOrLoadResource<T, TextureResource>();
		// Update Render Batch
		rCompManager.OnComponentSetTexture(*this, T::ID, transulcent);
		// Assign texture to component
		m_TextureResource = pTextureResource;

		sf::Texture& texture = m_TextureResource->GetTexture();
		m_TextureLoaded = true;
		SetClipRect(sf::IntRect{ 0, 0, (int)(texture.getSize().x), (int)(texture.getSize().y) });
		GetClipTransform(); // Cache the clip transform
		return *this;
	}

}