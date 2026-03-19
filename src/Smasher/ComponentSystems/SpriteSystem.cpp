#include "Smasher/Base.h"
#include <algorithm>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
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
#include "entt/entity/registry.hpp"
#include "entt/signal/delegate.hpp"
#include "Smasher/ErrorCodes.h"
#include "Smasher/Resources.h"
#include "Smasher/ComponentSystems/SpriteSystem.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ComponentSystems/EngineSystem.h"
#include "Smasher/ComponentManagers/RenderBatch.h"
#include "Smasher/Engine.h"

namespace Smasher {
	namespace SpriteSystem {
		RenderBatch::RenderBatch(std::list<RenderBatch>& list, GLuint quadVBO, GLuint quadEBO) :
			ownerBatchList(list),
			quadVBO(quadVBO),
			quadEBO(quadEBO)
		{
			InitGLObjects();
		}

		RenderBatch::~RenderBatch() {
			glDeleteVertexArrays(1, &instanceVAO);
			glDeleteBuffers(1, &instanceVBO);
		}

		RenderBatch::RenderBatch(RenderBatch&& other) noexcept :
			ownerBatchList(other.ownerBatchList),
			quadVBO(other.quadVBO),
			quadEBO(other.quadEBO) {
			models = other.models;
			pTexture = other.pTexture;
			dirty = true; // Has the render batch or any elemnts inside changed?
			modelCount = other.modelCount; // Keeps accurate track of model count
			models = std::move(other.models);
			InitGLObjects();
		};
		RenderBatch& RenderBatch::operator = (RenderBatch&& other) noexcept {
			quadVBO = other.quadVBO;
			quadEBO = other.quadEBO;
			models = other.models;
			pTexture = other.pTexture;
			dirty = true; // Has the render batch or any elemnts inside changed?
			modelCount = other.modelCount; // Keeps accurate track of model count
			models = std::move(other.models);
			InitGLObjects();
			return *this;
		};

		void RenderBatch::InitGLObjects() {
			// Cache the current GL_VERTEX_ARRAY_BINDING, GL_ARRAY_BUFFER_BINDING values
			// so that they can be restored after, InitGLObjects
			GLint currentVAO, currentVBO;
			glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
			glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

			glGenVertexArrays(1, &instanceVAO);
			glGenBuffers(1, &instanceVBO);

			glBindVertexArray(instanceVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO); // Use preloaded VBO data from RenderBatch::STATIC_VERTICES

			// Vertex Coord (in quad)
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
			glEnableVertexAttribArray(0);
			// Texture Coord
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO); // Use preloaded EBO data from RenderBatch::STATIC_INDICES
			glBindVertexArray(0);

			glBindVertexArray(instanceVAO);
			glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
			glBufferData(GL_ARRAY_BUFFER, models.max_size() * sizeof(Smasher::ModelData), models.data(), GL_DYNAMIC_DRAW);
			glBindVertexArray(0);

			glBindVertexArray(instanceVAO);
			glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

			// Instance Vertex Transform Matrix
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, vertTransform) + (0 * sizeof(float))));
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, vertTransform) + (3 * sizeof(float))));
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, vertTransform) + (6 * sizeof(float))));


			// Instance Tex Transform Matrix
			glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, texTransform) + (0 * sizeof(float))));
			glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, texTransform) + (3 * sizeof(float))));
			glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, texTransform) + (6 * sizeof(float))));

			// Instance Color Code
			glVertexAttribIPointer(8, 1, GL_UNSIGNED_INT, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, color)));

			// Instance Depth
			glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, depth)));

			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			glEnableVertexAttribArray(4);
			glEnableVertexAttribArray(5);
			glEnableVertexAttribArray(6);
			glEnableVertexAttribArray(7);
			glEnableVertexAttribArray(8);
			glEnableVertexAttribArray(9);

			glVertexAttribDivisor(2, 1);  // Instance attribute
			glVertexAttribDivisor(3, 1);  // Instance attribute
			glVertexAttribDivisor(4, 1);  // Instance attribute
			glVertexAttribDivisor(5, 1);  // Instance attribute
			glVertexAttribDivisor(6, 1);  // Instance attribute
			glVertexAttribDivisor(7, 1);  // Instance attribute
			glVertexAttribDivisor(8, 1);  // Instance attribute
			glVertexAttribDivisor(9, 1);  // Instance attribute

			glBindVertexArray(0);
			glBindVertexArray(currentVAO);
			glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
		}

		void DrawBatch(RenderBatch& batch) {
			batch.UpdateGLBufferData(); // Updates only if dirty
			glBindVertexArray(batch.instanceVAO);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, RenderBatch::StaticIndices);
			glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_BYTE, (GLvoid*)0, (GLsizei)batch.modelCount);

			glBindVertexArray(0);
		}

		void RenderBatch::RemoveModel(BatchContext& context) {
			assert(context.batch != nullptr);
			assert(context.index <= (modelCount - 1));
			assert(models[context.index].ownerContext == &context);
			dirty = true;
			full = false;

			if (context.index < (modelCount - 1)) {
				// Swap and pop from old batch
				const std::size_t index = context.index;
				models.at(index).ownerContext = nullptr;

				const BatchContext* newOwnerContex = models.at(modelCount - 1).ownerContext;
				std::swap(models.at(index), models.at(modelCount - 1));
				if (models.at(index).ownerContext != nullptr) {
					models.at(index).ownerContext->index = index;
				}
				assert(models.at(index).ownerContext == newOwnerContex);
				assert(models.at(index).ownerContext != nullptr);
			}
			else if (context.index == (modelCount - 1)) {
				// Nothing
			}
			else {
				assert(false); // should be unreachable
			}

			--modelCount;
			context.batch = nullptr;
			context.index = SIZE_MAX;

			// Leave 1 empty list
			if (modelCount == 0 && ownerBatchList.size() > 1) {
				ownerBatchList.erase(iterator);
				return;
			}
			// Move to front of list
			ownerBatchList.splice(iterator, ownerBatchList, ownerBatchList.begin());
		}

		void RenderBatch::AddModel(BatchContext& context) {
			dirty = true; // Data will be copied in later
			context.index = modelCount;
			context.batch = this;
			models[modelCount].ownerContext = &context;
			++modelCount;
			full = (modelCount == RenderBatch::MAX_MODEL_COUNT);
			if (full) {
				// Move to back of list since it's full
				ownerBatchList.splice(ownerBatchList.end(), ownerBatchList, iterator);
			}
		}

		// Copies models array into buffer
		// Try to call this as infrequently as possible
		void RenderBatch::UpdateGLBufferData() {
			if (!dirty) {
				return;
			}
			glBindVertexArray(instanceVAO);
			glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, modelCount * sizeof(Smasher::ModelData), models.data());
			glBindVertexArray(0);
			dirty = false;
		}

		Context::Context() {}

		Context::~Context() {}

		Context::Context(Context&& other) noexcept :
			opaqueBatches(std::move(other.opaqueBatches)),
			translucentBatches(std::move(other.translucentBatches)),
			shaderResourcePtr(std::move(other.shaderResourcePtr)),
			defaultShaderPtr(std::move(other.defaultShaderPtr)),
			quadVBO(other.quadVBO),
			quadEBO(other.quadEBO)
		{

		}

		Context& Context::operator= (Context&& other) noexcept {
			if (this != &other) {
				opaqueBatches = std::move(other.opaqueBatches);
				translucentBatches = std::move(other.translucentBatches);
				shaderResourcePtr = std::move(other.shaderResourcePtr);
				defaultShaderPtr = std::move(other.defaultShaderPtr);
				quadVBO = other.quadVBO;
				quadEBO = other.quadEBO;
			}
			return *this;
		}

		ErrorCode InitQuadGLBuffers(Context& ctx) {
			// Cache the current GL_VERTEX_ARRAY_BINDING, GL_ARRAY_BUFFER_BINDING values
			// so that they can be restored after, InitGLObjects (for SFML compatability)
			GLint currentVAO;
			GLint currentVBO;
			glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
			glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

			glGenBuffers(1, &ctx.quadVBO);
			glGenBuffers(1, &ctx.quadEBO);

			glBindBuffer(GL_ARRAY_BUFFER, ctx.quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(RenderBatch::STATIC_VERTICES), RenderBatch::STATIC_VERTICES, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx.quadEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(RenderBatch::STATIC_INDICES), RenderBatch::STATIC_INDICES, GL_STATIC_DRAW);

			glBindVertexArray(currentVAO);
			glBindBuffer(GL_ARRAY_BUFFER, currentVBO);

			return ERROR_NoError;
		}

		void OnDestroyComponent(entt::registry &registry, entt::entity entity) {
			Component& component = registry.get<Component>(entity);
			if (component._opaqueBatchContext) {
				component._opaqueBatchContext.batch->RemoveModel(component._opaqueBatchContext);
			}
			if (component._translucentBatchContext) {
				component._translucentBatchContext.batch->RemoveModel(component._translucentBatchContext);
			}
		}

		ErrorCode Initialize(entt::registry& registry) {
			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			Context &ctx = registry.ctx().emplace<Context>();
			Engine& engine = registry.ctx().get<EngineSystem::Context>().engineRef;

			registry.on_destroy<Component>().connect<&OnDestroyComponent>();

			if (!engine.IsHeadless()) {
				InitQuadGLBuffers(ctx);
			}

			std::shared_ptr<Smasher::ShaderResource> pShader = engine.GetResourceManager().LoadVertFragShaderResource(EngineConfig::DRAWABLE_COMPONENT_VERT_SHADER, EngineConfig::DRAWABLE_COMPONENT_FRAG_SHADER);
			sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(engine.GetWindow().getView().getTransform().getMatrix());
			pShader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);
			ctx.defaultShaderPtr = pShader;
			ctx.shaderResourcePtr = pShader;

			return ERROR_NoError;
		}

		ErrorCode Teardown(entt::registry& registry) {
			if (registry.ctx().contains<Context>()) {
				Engine& engine = registry.ctx().get<EngineSystem::Context>().engineRef;

				if (!engine.IsHeadless()) {
					Context& ctx = registry.ctx().get<Context>();
					GLuint buffers[] = { ctx.quadEBO, ctx.quadVBO };
					glDeleteBuffers(2, buffers);
				}
			}
			return ERROR_NoError;
		}

		// Update clip transform of component using
		// position, rotation, and dimensions of the clip rect
		glm::mat3 GenerateClipTransform(Component &component)
		{
			assert(component._textureLoaded);
			//glm::mat3 clipTransform = glm::mat3(1.f);

			// Extract Z-Axis rotation from rotation
			float angle = component._clipRotation;
			float cosine = std::cos(angle);
			float sine = std::sin(angle);

			return glm::mat3 {
				component._clipSize.x * cosine,		component._clipSize.x * sine,	0.f,
				component._clipSize.y * sine,		component._clipSize.y * cosine,	0.f,
				component._clipOffset.x,			component._clipOffset.y,		1.f
			};


			//clipTransform = glm::scale(clipTransform, component._clipSize);
			//clipTransform = glm::rotate(clipTransform, component._clipRotation);
			//clipTransform = glm::translate(clipTransform, component._clipOffset);
			//
			//return clipTransform;
		}

		// Updates batch data using component and transform information
		void UpdateBatchData(Component& drawable, TransformSystem::Component &transform) {
			drawable._changed = false;
			static const glm::mat3 IDENTITY_TRANSFORM_MATRIX_2D{ 1.f };

			glm::mat3 clipTransform = IDENTITY_TRANSFORM_MATRIX_2D;

			if (drawable._textureLoaded) {
				clipTransform = GenerateClipTransform(drawable);
			}

			uint32_t colorData = (uint32_t)drawable._color.toInteger();
			ModelData data = ModelData {
				TransformSystem::Compute2DTransform(transform),
				clipTransform,
				drawable._depth,
				colorData,
				nullptr
			};

			if (drawable._opaqueBatchContext) {
				memcpy(&drawable._opaqueBatchContext.batch->models[drawable._opaqueBatchContext.index], &data, sizeof(ModelData));
				drawable._opaqueBatchContext.batch->models[drawable._opaqueBatchContext.index].ownerContext = &drawable._opaqueBatchContext;
				drawable._opaqueBatchContext.batch->dirty = true;
			}

			if (drawable._translucentBatchContext) {
				memcpy(&drawable._translucentBatchContext.batch->models[drawable._translucentBatchContext.index], &data, sizeof(ModelData));
				drawable._translucentBatchContext.batch->models[drawable._translucentBatchContext.index].ownerContext = &drawable._translucentBatchContext;
				drawable._translucentBatchContext.batch->dirty = true;
			}
		}

		ErrorCode Render(entt::registry& registry) {
#ifdef	BENCHMARK
			std::chrono::time_point<std::chrono::system_clock> BENCHMARK_now = std::chrono::system_clock::now();
#endif
			if (!registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}
			if (!registry.ctx().contains<EngineSystem::Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			Context& ctx = registry.ctx().get<Context>();
			Engine& engine = registry.ctx().get<EngineSystem::Context>().engineRef;

			auto view = registry.view<Component, TransformSystem::Component>();
			for (auto [entity, drawable, transform] : view.each()) {
				if (!TransformSystem::HasChanged(transform) && !drawable._changed)
					continue;
				UpdateBatchData(drawable, transform);
			}

#ifdef	BENCHMARK
			std::chrono::microseconds BENCHMARK_diff = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - BENCHMARK_now);
			Smasher::InternalTimers::SMASHER_TimeAccumulator += BENCHMARK_diff;
#endif

			GLint currentVAO;
			GLint currentVBO;
			glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
			glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

			sf::Glsl::Mat4 viewProjectionMatrix{ engine.GetWindow().getView().getTransform().getMatrix() };
			sf::Shader::bind(&ctx.shaderResourcePtr->GetShader());
			static_assert(std::is_same_v<std::shared_ptr<ShaderResource>, decltype(ctx.shaderResourcePtr)>, "fail");
			ctx.shaderResourcePtr->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);
			ctx.shaderResourcePtr->GetShader().setUniform("translucentPass", false);

			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);
			glDepthRange(1.f, -1.f); // top = 1, bottom = 0
			glDisable(GL_BLEND);

			sf::Texture* pTexture = nullptr;
			for (auto& itr : ctx.opaqueBatches) {
				std::list<RenderBatch>& batchList = itr.second;
				if (!batchList.empty()) {
					pTexture = batchList.back().pTexture;
				}
				bool hasTexture = pTexture != nullptr;
				ctx.shaderResourcePtr->GetShader().setUniform("hasTexture", hasTexture);

				if (hasTexture) {
					sf::Texture::bind(pTexture, sf::Texture::Pixels);
				}

				for (auto& batch : batchList) {
					DrawBatch(batch);
				}
				sf::Texture::bind(NULL);
			}

			ctx.shaderResourcePtr->GetShader().setUniform("translucentPass", true);
			glDepthMask(GL_FALSE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			for (auto& itr : ctx.translucentBatches) {
				std::list<RenderBatch>& batchList = itr.second;
				if (!batchList.empty()) {
					pTexture = batchList.back().pTexture;
				}
				bool hasTexture = pTexture != nullptr;
				ctx.shaderResourcePtr->GetShader().setUniform("hasTexture", hasTexture);

				if (hasTexture) {
					sf::Texture::bind(batchList.back().pTexture, sf::Texture::Pixels);
				}

				for (auto& batch : batchList) {
					DrawBatch(batch);
				}
				sf::Texture::bind(NULL);
			}

			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_ALWAYS);
			glDepthRange(1.f, -1.f); // top = 1, bottom = 0

			sf::Shader::bind(NULL);
			glBindVertexArray(currentVAO);
			glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
			engine.GetWindow().resetGLStates();

			GLenum err;
			while ((err = glGetError()) != GL_NO_ERROR)
			{
				std::cout << "GL Error: \"" << gluErrorString(err) << "\" Code: " << err << std::endl;
			}

			return ERROR_NoError;
		}

		Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity) {
			assert(registry.all_of<TransformSystem::Component>(entity) && "SpriteSystem::Component requires TransformSystem::Component");

			if (!registry.ctx().contains<Context>()) {
				return Expected<std::reference_wrapper<Component>>::Error(ERROR_SystemNotInitialized);
			}

			Context& ctx = registry.ctx().get<Context>();
			Component& component = registry.emplace<Component>(entity);

			component._defaultShaderPtr = ctx.defaultShaderPtr;
			component._changed = true;

			return std::ref(component);
		}

		ErrorCode SetSystemShader(Context& ctx, std::shared_ptr<ShaderResource> shaderPtr) {
			ctx.shaderResourcePtr = shaderPtr;
			return ERROR_NoError;
		}

		ErrorCode SetShader(Component& component, std::shared_ptr<ShaderResource> shaderPtr) {
			component._shaderResourcePtr = shaderPtr;
			component._changed = true;
			return ERROR_NoError;
		}
		ErrorCode SetTextureClipOffset(Component& component, float x, float y) {
			return SetTextureClipOffset(component, glm::vec2 { x, y });
		}

		ErrorCode SetTextureClipOffset(Component& component, glm::vec2 offset) {
			if (!component._textureLoaded) {
				return ERROR_ResourceNotLoaded;
			}
			glm::vec2 textSize { component._textureResourcePtr->GetTexture().getSize().x, component._textureResourcePtr->GetTexture().getSize().y };
			textSize.x = std::max(1.f, textSize.x);
			textSize.y = std::max(1.f, textSize.y);
			component._clipOffset = offset / textSize;
			component._changed = true;
			return ERROR_NoError;
		}

		ErrorCode SetTextureClipSize(Component& component, float x, float y) {
			return SetTextureClipSize(component, glm::vec2 { x, y });
		}

		ErrorCode SetTextureClipSize(Component& component, glm::vec2 size) {
			if (!component._textureLoaded) {
				return ERROR_ResourceNotLoaded;
			}
			glm::vec2 textSize { component._textureResourcePtr->GetTexture().getSize().x, component._textureResourcePtr->GetTexture().getSize().y };
			textSize.x = std::max(1.f, textSize.x);
			textSize.y = std::max(1.f, textSize.y);
			component._clipSize = size / textSize;
			component._changed = true;
			return ERROR_NoError;
		}

		ErrorCode SetTextureClipRotation(Component& component, Radians rotation) {
			component._clipRotation = rotation;
			component._changed = true;
			return ERROR_NoError;
		}

		ErrorCode SetTextureClipRotationDeg(Component& component, Degrees rotation) {
			component._clipRotation = glm::degrees(rotation);
			component._changed = true;
			return ERROR_NoError;
		}

		ErrorCode SetColor(Component& component, sf::Color color) {
			component._color = color;
			component._changed = true;
			return ERROR_NoError;
		}

		ErrorCode SetTexture(Component& component, entt::registry& registry, ResourceId textureId, const ResourcePath* paths, const std::size_t numPaths, const TextureOptions& opts) {
			if (!registry.ctx().contains<EngineSystem::Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}
			if (!registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			Context& ctx = registry.ctx().get<Context>();
			Engine& engine = registry.ctx().get<EngineSystem::Context>().engineRef;

			auto& rResourceManager = engine.GetResourceManager();
			// Load Resource
			auto pTextureResource = rResourceManager.template GetOrLoadResource<TextureResource>(textureId, paths, numPaths);
			// Update Render Batch
			// Update Component System Context batches
			if (component._opaqueBatchContext) {
				component._opaqueBatchContext.batch->RemoveModel(component._opaqueBatchContext);
			}
			if (component._translucentBatchContext) {
				component._translucentBatchContext.batch->RemoveModel(component._translucentBatchContext);
			}

			std::list<RenderBatch>& opaqueBatches = ctx.opaqueBatches[textureId];
			// Find batch that isn't full
			// Assume front of list is empty
			auto itr = opaqueBatches.begin();
			if (opaqueBatches.size() == 0) {
				RenderBatch& batch = opaqueBatches.emplace_front(opaqueBatches, ctx.quadVBO, ctx.quadEBO);
				batch.iterator = opaqueBatches.begin();

				// Set texture pointer
				if (textureId != EMPTY_TEXTURE_ID) {
					batch.pTexture = &pTextureResource->GetTexture();
				}

				itr = opaqueBatches.begin();
			}
			else {
				while (itr != opaqueBatches.end()) {
					if (!itr->full) {
						break;
					}
					++itr;
				}

				// Everything was full, add a new RenderBatch
				if (itr == opaqueBatches.end()) {
					RenderBatch& batch = opaqueBatches.emplace_front(opaqueBatches, ctx.quadVBO, ctx.quadEBO);
					batch.iterator = opaqueBatches.begin();
					batch.pTexture = opaqueBatches.back().pTexture;
					itr = opaqueBatches.begin();
				}
			}
			itr->AddModel(component._opaqueBatchContext);


			if (opts.transluscent) {
				std::list<RenderBatch>& translucentBatches = ctx.translucentBatches[textureId];
				// Find batch that isn't full
				// Assume front of list is empty
				auto itr = translucentBatches.begin();
				if (translucentBatches.size() == 0) {
					RenderBatch& batch = translucentBatches.emplace_front(translucentBatches, ctx.quadVBO, ctx.quadEBO);
					batch.iterator = translucentBatches.begin();

					// Set texture pointer
					if (textureId != EMPTY_TEXTURE_ID) {
						batch.pTexture = &pTextureResource->GetTexture();
					}
					itr = translucentBatches.begin();
				}
				else {
					while (itr != translucentBatches.end()) {
						if (!itr->full) {
							break;
						}
						++itr;
					}

					// Everything was full, add a new RenderBatch
					if (itr == translucentBatches.end()) {
						RenderBatch& batch = translucentBatches.emplace_front(translucentBatches, ctx.quadVBO, ctx.quadEBO);
						batch.iterator = translucentBatches.begin();
						batch.pTexture = translucentBatches.back().pTexture;
						itr = translucentBatches.begin();
					}
				}
				itr->AddModel(component._translucentBatchContext);
			}

			// Assign texture to component
			component._textureResourcePtr = pTextureResource;

			sf::Texture& texture = pTextureResource->GetTexture();
			component._textureLoaded = true;
			SetTextureClipSize(component, glm::vec2(texture.getSize().x, texture.getSize().y));

			return ERROR_NoError;
		}

		ErrorCode SetSize(Component& component, glm::vec2 size) {
			component._size = size;
			component._changed = true;
			return ERROR_NoError;
		}

		ErrorCode SetSize(Component& component, float width, float height) {
			component._size.x = width;
			component._size.y = height;
			component._changed = true;
			return ERROR_NoError;
		}

		SMASHER_API ErrorCode SetDepth(Component& component, float depth) {
			component._depth = depth;
			component._changed = true;
			return ERROR_NoError;
		}


		std::shared_ptr<ShaderResource> GetDefaultShader(Component& component) { return component._defaultShaderPtr; }
		std::shared_ptr<ShaderResource> GetShader(Component& component) { return component._shaderResourcePtr; }
		glm::vec2 GetTextureClipSize(Component& component) { return component._clipSize; }
		Radians GetTextureClipRotation(Component& component) { return component._clipRotation; }
		Degrees GetTextureClipRotationDeg(Component& component) { return glm::degrees(component._clipRotation); }
		glm::vec2 GetTextureClipOffset(Component& component) { return component._clipOffset; }
		sf::Color GetColor(Component& component) { return component._color; }
		glm::vec2 GetSize(Component& component) { return component._size; };
		float GetDepth(Component& component) { return component._depth; };
		bool HasTextureAsset(Component& component) { return (bool)component._textureResourcePtr; }
		std::shared_ptr<TextureResource> GetTextureAsset(Component& component) { return component._textureResourcePtr; }
	};
}