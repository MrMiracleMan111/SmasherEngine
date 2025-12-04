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
		IComponent(const IComponent& other) = delete;
		IComponent(IComponent&& other) noexcept :
			m_Entity(other.m_Entity), m_Manager(other.m_Manager), m_Status(other.m_Status) {
			other.m_Status = ComponentStatus::INVALID;
		};

		IComponent& operator = (const IComponent& other) = delete;
		IComponent& operator = (IComponent&& other) noexcept;
		virtual ~IComponent();

		template<class T>
		T& GetSiblingComponent() const;

		ComponentStatus GetStatus() const { return m_Status; }
		Entity& GetEntity() const { return *m_Entity; }
		IComponentManager& GetManager() const { return *m_Manager; }

	protected:
		IComponent() :
			m_Status(ComponentStatus::INVALID),
			m_ItrPtr(nullptr),
			m_Entity(nullptr),
			m_Manager(nullptr) {};

		void SetStatus(ComponentStatus status) { m_Status = status; }
		virtual void SetEntity(Entity& pEntity) { m_Entity = &pEntity; }
		void SetManager(IComponentManager& pManager) { m_Manager = &pManager; }
		virtual void OnAddComponent() {}; // called when component has been initialiazed and added
		
		template<class T>
		void SetIterator(typename plf::colony<T>::iterator* itrPtr) { m_ItrPtr = itrPtr; }

		template<class T>
		typename plf::colony<T>::iterator* GetIterator() { return static_cast<typename plf::colony<T>::iterator*>(m_ItrPtr); }

	private:
		void* m_ItrPtr = nullptr; // Taboo but necessary
		Entity* m_Entity = nullptr;
		IComponentManager* m_Manager = nullptr;
		ComponentStatus m_Status = ComponentStatus::INVALID; // Component not yet added to manager
	};
}