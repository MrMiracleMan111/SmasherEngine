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
		static const int MAX_STATIC_MESH_COUNT = 512 * 32;
		using DirtyMeshStorage = entt::reactive_mixin<entt::storage<void>>;

		struct StaticMeshBatchContext;

		struct StaticMeshData {
			glm::mat4 transform{};
			// First 16 bits materialId (index in materialProps array)
			// Second 16 bits MeshId
			uint32_t materialIdMeshId = 0;
			uint32_t padding1 = 0;
			uint32_t padding2 = 0;
			uint32_t padding3 = 0;
		};

		struct MortonMeshData {
			glm::vec3 aabbMin;
			uint32_t mortonCode = 0;
			glm::vec3 aabbMax;
			uint32_t meshId = 0;
		};

		class StaticMeshBatch;

		struct StaticMeshBatchContext {
			StaticMeshBatch *batch;
			int index = -1;
			//int count = 0;
		};

		struct StaticMeshBatchList {
			std::list<StaticMeshBatch> batchList;

			// Adjacent GPU Blocks are Coalesced and stored as a range
			std::vector<std::pair<GraphicsUtil::GPUBlockHandle, int>> gpuBlockRanges;

			StaticMeshBatch& AddBatch(GraphicsUtil::GPUBlockPool<StaticMeshData>& blockPool);
			void RemoveBatch(std::list<StaticMeshBatch>::iterator itr);
		};

		struct Component;

		class StaticMeshBatch {
		public:
			StaticMeshBatch() = delete;
			StaticMeshBatch(StaticMeshBatchList& ownerBatchList, GraphicsUtil::GPUBlockHandle block);
			StaticMeshBatch(const StaticMeshBatch&) = delete;
			StaticMeshBatch(StaticMeshBatch&& other);
			StaticMeshBatch& operator= (const StaticMeshBatch&) = delete;
			StaticMeshBatch& operator= (StaticMeshBatch&&) = delete;

			void AddInstance(StaticMeshBatchContext& ownerContext);
			void RemoveInstance(StaticMeshBatchContext& batchCtx);
			bool IsFull() const;

			static const std::size_t MAX_MODEL_COUNT = 32; // Arbitrary
			std::array<StaticMeshData, StaticMeshBatch::MAX_MODEL_COUNT> instances; // doesn't keep accurate track of model count
			std::array<StaticMeshBatchContext*, StaticMeshBatch::MAX_MODEL_COUNT> instanceOwners; // keeps references to Component::batchContext pointers
			std::reference_wrapper<StaticMeshBatchList> ownerBatchList;
			std::list<StaticMeshBatch>::iterator iterator;
			bool dirty = false; // Has the render batch or any elements inside changed?
			bool full = false; // Can more instances be added to this batch
			std::size_t modelCount = 0; // Keeps accurate track of model count
			GraphicsUtil::GPUBlockHandle block; // Offset in megabuffer
		};


		struct SMASHER_API Context {
			std::map<ResourceId, StaticMeshBatchList> batches; // Linked List of Batches
			DirtyMeshStorage dirtyStaticMeshes; // Static Meshes that were modified

			// AABB encompassing ALL static meshes
			glm::vec4 minAABB;
			glm::vec4 maxAABB;
			int totalNumInstances = 0;
		};

		struct SMASHER_API Component {
			ResourceId materialResourceId;
			std::shared_ptr<StaticMeshResource> meshResource;
			StaticMeshBatchContext batchContext;
		};

		SMASHER_API ErrorCode Initialize(entt::registry& registry);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
		SMASHER_API Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity, StaticMeshBinding staticMeshBinding, GraphicsUtil::GPUBlockPool<StaticMeshData> &blockPool);
		SMASHER_API Expected<std::reference_wrapper<Component>> RemoveComponent(entt::registry& registry, entt::entity entity, GraphicsUtil::GPUBlockPool<StaticMeshData>& blockPool);

		SMASHER_API ErrorCode SetMaterial(Component &component, MaterialBinding material);
		SMASHER_API ErrorCode SyncStaticMeshTransforms(entt::registry& registry, Context& ctx);

		uint32_t SetMeshId(uint32_t materialMeshId, uint32_t meshId);
		uint32_t SetMaterialId(uint32_t materialMeshId, uint32_t materialId);
		uint32_t GetMeshId(uint32_t materialMeshId);
		uint32_t GetMaterialId(uint32_t materialMeshId);
	}
}
