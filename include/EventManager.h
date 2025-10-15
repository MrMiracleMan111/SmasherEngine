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
#include "Base.h"

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


		bool IsValid() const { return m_Valid; }

		void Unsubscribe();

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
		std::list<EventSubscription>* m_SubscriptionListPtr = nullptr; // List containing this subscription
		std::list<EventSubscription>::iterator m_Itr; // Location of this subscription within the list
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
		std::unordered_map<std::type_index, std::list<EventSubscription>> m_EventSubscriptionsByType;
		std::unordered_map<std::type_index, std::list<EventSubscription>> m_AsyncEventSubscriptionsByType;

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
	};
}

#include "EventManager.inl"