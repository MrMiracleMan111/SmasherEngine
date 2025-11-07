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
#include "Smasher/Resources.h"

#include "Smasher/ComponentManagers/DrawableComponentManager.h"
#include "Smasher/Components/DrawableComponent.h"
#include "Smasher/Layer.h"
#include "Smasher/ResourceManager.h"
#include "Smasher/ComponentManagers/RenderBatch.h"
#include "Smasher/EventManager.h"
#include "Smasher/Events.h"

namespace Smasher {
	DrawableComponentManager::DrawableComponentManager(Layer& state) :
		BaseComponentManager<DrawableComponent>(state) {
		Engine& engine = state.GetEngine();

		EventManager& rEventManager = engine.GetEventManager();
		EventSubscriptionHandle handle = GetLayer().Subscribe<Events::WindowCloseEvent>(
			&DrawableComponentManager::OnWindowClose, this
		);

		if (!engine.IsHeadless()) {
			InitQuadGLBuffers();
		}

		// Load the basic shader 
		static_assert(Smasher::HasRenderCapability<Smasher::DrawableComponentManager>, "DrawableComponentManager should have the render capability");

		std::shared_ptr<Smasher::ShaderResource> shader = engine.GetResourceManager().LoadVertFragShaderResource(EngineConfig::DRAWABLE_COMPONENT_VERT_SHADER, EngineConfig::DRAWABLE_COMPONENT_FRAG_SHADER);
		sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(engine.GetWindow().getView().getTransform().getMatrix());
		shader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);
		m_DefaultShader = shader;
		SetShaderResource(shader);
	}

	DrawableComponentManager::~DrawableComponentManager()
	{
		m_Components.clear();
	}

	void DrawableComponentManager::InitQuadGLBuffers() {
		// Cache the current GL_VERTEX_ARRAY_BINDING, GL_ARRAY_BUFFER_BINDING values
		// so that they can be restored after, InitGLObjects
		GLint currentVAO, currentVBO;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

		glGenBuffers(1, &m_QuadVBO);
		glGenBuffers(1, &m_QuadEBO);

		glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(RenderBatch::StaticVertices), RenderBatch::StaticVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(RenderBatch::StaticIndices), RenderBatch::StaticIndices, GL_STATIC_DRAW);

		glBindVertexArray(currentVAO);
		glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
	}


	void DrawableComponentManager::Render(sf::RenderWindow& rWindow) {
#ifdef	BENCHMARK
		std::chrono::time_point<std::chrono::system_clock> BENCHMARK_now = std::chrono::system_clock::now();
#endif
		for (auto& itr : m_Components) {
			if (!itr.m_TransformChanged)
				continue;
			OnComponentChangeData(itr);
		}

#ifdef	BENCHMARK
		std::chrono::microseconds BENCHMARK_diff = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - BENCHMARK_now);
		Smasher::InternalTimers::SMASHER_TimeAccumulator += BENCHMARK_diff;
#endif

		GLint currentVAO, currentVBO;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

		sf::Glsl::Mat4 ViewProjectionMatrix(rWindow.getView().getTransform().getMatrix());
		sf::Shader::bind(&m_ShaderResource->GetShader());
		static_assert(std::is_same_v<std::shared_ptr<ShaderResource>, decltype(m_ShaderResource)>, "fail");
		m_ShaderResource->GetShader().setUniform("ViewProjectionMatrix", ViewProjectionMatrix);
		m_ShaderResource->GetShader().setUniform("translucentPass", false);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDepthRange(1.0f, -1.0f); // top = 1, bottom = 0
		glDisable(GL_BLEND);

		sf::Texture* pTexture = nullptr;
		for (auto& itr : m_OpaqueBatches) {
			std::list<RenderBatch>& batchList = itr.second;
			if (!batchList.empty()) {
				pTexture = batchList.back().pTexture;
			}
			bool hasTexture = pTexture != nullptr;
			m_ShaderResource->GetShader().setUniform("hasTexture", hasTexture);

			if (hasTexture) {
				sf::Texture::bind(pTexture, sf::Texture::Pixels);
			}

			for (auto& batch : batchList) {
				DrawBatch(batch);
			}
			sf::Texture::bind(NULL);
		}

		m_ShaderResource->GetShader().setUniform("translucentPass", true);
		glDepthMask(GL_FALSE); // Disable depth writes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (auto& itr : m_TranslucentBatches) {
			std::list<RenderBatch>& batchList = itr.second;
			if (!batchList.empty()) {
				pTexture = batchList.back().pTexture;
			}
			bool hasTexture = pTexture != nullptr;
			m_ShaderResource->GetShader().setUniform("hasTexture", hasTexture);

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
		glDepthRange(1.0f, -1.0f); // top = 1, bottom = 0

		sf::Shader::bind(NULL);
		glBindVertexArray(currentVAO);
		glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
		rWindow.resetGLStates();
	}

	void DrawableComponentManager::DrawBatch(RenderBatch& rRenderBatch) {
		rRenderBatch.UpdateGLBufferData(); // Updates only if dirty
		glBindVertexArray(rRenderBatch.instanceVAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, RenderBatch::StaticIndices);
		glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_BYTE, (GLvoid*)0, (GLsizei)rRenderBatch.modelCount);

		glBindVertexArray(0);
	}

	void DrawableComponentManager::OnComponentChangeData(DrawableComponent& rComponent) {
		static const float clipTransformMatrixArr[] = {
		  1.f, 0.f, 0.f,
		  0.f, 1.f, 0.f,
		  0.f, 0.f, 0.f
		};

		rComponent.m_TransformChanged = false;

		Mat3 clipTransform{ clipTransformMatrixArr };
		
		if (rComponent.m_TextureLoaded) {
			clipTransform = Mat3(rComponent.GetClipTransform());
		}

		uint32_t colorData = (uint32_t)rComponent.GetColor().toInteger();
		Radians rotation = Radians{ (float)rComponent.GetRotation() * ((float)std::numbers::pi / 180.0f) };
		ModelData data = ModelData{
			{ rComponent.GetPosition().x, rComponent.GetPosition().y, rComponent.GetDepth() },
			{ rComponent.GetScale().x, rComponent.GetScale().y },
			clipTransform,
			rotation,
			colorData,
			nullptr
		};

		if (rComponent.m_OpaqueBatchContext) {
			memcpy(&rComponent.m_OpaqueBatchContext.batch->models[rComponent.m_OpaqueBatchContext.index], &data, sizeof(ModelData));
			//rComponent.m_OpaqueBatchContext.batch->models[rComponent.m_OpaqueBatchContext.index] = data;
			rComponent.m_OpaqueBatchContext.batch->models[rComponent.m_OpaqueBatchContext.index].ownerContext = &rComponent.m_OpaqueBatchContext;
			rComponent.m_OpaqueBatchContext.batch->dirty = true;
		}
		if (rComponent.m_TranslucentBatchContext) {
			memcpy(&rComponent.m_TranslucentBatchContext.batch->models[rComponent.m_TranslucentBatchContext.index], &data, sizeof(ModelData));
			//rComponent.m_TranslucentBatchContext.batch->models[rComponent.m_TranslucentBatchContext.index] = data;
			rComponent.m_TranslucentBatchContext.batch->models[rComponent.m_TranslucentBatchContext.index].ownerContext = &rComponent.m_TranslucentBatchContext;
			rComponent.m_TranslucentBatchContext.batch->dirty = true;
		}
	}

	void DrawableComponentManager::OnComponentSetTexture(DrawableComponent& rComponent, ResourceID id, bool translucent) {
		if (rComponent.m_TextureResource != nullptr &&
			id == rComponent.m_TextureResource->GetID()) {
			return;
		}

		if (rComponent.m_OpaqueBatchContext) {
			rComponent.m_OpaqueBatchContext.batch->RemoveModel(rComponent.m_OpaqueBatchContext);
		}
		if (rComponent.m_TranslucentBatchContext) {
			rComponent.m_TranslucentBatchContext.batch->RemoveModel(rComponent.m_TranslucentBatchContext);
		}

		std::list<RenderBatch>& opaqueBatches = m_OpaqueBatches[id];
		// Find batch that isn't full
		// Assume front of list is empty
		auto itr = opaqueBatches.begin();
		if (opaqueBatches.size() == 0) {
			RenderBatch& batch = opaqueBatches.emplace_front(opaqueBatches, m_QuadVBO, m_QuadEBO);
			batch.iterator = opaqueBatches.begin();
			
			// Set texture pointer
			if (id != DrawableComponentManager::EMPTY_TEXTURE_ID) {
				ResourceManager& rResourceManager = GetLayer().GetEngine().GetResourceManager();
				auto pTexture = rResourceManager.GetResource<TextureResource>(id);
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
		itr->AddModel(rComponent.m_OpaqueBatchContext);


		if (translucent) {
			std::list<RenderBatch>& translucentBatches = m_TranslucentBatches[id];
			// Find batch that isn't full
			// Assume front of list is empty
			auto itr = translucentBatches.begin();
			if (translucentBatches.size() == 0) {
				RenderBatch& batch = translucentBatches.emplace_front(translucentBatches, m_QuadVBO, m_QuadEBO);
				batch.iterator = translucentBatches.begin();
				ResourceManager& rResourceManager = GetLayer().GetEngine().GetResourceManager();

				// Set texture pointer
				if (id != DrawableComponentManager::EMPTY_TEXTURE_ID) {
					auto pTexture = rResourceManager.GetResource<TextureResource>(id);
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
					RenderBatch& batch = translucentBatches.emplace_front(translucentBatches, m_QuadVBO, m_QuadEBO);
					batch.iterator = translucentBatches.begin();
					batch.pTexture = translucentBatches.back().pTexture;
					itr = translucentBatches.begin();
				}
			}
			itr->AddModel(rComponent.m_TranslucentBatchContext);
		}
	}

	void DrawableComponentManager::OnComponentDelete(DrawableComponent& rComponent) {
		if (rComponent.m_OpaqueBatchContext) {
			rComponent.m_OpaqueBatchContext.batch->RemoveModel(rComponent.m_OpaqueBatchContext);
		}
		if (rComponent.m_TranslucentBatchContext) {
			rComponent.m_TranslucentBatchContext.batch->RemoveModel(rComponent.m_TranslucentBatchContext);
		}
	}

	void DrawableComponentManager::OnWindowClose(Events::WindowCloseEvent& event) {
		// Invalidate all Batch Contexts
		for (auto& itr : m_Components) {
			itr.m_OpaqueBatchContext.Invalidate(); // Invalidate it
			itr.m_TranslucentBatchContext.Invalidate(); // Invalidate it
		}
		
		// Clear all OpenGL Buffers
		m_OpaqueBatches.clear();
		m_TranslucentBatches.clear();
	}
}