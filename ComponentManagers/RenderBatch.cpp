#pragma once
#include "RenderBatch.h"
#include "Base.h"

#include <iostream>
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

namespace Smasher {
	RenderBatch::RenderBatch() {
		InitGlObjects();
	}

	RenderBatch::~RenderBatch() {
		glDeleteVertexArrays(1, &instanceVAO);
		glDeleteBuffers(1, &instanceVBO);
		glDeleteBuffers(1, &quadEBO);
		glDeleteBuffers(1, &quadVBO);

	}

	RenderBatch::RenderBatch(const RenderBatch& other) {
		models = other.models;
		pTexture = other.pTexture;
		dirty = true; // Has the render batch or any elemnts inside changed?
		modelCount = other.modelCount; // Keeps accurate track of model count
		InitGlObjects();
	};
	RenderBatch& RenderBatch::operator = (const RenderBatch& other) {
		models = other.models;
		pTexture = other.pTexture;
		dirty = true; // Has the render batch or any elemnts inside changed?
		modelCount = other.modelCount; // Keeps accurate track of model count
		InitGlObjects();
		return *this;
	};

	void RenderBatch::InitGlObjects() {
		glGenVertexArrays(1, &instanceVAO);
		glGenBuffers(1, &instanceVBO);
		glGenBuffers(1, &quadVBO);
		glGenBuffers(1, &quadEBO);

		glBindVertexArray(instanceVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(RenderBatch::StaticVertices), RenderBatch::StaticVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(RenderBatch::StaticIndices), RenderBatch::StaticIndices, GL_STATIC_DRAW);
		glBindVertexArray(0);

		ResizeBuffer(RenderBatch::RESERVE);

		glBindVertexArray(instanceVAO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

		// Instance Position Vec3
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, position)));
		
		// Instance Scale Vec 2
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, scale)));

		// Instance Tex Transform Matrix
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, texTransform) + (0 * sizeof(float))));
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, texTransform) + (3 * sizeof(float))));
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, texTransform) + (6 * sizeof(float))));

		// Instance Color Code
		glVertexAttribIPointer(7, 1, GL_UNSIGNED_INT, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, color)));

		// Instance Rotation
		glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(Smasher::ModelData), (GLvoid*)(offsetof(Smasher::ModelData, rotation)));

		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glEnableVertexAttribArray(7);
		glEnableVertexAttribArray(8);

		glVertexAttribDivisor(2, 1);  // Instance attribute
		glVertexAttribDivisor(3, 1);  // Instance attribute
		glVertexAttribDivisor(4, 1);  // Instance attribute
		glVertexAttribDivisor(5, 1);  // Instance attribute
		glVertexAttribDivisor(6, 1);  // Instance attribute
		glVertexAttribDivisor(7, 1);  // Instance attribute
		glVertexAttribDivisor(8, 1);  // Instance attribute

		glBindVertexArray(0);
	}

	// Resize buffer to fit "count" entries
	void RenderBatch::ResizeBuffer(std::size_t count) {
		dirty = false;
		assert(count != models.size()); // Weird edge case
		models.resize(count);
		if (count < models.size()) {
			models.shrink_to_fit();
		}
		// Resize buffer data to (we want to use models.size no models.capacity)
		glBindVertexArray(instanceVAO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(Smasher::ModelData), models.data(), GL_DYNAMIC_DRAW);
		glBindVertexArray(0);
	}

	void RenderBatch::RemoveModel(BatchContext& context) {
		dirty = true;

		assert(context.index <= modelCount);
		if (context.index < modelCount) {
			// Swap and pop from old batch
			std::swap(models.at(context.index), models.at(modelCount));
		}

		--modelCount;
		context.batch = nullptr;
		context.index = SIZE_MAX;

		// Remove unused extra space
		if ((modelCount - models.size()) >= (2 * RenderBatch::RESERVE)) {
			ResizeBuffer(modelCount - RenderBatch::RESERVE); // Leave 32 extra slots
		}
	}

	void RenderBatch::AddModel(BatchContext& context) {
		dirty = true; // Data will be copied in later
		context.index = modelCount;
		context.batch = this;
		++modelCount;
		if (modelCount >= models.size()) {
			ResizeBuffer(modelCount + RenderBatch::RESERVE);
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
}