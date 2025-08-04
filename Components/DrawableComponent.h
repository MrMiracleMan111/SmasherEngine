#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "Base.h"
#include "Resources.h"
#include "IComponent.h"
#include "RenderBatch.h"

//#include "IComponent.h"
//#include "Transform2DComponent.h"
//#include "ResourceManager.h"
//#include "Entity.h"
//#include "RenderBatch.h"
//#include "ComponentManagers/DrawableComponentManager.h"

namespace Smasher {
	class DrawableComponentManager;
	class IComponent;

	class SMASHER_API DrawableComponent : public IComponent {
	friend class DrawableComponentManager;

	SMASHER_USE_COMPONENT_MANAGER(DrawableComponentManager)

	public:
		DrawableComponent() : IComponent(),
			m_TransformRef(nullptr),
			m_OpaqueBatchContext(nullptr, SIZE_MAX),
			m_TranslucentBatchContext(nullptr, SIZE_MAX) {
		}
		DrawableComponent(
			std::shared_ptr<TextureResource> texturePtr,
			std::shared_ptr<ShaderResource> shaderPtr) : IComponent(),
			m_ShaderResource(shaderPtr),
			m_TextureResource(texturePtr),
			m_TransformRef(nullptr),
			m_OpaqueBatchContext(nullptr, SIZE_MAX),
			m_TranslucentBatchContext(nullptr, SIZE_MAX) {}
		~DrawableComponent();

		template <class T>
		DrawableComponent& SetTextureAsset();

		template <class T>
		DrawableComponent& SetTextureAsset(bool transulcent);

		virtual void SetEntity(Entity& pEntity);

		// Depth bassed to depth buffer shader [0 - 1] with [0] being top and [1] being bottom
		DrawableComponent& SetDepth(float depth) {
			m_Depth = depth;
			return *this;
		}

		DrawableComponent& SetColor(sf::Color color) {
			m_Color = color;
			return *this;
		}

		DrawableComponent& SetShader(std::shared_ptr<ShaderResource> pShader) {
			m_ShaderResource = pShader;
			return *this;
		}

		DrawableComponent& SetClipRect(sf::IntRect clipRect) {
			m_ClipRect = clipRect;
			return *this;
		}

		// Pushes transform to RenderBatch for rendering
		// This MUST be called after changing Transform
		// Try to call this as infrequently as possible
		DrawableComponent& PushToGPU();

		const sf::Transform& GetTransformRef() {
			assert(m_TransformRef != nullptr);
			return *m_TransformRef;
		}

		float GetDepth() const { return m_Depth; }

		sf::Color GetColor() const { return m_Color; }

		sf::Transform GetClipTransform();

	protected:
		BatchContext m_OpaqueBatchContext;
		BatchContext m_TranslucentBatchContext;
		std::shared_ptr<TextureResource> m_TextureResource; // Solely for preventing destruction of resource object
		std::shared_ptr<ShaderResource> m_ShaderResource; // Solely for preventing destruction of resource object
	private:
		// Draws to render target
		// Code pulled from https://jordansavant.com/book/graphics/sfml/sfml2_depth_buffering.md
		void applyCurrentView(sf::RenderTarget& target) const;
		sf::IntRect getViewport(sf::RenderTarget& target, const sf::View& view) const;
		sf::IntRect m_ClipRect{0, 0, 100, 100};
		sf::Color m_Color = sf::Color::White;
		sf::Transform const* m_TransformRef = nullptr; // Cache it's location
		bool m_TextureLoaded = false;
		float m_Depth = 0.0f;
	};
}

#include "Components/DrawableComponent.inl"