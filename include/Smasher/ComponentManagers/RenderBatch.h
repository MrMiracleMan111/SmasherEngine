#pragma once
#include <list>
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
#include <array>
#include "Smasher/Base.h"

namespace Smasher {
	struct BatchContext;

	struct SMASHER_API ModelData {
		Mat3 vertTransform = Mat3{};
		Mat3 texTransform = Mat3{};
		float depth = 0.f;
		uint32_t color = 0;
		BatchContext* ownerContext = nullptr;
		bool hasTexture = false;
	};

	// Models will use 2D plane
	struct SMASHER_API RenderBatch {
		RenderBatch() = delete;
		RenderBatch(std::list<RenderBatch> &list, GLuint quadVBO, GLuint quadEBO);
		~RenderBatch();
		RenderBatch(const RenderBatch &other) = delete;
		RenderBatch(RenderBatch&&);
		RenderBatch& operator = (const RenderBatch &other) = delete;
		RenderBatch& operator = (RenderBatch&&);

		static const std::size_t MAX_MODEL_COUNT = 512; // Arbitrary
		std::array<ModelData, RenderBatch::MAX_MODEL_COUNT> models; // doesn't keep accurate track of model count
		sf::Texture *pTexture = nullptr;
		std::list<RenderBatch> &ownerBatchList;
		std::list<RenderBatch>::iterator iterator;
		bool dirty = false; // Has the render batch or any elemnts inside changed?
		bool full = false; // Can more models be added to this batch
		std::size_t modelCount = 0; // Keeps accurate track of model count
		GLuint instanceVAO;
		GLuint instanceVBO;
		GLuint quadVBO; // Quad mesh data (buffers are created DrawableComponentManager)
		GLuint quadEBO; // Quad mesh data (buffers are created DrawableComponentManager)
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

	struct SMASHER_API BatchContext {
		RenderBatch *batch = nullptr; // Pointer to a render batch the drawable component is a part of
		std::size_t index = SIZE_MAX; // Index of the drawable component within the Render Batch

		BatchContext() = default; // Invalid state
		BatchContext(RenderBatch *batch, std::size_t index) :
			batch(batch), index(index) {}

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

}