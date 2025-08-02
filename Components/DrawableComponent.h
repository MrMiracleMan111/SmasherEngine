#pragma once
#include <SFML/Graphics.hpp>
#include "Base.h"
#include "IComponent.h"
#include "Transform2DComponent.h"
#include "ResourceManager.h"
#include "Entity.h"

namespace Smasher {
	struct Vertex2 {
		float x = 0;
		float y = 0;

		Vertex2& operator = (const sf::Vector2f& vec) {
			x = vec.x;
			y = vec.y;
			return *this;
		}
	};
	struct Vertex3 {
		float x = 0;
		float y = 0;
		float z = 0;

		Vertex3& operator = (const sf::Vector3f& vec) {
			x = vec.x;
			y = vec.y;
			z = vec.z;
			return *this;
		}
	};

	struct Vertex4 {
		float x = 0;
		float y = 0;
		float z = 0;
		float w = 0;
	};

	struct SMASHER_API QuadVertex {
		Vertex3 position;
		Vertex4 color;
		Vertex2 texCoords;

		void SetPosition(sf::Vector3f& _position) {
			position.x = _position.x;
			position.y = _position.y;
			position.z = _position.z;
		}

		void SetColor(float x, float y, float z, float w) {
			color.x = x;
			color.y = y;
			color.z = z;
			color.w = w;
		}


		void SetTexCoords(sf::Vector2f& _texCoords) {
			texCoords.x = _texCoords.x;
			texCoords.y = _texCoords.y;
		}
	};

	class SMASHER_API DrawableComponent : public IComponent {
	public:
		DrawableComponent() : IComponent(), m_Vertices(sf::PrimitiveType::Quads, 4),
			m_TransformRef() {
			InitVertices();
		}
		DrawableComponent(
			std::shared_ptr<TextureResource> texturePtr,
			std::shared_ptr<ShaderResource> shaderPtr) : IComponent(),
			m_Vertices(sf::PrimitiveType::Quads, 4),
			m_ShaderResource(shaderPtr), m_TextureResource(texturePtr), m_TransformRef(nullptr),
			m_RenderState(sf::BlendNone, GetEntity().GetComponent<Transform2DComponent>().GetTransform(), &m_TextureResource->GetTexture(), &m_ShaderResource->GetShader()) {
			InitVertices();
		}

		template <class T>
		DrawableComponent& SetTextureAsset() {
			auto& rResourceManager = GetEntity().GetEngine().GetResourceManager();
			m_TextureResource = rResourceManager.GetOrLoadResource<T, TextureResource>();

			sf::Texture& texture = m_TextureResource->GetTexture();
			m_ClipRect = sf::IntRect{ 0, 0, (int)(texture.getSize().x), (int)(texture.getSize().y)};
			m_Vertices[0].texCoords = sf::Vector2f(0, 0);
			m_Vertices[1].texCoords = sf::Vector2f(0, texture.getSize().y);
			m_Vertices[2].texCoords = sf::Vector2f(texture.getSize().x, texture.getSize().y);
			m_Vertices[3].texCoords = sf::Vector2f(texture.getSize().x, 0);

			m_TextureLoaded = true;
			const sf::Transform& rTransform = GetEntity().template GetComponent<Transform2DComponent>().GetTransform();
			m_RenderState = sf::RenderStates(sf::BlendNone,
				rTransform,
				&m_TextureResource->GetTexture(),
				&m_ShaderResource->GetShader());
			return *this;
		}

		virtual void SetEntity(Entity& pEntity) {
			IComponent::SetEntity(pEntity);
			m_TransformRef = &GetEntity().GetComponent<Transform2DComponent>().GetTransform();
		}

		// Depth bassed to depth buffer shader [0 - 1] with [0] being top and [1] being bottom
		DrawableComponent& SetDepth(float depth) {
			m_Depth = depth;
			return *this;
		}

		DrawableComponent& SetShader(std::shared_ptr<ShaderResource> pShader) {
			m_ShaderResource = pShader;
			m_RenderState = sf::RenderStates(sf::BlendNone, GetEntity().GetComponent<Transform2DComponent>().GetTransform(), &m_TextureResource->GetTexture(), &m_ShaderResource->GetShader());
			return *this;
		}

		DrawableComponent& SetClipRect(sf::IntRect clipRect) {
			m_ClipRect = clipRect;
			return *this;
		}

		DrawableComponent& SetSize(sf::Vector2f size) {
			float depth = 1.0f;
			m_Vertices[0].position = sf::Vector3f(0, 0, depth);
			m_Vertices[1].position = sf::Vector3f(0, size.y, depth);
			m_Vertices[2].position = sf::Vector3f(size.x, size.y, depth);
			m_Vertices[3].position = sf::Vector3f(size.x, 0, depth);
			return *this;
		}

		static void StaticRenderComponent(DrawableComponent& self, sf::RenderWindow& rWindow);

	private:
		void InitVertices() {
			float depth = 1.0f;
			m_Vertices[0].position = sf::Vector3f(0, 0, depth);
			m_Vertices[1].position = sf::Vector3f(0, 100, depth);
			m_Vertices[2].position = sf::Vector3f(100, 100, depth);
			m_Vertices[3].position = sf::Vector3f(100, 0, depth);

			m_Vertices[0].color = Vertex4(1.0f, 1.0f, 1.0f, 1.0f);
			m_Vertices[1].color = Vertex4(1.0f, 1.0f, 1.0f, 1.0f);
			m_Vertices[2].color = Vertex4(1.0f, 1.0f, 1.0f, 1.0f);
			m_Vertices[3].color = Vertex4(1.0f, 1.0f, 1.0f, 1.0f);

			m_Vertices[0].texCoords = sf::Vector2f(0, 0);
			m_Vertices[1].texCoords = sf::Vector2f(0, 1);
			m_Vertices[2].texCoords = sf::Vector2f(1, 1);
			m_Vertices[3].texCoords = sf::Vector2f(1, 0);
		}

		// Draws to render target
		// Code pulled from https://jordansavant.com/book/graphics/sfml/sfml2_depth_buffering.md
		void draw(sf::RenderTarget& rTarget) const;
		void applyCurrentView(sf::RenderTarget& target) const;
		sf::IntRect getViewport(sf::RenderTarget& target, const sf::View& view) const;

		sf::RenderStates m_RenderState;
		std::shared_ptr<TextureResource> m_TextureResource; // Solely for preventing destruction of resource object
		std::shared_ptr<ShaderResource> m_ShaderResource; // Solely for preventing destruction of resource object
		//sf::Sprite m_Sprite;
		//sf::VertexArray m_Vertices;
		QuadVertex m_Vertices[4]; // 
		sf::IntRect m_ClipRect{0, 0, 1, 1};
		sf::Transform const* m_TransformRef = nullptr; // Cache it's location
		bool m_TextureLoaded = false;
		float m_Depth = 0.0f;
	};
}
