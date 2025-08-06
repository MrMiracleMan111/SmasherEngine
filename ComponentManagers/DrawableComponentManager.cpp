#include <numbers>

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
#include "Resources.h"

#include "ComponentManagers/DrawableComponentManager.h"
#include "Components/DrawableComponent.h"
#include "Components/Transform2DComponent.h"
#include "GameState.h"
#include "ResourceManager.h"
#include "RenderBatch.h"
#include <iostream>

namespace Smasher {
	void DrawableComponentManager::Render(sf::RenderWindow& rWindow) {
#ifdef	BENCHMARK
		double milliseconds = (double)DrawableComponent::s_TimeSum.count() / 1000.0;
		std::cout << "Drawable Component PushToGPU time sum microseconds: " << milliseconds << std::endl;
		DrawableComponent::s_TimeSum = std::chrono::microseconds::zero();
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

		for (auto& itr : OpaqueBatches) {
			RenderBatch& batch = itr.second;
			DrawBatch(batch);
		}

		m_ShaderResource->GetShader().setUniform("translucentPass", true);
		glDepthMask(GL_FALSE); // Disable depth writes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (auto& itr : TranslucentBatches) {
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

	void DrawableComponentManager::OnComponentChangeData(DrawableComponent& rComponent) {
		if (!rComponent.m_TextureLoaded) {
			return;
		}

#ifdef BENCHMARK
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
#endif

		// Force Transform Update
		// rComponent.GetEntity().GetComponent<Transform2DComponent>().GetTransform();
		Transform2DComponent& rTransfom2DComp = rComponent.GetEntity().GetComponent<Transform2DComponent>();

		// Overwrite transform data in models 
		//Smasher::Mat4 matrix(rComponent.GetTransformPtr().getMatrix());
		//matrix.array[3 * 4 + 2] = rComponent.GetDepth();

#ifdef	BENCHMARK
		auto diff = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - now);
		DrawableComponent::s_TimeSum += diff;
#endif

		sf::Color color = rComponent.GetColor();
		uint32_t colorData =
			((unsigned char)(color.r) << 0)  |
			((unsigned char)(color.g) << 8)  |
			((unsigned char)(color.b) << 16) |
			((unsigned char)(color.a) << 24);

		Radians rotation = Radians{ (float)rTransfom2DComp.GetRotation() * ((float)std::numbers::pi / 180.0f) };
		ModelData data = ModelData{
			{ rTransfom2DComp.GetPosition().x, rTransfom2DComp.GetPosition().y, rComponent.GetDepth() },
			{ rTransfom2DComp.GetScale().x, rTransfom2DComp.GetScale().y },
			Mat3(rComponent.GetClipTransform()),
			rotation,
			colorData
		};
		if (rComponent.m_OpaqueBatchContext) {
			rComponent.m_OpaqueBatchContext.batch->models.at(rComponent.m_OpaqueBatchContext.index) = data;
			rComponent.m_OpaqueBatchContext.batch->dirty = true;
		}
		if (rComponent.m_TranslucentBatchContext) {
			rComponent.m_TranslucentBatchContext.batch->models.at(rComponent.m_TranslucentBatchContext.index) = data;
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

		RenderBatch* opaqueBatch = &OpaqueBatches[id];
		if (opaqueBatch->pTexture == nullptr) {
			ResourceManager& rResourceManager = GetGameState().GetEngine().GetResourceManager();
			auto pTexture = rResourceManager.GetResource<TextureResource>(id);
			opaqueBatch->pTexture = &pTexture->GetTexture();
		}

		opaqueBatch->AddModel(rComponent.m_OpaqueBatchContext);

		if (translucent) {
			RenderBatch* translucentBatch = &TranslucentBatches[id];
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
}