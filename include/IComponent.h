#pragma once
#include "Base.h"
#include "Entity.h"

#define SMASHER_USE_COMPONENT_MANAGER(manager) std::unique_ptr<manager> StaticInstantiateManager(Smasher::GameState& state) { return std::make_unique<manager>(state); }

namespace Smasher {
	class IComponentManager;
	class Entity;

	enum class ComponentStatus {
		VALID,   // Component is active within a manager (not removed)
		INVALID, // Ready to be removed or just created (not assigned to manager yet)
		REMOVED  // Component was removed 
	};

	class SMASHER_API IComponent {
		friend class IComponentManager;
	public:
		IComponent() = delete;
		IComponent(const IComponent& other) : m_Entity(other.m_Entity), m_Status(other.m_Status) {};
		IComponent(IComponent&& other) noexcept : m_Entity(other.m_Entity), m_Status(other.m_Status) {
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
		Entity& GetEntity() const { return m_Entity; }
		size_t GetIndex() const { return m_Index; }

	protected:
		IComponent(Entity& entity) : m_Entity(entity), m_Index(SIZE_MAX), m_Status(ComponentStatus::INVALID) {};

		void SetStatus(ComponentStatus status) { m_Status = status; }
		void SetIndex(size_t index) { m_Index = index; }

	private:
		std::reference_wrapper<Entity> m_Entity;
		size_t m_Index = SIZE_MAX; // Index component is located at within owning ComponentManager
		ComponentStatus m_Status = ComponentStatus::INVALID; // Used by manager to prevent double remove
	};
}