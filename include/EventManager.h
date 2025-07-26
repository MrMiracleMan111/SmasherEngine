#pragma once
#include <list>
#include <unordered_map>
#include <typeindex>
#include <array>
#include <chrono>
#include <functional>
#include "Base.h"

namespace Smasher {
	class Event;

	struct EventSubscription {
		std::function<void(Event*)> Callback;
		EventSubscription(std::function<void(Event*)> callback) : Callback(callback) {}
		~EventSubscription() {};

		EventSubscription(EventSubscription&) = delete;
		EventSubscription(EventSubscription&&) = default;
		EventSubscription& operator=(EventSubscription&) = delete;
		EventSubscription& operator=(EventSubscription&&) = delete;
	};

	struct SMASHER_API EventSubscriptionHandle {
	friend class EventManager;
	public:
		EventSubscriptionHandle(EventSubscriptionHandle&& other) noexcept :
			m_List(other.m_List),
			m_Itr(other.m_Itr),
			m_Valid(other.m_Valid) {
			other.m_Valid = false;
		}
		EventSubscriptionHandle() = delete;
		EventSubscriptionHandle(EventSubscriptionHandle&) = delete;
		EventSubscriptionHandle& operator =(EventSubscriptionHandle&) = delete;
		EventSubscriptionHandle& operator =(EventSubscriptionHandle&&) = delete;


		bool IsValid() const { return m_Valid; }

	// Only EventManagers should have privilege to instantiate EventSubscriptionHandles
	protected:
		EventSubscriptionHandle(std::list<EventSubscription>& list,
			std::list<EventSubscription>::iterator itr) :
			m_List(list),
			m_Itr(itr),
			m_Valid(true) {
		};

	private:
		std::list<EventSubscription>& m_List;
		std::list<EventSubscription>::iterator m_Itr;
		bool m_Valid = false;
	};

	class SMASHER_API EventManager {
	public:
		EventManager();
		~EventManager();
		EventManager(EventManager&) = delete;
		EventManager(EventManager&&) = delete;
		EventManager& operator= (EventManager&) = delete;
		EventManager& operator= (EventManager&&) = delete;

		template<class T>
		EventSubscriptionHandle Subscribe(std::function<void(T*)> callback) {
			static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");
			constexpr size_t index = static_cast<size_t>(T::GetStaticEventType());
			std::list<EventSubscription>& list = m_EventSubscriptionsByType.at(index);
			auto bound = [callback](Event* arg) {callback(static_cast<T*>(arg)); };
			list.push_back(EventSubscription{ bound });
			return EventSubscriptionHandle{ list, std::prev(list.end()) };
		}
		void Unsubscribe(EventSubscriptionHandle handle);

		template<class T, typename... Args>
		void Publish(Args&&... eventArgs) {
			static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");
			std::unique_ptr<T> pEvent = std::make_unique<T>(std::chrono::system_clock::now(), std::forward<Args>(eventArgs)...);
			m_EventQueue.push_back(std::move(pEvent));
		};
		void Dispatch();

	private:
		std::array< std::list<EventSubscription>, static_cast<size_t>(EventType::END)> m_EventSubscriptionsByType{};
		std::vector<std::unique_ptr<Event>> m_EventQueue;
	};
}