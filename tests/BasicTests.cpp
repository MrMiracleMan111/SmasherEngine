#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include <type_traits>
#include "Core.h"
#include "BaseComponentManager.h"
#include "Entity.h"
#include "ResourceManager.h"
#include "Resources.h"
#include "Layer.h"

class DummyLayer : public Smasher::Layer {
public:
	DummyLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {}
};

class InitTestLayer : public Smasher::Layer {
public:
	InitTestLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {}

	void Init() override {
		++m_Value;
	};

	int GetValue() const { return m_Value; }
private:
	int m_Value = 0;
};

class ShutdownEngineLayer : public Smasher::Layer {
public:
	ShutdownEngineLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {}
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

class CustomComponentManager;

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
	CustomComponentManager(Smasher::Layer& layer) : Smasher::BaseComponentManager<CustomComponent>(layer) {}
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
	InitEntityTest(Smasher::Layer& layer, Smasher::UUID uuid) : Smasher::Entity(layer, uuid) {}

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
	InitRemoveEntityTest(Smasher::Layer& layer, Smasher::UUID uuid) : Smasher::Entity(layer, uuid) {}

	void Init() {
		++m_Value;
		GetLayer().RemoveEntity(GetUUID());
	}

	int GetValue() const { return m_Value; }
private:
	int m_Value = 0;
};

TEST(TraitChecks, EngineTraits) {
	// Engine should generally be non-copyable, moveable
	static_assert(!std::is_copy_constructible_v<Smasher::Engine>);
	static_assert(!std::is_copy_assignable_v<Smasher::Engine>);
	static_assert(std::is_move_constructible_v<Smasher::Engine>);
	static_assert(std::is_move_assignable_v<Smasher::Engine>);
	EXPECT_TRUE(true);
}

TEST(TraitChecks, ManagerTraits) {
	// Managers should generally be non-copyable, moveable
	static_assert(!std::is_copy_constructible_v<Smasher::EventManager>);
	static_assert(!std::is_copy_assignable_v<Smasher::EventManager>);
	static_assert(std::is_move_constructible_v<Smasher::EventManager>);
	static_assert(std::is_move_assignable_v<Smasher::EventManager>);

	// ResourceManager is non-copyable, moveable
	static_assert(!std::is_copy_constructible_v<Smasher::ResourceManager>);
	static_assert(!std::is_copy_assignable_v<Smasher::ResourceManager>);
	static_assert(std::is_move_constructible_v<Smasher::ResourceManager>);
	static_assert(std::is_move_assignable_v<Smasher::ResourceManager>);
	EXPECT_TRUE(true);
}


TEST(TraitChecks, UUID) {
	// UUID should be copyable, moveable
	static_assert(std::is_copy_constructible_v<Smasher::UUID>);
	static_assert(std::is_copy_assignable_v<Smasher::UUID>);
	static_assert(std::is_move_constructible_v<Smasher::UUID>);
	static_assert(std::is_move_assignable_v<Smasher::UUID>);
	EXPECT_TRUE(true);
}

TEST(TraitChecks, EventSubscription) {
	// EventSubscriptionHandle should be non-copyable, non-move-assignable, move-constructible
	static_assert(!std::is_copy_constructible_v<Smasher::EventSubscription>);
	static_assert(!std::is_copy_assignable_v<Smasher::EventSubscription>);
	static_assert(std::is_move_constructible_v<Smasher::EventSubscription>);
	static_assert(!std::is_move_assignable_v<Smasher::EventSubscription>);

	// EventSubscriptionHandle should be non-copyable, move-assignable, move-constructible
	static_assert(!std::is_copy_constructible_v<Smasher::EventSubscriptionHandle>);
	static_assert(!std::is_copy_assignable_v<Smasher::EventSubscriptionHandle>);
	static_assert(std::is_move_constructible_v<Smasher::EventSubscriptionHandle>);
	static_assert(std::is_move_assignable_v<Smasher::EventSubscriptionHandle>);
	EXPECT_TRUE(true);
}

TEST(EntityTest, AddEnttiy) {
	Smasher::Engine engine(640, 420);
	Smasher::UUID entityUUID{ 0 };
	DummyLayer& layer = engine.PushLayer<DummyLayer>();

	EXPECT_FALSE(layer.HasEntity(entityUUID));

	EXPECT_NO_THROW({
		Smasher::Entity & entity = layer.AddEntity<Smasher::Entity>();
		entityUUID = entity.GetUUID();
	});

	EXPECT_TRUE(layer.HasEntity(entityUUID));
}

TEST(EntityTest, GetEntity) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::Entity& entity = layer.AddEntity<Smasher::Entity>();
	EXPECT_NO_THROW({ layer.GetEntity(entity.GetUUID()); });
}

TEST(EntityTest, InitEntity) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	InitEntityTest& entity = layer.AddEntity<InitEntityTest>();
	EXPECT_EQ(1, entity.GetValue());
}

TEST(EntityTest, InitRemoveEntity) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	InitRemoveEntityTest& entity = layer.AddEntity<InitRemoveEntityTest>();
	EXPECT_FALSE(layer.HasEntity(entity.GetUUID()));
}

TEST(EntityTest, MissingEntity) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	EXPECT_THROW({ layer.GetEntity(Smasher::UUID{10}); }, Smasher::Exceptions::LayerEntityNotFound);
	Smasher::Entity& entity = layer.AddEntity<Smasher::Entity>();
	EXPECT_THROW({ layer.GetEntity(Smasher::UUID{entity.GetUUID() + 1});}, Smasher::Exceptions::LayerEntityNotFound);
}

TEST(EntityTest, MoveEntity) {
	Smasher::Engine engine(640, 420);
	Smasher::BaseLayer& baseLayer = engine.GetLayer<Smasher::BaseLayer>();
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	InitEntityTest& entity = layer.AddEntity<InitEntityTest>();
	EXPECT_EQ(1, entity.GetValue());

	EXPECT_FALSE(baseLayer.HasEntity(entity.GetUUID()));
	EXPECT_TRUE(layer.HasEntity(entity.GetUUID()));

	layer.MoveEntity(entity);

	EXPECT_FALSE(baseLayer.HasEntity(entity.GetUUID()));
	EXPECT_TRUE(layer.HasEntity(entity.GetUUID()));

	baseLayer.MoveEntity(entity);

	EXPECT_TRUE(baseLayer.HasEntity(entity.GetUUID()));
	EXPECT_FALSE(layer.HasEntity(entity.GetUUID()));

	baseLayer.MoveEntity(entity);

	EXPECT_TRUE(baseLayer.HasEntity(entity.GetUUID()));
	EXPECT_FALSE(layer.HasEntity(entity.GetUUID()));
}


TEST(ComponentsTest, CreateComponent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	TestComponent& test = layer.AddEntity<Smasher::Entity>().AddComponent<TestComponent>(10);
	EXPECT_EQ(10, test.GetValue());
}

TEST(ComponentsTest, DuplicateComponent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::Entity entity = layer.AddEntity<Smasher::Entity>();
	entity.AddComponent<TestComponent>(10);
	EXPECT_THROW({
			entity.AddComponent<TestComponent>(20);
	}, Smasher::Exceptions::EntityDuplicateComponent);
	EXPECT_EQ(entity.GetComponent<TestComponent>().GetValue(), 10);
}

TEST(ComponentsTest, MissingComponent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::Entity entity = layer.AddEntity<Smasher::Entity>();
	entity.AddComponent<TestComponent>(10);
	EXPECT_THROW({
			entity.GetComponent<CustomComponent>();
	}, Smasher::Exceptions::EntityComponentNotFound);
}

TEST(ComponentsTest, RemoveComponent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::Entity entity = layer.AddEntity<Smasher::Entity>();
	entity.AddComponent<TestComponent>(10);
	EXPECT_TRUE(entity.HasComponent<TestComponent>());
	EXPECT_NO_THROW({
		entity.RemoveComponent<TestComponent>();
	});
	EXPECT_FALSE(entity.HasComponent<TestComponent>());
	EXPECT_THROW({
		entity.RemoveComponent<TestComponent>();
	}, Smasher::Exceptions::EntityComponentNotFound);
}

TEST(ComponentsTest, RemoveComponentDataChange) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	layer.Activate();
	Smasher::Entity entity = layer.AddEntity<Smasher::Entity>();
	entity.AddComponent<DeleteTestComponent>();
	EXPECT_TRUE(entity.HasComponent<DeleteTestComponent>());
	EXPECT_NO_THROW({ engine.Update(Smasher::Millisecond{10}); });
	EXPECT_FALSE(entity.HasComponent<DeleteTestComponent>());
	EXPECT_THROW({
		entity.RemoveComponent<TestComponent>();
	}, Smasher::Exceptions::EntityComponentNotFound);
}

TEST(ComponentsTest, ExceptionRemoveComponentDataChange) {
	Smasher::Engine engine(640, 420);
	engine.PushLayer<DummyLayer>().Activate();
	Smasher::Entity entity = engine.GetLayer<DummyLayer>().AddEntity<Smasher::Entity>();
	entity.AddComponent<SpicyDeleteTestComponent>();
	EXPECT_TRUE(entity.HasComponent<SpicyDeleteTestComponent>());
	EXPECT_NO_THROW({ engine.Update(Smasher::Millisecond{10}); });
	EXPECT_NO_THROW({ engine.Update(Smasher::Millisecond{10}); });
}

TEST(ResourcesTest, OpenFileResource) {
	Smasher::Engine engine(640, 420);
	Smasher::ResourceManager& rResourceManager = engine.GetResourceManager();
	std::ios_base::openmode flags = std::ios_base::out;
	Smasher::ResourceID fileResourceID{ 1 };
	auto pFileResource = rResourceManager.GetOrLoadResource<Smasher::FileResource>(fileResourceID, Smasher::ResourcePath{ "test_file" }, flags);

	EXPECT_NO_THROW({ rResourceManager.GetResource<Smasher::FileResource>(fileResourceID); });
	EXPECT_TRUE(pFileResource->GetFileStream().is_open());
	pFileResource->GetFileStream() << "message";
	EXPECT_TRUE(pFileResource->GetFileStream().is_open());
	rResourceManager.ReleaseResource(fileResourceID);
	pFileResource.reset();
	EXPECT_THROW({ rResourceManager.GetResource<Smasher::FileResource>(fileResourceID); }, Smasher::Exceptions::ResourceNotLoaded);
	EXPECT_EQ(nullptr, pFileResource.get());

	std::string line;
	std::ifstream file("test_file", std::ios_base::in);
	EXPECT_TRUE(file.is_open());
	std::getline(file, line);
	EXPECT_STREQ("message", line.c_str());
}

TEST(ResourcesTest, OpenReleaseFileResource) {
	Smasher::Engine engine(640, 420);
	Smasher::ResourceManager& rResourceManager = engine.GetResourceManager();
	std::ios_base::openmode flags = std::ios_base::out;
	Smasher::ResourcePath paths[] = { Smasher::ResourcePath{"test_file"} };
	auto pFileResource = rResourceManager.GetOrLoadResource<Smasher::FileResource>(Smasher::ResourceID{ 1 }, paths, size_t{ 1 }, flags);
	pFileResource->GetFileStream() << "message";
}


void TestCallback(Smasher::Events::DummyEvent& e) {};


TEST(EventsTest, InvalidEventHandle) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();


	Smasher::EventManager& manager = layer.GetEventManager();
	Smasher::EventSubscriptionHandle handle = layer.Subscribe<Smasher::Events::DummyEvent>(TestCallback);
	Smasher::EventSubscriptionHandle other_handle = std::move(handle);
	EXPECT_THROW({
		handle.Unsubscribe();
		}, Smasher::Exceptions::EventHandleInvalid);

	EXPECT_NO_THROW({ other_handle.Unsubscribe(); });

	EXPECT_THROW({
		other_handle.Unsubscribe();
		}, Smasher::Exceptions::EventHandleInvalid);
}

TEST(EventsTest, SubscribeEvent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::EventSubscriptionHandle handle = layer.Subscribe<Smasher::Events::DummyEvent>(TestCallback);
}

TEST(EventsTest, SinglePublishEvent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::EventManager& manager = layer.GetEventManager();

	int triggered_count = 0;
	std::function<void(Smasher::Events::DummyEvent&)> callback = [&triggered_count](Smasher::Events::DummyEvent& event) {
		++triggered_count;
	};
	Smasher::EventSubscriptionHandle handle = layer.Subscribe<Smasher::Events::DummyEvent>(callback);

	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	manager.Dispatch();
	manager.Dispatch();
	EXPECT_EQ(1, triggered_count);
}

TEST(EventsTest, MultiplePublishEvent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::EventManager& manager = layer.GetEventManager();

	int triggered_count = 0;
	std::function<void(Smasher::Events::DummyEvent&)> callback =
	[&triggered_count](Smasher::Events::DummyEvent& event) {
		triggered_count++;
	};
	Smasher::EventSubscriptionHandle handle = layer.Subscribe<Smasher::Events::DummyEvent>(callback);

	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 2");
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 3");
	manager.Dispatch();
	manager.Dispatch();
	EXPECT_EQ(3, triggered_count);
}


TEST(EventsTest, StopPropagateEvent) {
	Smasher::Engine engine(640, 420);
	Smasher::BaseLayer& baseLayer = engine.GetLayer<Smasher::BaseLayer>();
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::EventManager& manager = layer.GetEventManager();

	int triggered_count = 0;
	std::function<void(Smasher::Events::DummyEvent&)> callback1 = [&triggered_count](Smasher::Events::DummyEvent& event) {
		++triggered_count;
	};
	Smasher::EventSubscriptionHandle handle1 = layer.Subscribe<Smasher::Events::DummyEvent>(callback1);
	Smasher::EventSubscriptionHandle handle2 = baseLayer.Subscribe<Smasher::Events::DummyEvent>(callback1);

	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	manager.Dispatch();
	manager.Dispatch();
	handle1.Unsubscribe();
	handle2.Unsubscribe();
	EXPECT_EQ(2, triggered_count);

	triggered_count = 0;

	std::function<void(Smasher::Events::DummyEvent&)> callback2 = [&triggered_count](Smasher::Events::DummyEvent& event) {
		++triggered_count;
		event.StopPropagate();
	};
	Smasher::EventSubscriptionHandle handle3 = layer.Subscribe<Smasher::Events::DummyEvent>(callback2);
	Smasher::EventSubscriptionHandle handle4 = baseLayer.Subscribe<Smasher::Events::DummyEvent>(callback2);
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	manager.Dispatch();
	manager.Dispatch();
	EXPECT_EQ(1, triggered_count);
}

TEST(EventsTest, SubscriptionLifetimeTest) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::EventManager& manager = layer.GetEventManager();

	int triggered_count = 0;
	std::function<void(Smasher::Events::DummyEvent&)> callback =
		[&triggered_count](Smasher::Events::DummyEvent& event) {
		triggered_count++;
		};

	{
		// "handle" will be unsubscribed once it goes out of scope
		Smasher::EventSubscriptionHandle handle = layer.Subscribe<Smasher::Events::DummyEvent>(callback);
		manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
		manager.Dispatch();
		manager.Dispatch();
	}

	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 2");
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 3");
	manager.Dispatch();
	manager.Dispatch();
	EXPECT_EQ(1, triggered_count);
}

TEST(EventsTest, SubscriptionLifetimeHandoffTest) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::EventManager& manager = layer.GetEventManager();

	int triggered_count = 0;
	std::function<void(Smasher::Events::DummyEvent&)> callback =
		[&triggered_count](Smasher::Events::DummyEvent& event) {
			triggered_count++;
		};

	{
		// "tmp" will be unsubscribed once it goes out of scope
		Smasher::EventSubscriptionHandle tmp;
		manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
		manager.Dispatch();
		manager.Dispatch();

		{
			// "handle" will be unsubscribed once it goes out of scope
			Smasher::EventSubscriptionHandle handle = layer.Subscribe<Smasher::Events::DummyEvent>(callback);
			manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
			manager.Dispatch();
			manager.Dispatch();
			tmp = std::move(handle); // handoff
		}

		manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
		manager.Dispatch();
		manager.Dispatch();
		manager.Dispatch();
	}

	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 2");
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 3");
	manager.Dispatch();
	manager.Dispatch();
	EXPECT_EQ(2, triggered_count);
}


TEST(EventsTest, SinglePublishUnsubscribeEvent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::EventManager& manager = layer.GetEventManager();

	int triggered_count = 0;
	std::function<void(Smasher::Events::DummyEvent&)> callback = [&triggered_count](Smasher::Events::DummyEvent& event) {
		++triggered_count;
		};
	Smasher::EventSubscriptionHandle handle = layer.Subscribe<Smasher::Events::DummyEvent>(callback);

	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	handle.Unsubscribe();
	EXPECT_THROW({
		handle.Unsubscribe();
	}, Smasher::Exceptions::EventHandleInvalid);
	manager.Dispatch();
	manager.Dispatch();
	EXPECT_EQ(0, triggered_count);
}


TEST(EventsTest, MultiplePublishMultipleSubscribeEvent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::EventManager& manager = layer.GetEventManager();

	int triggered_count_1 = 0;
	std::function<void(Smasher::Events::DummyEvent&)> callback1 =
		[&triggered_count_1](Smasher::Events::DummyEvent& event) {
		triggered_count_1++;
		};

	int triggered_count_2 = 0;
	std::function<void(Smasher::Events::DummyEventExtra&)> callback2 =
		[&triggered_count_2](Smasher::Events::DummyEventExtra& event) {
		triggered_count_2++;
		};

	Smasher::EventSubscriptionHandle handle1 = layer.Subscribe<Smasher::Events::DummyEvent>(callback1);
	Smasher::EventSubscriptionHandle handle2 = layer.Subscribe<Smasher::Events::DummyEventExtra>(callback2);

	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 2");
	manager.Publish<Smasher::Events::DummyEventExtra>("Dummy Event Extra 1");
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 3");
	manager.Publish<Smasher::Events::DummyEventExtra>("Dummy Event Extra 2");
	manager.Dispatch();
	manager.Dispatch();
	EXPECT_EQ(3, triggered_count_1);
	EXPECT_EQ(2, triggered_count_2);
}

// Move with synchronous events in queue
TEST(EventsTest, MoveEventManagerSync) {
	try {
		Smasher::Engine engine(640, 420);
		DummyLayer& layer = engine.PushLayer<DummyLayer>();
		Smasher::EventManager& manager = layer.GetEventManager();

		int count = 10;
		auto incrementFunc = [&count](Smasher::Events::DummyEvent&) {
			count += 5;
		};

		Smasher::EventSubscriptionHandle handle = layer.Subscribe<Smasher::Events::DummyEvent>(incrementFunc);
		manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");

		// Should wait for all async events to finish before moving
		EXPECT_EQ(10, count);
		Smasher::EventManager tmp = std::move(manager);
		tmp.Dispatch();
		EXPECT_EQ(15, count);
		handle.Unsubscribe(); // tmp would be deconstructed before "handle"
	}
	catch (...) {
		FAIL(); // No Throw is allowed
	}
}

// Move with async events in queue
TEST(EventsTest, MoveEventManagerAsync) {
	try {
		Smasher::Engine engine(640, 420);
		DummyLayer& layer = engine.PushLayer<DummyLayer>();
		Smasher::EventManager& manager = layer.GetEventManager();

		int count = 10;
		auto incrementFunc = [&count](Smasher::Events::DummyEvent&) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			count += 5;
		};

		Smasher::EventSubscriptionHandle handle = layer.SubscribeAsync<Smasher::Events::DummyEvent>(incrementFunc);
		manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");

		// Should wait for all async events to finish before moving
		EXPECT_EQ(10, count);
		Smasher::EventManager tmp = std::move(manager);
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		EXPECT_EQ(15, count);
		handle.Unsubscribe(); // tmp would be deconstructed before "handle"
	}
	catch (...) {
		FAIL(); // No Throw is allowed
	}
}

TEST(AsyncEventTest, SubscribeEvent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::EventManager& manager = layer.GetEventManager();
	Smasher::EventSubscriptionHandle handle = layer.SubscribeAsync<Smasher::Events::DummyEvent>(TestCallback);
}

TEST(AsyncEventTest, PublishEvent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::EventManager& manager = layer.GetEventManager();
	int count = 10;
	auto incrementFunc = [&count] (Smasher::Events::DummyEvent&) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		count += 5;
	};

	Smasher::EventSubscriptionHandle handle = layer.SubscribeAsync<Smasher::Events::DummyEvent>(incrementFunc);
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	EXPECT_EQ(10, count);
	std::this_thread::sleep_for(std::chrono::seconds(2));
	EXPECT_EQ(15, count);
}

TEST(AsyncEventTest, MultiplePublishEvent) {
	Smasher::Engine engine(640, 420);
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::EventManager& manager = layer.GetEventManager();
	int count = 10;
	auto incrementFunc = [&count](Smasher::Events::DummyEvent&) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		count += 5;
	};

	Smasher::EventSubscriptionHandle handle = layer.SubscribeAsync<Smasher::Events::DummyEvent>(incrementFunc);
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	manager.Publish<Smasher::Events::DummyEvent>("Dummy Event 1");
	EXPECT_EQ(10, count);
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	EXPECT_EQ(15, count);
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	EXPECT_EQ(20, count);
}

TEST(EngineTest, MoveEngine) {
	try {
		Smasher::Engine engine(640, 420);
		Smasher::Engine tmp = std::move(engine);
	}
	catch (...) {
		FAIL(); // No throws should occur
	}
}

TEST(EngineTest, HeadlessEngine) {
	try {
		bool failed = false;
		Smasher::Engine engine = Smasher::Engine::CreateHeadless();

		ShutdownEngineLayer& layer = engine.PushLayer<ShutdownEngineLayer>();
		layer.Activate();

		std::thread worker([&engine]() {
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
	catch (...) {
		FAIL();
	}
}

TEST(EngineTest, MoveHeadlessEngine) {
	try {
		bool failed = false;
		Smasher::Engine engine = Smasher::Engine::CreateHeadless();
		Smasher::Engine tmp = std::move(engine);

		ShutdownEngineLayer& layer = tmp.PushLayer<ShutdownEngineLayer>();
		layer.Activate();

		std::thread worker([&tmp]() {
			tmp.Run(); // Engine should shutdown after first update
			});

		std::this_thread::sleep_for(std::chrono::seconds(3));
		failed = tmp.IsRunning();
		tmp.Shutdown();
		worker.join();

		if (failed) {
			FAIL() << "Engine should have shutdown";
		}
	}
	catch (...) {
		FAIL();
	}
}

TEST(EngineTest, NoShutdownEngine) {
	bool passed = false;
	Smasher::Engine engine(640, 420);

	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	layer.Activate();

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

	ShutdownEngineLayer& layer = engine.PushLayer<ShutdownEngineLayer>();
	layer.Activate();

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

TEST(LayerTest, InitLayer) {
	Smasher::Engine engine(640, 420);
	InitTestLayer& layer = engine.PushLayer<InitTestLayer>();
	EXPECT_EQ(1, layer.GetValue());
	layer.Activate();
	engine.Update(Smasher::Millisecond{ 10 });
	engine.Update(Smasher::Millisecond{ 10 });
	engine.Render(engine.GetWindow());
	EXPECT_EQ(1, layer.GetValue());
}

TEST(EngineTest, ExplicitDoubleShutdownEngine) {
	bool failed = false;
	Smasher::Engine engine(640, 420);
	ShutdownEngineLayer& layer = engine.PushLayer<ShutdownEngineLayer>();
	layer.Activate();
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