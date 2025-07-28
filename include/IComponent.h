#pragma once
#include "Base.h"

#define SMASHER_USE_COMPONENT_MANAGER(manager) std::unique_ptr<manager> StaticInstantiateManager(Smasher::GameState& state) { return std::make_unique<manager>(state); }

namespace Smasher {
	class IComponentManager;
	class Entity;

	class SMASHER_API IComponent {
		friend class IComponentManager;
	public:
		IComponent(const IComponent& other) :
			m_Entity(other.m_Entity), m_Manager(other.m_Manager), m_Status(other.m_Status) {};
		IComponent(IComponent&& other) noexcept :
			m_Entity(other.m_Entity), m_Manager(other.m_Manager), m_Status(other.m_Status) {
			other.m_Status = ComponentStatus::INVALID;
		};
		IComponent& operator = (const IComponent& other) {
			m_Entity = std::ref(other.m_Entity);
			m_Status = other.m_Status;
			return *this;
		};
		IComponent& operator = (IComponent&& other) noexcept {
			if (this != &other) {
				m_Entity = std::ref(other.m_Entity);
				m_Status = other.m_Status;
				other.m_Status = ComponentStatus::INVALID;
			}
			return other;
		}
		virtual ~IComponent() = default;

		ComponentStatus GetStatus() const { return m_Status; }
		Entity& GetEntity() const { return *m_Entity; }
		IComponentManager& GetManager() const { return *m_Manager; }

		size_t GetIndex() const { return m_Index; }

		void SetEntity(Entity* pEntity) { m_Entity = pEntity; }
		void SetManager(IComponentManager* pManager) { m_Manager = pManager; }

	protected:
		IComponent() : m_Index(SIZE_MAX), m_Status(ComponentStatus::INVALID) {};

		void SetStatus(ComponentStatus status) { m_Status = status; }
		void SetIndex(size_t index) { m_Index = index; }

	private:
		Entity* m_Entity;
		IComponentManager* m_Manager;
		size_t m_Index = SIZE_MAX; // Index component is located at within owning ComponentManager
		ComponentStatus m_Status = ComponentStatus::INVALID; // Used by manager to prevent double remove
	};
}