#pragma once
#include <iostream>
#include "plf_colony.h"
#include "Base.h"

#define SMASHER_USE_COMPONENT_MANAGER(manager) std::unique_ptr<manager> StaticInstantiateManager(Smasher::GameState& state) { return std::make_unique<manager>(state); }

namespace Smasher {
	class IComponentManager;
	class Entity;

	class SMASHER_API IComponent {
		friend class IComponentManager;
	public:
		IComponent(const IComponent& other) = delete;
		IComponent(IComponent&& other) noexcept :
			m_Entity(other.m_Entity), m_Manager(other.m_Manager), m_Status(other.m_Status) {
			other.m_Status = ComponentStatus::INVALID;
		};

		IComponent& operator = (const IComponent& other) = delete;
		IComponent& operator = (IComponent&& other) noexcept {
			if (this != &other) {
				m_Entity = other.m_Entity;
				m_Status = other.m_Status;
				m_Manager = other.m_Manager;
				other.m_Status = ComponentStatus::INVALID;
			}
			return *this;
		}
		virtual ~IComponent() {
			m_Entity = nullptr;
			m_Manager = nullptr;
			m_Status = ComponentStatus::INVALID;
		};

		ComponentStatus GetStatus() const { return m_Status; }
		Entity& GetEntity() const { return *m_Entity; }
		IComponentManager& GetManager() const { return *m_Manager; }

	protected:
		IComponent() : m_Status(ComponentStatus::INVALID) {};

		void SetStatus(ComponentStatus status) { m_Status = status; }
		void SetEntity(Entity& pEntity) { m_Entity = &pEntity; }
		void SetManager(IComponentManager& pManager) { m_Manager = &pManager; }
		template<class T>
		void SetIterator(typename plf::colony<T>::iterator* itrPtr) {
			m_ItrPtr = itrPtr;
		}
		template<class T>
		typename plf::colony<T>::iterator* GetIterator() {
			return static_cast<typename plf::colony<T>::iterator*>(m_ItrPtr);
		}

	private:
		void* m_ItrPtr; // Taboo but necessary
		Entity* m_Entity;
		IComponentManager* m_Manager;
		ComponentStatus m_Status = ComponentStatus::INVALID; // Component not yet added to manager
	};
}