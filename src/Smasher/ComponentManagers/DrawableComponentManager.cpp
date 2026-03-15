#include <numbers>
#include <chrono>
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
#include "Smasher/Base.h"
#include "Smasher/EngineConfig.h"
#include "Smasher/Resources.h"

#include "Smasher/ComponentManagers/DrawableComponentManager.h"
#include "Smasher/Components/DrawableComponent.h"
#include "Smasher/Layer.h"
#include "Smasher/ResourceManager.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ComponentManagers/RenderBatch.h"
#include "Smasher/EventManager.h"
#include "Smasher/Events.h"
#include "Smasher/Engine.h"

namespace Smasher {
	DrawableComponentManager::DrawableComponentManager(Layer &state) :
		BaseComponentManager<DrawableComponent>(state) {
		Engine &engine = state.GetEngine();

		EventManager &eventManager = engine.GetEventManager();
		EventSubscriptionHandle handle = GetLayer().Subscribe<Events::WindowCloseEvent>(
			&DrawableComponentManager::OnWindowClose, this
		);

		if (!engine.IsHeadless()) {
			InitQuadGLBuffers();
		}

		static_assert(Smasher::HasRenderCapability<Smasher::DrawableComponentManager>, "DrawableComponentManager should have the render capability");

		std::shared_ptr<Smasher::ShaderResource> pShader = engine.GetResourceManager().LoadVertFragShaderResource(EngineConfig::DRAWABLE_COMPONENT_VERT_SHADER, EngineConfig::DRAWABLE_COMPONENT_FRAG_SHADER);
		sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(engine.GetWindow().getView().getTransform().getMatrix());
		pShader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);
		m_DefaultShader = pShader;
		SetShaderResource(pShader);
	}

	DrawableComponentManager::~DrawableComponentManager()
	{
		m_Components.clear();
	}

	void DrawableComponentManager::InitQuadGLBuffers() {
		// Cache the current GL_VERTEX_ARRAY_BINDING, GL_ARRAY_BUFFER_BINDING values
		// so that they can be restored after, InitGLObjects (for SFML compatability)
		GLint currentVAO;
		GLint currentVBO;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

		glGenBuffers(1, &m_QuadVBO);
		glGenBuffers(1, &m_QuadEBO);

		glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(RenderBatch::STATIC_VERTICES), RenderBatch::STATIC_VERTICES, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(RenderBatch::STATIC_INDICES), RenderBatch::STATIC_INDICES, GL_STATIC_DRAW);

		glBindVertexArray(currentVAO);
		glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
	}


	void DrawableComponentManager::Render(sf::RenderWindow &window) {
#ifdef	BENCHMARK
		std::chrono::time_point<std::chrono::system_clock> BENCHMARK_now = std::chrono::system_clock::now();
#endif
		for (auto &itr : m_Components) {
			if (!itr.m_TransformChanged && !itr.m_ClipChanged)
				continue;
			OnComponentChangeData(itr);
		}

#ifdef	BENCHMARK
		std::chrono::microseconds BENCHMARK_diff = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - BENCHMARK_now);
		Smasher::InternalTimers::SMASHER_TimeAccumulator += BENCHMARK_diff;
#endif

		GLint currentVAO;
		GLint currentVBO;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

		sf::Glsl::Mat4 viewProjectionMatrix{ window.getView().getTransform().getMatrix() };
		sf::Shader::bind(&m_ShaderResourcePtr->GetShader());
		static_assert(std::is_same_v<std::shared_ptr<ShaderResource>, decltype(m_ShaderResourcePtr)>, "fail");
		m_ShaderResourcePtr->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);
		m_ShaderResourcePtr->GetShader().setUniform("translucentPass", false);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDepthRange(1.f, -1.f); // top = 1, bottom = 0
		glDisable(GL_BLEND);

		sf::Texture *pTexture = nullptr;
		for (auto &itr : m_OpaqueBatches) {
			std::list<RenderBatch> &batchList = itr.second;
			if (!batchList.empty()) {
				pTexture = batchList.back().pTexture;
			}
			bool hasTexture = pTexture != nullptr;
			m_ShaderResourcePtr->GetShader().setUniform("hasTexture", hasTexture);

			if (hasTexture) {
				sf::Texture::bind(pTexture, sf::Texture::Pixels);
			}

			for (auto &batch : batchList) {
				DrawBatch(batch);
			}
			sf::Texture::bind(NULL);
		}

		m_ShaderResourcePtr->GetShader().setUniform("translucentPass", true);
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (auto &itr : m_TranslucentBatches) {
			std::list<RenderBatch> &batchList = itr.second;
			if (!batchList.empty()) {
				pTexture = batchList.back().pTexture;
			}
			bool hasTexture = pTexture != nullptr;
			m_ShaderResourcePtr->GetShader().setUniform("hasTexture", hasTexture);

			if (hasTexture) {
				sf::Texture::bind(batchList.back().pTexture, sf::Texture::Pixels);
			}

			for (auto &batch : batchList) {
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
		window.resetGLStates();
	}

	void DrawableComponentManager::DrawBatch(RenderBatch &renderBatch) {
		renderBatch.UpdateGLBufferData(); // Updates only if dirty
		glBindVertexArray(renderBatch.instanceVAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, RenderBatch::StaticIndices);
		glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_BYTE, (GLvoid*)0, (GLsizei)renderBatch.modelCount);

		glBindVertexArray(0);
	}

	void DrawableComponentManager::OnComponentChangeData(DrawableComponent &component) {
		static const float IDENTITY_TRANSFORM_MATRIX_ARRAY[] = {
		  1.f, 0.f, 0.f,
		  0.f, 1.f, 0.f,
		  0.f, 0.f, 0.f
		};

		//transform.m_Changed = false;
		component.m_TransformChanged = false;

		Mat3 clipTransform{ IDENTITY_TRANSFORM_MATRIX_ARRAY };

		if (component.m_TextureLoaded) {
			clipTransform = Mat3(component.GetClipTransform());
		}

		const Mat3 &vertTransform = component.GetTransform();

		uint32_t colorData = (uint32_t)component.GetColor().toInteger();
		ModelData data = ModelData{
			vertTransform,
			clipTransform,
			component.GetDepth(),
			colorData,
			nullptr
		};

		if (component.m_OpaqueBatchContext) {
			memcpy(&component.m_OpaqueBatchContext.batch->models[component.m_OpaqueBatchContext.index], &data, sizeof(ModelData));
			component.m_OpaqueBatchContext.batch->models[component.m_OpaqueBatchContext.index].ownerContext = &component.m_OpaqueBatchContext;
			component.m_OpaqueBatchContext.batch->dirty = true;
		}

		if (component.m_TranslucentBatchContext) {
			memcpy(&component.m_TranslucentBatchContext.batch->models[component.m_TranslucentBatchContext.index], &data, sizeof(ModelData));
			component.m_TranslucentBatchContext.batch->models[component.m_TranslucentBatchContext.index].ownerContext = &component.m_TranslucentBatchContext;
			component.m_TranslucentBatchContext.batch->dirty = true;
		}
	}

	void DrawableComponentManager::OnComponentSetTexture(DrawableComponent &component, ResourceId textureId, bool translucent) {
		if (component.m_TextureResourcePtr != nullptr &&
			textureId == component.m_TextureResourcePtr->GetId()) {
			return;
		}

		if (component.m_OpaqueBatchContext) {
			component.m_OpaqueBatchContext.batch->RemoveModel(component.m_OpaqueBatchContext);
		}
		if (component.m_TranslucentBatchContext) {
			component.m_TranslucentBatchContext.batch->RemoveModel(component.m_TranslucentBatchContext);
		}

		std::list<RenderBatch>& opaqueBatches = m_OpaqueBatches[textureId];
		// Find batch that isn't full
		// Assume front of list is empty
		auto itr = opaqueBatches.begin();
		if (opaqueBatches.size() == 0) {
			RenderBatch& batch = opaqueBatches.emplace_front(opaqueBatches, m_QuadVBO, m_QuadEBO);
			batch.iterator = opaqueBatches.begin();

			// Set texture pointer
			if (textureId != DrawableComponentManager::EMPTY_TEXTURE_ID) {
				ResourceManager& resourceManager = GetLayer().GetEngine().GetResourceManager();
				auto pTexture = resourceManager.GetResource<TextureResource>(textureId);
				batch.pTexture = &pTexture->GetTexture();
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
				RenderBatch& batch = opaqueBatches.emplace_front(opaqueBatches, m_QuadVBO, m_QuadEBO);
				batch.iterator = opaqueBatches.begin();
				batch.pTexture = opaqueBatches.back().pTexture;
				itr = opaqueBatches.begin();
			}
		}
		itr->AddModel(component.m_OpaqueBatchContext);


		if (translucent) {
			std::list<RenderBatch>& translucentBatches = m_TranslucentBatches[textureId];
			// Find batch that isn't full
			// Assume front of list is empty
			auto itr = translucentBatches.begin();
			if (translucentBatches.size() == 0) {
				RenderBatch &batch = translucentBatches.emplace_front(translucentBatches, m_QuadVBO, m_QuadEBO);
				batch.iterator = translucentBatches.begin();
				ResourceManager &resourceManager = GetLayer().GetEngine().GetResourceManager();

				// Set texture pointer
				if (textureId != DrawableComponentManager::EMPTY_TEXTURE_ID) {
					auto pTexture = resourceManager.GetResource<TextureResource>(textureId);
					batch.pTexture = &pTexture->GetTexture();
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
					RenderBatch &batch = translucentBatches.emplace_front(translucentBatches, m_QuadVBO, m_QuadEBO);
					batch.iterator = translucentBatches.begin();
					batch.pTexture = translucentBatches.back().pTexture;
					itr = translucentBatches.begin();
				}
			}
			itr->AddModel(component.m_TranslucentBatchContext);
		}
	}

	void DrawableComponentManager::OnComponentDelete(DrawableComponent &component) {
		if (component.m_OpaqueBatchContext) {
			component.m_OpaqueBatchContext.batch->RemoveModel(component.m_OpaqueBatchContext);
		}
		if (component.m_TranslucentBatchContext) {
			component.m_TranslucentBatchContext.batch->RemoveModel(component.m_TranslucentBatchContext);
		}
	}

	void DrawableComponentManager::OnWindowClose(Events::WindowCloseEvent &event) {
		// Invalidate all Batch Contexts
		for (auto &itr : m_Components) {
			itr.m_OpaqueBatchContext.Invalidate(); // Invalidate it
			itr.m_TranslucentBatchContext.Invalidate(); // Invalidate it
		}
		
		// Clear all OpenGL Buffers
		m_OpaqueBatches.clear();
		m_TranslucentBatches.clear();
	}
}