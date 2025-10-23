#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "Smasher/Base.h"
#include "Resources.h"
#include "Smasher/IComponent.h"
#include "RenderBatch.h"
#include "Components/Transform2DWrapper.h"

namespace Smasher {
	class DrawableComponentManager;
	class IComponent;

//////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Component meant for drawing textures to screen. Textures can be passed to the @ref SetTextureAsset
/// as either Manifest objects or by using the @ref ResourceID of the texture.
/// 
/// @details
/// The @ref DrawableComponent stores the transforms of the image and texture transforms. These
/// transforms are passed into the base shaders use by @ref DrawableComponentManager. The image
/// transform is stored internally for cache locality (~100x speedup in comparison to
/// having a separate component store transform data).
/// 
/// The @ref SetTextureAsset method is used to specify the texture to render. For transulscent texture
/// (ex. a window pane) the @ref TextureOptions::transluscent should be set to `true` like so:
/// 
/// @code
/// GetEngine().GetResourceManager().GetOrLoadResource<Resources::Textures::Potrait, TextureResource>();
/// Entity& image = state.AddEntity<Entity>();
/// image.AddComponent<DrawableComponent>()
///			.SetPosition(sf::Vector2f(100.0f, 50.0f))
/// 		.SetScale(sf::Vector2f(20.0f, 20.0f))
///			.SetTextureAsset<Resources::Textures::Portrait>({.transluscent = true});
/// @endcode
/// 
/// In this example, the texture is loaded using the filepath instead of the Manifest Object.
/// @code
/// state.GetEngine().GetResourceManager().GetOrLoadResource(ResourceID { 10 }, ResourcePath{ "Resources/Textures/Portrait.png" });
/// Entity& image = state.AddEntity<Entity>();
/// image.AddComponent<DrawableComponent>()
///			.SetPosition(sf::Vector2f(100.0f, 50.0f))
/// 		.SetScale(sf::Vector2f(20.0f, 20.0f))
///			.SetTextureAsset(ResourceID { 10 }, {.transluscent = true});
/// @endcode
//////////////////////////////////////////////////////////////////////////////////////////////////

	class SMASHER_API DrawableComponent : public IComponent, public Transform2DWrapper<DrawableComponent> {
		friend class DrawableComponentManager;

		SMASHER_USE_COMPONENT_MANAGER(DrawableComponentManager)

	public:
		DrawableComponent();

		DrawableComponent(std::shared_ptr<TextureResource> texturePtr, std::shared_ptr<ShaderResource> shaderPtr);

		~DrawableComponent();

		template <class T>
		DrawableComponent& SetTextureAsset(const TextureOptions& opts);

		virtual void SetEntity(Entity& pEntity);

		// Depth bassed to depth buffer shader [0 - 1] with [0] being top and [1] being bottom
		DrawableComponent& SetDepth(float depth);

		DrawableComponent& SetColor(sf::Color color);

		DrawableComponent& SetShader(std::shared_ptr<ShaderResource> pShader);

		DrawableComponent& SetClipRect(sf::IntRect clipRect);

		DrawableComponent& SetClipRotation(Degrees angle);

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
	private:
		sf::IntRect m_ClipRect{ 0, 0, 0, 0 };
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