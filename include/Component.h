#pragma once
#include "Base.h"
#include "Entity.h"

#define SMASHER_USE_COMPONENT_MANAGER(manager) std::unique_ptr<manager> StaticInstantiateManager(Smasher::GameState& state) { return std::make_unique<manager>(state); }

namespace Smasher {
	class ComponentManager;
	class Entity;

	enum class ComponentStatus {
		VALID,   // Component is active within a manager (not removed)
		INVALID, // Ready to be removed or just created (not assigned to manager yet)
		REMOVED  // Component was removed 
	};

	class SMASHER_API Component {
		friend class ComponentManager;
	public:
		Component(Entity& entity) : m_Entity(entity), m_Index(SIZE_MAX), m_Status(ComponentStatus::INVALID) {};
		Component() = delete;
		virtual ~Component() = default;
		Component(Component&) = delete;
		Component(Component&& other) noexcept : m_Entity(other.m_Entity), m_Status(other.m_Status) { other.m_Status = ComponentStatus::INVALID; };
		Component& operator = (Component&) = delete;
		Component& operator = (Component&& other) noexcept {
			if (this != &other) {
				other.m_Status = ComponentStatus::INVALID;
			}
			return other;
		}
		ComponentStatus GetStatus() { return m_Status; }
		Entity& GetEntity() { return m_Entity; }


	protected:
		void SetStatus(ComponentStatus status) { m_Status = status; }
		void SetIndex(size_t index) { m_Index = index; }
		size_t GetIndex() const { return m_Index; }

	private:
		Entity& m_Entity;
		size_t m_Index = SIZE_MAX; // Index component is located at within owning ComponentManager
		ComponentStatus m_Status = ComponentStatus::INVALID; // Used by manager to prevent double remove
	};
}