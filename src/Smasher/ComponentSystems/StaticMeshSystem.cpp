#include "Smasher/Base.h"
#include <tracy/Tracy.hpp>
#include <entt/entity/registry.hpp>
#include <cstdlib>
#include "Smasher/ComponentSystems/StaticMeshSystem.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	namespace StaticMeshSystem {

		StaticMeshBatch& StaticMeshBatchList::AddBatch(GPUBlockPool<StaticMeshData>& blockPool) {
			GPUBlockHandle block = blockPool.AllocateBlock().Get();
			StaticMeshBatch& batch = batchList.emplace_front(*this, block);
			// Is part of first range
			if (gpuBlockRanges.size() > 0 &&
				(block.index + 1) == gpuBlockRanges.front().first.index) {
				gpuBlockRanges.front().first.index = block.index;
			}
			else if (gpuBlockRanges.size() == 0) {
				gpuBlockRanges.push_back(std::make_pair(block, 1));
			}
			else {
				for (auto it = gpuBlockRanges.begin(); it != gpuBlockRanges.end(); ++it) {
					if (block.index < it->first.index) {
						gpuBlockRanges.insert(it, std::make_pair(block, 1));
						break;
					}

					// Out of bounds, stop
					if (it == (gpuBlockRanges.end() - 1)) {
						gpuBlockRanges.push_back(std::make_pair(block, 1));
						break;
					}
				}
			}

			// Coalesce adjacent ranges
			int i = 1;
			while (i < gpuBlockRanges.size()) {
				std::pair<GPUBlockHandle, int>& left = gpuBlockRanges[i - 1];
				std::pair<GPUBlockHandle, int>& right = gpuBlockRanges[i];
				if (right.first.index == (left.first.index + left.second)) {
					left.second = left.second + right.second;
					gpuBlockRanges.erase(gpuBlockRanges.begin() + i);
					continue;
				}
				i++;
			}
			return batch;
		}

		void StaticMeshBatchList::RemoveBatch(std::list<StaticMeshBatch>::iterator itr) {
			GPUBlockHandle block = itr->block;
			batchList.erase(itr);
			for (auto it = gpuBlockRanges.begin(); it != gpuBlockRanges.end(); ++it) {
				// Out of bounds
				if (block.index < it->first.index) {
					continue;
				}

				// Out of bounds, stop
				if (block.index > (it->first.index + it->second)) {
					break;
				}

				// Decrement the block
				it->second--;
				// Remove range
				if (it->second == 0) {
					gpuBlockRanges.erase(it);
					break;
				}
				// Remove from beginning of range
				else if (it->first.index == block.index) {
					it->first.index++;
				}
				// Remove from end of range
				else if (block.index == (it->first.index + it->second - 1)) {
					break;
				}
				// Split the range
				else {
					int leftSize = (block.index - it->first.index);
					int rightSize = (it->second) - leftSize;
					int rightStart = it->first.index + it->second - rightSize + 1;

					it->second = leftSize;
					GPUBlockHandle rightBlock{ rightStart };
					auto rightRange = std::make_pair(rightBlock, rightSize);
					gpuBlockRanges.insert(it + 1, rightRange);
				}
			}
		}

		StaticMeshBatch::StaticMeshBatch(StaticMeshBatchList& ownerBatchList, GPUBlockHandle block) :
			ownerBatchList(ownerBatchList),
			block(block)
		{
			instanceOwners.fill(nullptr);
		}

		StaticMeshBatch::StaticMeshBatch(StaticMeshBatch&& other) :
			ownerBatchList(std::move(other.ownerBatchList)),
			instances(std::move(other.instances)), 
			instanceOwners(std::move(other.instanceOwners)),
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

		void StaticMeshBatch::AddInstance(StaticMeshBatchContext& ownerContext) {
			dirty = true; // Data will be copied in later
			ownerContext.index = modelCount;
			ownerContext.batch = this;
			instanceOwners[modelCount] = &ownerContext;
			instances[modelCount].materialId = 1;
			//instances[modelCount].owningContext = &batchCtx;
			++modelCount;
			full = (modelCount == StaticMeshBatch::MAX_MODEL_COUNT);
			if (full) {
				auto& batchList = ownerBatchList.get().batchList;
				// Move to back of list since it's full
				batchList.splice(batchList.end(), batchList, iterator);
			}
		}

		void StaticMeshBatch::RemoveInstance(StaticMeshBatchContext& batchCtx) {
			assert(batchCtx.batch != nullptr);
			assert(batchCtx.index <= (modelCount - 1));
			//assert(instances[context.index].ownerContext == &context);
			dirty = true;
			full = false;

			if (batchCtx.index < (modelCount - 1)) {
				// Swap and pop from old batch
				const std::size_t index = batchCtx.index;
				instanceOwners.at(index) = nullptr;
				//instances.at(index).owningContext = nullptr;

				//const StaticMeshBatchContext* newOwnerContex = instances.at(modelCount - 1).owningContext;
				StaticMeshBatchContext* newOwnerContex = instanceOwners.at(modelCount - 1);
				if (newOwnerContex != nullptr) {
					newOwnerContex->index = index;
				}
				std::swap(instances.at(index), instances.at(modelCount - 1));
				std::swap(instanceOwners.at(index), instanceOwners.at(modelCount - 1));
				//if (instances.at(index).owningContext != nullptr) {
				//	instances.at(index).owningContext->index = index;
				//}
				//assert(instances.at(index).owningContext == newOwnerContex);
				//assert(instances.at(index).owningContext != nullptr);


				if (instanceOwners.at(index) != nullptr) {
					instanceOwners.at(index)->index = index;
				}
				assert(instanceOwners.at(index) == newOwnerContex);
				assert(instanceOwners.at(index) != nullptr);
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
			auto& batchList = batch.ownerBatchList.get().batchList;
			// Leave at least 1 empty batch in the list
			if (batch.modelCount == 0 && batchList.size() > 1) {
				batchList.erase(batch.iterator);
				return;
			}
			// Move to front of list
			batchList.splice(batch.iterator, batchList, batchList.begin());
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
		}

		ErrorCode Initialize(entt::registry& registry) {
			ZoneScoped;
			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			auto& ctx = registry.ctx().emplace<Context>();

			entt::connection onConstructStaticMeshListener = registry.on_construct<StaticMeshSystem::Component>().connect<&OnConstructStaticMesh>();
			entt::connection onDestroyStaticMeshListener = registry.on_destroy<StaticMeshSystem::Component>().connect<&OnDestroyStaticMesh>();

			ctx.dirtyStaticMeshes.bind(registry);
			// Observer changes to object transform or static mesh component
			ctx.dirtyStaticMeshes.on_update<TransformSystem::Component, &DirtyMeshCallback>()
				.on_update<Component, &DirtyMeshCallback>();


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
			StaticMeshBatchList& batches = ctx.batches[meshResource->GetId()];
			
			component.meshResource = meshResource;


			if (batches.batchList.size() == 0) {
				StaticMeshBatch& batch = batches.AddBatch(blockPool);
				batch.iterator = batches.batchList.begin();
			}
			// Everything is full (unfilled batches are ALWAYS at the front)
			else if (batches.batchList.front().IsFull()) {
				StaticMeshBatch& batch = batches.AddBatch(blockPool);
				batch.iterator = batches.batchList.begin();
			}
			batches.batchList.front().AddInstance(component.batchContext);

			DirtyMeshCallback(ctx.dirtyStaticMeshes, registry, entity);
			return std::ref(component);
		}

		ErrorCode SetMaterial(Component& component, MaterialBinding material) {
			component.materialResourceId = material.materialId;
			component.batchContext.batch->dirty = true;
			component.batchContext.batch->instances[component.batchContext.index].materialId = material.index;
			return ERROR_NoError;
		}

		ErrorCode SyncStaticMeshTransforms(entt::registry& registry, Context& ctx) {
			ZoneScoped;
			for (auto &itr : ctx.batches) {
				for (auto &batch : itr.second.batchList) {
					batch.dirty = false;
				}
			}

			auto& changedTransforms = TransformSystem::GetDirty(registry);
			for (auto [entity, transform, mesh] : ctx.dirtyStaticMeshes.view<TransformSystem::Component, StaticMeshSystem::Component>().each()) {
				StaticMeshSystem::StaticMeshData& instanceData = mesh.batchContext.batch->instances[mesh.batchContext.index];
				mesh.batchContext.batch->dirty = true;
				instanceData.transform = TransformSystem::GetTransform(transform);
			}
			ctx.dirtyStaticMeshes.clear();
			return ERROR_NoError;
		}
	}
}
