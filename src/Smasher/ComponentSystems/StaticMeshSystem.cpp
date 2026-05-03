#include "Smasher/Base.h"
#include <entt/entity/registry.hpp>
#include "Smasher/ComponentSystems/StaticMeshSystem.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	namespace StaticMeshSystem {
		StaticMeshBatch::StaticMeshBatch(std::list<StaticMeshBatch>& ownerBatchList, GPUBlockHandle block) :
			ownerBatchList(ownerBatchList),
			block(block)
		{

		}

		StaticMeshBatch::StaticMeshBatch(StaticMeshBatch&& other) :
			ownerBatchList(std::move(other.ownerBatchList)),
			models(std::move(other.models)), 
			batchContexts(std::move(other.batchContexts)), 
			iterator(std::move(other.iterator)),
			dirty(other.dirty), 
			full(other.full), 
			modelCount(other.modelCount), 
			block(std::move(other.block))
		{

		}

		bool StaticMeshBatch::IsFull() const {
			return full;
		}

		void StaticMeshBatch::AddInstance(StaticMeshBatchContext& batchCtx) {
			dirty = true; // Data will be copied in later
			batchCtx.index = modelCount;
			batchCtx.batch = this;
			models[modelCount].owningContext = &batchCtx;
			++modelCount;
			full = (modelCount == StaticMeshBatch::MAX_MODEL_COUNT);
			if (full) {
				// Move to back of list since it's full
				ownerBatchList.get().splice(ownerBatchList.get().end(), ownerBatchList, iterator);
			}
		}

		void StaticMeshBatch::RemoveInstance(StaticMeshBatchContext& batchCtx) {
			assert(batchCtx.batch != nullptr);
			assert(batchCtx.index <= (modelCount - 1));
			//assert(models[context.index].ownerContext == &context);
			dirty = true;
			full = false;

			if (batchCtx.index < (modelCount - 1)) {
				// Swap and pop from old batch
				const std::size_t index = batchCtx.index;
				models.at(index).owningContext = nullptr;

				const StaticMeshBatchContext* newOwnerContex = models.at(modelCount - 1).owningContext;
				std::swap(models.at(index), models.at(modelCount - 1));
				if (models.at(index).owningContext != nullptr) {
					models.at(index).owningContext->index = index;
				}
				assert(models.at(index).owningContext == newOwnerContex);
				assert(models.at(index).owningContext != nullptr);
			}
			else if (batchCtx.index == (modelCount - 1)) {
				// Nothing
			}
			else {
				assert(false); // should be unreachable
			}

			--modelCount;
			batchCtx.batch = nullptr;
			batchCtx.index = SIZE_MAX;
		}


		void RemoveInstaceFromBatch(Context& ctx, Component& component) {
			component.batchContext.batch->RemoveInstance(component.batchContext);
			StaticMeshBatch& batch = *component.batchContext.batch;
			// Leave at least 1 empty batch in the list
			if (batch.modelCount == 0 && batch.ownerBatchList.get().size() > 1) {
				batch.ownerBatchList.get().erase(batch.iterator);
				return;
			}
			// Move to front of list
			batch.ownerBatchList.get().splice(batch.iterator, batch.ownerBatchList, batch.ownerBatchList.get().begin());
		}

		static void OnConstructStaticMesh(const entt::entity entity) {
			// Add to batch
		};

		static void OnDestroyStaticMesh(entt::registry &registry, const entt::entity entity) {
			// Remove from batch
			Context& ctx = registry.ctx().get<Context>();
			Component& component = registry.get<Component>(entity);
			RemoveInstaceFromBatch(ctx, component);
		};

		static void DirtyMeshCallback(DirtyMeshStorage& storage, const entt::registry& registry, const entt::entity entity) {
			bool hasComponents = registry.all_of<TransformSystem::Component, StaticMeshSystem::Component>(entity);
			bool hasEntity = storage.contains(entity);
			if (hasEntity && !hasComponents) {
				storage.remove(entity);
			}
			else if (!hasEntity && hasComponents) {
				storage.emplace(entity);
			}
			//storage.contains(entity) ? (storage.get(entity) = hasComponents) : storage.emplace(entity, false);
		}

		ErrorCode Initialize(entt::registry& registry) {
			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			auto& ctx = registry.ctx().emplace<Context>();

			entt::connection onConstructStaticMeshListener = registry.on_construct<StaticMeshSystem::Component>().connect<&OnConstructStaticMesh>();
			entt::connection onDestroyStaticMeshListener = registry.on_destroy<StaticMeshSystem::Component>().connect<&OnDestroyStaticMesh>();

			ctx.dirtyStaticMeshes.bind(registry);
			// Observer changes to object transform or static mesh component
			ctx.dirtyStaticMeshes.on_update<TransformSystem::Component, &DirtyMeshCallback>()
				.on_update<StaticMeshSystem::Component, &DirtyMeshCallback>();


			return ERROR_NoError;
		}

		ErrorCode Teardown(entt::registry& registry) {
			return ERROR_NoError;
		}


		Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity, std::shared_ptr<StaticMeshResource> meshResource, GPUBlockPool<StaticMeshData> &blockPool) {
			assert(registry.all_of<TransformSystem::Component>(entity) && "StaticMeshSystem::Component requires TransformSystem::Component");

			if (!registry.ctx().contains<Context>()) {
				return Expected<std::reference_wrapper<Component>>::Error(ERROR_SystemNotInitialized);
			}

			Context& ctx = registry.ctx().get<Context>();
			Component& component = registry.emplace<Component>(entity);
			std::list<StaticMeshBatch>& batches = ctx.batches[meshResource->GetId()];
			
			component.meshResource = meshResource;


			if (batches.size() == 0) {
				GPUBlockHandle block = blockPool.AllocateBlock().Get();
				StaticMeshBatch& batch = batches.emplace_front(batches, block);
				batch.iterator = batches.begin();
			}
			// Everything is full (unfilled batches are ALWAYS at the front)
			else if (batches.front().IsFull()) {
				GPUBlockHandle block = blockPool.AllocateBlock().Get();
				StaticMeshBatch& batch = batches.emplace_front(batches, block);
				batch.iterator = batches.begin();
			}
			batches.front().AddInstance(component.batchContext);

			DirtyMeshCallback(ctx.dirtyStaticMeshes, registry, entity);
			return std::ref(component);
		}

		ErrorCode SyncStaticMeshTransforms(entt::registry& registry, Context& ctx) {
			auto start = std::chrono::system_clock::now();
			for (auto &itr : ctx.batches) {
				for (auto &batch : itr.second) {
					batch.dirty = false;
				}
			}

			auto& changedTransforms = TransformSystem::GetDirty(registry);
			for (auto [entity, transform, mesh] : ctx.dirtyStaticMeshes.view<TransformSystem::Component, StaticMeshSystem::Component>().each()) {
				StaticMeshSystem::StaticMeshData& instanceData = mesh.batchContext.batch->models[mesh.batchContext.index];
				mesh.batchContext.batch->dirty = true;
				instanceData.transform = TransformSystem::GetTransform(transform);
			}
			ctx.dirtyStaticMeshes.clear();
			auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
			std::cout << "SyncTransformTime: " << delta.count() << "ms\n";
			return ERROR_NoError;
		}
	}
}
