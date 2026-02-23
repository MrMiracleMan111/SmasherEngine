#pragma once
#include "Smasher/Base.h"
#include "Smasher/Resources.h"
#include "Smasher/ComponentManagers/RenderBatch.h"
#include "entt/entity/entity.hpp"
#include "entt/entity/registry.hpp"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

namespace Smasher {
	namespace DrawableSystem {
		struct SMASHER_API Component {
			//BatchContext m_OpaqueBatchContext;
			//BatchContext m_TranslucentBatchContext;
			std::shared_ptr<TextureResource> m_TextureResourcePtr; // Solely for preventing destruction of resource object
			std::shared_ptr<ShaderResource> m_ShaderResourcePtr; // Solely for preventing destruction of resource object

			sf::IntRect m_ClipRect{ 0, 0, 0, 0 };
			sf::Color m_Color = sf::Color::White;
			sf::Transform m_ClipTransform;
			sf::Transformable m_Transformable;
			bool m_TextureLoaded = false;
			bool m_ClipChanged = false;
			float m_Depth = 0.f;
			Degrees m_ClipRotation = 0.f;
		};

		// Contains state information for the "DrawableSystem"
		struct SMASHER_API Context {
			//std::map<ResourceId, std::list<RenderBatch>> m_OpaqueBatches; // Linked List of Batches
			//std::map<ResourceId, std::list<RenderBatch>> m_TranslucentBatches;// Linked List of Batches

			std::shared_ptr<ShaderResource> m_ShaderResourcePtr; // Solely for preventing destruction of resource object
			std::shared_ptr<ShaderResource> m_DefaultShader; // Shader loaded from EngineConfig.h

			// Quad instanced used by all DrawableComponent
			GLuint m_QuadVBO;
			GLuint m_QuadEBO;
		};

		SMASHER_API const ErrorCode Initialize(entt::registry& registry);
		SMASHER_API const ErrorCode Render(entt::registry& registry);
		SMASHER_API const Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity);
	}
}