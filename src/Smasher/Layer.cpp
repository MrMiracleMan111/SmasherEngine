#include <optional>
#include <unordered_map>
#include "Smasher/Layer.h"
#include "Smasher/Entity.h"
#include "Smasher/Engine.h"

namespace Smasher {

	BaseLayer::BaseLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {};

	Layer::~Layer() {
		m_EntityMap.clear();
		m_ComponentManagersWithRender.clear();
		m_ComponentManagersWithUpdate.clear();
		m_ComponentManagers.clear();
		m_EventSubscriptionsByType.clear();
		m_AsyncEventSubscriptionsByType.clear();
	}

	void Layer::RenderComponentManagers(sf::RenderWindow& window) {
		for (IComponentManager* pManager : m_ComponentManagersWithRender) {
			pManager->Render(window);
		}
	}

	void Layer::PreUpdateComponentManagers(Millisecond delta) {
		for (auto& [key, pManager] : m_ComponentManagers) {
			pManager->PreUpdate(delta);
		}
	}

	void Layer::UpdateComponentManagers(Millisecond delta) {
		for (auto pManager : m_ComponentManagersWithUpdate) {
			pManager->Update(delta);
		}
	}

	void Layer::ShutdownEngine() {
		m_Engine.Shutdown();
	}

	void Layer::ProcessEvent(Event& event)
	{
		const std::type_index index = event.GetEventType();
		std::string typeName = index.name();
		auto& subscriptionList = m_EventSubscriptionsByType[index];
		for (auto& subsription : subscriptionList) {
			subsription->Callback(event);
		}
	}

	void Layer::ProcessAsyncEvent(Event& event)
	{
		const std::type_index index = event.GetEventType();
		std::string typeName = index.name();
		auto& subscriptionList = m_AsyncEventSubscriptionsByType[index];
		for (auto& subsription : subscriptionList) {
			subsription->Callback(event);
		}
	}

	Entity& Layer::MoveEntity(Entity& entity)
	{
		if (&entity.GetLayer() == this) {
			return entity; // Can't move entity to this layer
		}

		assert(!HasEntity(entity.GetUUID())); // Entity UUID should not be taken in this layer
		
		auto itr = entity.GetLayer().m_EntityMap.find(entity.GetUUID());

		std::unique_ptr<Entity> pEntity = std::move(itr->second);
		entity.GetLayer().m_EntityMap.erase(entity.GetUUID());
		m_EntityMap.insert({ entity.GetUUID(), std::move(pEntity) });
		entity.SetLayer(*this);

		return entity;
	}

	Entity& Layer::AddEntity() {
		return AddEntity<Entity>();
	};

	Entity& Layer::GetEntity(UUID uuid) {
		auto itr = m_EntityMap.find(uuid);
		if (itr == m_EntityMap.end()) {
			throw Exceptions::LayerEntityNotFound(std::format("Could not find entity with UUID: {}", (uint64_t)uuid));
		}
		return *itr->second;
	}

	bool Layer::HasEntity(UUID uuid) const {
		return (m_EntityMap.find(uuid) != m_EntityMap.end());
	}

	void Layer::RemoveEntity(UUID uuid) {
		if (!HasEntity(uuid)) {
			throw Exceptions::LayerEntityNotFound(std::format("Could not find entity with UUID: {}", (uint64_t)uuid));
		}
		m_EntityMap.erase(uuid);
	}
}