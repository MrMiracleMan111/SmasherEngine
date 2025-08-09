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
	RenderBatch::RenderBatch(std::list<RenderBatch>& list) : ownerBatchList(list) {
		InitGLObjects();
	}

	RenderBatch::~RenderBatch() {
		glDeleteVertexArrays(1, &instanceVAO);
		glDeleteBuffers(1, &instanceVBO);
		glDeleteBuffers(1, &quadEBO);
		glDeleteBuffers(1, &quadVBO);

	}

	RenderBatch::RenderBatch(RenderBatch&& other) : ownerBatchList(other.ownerBatchList) {
		models = other.models;
		pTexture = other.pTexture;
		dirty = true; // Has the render batch or any elemnts inside changed?
		modelCount = other.modelCount; // Keeps accurate track of model count
		models = std::move(other.models);
		InitGLObjects();
	};
	RenderBatch& RenderBatch::operator = (RenderBatch&& other) {
		models = other.models;
		pTexture = other.pTexture;
		dirty = true; // Has the render batch or any elemnts inside changed?
		modelCount = other.modelCount; // Keeps accurate track of model count
		models = std::move(other.models);
		InitGLObjects();
		return *this;
	};

	void RenderBatch::InitGLObjects() {
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

		glBindVertexArray(instanceVAO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, models.max_size() * sizeof(Smasher::ModelData), models.data(), GL_DYNAMIC_DRAW);
		glBindVertexArray(0);

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
			assert(false); // unreachable
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
}