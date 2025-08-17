#pragma once
#include <list>
#include <unordered_map>
#include <typeindex>
#include <array>
#include <chrono>
#include <functional>
#include <mutex>
#include "Base.h"

namespace Smasher {
	struct Event;

	struct EventSubscription {
		std::function<void(const Event&)> Callback;
		EventSubscription(std::function<void(const Event&)> callback) : Callback(callback) {}
		~EventSubscription() {};

		EventSubscription(EventSubscription&) = delete;
		EventSubscription(EventSubscription&&) = default;
		EventSubscription& operator=(EventSubscription&) = delete;
		EventSubscription& operator=(EventSubscription&&) = delete;
	};

	// Event subscriptions last for the lifetime of EventSubscriptionHandle
	struct SMASHER_API EventSubscriptionHandle {
	friend class EventManager;
	public:
		EventSubscriptionHandle() = default;
		~EventSubscriptionHandle();
		EventSubscriptionHandle(const EventSubscriptionHandle&) = delete;
		EventSubscriptionHandle(EventSubscriptionHandle&& other) noexcept;
		EventSubscriptionHandle& operator =(EventSubscriptionHandle&) = delete;
		EventSubscriptionHandle& operator =(EventSubscriptionHandle&&) noexcept;


		bool IsValid() const { return m_Valid; }

	// Only EventManagers should have privilege to instantiate EventSubscriptionHandles
	protected:
		EventSubscriptionHandle(std::list<EventSubscription>& list,
			std::list<EventSubscription>::iterator itr, EventManager& rEventManager) :
			m_SubscriptionListPtr(&list),
			m_Itr(itr),
			m_Valid(true),
			m_EventManagerPtr(&rEventManager) {};

		void Invalidate() { m_Valid = false; }

	private:
		std::list<EventSubscription>* m_SubscriptionListPtr = nullptr;
		std::list<EventSubscription>::iterator m_Itr;
		EventManager* m_EventManagerPtr = nullptr;
		bool m_Valid = false;
	};

	// Non-Copyable
	class SMASHER_API EventManager final {
		friend struct EventSubscriptionHandle;
	public:
		EventManager() :
			m_EventSubscriptionsByTypePtr(std::make_unique<std::array<std::list<EventSubscription>, static_cast<std::size_t>(EventType::END)>>()),
			m_AsyncEventSubscriptionsByTypePtr(std::make_unique<std::array<std::list<EventSubscription>, static_cast<std::size_t>(EventType::END)>>()),
			m_AsyncEventConsumerThread(&EventManager::AsyncEventConsumer, this),
			m_EventQueue(),
			m_AsyncEventQueue() {}
		~EventManager();
		EventManager(EventManager&) = delete;
		EventManager(EventManager&& other) noexcept;
		EventManager& operator= (EventManager&) = delete;
		EventManager& operator= (EventManager&& other) noexcept;

		// Subscribe to synchronous event handling (immediately after its called)
		template<class T>
		EventSubscriptionHandle Subscribe(std::function<void(const T&)> callback) {
			static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");
			
			constexpr std::size_t index = static_cast<std::size_t>(T::GetStaticEventType());
			std::list<EventSubscription>& list = m_EventSubscriptionsByTypePtr->at(index);
			
			auto bound = [callback](const Event& arg) {callback(static_cast<const T&>(arg)); };
			
			list.push_back(EventSubscription{ bound });
			return EventSubscriptionHandle{ list, std::prev(list.end()), *this };
		}

		// Manually unsubscribes an event handle
		// EventSubscriptionHandles will automatically unsubscribe
		// at the end of their lifetime
		void Unsubscribe(EventSubscriptionHandle& handle);

		// Overload for class memebr function ex:
		// Subscribe<EventType>(&Class::MemberFunc, classInstancePointer);
		template<class T, class C>
		EventSubscriptionHandle Subscribe(void (C::* method)(const T&), C* instance) {
			return Subscribe<T>(std::bind(method, instance, std::placeholders::_1));
		}

		// Subscribe to asynchronous event handling (uses separate Event thread)
		template<class T>
		EventSubscriptionHandle SubscribeAsync(std::function<void(const T&)> callback) {
			std::scoped_lock lock(m_AsyncEventSwapQueueMutex);
			static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");

			constexpr std::size_t index = static_cast<std::size_t>(T::GetStaticEventType());
			std::list<EventSubscription>& list = m_AsyncEventSubscriptionsByTypePtr->at(index);

			auto bound = [callback](const Event& arg) {callback(static_cast<const T&>(arg)); };

			list.push_back(EventSubscription{ bound });
			return EventSubscriptionHandle{ list, std::prev(list.end()), *this };
		}

		template<class T, class C>
		EventSubscriptionHandle SubscribeAsync(void (C::* method)(const T&), C* instance) {
			return SubscribeAsync<T>(std::bind(method, instance, std::placeholders::_1));
		}

		// Defaults to Synchronous
		template<class T, typename... Args>
		void Publish(Args&&... eventArgs) {
			static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");
			std::shared_ptr<const T> pEvent = std::make_shared<const T>(std::chrono::system_clock::now(), std::forward<Args>(eventArgs)...);
			m_EventQueue.push_back(pEvent);

			std::unique_lock lock(m_AsyncEventSwapQueueMutex);
			m_AsyncEventSwapQueue.push_back(pEvent);
			lock.unlock();

			std::unique_lock notifyLock(m_AsyncEventsMutex);
			m_AsyncEventsCV.notify_all();
		};

		void Dispatch();

		void DispatchAsync();

		void AsyncEventConsumer();

		void StopAsync(); // Stops async event thread

	private:
		std::unique_ptr<std::array<std::list<EventSubscription>, static_cast<std::size_t>(EventType::END)>> m_EventSubscriptionsByTypePtr;
		std::unique_ptr<std::array<std::list<EventSubscription>, static_cast<std::size_t>(EventType::END)>> m_AsyncEventSubscriptionsByTypePtr;

		std::vector<std::shared_ptr<const Event>> m_EventQueue;
		std::vector<std::shared_ptr<const Event>> m_AsyncEventQueue; // Events currently being processed in async queue

		std::vector<std::shared_ptr<const Event>> m_AsyncEventSwapQueue; // Events to be processed next in async queue
		std::condition_variable m_AsyncEventsCV;
		std::atomic_bool m_AsyncRunning = true;
		std::mutex m_AsyncEventsMutex;
		std::mutex m_AsyncEventSwapQueueMutex;
		std::mutex m_AsyncSubscriptionsMutex;

		std::thread m_AsyncEventConsumerThread;
	};
}