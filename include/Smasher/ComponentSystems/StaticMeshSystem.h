#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <entt/entity/registry.hpp>
#include "Smasher/Base.h"
#include "Smasher/Resources.h"
#include "Smasher/Exceptions.h"
#include "Smasher/Util/GraphicsUtil.h"
namespace Smasher {
	namespace StaticMeshSystem {
		using DirtyMeshStorage = entt::reactive_mixin<entt::storage<void>>;

		struct StaticMeshBatchContext;

		struct StaticMeshData {
			glm::mat4 transform;
			StaticMeshBatchContext* owningContext;
		};

		class StaticMeshBatch;

		struct StaticMeshBatchContext {
			StaticMeshBatch *batch;
			int index = -1;
		};

		class StaticMeshBatch {
		public:
			StaticMeshBatch() = delete;
			StaticMeshBatch(std::list<StaticMeshBatch> &ownerBatchList, GPUBlockHandle block);
			StaticMeshBatch(const StaticMeshBatch&) = delete;
			StaticMeshBatch(StaticMeshBatch &&other);
			StaticMeshBatch& operator= (const StaticMeshBatch&) = delete;
			StaticMeshBatch& operator= (StaticMeshBatch&&) = delete;

			void AddInstance(StaticMeshBatchContext& batchCtx);
			void RemoveInstance(StaticMeshBatchContext& batchCtx);
			bool IsFull() const;

			static const std::size_t MAX_MODEL_COUNT = 48; // Arbitrary
			std::array<StaticMeshData, StaticMeshBatch::MAX_MODEL_COUNT> models; // doesn't keep accurate track of model count
			std::array<StaticMeshBatchContext, StaticMeshBatch::MAX_MODEL_COUNT> batchContexts; // Batch Contexts for each model
			std::reference_wrapper<std::list<StaticMeshBatch>> ownerBatchList;
			std::list<StaticMeshBatch>::iterator iterator;
			bool dirty = false; // Has the render batch or any elements inside changed?
			bool full = false; // Can more models be added to this batch
			std::size_t modelCount = 0; // Keeps accurate track of model count
			GPUBlockHandle block; // Offset in megabuffer
		};


		struct SMASHER_API Context {
			SDL_GPUBuffer* buffer;
			std::map<ResourceId, std::list<StaticMeshBatch>> batches; // Linked List of Batches
			DirtyMeshStorage dirtyStaticMeshes; // Static Meshes that were modified
		};
		struct SMASHER_API Component {
			MaterialHandle material;
			std::shared_ptr<StaticMeshResource> meshResource;
			StaticMeshBatchContext batchContext;
		};

		SMASHER_API ErrorCode Initialize(entt::registry& registry);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
		SMASHER_API Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity, std::shared_ptr<StaticMeshResource> meshResource, GPUBlockPool<StaticMeshData> &blockPool);

		SMASHER_API ErrorCode SetMaterial(Context& ctx, Component &component, std::shared_ptr<MaterialResource> material);
		SMASHER_API ErrorCode SyncStaticMeshTransforms(entt::registry& registry, Context& ctx);

	}
}
