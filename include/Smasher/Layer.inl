#pragma once
#include "Smasher/Layer.h"
#include "Smasher/IComponentManager.h"
#include "Smasher/ComponentManagers/GenericComponentManager.h"
#include "Smasher/Entity.h"


namespace Smasher {
	/*
		Instantiates a Component Manager to handle the provided component type. Components
		may handle manager instantion through their StaticInstatiateManager<T>() method.
	
		If Component type T doesn't specify a Component type with a StaticInstantiateManager method, the GenericComponentManager
	*/
	template <class ComponentType>
	void Layer::LoadComponentManager()
	{
		static_assert(IComponentType<ComponentType>, "T should be derived from IComponent");

		// Component specifies a manager to use
		if constexpr (HasStaticInstantiateManager<ComponentType>) {
			using ManagerType = typename decltype(ComponentType::StaticInstantiateManager(*this))::element_type;
			static_assert(ComponentManagerHasAddComponent<ManagerType, ComponentType>, "ComponentManager is missing AddComponent method");
			static_assert(std::derived_from<ManagerType, IComponentManager>, "StaticInstantiateManager return type must derive from IComponentManager");

			m_ComponentManagers.emplace(std::type_index(typeid(ComponentType)), ComponentType::StaticInstantiateManager(*this));
			auto &pManager = m_ComponentManagers[std::type_index(typeid(ComponentType))];

			// Was Update method overriden?
			if constexpr (HasUpdateCapability<ManagerType>) {
				if constexpr (!std::is_same<decltype(&ManagerType::Update), decltype(&IComponentManager::Update)>::value) {
					m_ComponentManagersWithUpdate.push_back(pManager.get());
				}
			}

			// Was Render method overriden?
			if constexpr (HasRenderCapability<ManagerType>) {
				if constexpr (!std::is_same<decltype(&ManagerType::Render), decltype(&IComponentManager::Render)>::value) {
					m_ComponentManagersWithRender.push_back(pManager.get());
				}
			}
		}
		// Use a GenericComponentManager<T> if Component doesn't specify a manager to use
		else {
			// Move static_assert outside the class
			m_ComponentManagers.emplace(std::type_index(typeid(ComponentType)), std::make_unique<GenericComponentManager<ComponentType>>(*this));
			static_assert(ComponentManagerHasAddComponent<GenericComponentManager<ComponentType>, ComponentType>, "ComponentManager is missing AddComponent method");
			auto &pManager = m_ComponentManagers[std::type_index(typeid(ComponentType))];

			if constexpr (HasStaticRenderComponent<ComponentType>) {
				m_ComponentManagersWithRender.push_back(pManager.get());
			}

			if constexpr (HasStaticUpdateComponent<ComponentType>) {
				 m_ComponentManagersWithUpdate.push_back(pManager.get());
			}
		}
	}

	template <class T, typename... Args>
	T& Layer::AddEntity(Args&&... componentArgs) {
		static_assert(std::is_base_of<Entity, T>::value, "T must inherit from Entity");
		auto pEntity = std::make_unique<T>(*this, UUID::GetUUID(), std::forward<Args>(componentArgs)...);
		T *pEntityObserver = pEntity.get();
		UUID uuid = pEntity->GetUUID(); // To avoid possible invalidation during std::move(pEntity)
		m_EntityMap.insert({ uuid, std::move(pEntity) });
		pEntityObserver->Init();
		return *pEntityObserver;
	};

	template <class ComponentType>
	IComponentManager& Layer::GetComponentManager() {
		if (m_ComponentManagers.find(std::type_index(typeid(ComponentType))) == m_ComponentManagers.end()) {
			// Lazy Load the manager
			LoadComponentManager<ComponentType>();
		}
		return *m_ComponentManagers[std::type_index(typeid(ComponentType))].get();
	}

	// Subscribe to synchronous event handling (immediately after its called)
	template<class T>
	EventSubscriptionHandle Layer::Subscribe(std::function<void(T&)> callback) {
		static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");

		const std::type_index index = std::type_index(typeid(T));
		std::list<std::shared_ptr<EventSubscription>> &list = m_EventSubscriptionsByType[index];

		auto bound = [callback](Event& arg) { callback(static_cast<T&>(arg)); };
		std::shared_ptr<EventSubscription> subscriptionPtr = std::make_shared<EventSubscription>(bound);
		list.push_back(subscriptionPtr);

		return EventSubscriptionHandle(list, std::prev(list.end()), subscriptionPtr, GetEventManager());
	}

	// Overload for class memebr function ex:
	// Subscribe<EventType>(&Class::MemberFunc, classInstancePointer);
	template<class T, class C>
	EventSubscriptionHandle Layer::Subscribe(void (C:: *method)(T&), C *pInstance) {
		return Subscribe<T>(std::bind(method, pInstance, std::placeholders::_1));
	}

	// Subscribe to asynchronous event handling (uses separate Event thread)
	template<class T>
	EventSubscriptionHandle Layer::SubscribeAsync(std::function<void(T&)> callback) {
		std::scoped_lock lock(m_EventManager.GetAsyncSwapQueueMutex());
		static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");

		const std::type_index index = std::type_index(typeid(T));
		std::list<std::shared_ptr<EventSubscription>> &list = m_AsyncEventSubscriptionsByType[index];

		auto bound = [callback](Event &arg) { callback(static_cast<T&>(arg)); };
		std::shared_ptr<EventSubscription> subscriptionPtr = std::make_shared<EventSubscription>(bound);
		list.push_back(subscriptionPtr);

		return EventSubscriptionHandle(list, std::prev(list.end()), subscriptionPtr, GetEventManager());
	}

	template<class T, class C>
	EventSubscriptionHandle Layer::SubscribeAsync(void (C:: *method)(T&), C *pInstance) {
		return SubscribeAsync<T>(std::bind(method, pInstance, std::placeholders::_1));
	}
}