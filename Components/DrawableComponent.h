#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "Base.h"
#include "Resources.h"
#include "IComponent.h"
#include "RenderBatch.h"
#include "Components/Transform2DWrapper.h"

namespace Smasher {
	class DrawableComponentManager;
	class IComponent;

	class SMASHER_API DrawableComponent : public IComponent, public Transform2DWrapper<DrawableComponent>{
	friend class DrawableComponentManager;

	SMASHER_USE_COMPONENT_MANAGER(DrawableComponentManager)

	public:
		DrawableComponent() : IComponent(), Transform2DWrapper(*this, m_Transformable),
			m_OpaqueBatchContext(nullptr, SIZE_MAX),
			m_TranslucentBatchContext(nullptr, SIZE_MAX) {
			SetScale(sf::Vector2f(100.0f, 100.0f));
		}
		DrawableComponent(
			std::shared_ptr<TextureResource> texturePtr,
			std::shared_ptr<ShaderResource> shaderPtr) : IComponent(), Transform2DWrapper(*this, m_Transformable),
			m_ShaderResource(shaderPtr),
			m_TextureResource(texturePtr),
			m_OpaqueBatchContext(nullptr, SIZE_MAX),
			m_TranslucentBatchContext(nullptr, SIZE_MAX) {
			SetScale(sf::Vector2f(100.0f, 100.0f));
		}
		~DrawableComponent();

		template <class T>
		DrawableComponent& SetTextureAsset();

		template <class T>
		DrawableComponent& SetTextureAsset(bool transulcent);

		virtual void SetEntity(Entity& pEntity);

		// Depth bassed to depth buffer shader [0 - 1] with [0] being top and [1] being bottom
		DrawableComponent& SetDepth(float depth);

		DrawableComponent& SetColor(sf::Color color);

		DrawableComponent& SetShader(std::shared_ptr<ShaderResource> pShader);

		DrawableComponent& SetClipRect(sf::IntRect clipRect);

		DrawableComponent& SetClipRotation(Degrees angle);

		// Pushes transform to RenderBatch for rendering
		// This MUST be called after changing Transform or ClipRect
		// Try to call this as infrequently as possible
		inline DrawableComponent& PushToGPU();

		float GetDepth() const { return m_Depth; }

		const sf::Color& GetColor() const { return m_Color; }

		const sf::IntRect& GetClipRect() const { return m_ClipRect; }

		Degrees GetClipRotation() const { return m_ClipRotation; }

		const sf::Transform& GetClipTransform();

	protected:
		BatchContext m_OpaqueBatchContext;
		BatchContext m_TranslucentBatchContext;
		std::shared_ptr<TextureResource> m_TextureResource; // Solely for preventing destruction of resource object
		std::shared_ptr<ShaderResource> m_ShaderResource; // Solely for preventing destruction of resource object
		bool m_Changed = false;
	private:
		sf::IntRect m_ClipRect{0, 0, 0, 0};
		sf::Color m_Color = sf::Color::White;
		sf::Transform m_ClipTransform;
		sf::Transformable m_Transformable;
		bool m_TextureLoaded = false;
		bool m_ClipChanged = false;
		float m_Depth = 0.0f;
		Degrees m_ClipRotation = 0.0f;
	};
}

#include "Components/DrawableComponent.inl"