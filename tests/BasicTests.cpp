#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include "Core.h"
#include "BaseComponentManager.h"
#include "Entity.h"
#include "ResourceManager.h"
#include "Resources.h"

class DummyGameState : public Smasher::GameState {
public:
	DummyGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {}
};

class InitTestGameState : public Smasher::GameState {
public:
	InitTestGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {}

	void Init() override {
		++m_Value;
	};

	int GetValue() const { return m_Value; }
private:
	int m_Value = 0;
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
	TestComponent(int value) : m_Value(value) {};
	int GetValue() { return m_Value; }
	static void StaticUpdateComponent(TestComponent& self, Smasher::Millisecond delta) {};

protected:
	int m_Value;
};

struct CustomComponentManager;

struct CustomComponent : public Smasher::IComponent {
	SMASHER_USE_COMPONENT_MANAGER(CustomComponentManager)
public:
		CustomComponent(int value) : m_Value(value) {
	};
	~CustomComponent() = default;

	int GetValue() { return m_Value; }

protected:
	static void StaticUpdateComponent(CustomComponent& self, Smasher::Millisecond delta) {};
	int m_Value;
};


class CustomComponentManager : public Smasher::BaseComponentManager<CustomComponent> {
public:
	CustomComponentManager(Smasher::GameState& state) : Smasher::BaseComponentManager<CustomComponent>(state) {}
	CustomComponentManager(const CustomComponentManager&) = default;
	~CustomComponentManager() = default;

	void Update(Smasher::Millisecond delta) override {};
	void Render(sf::RenderWindow& rWindow) override {};
};

// Component that deletes itself during update
struct DeleteTestComponent : public Smasher::IComponent {
public:
	static void StaticUpdateComponent(DeleteTestComponent& self, Smasher::Millisecond delta) {
		self.GetEntity().RemoveComponent<DeleteTestComponent>();
	};
};

// Component that deletes itself during update
struct SpicyDeleteTestComponent : public Smasher::IComponent {
public:
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

class InitEntityTest : public Smasher::Entity {
public:
	InitEntityTest() = delete;
	InitEntityTest(Smasher::GameState& state, Smasher::UUID uuid) : Smasher::Entity(state, uuid) {}

	void Init() {
		++m_Value;
	}

	int GetValue() const{ return m_Value; }
private:
	int m_Value = 0;
};

class InitRemoveEntityTest : public Smasher::Entity {
public:
	InitRemoveEntityTest() = delete;
	InitRemoveEntityTest(Smasher::GameState& state, Smasher::UUID uuid) : Smasher::Entity(state, uuid) {}

	void Init() {
		++m_Value;
		GetGameState().RemoveEntity(GetUUID());
	}

	int GetValue() const { return m_Value; }
private:
	int m_Value = 0;
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

TEST(EntityTest, InitEntity) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	InitEntityTest& entity = state.AddEntity<InitEntityTest>();
	EXPECT_EQ(1, entity.GetValue());
}

TEST(EntityTest, InitRemoveEntity) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	InitRemoveEntityTest& entity = state.AddEntity<InitRemoveEntityTest>();
	EXPECT_FALSE(state.HasEntity(entity.GetUUID()));
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

TEST(ResourcesTest, OpenFileResource) {
	Smasher::Engine engine(640, 420);
	Smasher::ResourceManager& rResourceManager = engine.GetResourceManager();
	std::ios_base::openmode flags = std::ios_base::out;
	Smasher::ResourceID fileResourceID{ 1 };
	auto pFileResource = rResourceManager.GetOrLoadResource<Smasher::FileResource>(fileResourceID, Smasher::ResourcePath{ "test_file" }, flags);

	ASSERT_NO_THROW({ rResourceManager.GetResource<Smasher::FileResource>(fileResourceID); });
	ASSERT_TRUE(pFileResource->GetFileStream().is_open());
	pFileResource->GetFileStream() << "message";
	ASSERT_TRUE(pFileResource->GetFileStream().is_open());
	rResourceManager.ReleaseResource(fileResourceID);
	pFileResource.reset();
	ASSERT_THROW({ rResourceManager.GetResource<Smasher::FileResource>(fileResourceID); }, Smasher::Exceptions::ResourceNotLoaded);
	ASSERT_EQ(nullptr, pFileResource.get());

	std::string line;
	std::ifstream file("test_file", std::ios_base::in);
	ASSERT_TRUE(file.is_open());
	std::getline(file, line);
	ASSERT_STREQ("message", line.c_str());
}

TEST(ResourcesTest, OpenReleaseFileResource) {
	Smasher::Engine engine(640, 420);
	Smasher::ResourceManager& rResourceManager = engine.GetResourceManager();
	std::ios_base::openmode flags = std::ios_base::out;
	Smasher::ResourcePath paths[] = { Smasher::ResourcePath{"test_file"} };
	auto pFileResource = rResourceManager.GetOrLoadResource<Smasher::FileResource>(Smasher::ResourceID{ 1 }, paths, size_t{ 1 }, flags);
	pFileResource->GetFileStream() << "message";
}


void TestCallback(const Smasher::Events::DummyEvent& e) {};

TEST(EventsTest, InvalidEventHandle) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();


	Smasher::EventManager& manager = state.GetEventManager();
	Smasher::EventSubscriptionHandle handle = manager.Subscribe<Smasher::Events::DummyEvent>(TestCallback);
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
	Smasher::EventSubscriptionHandle handle = manager.Subscribe<Smasher::Events::DummyEvent>(TestCallback);
}

TEST(EventsTest, SinglePublishEvent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::EventManager& manager = state.GetEventManager();

	int triggered_count = 0;
	std::function<void(const Smasher::Events::DummyEvent&)> callback = [&triggered_count](const Smasher::Events::DummyEvent& event) {
		++triggered_count;
	};
	Smasher::EventSubscriptionHandle handle = manager.Subscribe<Smasher::Events::DummyEvent>(callback);

	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	manager.Dispatch();
	manager.Dispatch();
	ASSERT_EQ(1, triggered_count);
}

TEST(EventsTest, MultiplePublishEvent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::EventManager& manager = state.GetEventManager();

	int triggered_count = 0;
	std::function<void(const Smasher::Events::DummyEvent&)> callback =
	[&triggered_count](const Smasher::Events::DummyEvent& event) {
		triggered_count++;
	};
	Smasher::EventSubscriptionHandle handle = manager.Subscribe<Smasher::Events::DummyEvent>(callback);

	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 2");
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 3");
	manager.Dispatch();
	manager.Dispatch();
	ASSERT_EQ(3, triggered_count);
}


TEST(EventsTest, SinglePublishUnsubscribeEvent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::EventManager& manager = state.GetEventManager();

	int triggered_count = 0;
	std::function<void(const Smasher::Events::DummyEvent&)> callback = [&triggered_count](const Smasher::Events::DummyEvent& event) {
		++triggered_count;
		};
	Smasher::EventSubscriptionHandle handle = manager.Subscribe<Smasher::Events::DummyEvent>(callback);

	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	manager.Unsubscribe(std::move(handle));
	ASSERT_THROW({
		manager.Unsubscribe(std::move(handle));
	}, Smasher::Exceptions::EventHandleInvalid);
	manager.Dispatch();
	manager.Dispatch();
	ASSERT_EQ(0, triggered_count);
}


TEST(EventsTest, MultiplePublishMultipleSubscribeEvent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::EventManager& manager = state.GetEventManager();

	int triggered_count_1 = 0;
	std::function<void(const Smasher::Events::DummyEvent&)> callback1 =
		[&triggered_count_1](const Smasher::Events::DummyEvent& event) {
		triggered_count_1++;
		};

	int triggered_count_2 = 0;
	std::function<void(const Smasher::Events::DummyEventExtra&)> callback2 =
		[&triggered_count_2](const Smasher::Events::DummyEventExtra& event) {
		triggered_count_2++;
		};

	Smasher::EventSubscriptionHandle handle1 = manager.Subscribe<Smasher::Events::DummyEvent>(callback1);
	Smasher::EventSubscriptionHandle handle2 = manager.Subscribe<Smasher::Events::DummyEventExtra>(callback2);

	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 2");
	manager.Publish<Smasher::Events::DummyEventExtra>("Dummy Event Extra 1");
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 3");
	manager.Publish<Smasher::Events::DummyEventExtra>("Dummy Event Extra 2");
	manager.Dispatch();
	manager.Dispatch();
	ASSERT_EQ(3, triggered_count_1);
	ASSERT_EQ(2, triggered_count_2);
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

TEST(GameStateTest, InitGameState) {
	Smasher::Engine engine(640, 420);
	InitTestGameState& state = engine.AddState<InitTestGameState>();
	EXPECT_EQ(1, state.GetValue());
	state.Activate();
	engine.Update(Smasher::Millisecond{ 10 });
	engine.Update(Smasher::Millisecond{ 10 });
	engine.Render(engine.GetWindow());
	EXPECT_EQ(1, state.GetValue());
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