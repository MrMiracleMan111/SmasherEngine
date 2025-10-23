#pragma once
#include <list>
#include <unordered_map>
#include <typeindex>
#include <array>
#include <chrono>
#include <functional>
#include <condition_variable>
#include <thread>
#include <mutex>
#include "Smasher/Base.h"

namespace Smasher {
	struct Event;
	class EventManager;
	class Engine;

	struct EventSubscription {
		std::function<void(Event&)> Callback; // Event callback function
		EventSubscription(std::function<void(Event&)> callback) : Callback(callback) {}
		~EventSubscription() {};

		EventSubscription(EventSubscription&) = delete;
		EventSubscription(EventSubscription&&) = default;
		EventSubscription& operator=(EventSubscription&) = delete;
		EventSubscription& operator=(EventSubscription&&) = delete;
	};

	// Event subscriptions last for the lifetime of EventSubscriptionHandle
	struct SMASHER_API EventSubscriptionHandle {
	friend class EventManager;
	friend class Layer;
	public:
		EventSubscriptionHandle() = default;
		~EventSubscriptionHandle();
		EventSubscriptionHandle(const EventSubscriptionHandle&) = delete;
		EventSubscriptionHandle(EventSubscriptionHandle&& other) noexcept;
		EventSubscriptionHandle& operator =(EventSubscriptionHandle&) = delete;
		EventSubscriptionHandle& operator =(EventSubscriptionHandle&&) noexcept;


		bool IsValid() const { return m_Valid && !m_SubscriptionPtr.expired(); }

		void Unsubscribe();

	// Only EventManagers should have privilege to instantiate EventSubscriptionHandles
	protected:
		EventSubscriptionHandle(std::list<std::shared_ptr<EventSubscription>>& list,
			std::list<std::shared_ptr<EventSubscription>>::iterator itr, std::shared_ptr<EventSubscription>& subscriptionPtr, EventManager& rEventManager) :
			m_SubscriptionListPtr(&list),
			m_SubscriptionPtr(subscriptionPtr),
			m_Itr(itr),
			m_Valid(true),
			m_EventManagerPtr(&rEventManager) {};

		void Invalidate() { m_Valid = false; }

	private:
		std::list<std::shared_ptr<EventSubscription>>* m_SubscriptionListPtr = nullptr; // List containing this subscription
		std::list<std::shared_ptr<EventSubscription>>::iterator m_Itr; // Location of this subscription within the list
		std::weak_ptr<EventSubscription> m_SubscriptionPtr;
		EventManager* m_EventManagerPtr = nullptr;
		bool m_Valid = false;
	};

	// Non-Copyable
	class SMASHER_API EventManager final {
		friend struct EventSubscriptionHandle;
		friend class Layer;
	public:
		EventManager() = delete;
		EventManager(Engine& engine) :
			m_EngineRef(engine),
			m_AsyncEventConsumerThread(&EventManager::AsyncEventConsumer, this),
			m_EventQueue(),
			m_AsyncEventQueue() {}
		~EventManager();
		EventManager(EventManager&) = delete;
		EventManager(EventManager&& other) noexcept;
		EventManager& operator= (EventManager&) = delete;
		EventManager& operator= (EventManager&& other) noexcept;

		// Defaults to Synchronous
		template<class T, typename... Args>
		void Publish(Args&&... eventArgs);

		void Dispatch();

		void DispatchAsync(Engine& engine);

		void AsyncEventConsumer();

		void StopAsync(); // Stops async event thread

	protected:
		std::mutex& GetAsyncSwapQueueMutex() { return m_AsyncEventSwapQueueMutex; };
		std::mutex& GetAsyncSubscriptionsMutex() { return m_AsyncSubscriptionsMutex; };

	private:
		std::vector<std::shared_ptr<Event>> m_EventQueue;
		std::vector<std::shared_ptr<Event>> m_AsyncEventQueue; // Events currently being processed in async queue

		std::vector<std::shared_ptr<Event>> m_AsyncEventSwapQueue; // Events to be processed next in async queue
		std::condition_variable m_AsyncEventsCV;
		std::atomic_bool m_AsyncRunning = true;
		std::mutex m_AsyncEventsMutex;
		std::mutex m_AsyncEventSwapQueueMutex;
		std::mutex m_AsyncSubscriptionsMutex;

		std::thread m_AsyncEventConsumerThread;
		std::reference_wrapper<Engine> m_EngineRef;

		// Async Event Queues should be move constructible and move assignable
		static_assert(std::is_nothrow_move_constructible_v<decltype(m_AsyncEventQueue)>);
		static_assert(std::is_nothrow_move_assignable_v<decltype(m_AsyncEventQueue)>);
		static_assert(std::is_nothrow_move_constructible_v<decltype(m_AsyncEventSwapQueue)>);
		static_assert(std::is_nothrow_move_assignable_v<decltype(m_AsyncEventSwapQueue)>);
	};
}

#include "EventManager.inl"