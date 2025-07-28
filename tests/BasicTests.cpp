#include <gtest/gtest.h>
#include "Core.h"
#include "BaseComponentManager.h"

class DummyGameState : public Smasher::GameState {
public:
	DummyGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {}
};

class ShutdownEngineGameState : public Smasher::GameState {
public:
	ShutdownEngineGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {}
	void Update(Smasher::Millisecond delta) override {
		ShutdownEngine();
	};
};



struct TestComponent : public Smasher::IComponent {
public:
	TestComponent(Smasher::Entity& entity, int value) :
		Smasher::IComponent(entity),
		m_Value(value) {};
	int GetValue() { return m_Value; }
	static void StaticUpdateComponent(TestComponent& self, Smasher::Millisecond delta) {};

protected:
	int m_Value;
};

struct CustomComponent;

class CustomComponentManager : public Smasher::BaseComponentManager<CustomComponent> {
public:
	CustomComponentManager(Smasher::GameState& state) : Smasher::BaseComponentManager<CustomComponent>(state) {}
	CustomComponentManager(const CustomComponentManager&) = default;
	~CustomComponentManager() = default;

	void Update(Smasher::Millisecond delta) override {};
	void Render(sf::RenderWindow& rWindow) override {};
};

struct CustomComponent : public Smasher::IComponent {
public:
	SMASHER_USE_COMPONENT_MANAGER(CustomComponentManager)
	CustomComponent(Smasher::Entity& entity, int value) :
		Smasher::IComponent(entity),
		m_Value(value) {
	};
	~CustomComponent() = default;

	int GetValue() { return m_Value; }

protected:
	static void StaticUpdateComponent(CustomComponent& self, Smasher::Millisecond delta) {};
	int m_Value;
};


// Component that deletes itself during update
struct DeleteTestComponent : public Smasher::IComponent {
public:
	DeleteTestComponent(Smasher::Entity& entity) : Smasher::IComponent(entity){};
	static void StaticUpdateComponent(DeleteTestComponent& self, Smasher::Millisecond delta) {
		self.GetEntity().RemoveComponent<DeleteTestComponent>();
	};
};

// Component that deletes itself during update
struct SpicyDeleteTestComponent : public Smasher::IComponent {
public:
	SpicyDeleteTestComponent(Smasher::Entity& entity) : Smasher::IComponent(entity) {};
	static void StaticUpdateComponent(SpicyDeleteTestComponent& self, Smasher::Millisecond delta) {
		self.m_Count++;
		switch (self.m_Count) {
		case 1:
			self.GetEntity().RemoveComponent<SpicyDeleteTestComponent>();
			break;
		case 2:
			// Component should have been removed last update
			// so this case should never be hit
			throw std::runtime_error("This should never have been reached!");
			break;
		}
	};

protected:
	unsigned int m_Count = 0;
};

TEST(EntityTest, AddEnttiy) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	EXPECT_NO_THROW({ Smasher::Entity & entity = state.AddEntity<Smasher::Entity>(); });
}

TEST(EntityTest, GetEntity) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::Entity& entity = state.AddEntity<Smasher::Entity>();
	EXPECT_NO_THROW({ state.GetEntity(entity.GetUUID()); });
}

TEST(EntityTest, MissingEntity) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	EXPECT_THROW({ state.GetEntity(Smasher::UUID{10}); }, Smasher::Exceptions::GameStateEntityNotFound);
	Smasher::Entity& entity = state.AddEntity<Smasher::Entity>();
	EXPECT_THROW({ state.GetEntity(Smasher::UUID{entity.GetUUID() + 1});}, Smasher::Exceptions::GameStateEntityNotFound);
}

TEST(ComponentsTest, CreateComponent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	TestComponent& test = state.AddEntity<Smasher::Entity>().AddComponent<TestComponent>(10);
	EXPECT_EQ(10, test.GetValue());
}

TEST(ComponentsTest, DuplicateComponent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::Entity entity = state.AddEntity<Smasher::Entity>();
	entity.AddComponent<TestComponent>(10);
	EXPECT_THROW({
			entity.AddComponent<TestComponent>(20);
	}, Smasher::Exceptions::EntityDuplicateComponent);
	EXPECT_EQ(entity.GetComponent<TestComponent>().GetValue(), 10);
}

TEST(ComponentsTest, MissingComponent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::Entity entity = state.AddEntity<Smasher::Entity>();
	entity.AddComponent<TestComponent>(10);
	EXPECT_THROW({
			entity.GetComponent<CustomComponent>();
	}, Smasher::Exceptions::EntityComponentNotFound);
}

TEST(ComponentsTest, RemoveComponent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::Entity entity = state.AddEntity<Smasher::Entity>();
	entity.AddComponent<TestComponent>(10);
	ASSERT_TRUE(entity.HasComponent<TestComponent>());
	ASSERT_NO_THROW({
		entity.RemoveComponent<TestComponent>();
	});
	ASSERT_FALSE(entity.HasComponent<TestComponent>());
	ASSERT_THROW({
		entity.RemoveComponent<TestComponent>();
	}, Smasher::Exceptions::EntityComponentNotFound);
}

TEST(ComponentsTest, RemoveComponentDataChange) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	state.Activate();
	Smasher::Entity entity = state.AddEntity<Smasher::Entity>();
	entity.AddComponent<DeleteTestComponent>();
	ASSERT_TRUE(entity.HasComponent<DeleteTestComponent>());
	ASSERT_NO_THROW({ engine.Update(Smasher::Millisecond{10}); });
	ASSERT_FALSE(entity.HasComponent<DeleteTestComponent>());
	ASSERT_THROW({
		entity.RemoveComponent<TestComponent>();
	}, Smasher::Exceptions::EntityComponentNotFound);
}

TEST(ComponentsTest, ExceptionRemoveComponentDataChange) {
	Smasher::Engine engine(640, 420);
	engine.AddState<DummyGameState>().Activate();
	Smasher::Entity entity = engine.GetState<DummyGameState>().AddEntity<Smasher::Entity>();
	entity.AddComponent<SpicyDeleteTestComponent>();
	ASSERT_TRUE(entity.HasComponent<SpicyDeleteTestComponent>());
	ASSERT_NO_THROW({ engine.Update(Smasher::Millisecond{10}); });
	ASSERT_NO_THROW({ engine.Update(Smasher::Millisecond{10}); });
}

void TestCallback(Smasher::DummyEvent *e) {};

TEST(EventsTest, InvalidEventHandle) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();


	Smasher::EventManager& manager = state.GetEventManager();
	Smasher::EventSubscriptionHandle handle = manager.Subscribe<Smasher::DummyEvent>(TestCallback);
	Smasher::EventSubscriptionHandle other_handle = std::move(handle);
	ASSERT_THROW({
		manager.Unsubscribe(std::move(handle));
		}, Smasher::Exceptions::EventHandleInvalid);

	ASSERT_NO_THROW({ manager.Unsubscribe(std::move(other_handle)); });

	ASSERT_THROW({
		manager.Unsubscribe(std::move(other_handle));
		}, Smasher::Exceptions::EventHandleInvalid);
}

TEST(EventsTest, SubscribeEvent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::EventManager& manager = state.GetEventManager();
	Smasher::EventSubscriptionHandle handle = manager.Subscribe<Smasher::DummyEvent>(TestCallback);
}

TEST(EventsTest, SinglePublishEvent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::EventManager& manager = state.GetEventManager();

	int triggered_count = 0;
	std::function<void(Smasher::DummyEvent*)> callback = [&triggered_count](Smasher::DummyEvent* event) {
		++triggered_count;
	};
	Smasher::EventSubscriptionHandle handle = manager.Subscribe<Smasher::DummyEvent>(callback);

	manager.Publish<Smasher::DummyEvent>("Dummy Event 1");
	manager.Dispatch();
	manager.Dispatch();
	ASSERT_EQ(triggered_count, 1);
}

TEST(EventsTest, MultiplePublishEvent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::EventManager& manager = state.GetEventManager();

	int triggered_count = 0;
	std::function<void(Smasher::DummyEvent*)> callback =
	[&triggered_count](Smasher::DummyEvent* event) {
		triggered_count++;
	};
	Smasher::EventSubscriptionHandle handle = manager.Subscribe<Smasher::DummyEvent>(callback);

	manager.Publish<Smasher::DummyEvent>("Dummy Event 1");
	manager.Publish<Smasher::DummyEvent>("Dummy Event 2");
	manager.Publish<Smasher::DummyEvent>("Dummy Event 3");
	manager.Dispatch();
	manager.Dispatch();
	ASSERT_EQ(triggered_count, 3);
}


TEST(EventsTest, SinglePublishUnsubscribeEvent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::EventManager& manager = state.GetEventManager();

	int triggered_count = 0;
	std::function<void(Smasher::DummyEvent*)> callback = [&triggered_count](Smasher::DummyEvent* event) {
		++triggered_count;
		};
	Smasher::EventSubscriptionHandle handle = manager.Subscribe<Smasher::DummyEvent>(callback);

	manager.Publish<Smasher::DummyEvent>("Dummy Event 1");
	manager.Unsubscribe(std::move(handle));
	ASSERT_THROW({
		manager.Unsubscribe(std::move(handle));
	}, Smasher::Exceptions::EventHandleInvalid);
	manager.Dispatch();
	manager.Dispatch();
	ASSERT_EQ(triggered_count, 0);
}


TEST(EventsTest, MultiplePublishMultipleSubscribeEvent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::EventManager& manager = state.GetEventManager();

	int triggered_count_1 = 0;
	std::function<void(Smasher::DummyEvent*)> callback1 =
		[&triggered_count_1](Smasher::DummyEvent* event) {
		triggered_count_1++;
		};

	int triggered_count_2 = 0;
	std::function<void(Smasher::DummyEventExtra*)> callback2 =
		[&triggered_count_2](Smasher::DummyEventExtra* event) {
		triggered_count_2++;
		};

	Smasher::EventSubscriptionHandle handle1 = manager.Subscribe<Smasher::DummyEvent>(callback1);
	Smasher::EventSubscriptionHandle handle2 = manager.Subscribe<Smasher::DummyEventExtra>(callback2);

	manager.Publish<Smasher::DummyEvent>("Dummy Event 1");
	manager.Publish<Smasher::DummyEvent>("Dummy Event 2");
	manager.Publish<Smasher::DummyEventExtra>("Dummy Event Extra 1");
	manager.Publish<Smasher::DummyEvent>("Dummy Event 3");
	manager.Publish<Smasher::DummyEventExtra>("Dummy Event Extra 2");
	manager.Dispatch();
	manager.Dispatch();
	ASSERT_EQ(triggered_count_1, 3);
	ASSERT_EQ(triggered_count_2, 2);
}

TEST(EngineTest, NoShutdownEngine) {
	bool passed = false;
	Smasher::Engine engine(640, 420);

	DummyGameState& state = engine.AddState<DummyGameState>();
	state.Activate();

	engine.GetWindow().setActive(false);
	std::thread worker([&engine]() {
		engine.GetWindow().setActive(true);
		engine.Run(); // Engine should shutdown after first update
	});

	std::this_thread::sleep_for(std::chrono::seconds(3));
	passed = engine.IsRunning();
	engine.Shutdown();
	worker.join();

	if (!passed) {
		FAIL() << "Engine should not have shutdown";
	}
}

TEST(EngineTest, ShutdownEngine) {
	bool failed = false;
	Smasher::Engine engine(640, 420);

	ShutdownEngineGameState& state = engine.AddState<ShutdownEngineGameState>();
	state.Activate();

	engine.GetWindow().setActive(false);
	std::thread worker([&engine]() {
		engine.GetWindow().setActive(true);
		engine.Run(); // Engine should shutdown after first update
	});

	std::this_thread::sleep_for(std::chrono::seconds(3));
	failed = engine.IsRunning();
	engine.Shutdown();
	worker.join();

	if (failed) {
		FAIL() << "Engine should have shutdown";
	}
}

TEST(EngineTest, ExplicitDoubleShutdownEngine) {
	bool failed = false;
	Smasher::Engine engine(640, 420);
	ShutdownEngineGameState& state = engine.AddState<ShutdownEngineGameState>();
	state.Activate();
	engine.GetWindow().setActive(false);
	std::thread worker([&engine]() {
		engine.GetWindow().setActive(true);
		engine.Run(); // Engine should shutdown after first update
	});

	std::this_thread::sleep_for(std::chrono::seconds(3));
	failed = engine.IsRunning();
	EXPECT_NO_THROW({ engine.Shutdown(); });
	EXPECT_NO_THROW({ engine.Shutdown(); }); // Explicit second shutdown
	worker.join();

	if (failed) {
		FAIL() << "Engine should have shutdown";
	}
}