#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__linux__)
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#endif
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "entt/entity/entity.hpp"
#include "entt/entity/registry.hpp"
#include "Smasher/Base.h"
#include "Smasher/Resources.h"
#include "Smasher/ErrorCodes.h"
#include "Smasher/ComponentSystems/EngineSystem.h"

namespace Smasher {
	namespace SpriteSystem {
		struct BatchContext;

		struct ModelData {
			glm::mat3 vertTransform = glm::mat3{};
			glm::mat3 texTransform = glm::mat3{};
			float depth = 0.f;
			uint32_t color = 0;
			BatchContext* ownerContext = nullptr;
			bool hasTexture = false;
		};

		// Models will use 2D plane
		struct RenderBatch {
			RenderBatch() = delete;
			RenderBatch(std::list<RenderBatch>& list, GLuint quadVBO, GLuint quadEBO);
			~RenderBatch();
			RenderBatch(const RenderBatch& other) = delete;
			RenderBatch(RenderBatch&&) noexcept;
			RenderBatch& operator = (const RenderBatch& other) = delete;
			RenderBatch& operator = (RenderBatch&&) noexcept;

			static const std::size_t MAX_MODEL_COUNT = 512; // Arbitrary
			std::array<ModelData, RenderBatch::MAX_MODEL_COUNT> models; // doesn't keep accurate track of model count
			sf::Texture* pTexture = nullptr;
			std::list<RenderBatch>& ownerBatchList;
			std::list<RenderBatch>::iterator iterator;
			bool dirty = false; // Has the render batch or any elemnts inside changed?
			bool full = false; // Can more models be added to this batch
			std::size_t modelCount = 0; // Keeps accurate track of model count
			GLuint instanceVAO = 0;
			GLuint instanceVBO = 0;
			GLuint quadVBO = 0; // Quad mesh data (buffers are created DrawableComponentManager)
			GLuint quadEBO = 0; // Quad mesh data (buffers are created DrawableComponentManager)
			static inline const GLubyte STATIC_INDICES[6]{
				0, 1, 2,   // first triangle
				2, 3, 0    // second triangle
			};

			static inline const float STATIC_VERTICES[24]{
				//   Position       Tex Coord
				   -0.5f, -0.5f,     0.f, 0.f,   // bottom left
					0.5f, -0.5f,     1.f, 0.f,   // bottom right
					0.5f,  0.5f,     1.f, 1.f,   // top right
				   -0.5f,  0.5f,     0.f, 1.f,   // top left
			};

			void InitGLObjects();

			// Add model to RenderBatch and updates BatchContext instance
			// to refer to that model
			void AddModel(BatchContext& context);

			void RemoveModel(BatchContext& context);

			// Copies models array into buffer
			// Try to call this as infrequently as possible
			void UpdateGLBufferData();
		};

		struct BatchContext {
			RenderBatch* batch = nullptr; // Pointer to a render batch the drawable component is a part of
			std::size_t index = SIZE_MAX; // Index of the drawable component within the Render Batch

			BatchContext() = default; // Invalid state
			BatchContext(RenderBatch* batch, std::size_t index) :
				batch(batch), index(index) {
			}

			// Non-Copyable
			BatchContext(const BatchContext&) = delete;
			BatchContext& operator =(const BatchContext&) = delete;

			operator bool() const {
				return batch != nullptr && index != SIZE_MAX;
			}

			void Invalidate() {
				batch = nullptr;
				index = SIZE_MAX;
			}
		};


		static const ResourceId EMPTY_TEXTURE_ID = 1;

		struct SMASHER_API Component {
			BatchContext _opaqueBatchContext;
			BatchContext _translucentBatchContext;
			std::shared_ptr<TextureResource> _textureResourcePtr; // Solely for preventing destruction of resource object
			std::shared_ptr<ShaderResource> _shaderResourcePtr; // Solely for preventing destruction of resource object
			std::shared_ptr<ShaderResource> _defaultShaderPtr; // Shader loaded from EngineConfig.h

			glm::vec2 _size {10.f, 10.f};
			sf::Color _color = sf::Color::White;
			// Texture Clip
			glm::vec2 _clipOffset;
			glm::vec2 _clipSize;
			Radians _clipRotation;
			bool _textureLoaded = false;
			float _depth = 0.f;

			bool _changed = false;
		};

		// Contains state information for the "SpriteSystem"
		struct SMASHER_API Context {
			std::map<ResourceId, std::list<RenderBatch>> opaqueBatches; // Linked List of Batches
			std::map<ResourceId, std::list<RenderBatch>> translucentBatches;// Linked List of Batches

			std::shared_ptr<ShaderResource> shaderResourcePtr; // Solely for preventing destruction of resource object
			std::shared_ptr<ShaderResource> defaultShaderPtr; // Shader loaded from EngineConfig.h

			// Quad instanced used by all DrawableComponent
			GLuint quadVBO;
			GLuint quadEBO;

			Context();
			~Context();
			Context(Context &&other) noexcept;
			Context(const Context&) = delete;
			Context& operator=(Context&& other) noexcept;
			Context& operator=(const Context&) = delete;
		};

		SMASHER_API ErrorCode Initialize(entt::registry& registry);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
		SMASHER_API ErrorCode Render(entt::registry& registry);

		SMASHER_API Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity);

		SMASHER_API ErrorCode SetSystemShader(Context& ctx, std::shared_ptr<ShaderResource> shaderPtr);
		SMASHER_API ErrorCode SetShader(Component& component, std::shared_ptr<ShaderResource> shaderPtr);
		SMASHER_API ErrorCode SetTextureClipSize(Component& component, float x, float y);
		SMASHER_API ErrorCode SetTextureClipSize(Component& component, glm::vec2 size);
		SMASHER_API ErrorCode SetTextureClipRotation(Component& component, Radians rotation);
		SMASHER_API ErrorCode SetTextureClipRotationDeg(Component& component, Degrees rotation);
		SMASHER_API ErrorCode SetTextureClipOffset(Component& component, float x, float y);
		SMASHER_API ErrorCode SetTextureClipOffset(Component& component, glm::vec2 offset);
		SMASHER_API ErrorCode SetColor(Component& component, sf::Color color);
		SMASHER_API ErrorCode SetTexture(Component& component, entt::registry& registry, ResourceId textureId, const ResourcePath* paths, const std::size_t numPaths, const TextureOptions& opts);
		SMASHER_API ErrorCode SetSize(Component& component, glm::vec2 size);
		SMASHER_API ErrorCode SetSize(Component& component, float width, float height);
		SMASHER_API ErrorCode SetDepth(Component& component, float depth);

		template <class ManifestData>
		ErrorCode SetTexture(Component& component, entt::registry& registry, const TextureOptions& opts);

		SMASHER_API std::shared_ptr<ShaderResource> GetDefaultShader(Component& component);
		SMASHER_API std::shared_ptr<ShaderResource> GetShader(Component& component);
		SMASHER_API glm::vec2 GetTextureClipSize(Component& component);
		SMASHER_API Radians GetTextureClipRotation(Component& component);
		SMASHER_API Degrees GetTextureClipRotationDeg(Component& component);
		SMASHER_API glm::vec2 GetTextureClipOffset(Component& component);
		SMASHER_API sf::Color GetColor(Component& component);
		SMASHER_API glm::vec2 GetSize(Component& component);
		SMASHER_API float GetDepth(Component& component);
		SMASHER_API bool HasTextureAsset(Component& component);
		SMASHER_API std::shared_ptr<TextureResource> GetTextureAsset(Component& component);
	}
}

#include "Smasher/ComponentSystems/SpriteSystem.inl"