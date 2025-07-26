#include <gtest/gtest.h>
//#include "Engine.h"
//#include "Entity.h"
//#include "ComponentManager.h"
//#include "GenericComponentManager.h"
#include "Core.h"


class DummyGameState : public Smasher::GameState {
public:
	DummyGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {}
	void Reset() {};
	void Render(sf::RenderWindow& window) override {};
};


struct TestComponent : public Smasher::Component {
public:
	TestComponent(Smasher::Entity& entity, int value) :
		Smasher::Component(entity),
		m_Value(value) {};
	TestComponent& operator=(TestComponent&&) = default; // So that parent move assignment is called
	int GetValue() { return m_Value; }

protected:
	static void StaticUpdateComponent(Smasher::Millisecond delta) {};
	int m_Value;
};

TEST(EntityTest, AddEnttiy) {
	Smasher::Engine engine(640, 420);
	DummyGameState state(engine);
	EXPECT_NO_THROW({ Smasher::Entity & entity = state.AddEntity<Smasher::Entity>(); });
}

TEST(EntityTest, GetEntity) {
	Smasher::Engine engine(640, 420);
	DummyGameState state(engine);
	Smasher::Entity& entity = state.AddEntity<Smasher::Entity>();
	EXPECT_NO_THROW({ state.GetEntity(entity.GetUUID()); });
}

TEST(EntityTest, MissingEntity) {
	Smasher::Engine engine(640, 420);
	DummyGameState state(engine);
	EXPECT_THROW({ state.GetEntity(Smasher::UUID{10}); }, Smasher::Exceptions::GameStateEntityNotFound);
	Smasher::Entity& entity = state.AddEntity<Smasher::Entity>();
	EXPECT_THROW({ state.GetEntity(Smasher::UUID{entity.GetUUID() + 1});}, Smasher::Exceptions::GameStateEntityNotFound);
}

TEST(ComponentsTest, CreateComponent) {
	Smasher::Engine engine(640, 420);
	DummyGameState state(engine);
	TestComponent& test = state.AddEntity<Smasher::Entity>().AddComponent<TestComponent>(10);
	EXPECT_EQ(10, test.GetValue());
}

TEST(ComponentsTest, DuplicateComponent) {
	Smasher::Engine engine(640, 420);
	DummyGameState state(engine);
	Smasher::Entity entity = state.AddEntity<Smasher::Entity>();
	entity.AddComponent<TestComponent>(10);
	EXPECT_THROW({
			entity.AddComponent<TestComponent>(20);
	}, Smasher::Exceptions::EntityDuplicateComponent);
	EXPECT_EQ(entity.GetComponent<TestComponent>().GetValue(), 10);
}

TEST(ComponentsTest, MissingComponent) {
	Smasher::Engine engine(640, 420);
	DummyGameState state(engine);
	Smasher::Entity entity = state.AddEntity<Smasher::Entity>();
	EXPECT_THROW({
			entity.GetComponent<TestComponent>();
		}, Smasher::Exceptions::EntityComponentNotFound);
}

TEST(ComponentsTest, RemoveComponent) {
	Smasher::Engine engine(640, 420);
	DummyGameState state(engine);
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

void TestCallback(Smasher::DummyEvent *e) {};

TEST(EventsTest, InvalidEventHandle) {
	Smasher::Engine engine(640, 420);
	DummyGameState state(engine);


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
	DummyGameState state(engine);
	Smasher::EventManager& manager = state.GetEventManager();
	Smasher::EventSubscriptionHandle handle = manager.Subscribe<Smasher::DummyEvent>(TestCallback);
}

TEST(EventsTest, SinglePublishEvent) {
	Smasher::Engine engine(640, 420);
	DummyGameState state(engine);
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
	DummyGameState state(engine);
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
	DummyGameState state(engine);
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
	DummyGameState state(engine);
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

TEST(EngineTest, InitializeEngine) {
	Smasher::Exceptions::GameStateDuplicateUUID e;
	EXPECT_THROW({
		Smasher::Engine engine(640, 420);
		DummyGameState state(engine);
		engine.AddState(1, state);
		engine.Run();
		engine.Shutdown();
		}, Smasher::Exceptions::GameStateDuplicateUUID);
}