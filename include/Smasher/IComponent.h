#pragma once
#include <iostream>
#include "Smasher/plf_colony.h"
#include "Smasher/Base.h"

namespace Smasher {
	class IComponentManager;
	class Entity;

	class SMASHER_API IComponent {
		friend class IComponentManager;
	public:
		IComponent(const IComponent &other) = delete;
		IComponent(IComponent &&other) noexcept :
			m_EntityPtr(other.m_EntityPtr), m_ManagerPtr(other.m_ManagerPtr), m_Status(other.m_Status) {
			other.m_Status = ComponentStatus::INVALID;
		};

		IComponent& operator = (const IComponent &other) = delete;
		IComponent& operator = (IComponent &&other) noexcept;
		virtual ~IComponent();

		template<class T>
		T& GetSiblingComponent() const;

		ComponentStatus GetStatus() const { return m_Status; }
		Entity& GetEntity() const { return *m_EntityPtr; }
		IComponentManager& GetManager() const { return *m_ManagerPtr; }

	protected:
		IComponent() :
			m_Status(ComponentStatus::INVALID),
			m_ItrPtr(nullptr),
			m_EntityPtr(nullptr),
			m_ManagerPtr(nullptr) {};

		void SetStatus(ComponentStatus status) { m_Status = status; }
		virtual void SetEntity(Entity &entity) { m_EntityPtr = &entity; }
		void SetManager(IComponentManager &manager) { m_ManagerPtr = &manager; }
		virtual void OnAddComponent() {}; // called when component has been initialiazed and added
		
		template<class T>
		void SetIterator(typename plf::colony<T>::iterator *itrPtr) { m_ItrPtr = itrPtr; }

		template<class T>
		typename plf::colony<T>::iterator* GetIterator() { return static_cast<typename plf::colony<T>::iterator*>(m_ItrPtr); }

	private:
		void *m_ItrPtr = nullptr;
		Entity *m_EntityPtr = nullptr;
		IComponentManager *m_ManagerPtr = nullptr;
		ComponentStatus m_Status = ComponentStatus::INVALID; // Component not yet added to manager
	};
}