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
#include "Base.h"
#include "Resources.h"

#include "ComponentManagers/DrawableComponentManager.h"
#include "Components/DrawableComponent.h"
#include "Components/Transform2DComponent.h"
#include "GameState.h"
#include "ResourceManager.h"
#include "RenderBatch.h"
#include "EventManager.h"
#include "Events.h"
#include <iostream>

namespace Smasher {
	DrawableComponentManager::DrawableComponentManager(GameState& state) :
		BaseComponentManager<DrawableComponent>(state) {

		EventManager& rEventManager = state.GetEngine().GetEventManager();
		EventSubscriptionHandle handle = rEventManager.Subscribe<Events::WindowCloseEvent>(
			std::bind(&DrawableComponentManager::OnWindowClose, this, std::placeholders::_1)
		);
	}



	void DrawableComponentManager::Render(sf::RenderWindow& rWindow) {
#ifdef BENCHMARK
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

		for (auto& itr : m_OpaqueBatches) {
			RenderBatch& batch = itr.second;
			DrawBatch(batch);
		}

		m_ShaderResource->GetShader().setUniform("translucentPass", true);
		glDepthMask(GL_FALSE); // Disable depth writes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (auto& itr : m_TranslucentBatches) {
			RenderBatch& batch = itr.second;
			DrawBatch(batch);
		}

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_ALWAYS);
		glDepthRange(1.0f, -1.0f); // top = 1, bottom = 0

		sf::Shader::bind(NULL);

		glBindVertexArray(currentVAO);
		glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
	}

	void DrawableComponentManager::DrawBatch(RenderBatch& rRenderBatch) {
		rRenderBatch.UpdateGLBufferData(); // Updates only if dirty
		sf::Texture::bind(rRenderBatch.pTexture, sf::Texture::Pixels);
		glBindVertexArray(rRenderBatch.instanceVAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, RenderBatch::StaticIndices);
		glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_BYTE, (GLvoid*)0, (GLsizei)rRenderBatch.modelCount);
		glBindVertexArray(0);
		sf::Texture::bind(NULL);
	}

	inline void DrawableComponentManager::OnComponentChangeData(DrawableComponent& rComponent) {
		if (!rComponent.m_TextureLoaded) {
			return;
		}
		rComponent.m_TransformChanged = false;

		uint32_t colorData = (uint32_t)rComponent.GetColor().toInteger();

		Radians rotation = Radians{ (float)rComponent.GetRotation() * ((float)std::numbers::pi / 180.0f) };
		ModelData data = ModelData{
			{ rComponent.GetPosition().x, rComponent.GetPosition().y, rComponent.GetDepth() },
			{ rComponent.GetScale().x, rComponent.GetScale().y },
			Mat3(rComponent.GetClipTransform()),
			rotation,
			colorData
		};

		if (rComponent.m_OpaqueBatchContext) {
			memcpy(&rComponent.m_OpaqueBatchContext.batch->models[rComponent.m_OpaqueBatchContext.index], &data, sizeof(ModelData));
			//rComponent.m_OpaqueBatchContext.batch->models[rComponent.m_OpaqueBatchContext.index] = data;
			rComponent.m_OpaqueBatchContext.batch->dirty = true;
		}
		if (rComponent.m_TranslucentBatchContext) {
			memcpy(&rComponent.m_OpaqueBatchContext.batch->models[rComponent.m_OpaqueBatchContext.index], &data, sizeof(ModelData));
			//rComponent.m_TranslucentBatchContext.batch->models[rComponent.m_TranslucentBatchContext.index] = data;
			rComponent.m_TranslucentBatchContext.batch->dirty = true;
		}
	}

	void DrawableComponentManager::OnComponentSetTexture(DrawableComponent& rComponent, ResourceID id, bool translucent) {
		if (rComponent.m_TextureResource != nullptr &&
			id == rComponent.m_TextureResource->GetID()) {
			return;
		}

		if (rComponent.m_TextureLoaded) {
			if (rComponent.m_OpaqueBatchContext) {
				rComponent.m_OpaqueBatchContext.batch->RemoveModel(rComponent.m_OpaqueBatchContext);
			}
			if (rComponent.m_TranslucentBatchContext) {
				rComponent.m_TranslucentBatchContext.batch->RemoveModel(rComponent.m_TranslucentBatchContext);
			}
		}

		RenderBatch* opaqueBatch = &m_OpaqueBatches[id];
		if (opaqueBatch->pTexture == nullptr) {
			ResourceManager& rResourceManager = GetGameState().GetEngine().GetResourceManager();
			auto pTexture = rResourceManager.GetResource<TextureResource>(id);
			opaqueBatch->pTexture = &pTexture->GetTexture();
		}

		opaqueBatch->AddModel(rComponent.m_OpaqueBatchContext);

		if (translucent) {
			RenderBatch* translucentBatch = &m_TranslucentBatches[id];
			if (translucentBatch->pTexture == nullptr) {
				ResourceManager& rResourceManager = GetGameState().GetEngine().GetResourceManager();
				auto pTexture = rResourceManager.GetResource<TextureResource>(id);
				translucentBatch->pTexture = &pTexture->GetTexture();
			}

			translucentBatch->AddModel(rComponent.m_TranslucentBatchContext);
		}
	}

	void DrawableComponentManager::OnComponentDelete(DrawableComponent& rComponent) {
		if (!rComponent.m_TextureLoaded) {
			return;
		}

		if (rComponent.m_OpaqueBatchContext) {
			rComponent.m_OpaqueBatchContext.batch->RemoveModel(rComponent.m_OpaqueBatchContext);
		}
		if (rComponent.m_TranslucentBatchContext) {
			rComponent.m_TranslucentBatchContext.batch->RemoveModel(rComponent.m_TranslucentBatchContext);
		}
	}

	void DrawableComponentManager::OnWindowClose(const Events::WindowCloseEvent& event) {
		// Invalidate all Batch Contexts
		for (auto& itr : m_Components) {
			itr.m_OpaqueBatchContext = BatchContext{}; // Invalidate it
			itr.m_TranslucentBatchContext = BatchContext{}; // Invalidate it
		}
		
		// Clear all OpenGL Buffers
		m_OpaqueBatches.clear();
		m_TranslucentBatches.clear();
	}
}