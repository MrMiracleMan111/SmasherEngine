#include "Smasher/Base.h"
#include <tracy/Tracy.hpp>
#include <entt/entity/registry.hpp>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Smasher/ComponentSystems/StaticMeshSystem.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ErrorCodes.h"
#include "Smasher/Util/GraphicsUtil.h"

namespace Smasher {
	namespace StaticMeshSystem {

		uint32_t SetMeshId(uint32_t materialMeshId, uint32_t meshId) {
			return (materialMeshId & 0x0000FFFF) | ((meshId << 16) & 0xFFFF0000);
		}

		uint32_t SetMaterialId(uint32_t materialMeshId, uint32_t materialId) {
			return (materialMeshId & 0xFFFF0000) | (materialId & 0x0000FFFF);
		}

		uint32_t GetMeshId(uint32_t materialMeshId) {
			return (materialMeshId >> 16) & 0x0000FFFF;
		}

		uint32_t GetMaterialId(uint32_t materialMeshId) {
			return (materialMeshId) & 0x0000FFFF;
		}


		StaticMeshBatch& StaticMeshBatchList::AddBatch(GraphicsUtil::GPUBlockPool<StaticMeshData>& blockPool) {
			GraphicsUtil::GPUBlockHandle block = blockPool.AllocateBlock().Get();
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
				std::pair<GraphicsUtil::GPUBlockHandle, int>& left = gpuBlockRanges[i - 1];
				std::pair<GraphicsUtil::GPUBlockHandle, int>& right = gpuBlockRanges[i];
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
			GraphicsUtil::GPUBlockHandle block = itr->block;
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
					GraphicsUtil::GPUBlockHandle rightBlock{ rightStart };
					auto rightRange = std::make_pair(rightBlock, rightSize);
					gpuBlockRanges.insert(it + 1, rightRange);
				}
			}
		}

		StaticMeshBatch::StaticMeshBatch(StaticMeshBatchList& ownerBatchList, GraphicsUtil::GPUBlockHandle block) :
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
			instances[modelCount].materialIdMeshId = 0;
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
				batchCtx.batch->instances[batchCtx.index].materialIdMeshId = 0;
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
			//component.batchContext.batch->RemoveInstance(component.batchContext);
			//StaticMeshBatch& batch = *component.batchContext.batch;
			//auto& batchList = batch.ownerBatchList.get().batchList;
			//// Leave at least 1 empty batch in the list
			//if (batch.modelCount == 0 && batchList.size() > 1) {
			//	batchList.erase(batch.iterator);
			//	return;
			//}
			//// Move to front of list
			//batchList.splice(batch.iterator, batchList, batchList.begin());
		}

		static void OnConstructStaticMesh(const entt::entity entity) {
			// Add to batch
		};

		static void OnDestroyStaticMesh(entt::registry &registry, const entt::entity entity) {
			// Remove from batch
			Context& ctx = registry.ctx().get<Context>();
			Component& component = registry.get<Component>(entity);

			ctx.totalNumInstances--;
			component.batchContext.batch->RemoveInstance(component.batchContext);
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


		Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity, StaticMeshBinding staticMeshBinding, GraphicsUtil::GPUBlockPool<StaticMeshData> &blockPool) {
			assert(registry.all_of<TransformSystem::Component>(entity) && "StaticMeshSystem::Component requires TransformSystem::Component");

			if (!registry.ctx().contains<Context>()) {
				return Expected<std::reference_wrapper<Component>>::Error(ERROR_SystemNotInitialized);
			}

			Context& ctx = registry.ctx().get<Context>();
			Component& component = registry.emplace<Component>(entity);
			auto& meshResource = staticMeshBinding.resource;
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
			ctx.totalNumInstances++;

			StaticMeshSystem::StaticMeshData& instanceData = component.batchContext.batch->instances[component.batchContext.index];
			instanceData.materialIdMeshId = 0;
			instanceData.materialIdMeshId = SetMeshId(instanceData.materialIdMeshId, staticMeshBinding.index);
			//instanceData.transform = TransformSystem::GetTransform(transform);

			DirtyMeshCallback(ctx.dirtyStaticMeshes, registry, entity);
			return std::ref(component);
		}

		ErrorCode SetMaterial(Component& component, MaterialBinding material) {
			component.materialResourceId = material.materialId;
			component.batchContext.batch->dirty = true;
			auto& instance = component.batchContext.batch->instances[component.batchContext.index];
			instance.materialIdMeshId = SetMaterialId(instance.materialIdMeshId, material.index);

			return ERROR_NoError;
		}

		void ComputeInstanceAABB(glm::vec4& aabbMinOut, glm::vec4& aabbMaxOut, const glm::vec3 aabbMinIn, const glm::vec3 aabbMaxIn, const glm::mat4x4& transform) {
			// Convert AABB to center (midpoint of box) and extent (half width + half height + half depth)
			glm::vec3 center = (aabbMaxIn + aabbMinIn) / 2.f;
			glm::vec3 extend = (aabbMaxIn - aabbMinIn) / 2.f;

			glm::mat4x4 tmp = transform;
			float* p = glm::value_ptr(tmp);
			for (int i = 0; i < 16; ++i) {
				p[i] = glm::abs(p[i]);
			}
			glm::vec4 newCenter = transform * glm::vec4(center, 1.f);
			glm::vec4 newExtend = tmp * glm::vec4(glm::abs(extend), 0.f); // Just rotation + scale
			aabbMaxOut = newCenter + newExtend;
			aabbMinOut = newCenter - newExtend;
		}

		ErrorCode SyncStaticMeshTransforms(entt::registry& registry, Context& ctx) {
			ZoneScoped;
			for (auto &itr : ctx.batches) {
				for (auto &batch : itr.second.batchList) {
					batch.dirty = false;
				}
			}

			auto& changedTransforms = TransformSystem::GetDirty(registry);
			bool first = true;

			//for (auto [entity, transform, mesh] : ctx.dirtyStaticMeshes.view<TransformSystem::Component, StaticMeshSystem::Component>().each()) {
			auto view = registry.view<TransformSystem::Component, StaticMeshSystem::Component>();
			for (auto [entity, transform, mesh] : view.each()) {
				StaticMeshSystem::StaticMeshData& instanceData = mesh.batchContext.batch->instances[mesh.batchContext.index];
				mesh.batchContext.batch->dirty = true;
				instanceData.transform = TransformSystem::GetTransform(transform);
				glm::vec4 minAABB, maxAABB;
				ComputeInstanceAABB(minAABB, maxAABB, mesh.meshResource->GetMinAABB(), mesh.meshResource->GetMaxAABB(), instanceData.transform);

				if (first) {
					ctx.minAABB = minAABB;
					ctx.maxAABB = maxAABB;
				}

				//minAABB.w = 1.f;
				//maxAABB.w = 1.f;
				ctx.minAABB = glm::min(ctx.minAABB, minAABB);
				ctx.maxAABB = glm::max(ctx.maxAABB, maxAABB);
				first = false;
			}
			ctx.dirtyStaticMeshes.clear();
			return ERROR_NoError;
		}
	}
}
