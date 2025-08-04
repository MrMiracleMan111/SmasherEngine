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
#include "GameState.h"
#include "ResourceManager.h"
#include "RenderBatch.h"

namespace Smasher {
	void DrawableComponentManager::Render(sf::RenderWindow& rWindow) {
		rWindow.pushGLStates();
		sf::Glsl::Mat4 ViewProjectionMatrix(rWindow.getView().getTransform().getMatrix());
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
		rWindow.popGLStates();
	}

	void DrawableComponentManager::DrawBatch(RenderBatch& rRenderBatch) {
		rRenderBatch.UpdateGLBufferData(); // Updates only if dirty
		sf::Shader& rShader = m_ShaderResource->GetShader();
		sf::Shader::bind(&rShader);
		sf::Texture::bind(rRenderBatch.pTexture, sf::Texture::Pixels);
		glBindVertexArray(rRenderBatch.instanceVAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, RenderBatch::StaticIndices);
		glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_BYTE, (GLvoid*)0, (GLsizei)rRenderBatch.modelCount);
		glBindVertexArray(0);
		sf::Texture::bind(NULL);
		sf::Shader::bind(NULL);
	}

	void DrawableComponentManager::OnComponentChangeData(DrawableComponent& rComponent) {
		if (!rComponent.m_TextureLoaded) {
			return;
		}

		// Overwrite transform data in models 
		Smasher::Mat4 matrix(rComponent.GetTransformRef().getMatrix());
		matrix.array[3 * 4 + 2] = rComponent.GetDepth();

		sf::Color color = rComponent.GetColor();
		uint32_t colorData =
			((unsigned char)(color.r) << 0)  |
			((unsigned char)(color.g) << 8)  |
			((unsigned char)(color.b) << 16) |
			((unsigned char)(color.a) << 24);

		ModelData data = ModelData{ matrix, Mat3(rComponent.GetClipTransform()), colorData };
		if (rComponent.m_OpaqueBatchContext) {
			rComponent.m_OpaqueBatchContext.batch->models[rComponent.m_OpaqueBatchContext.index] = data;
			rComponent.m_OpaqueBatchContext.batch->dirty = true;
		}
		if (rComponent.m_TranslucentBatchContext) {
			rComponent.m_TranslucentBatchContext.batch->models[rComponent.m_TranslucentBatchContext.index] = data;
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