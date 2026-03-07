#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include <type_traits>
#include <array>
#include "Smasher/Core.h"
#include "Smasher/ComponentSystems/DrawableSystem.h"
#include "Smasher/ComponentSystems/FrameTimeSystem.h"

class DummyLayer : public Smasher::Layer {
public:
	DummyLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {}
};

namespace CursorInteractSystem {
	struct Context {
		// Mouse velocity
		entt::entity selectedBall;
	};

	Smasher::ErrorCode Update(entt::registry& registry) {
		if (!registry.ctx().contains<Context>()) {
			return ERROR_SystemNotInitialized;
		}

		Context& ctx = registry.ctx().get<Context>();
		// Make selected ball fly toward cursor
		return ERROR_NoError;
	};

	Smasher::ErrorCode Initialize(entt::registry& registry) {
		registry.ctx().emplace<Context>();
		return ERROR_NoError;
	}

	Smasher::ErrorCode Teardown(entt::registry& registry) {
		return ERROR_NoError;
	}

	Smasher::ErrorCode OnClick(entt::registry& registry) {
		if (!registry.ctx().contains<Context>()) {
			return ERROR_SystemNotInitialized;
		}

		return ERROR_NoError;
	}
}

namespace CollisionAvoidSystem {
	struct Context {
		// Quad tree
	};

	Smasher::ErrorCode Update(entt::registry& registry) {
		if (!registry.ctx().contains<Context>()) {
			return ERROR_SystemNotInitialized;
		}
		Context& collisionAvoidCtx = registry.ctx().get<Context>();
		CursorInteractSystem::Context& cursorMagnetCtx = registry.ctx().emplace<CursorInteractSystem::Context>();

		return ERROR_NoError;
	};

	Smasher::ErrorCode Initialize(entt::registry& registry) {
		registry.ctx().emplace<Context>();
		return ERROR_NoError;
	}

	Smasher::ErrorCode Teardown(entt::registry& registry) {
		return ERROR_NoError;
	}
}

namespace GameLogicSystem {
	struct Context {

	};

	Smasher::ErrorCode Initialize(entt::registry& registry) {
		registry.ctx().emplace<Context>();
		return ERROR_NoError;
	}

	Smasher::ErrorCode Update(entt::registry& registry) {
		// When to shutdown game
		if (!registry.ctx().contains<Context>()) {
			return ERROR_SystemNotInitialized;
		}
		return ERROR_NoError;
	};

	Smasher::ErrorCode Teardown(entt::registry & registry) {
		return ERROR_NoError;
	}
}

// Make an interactible graphics demo
// showing off how to use the Smasher ECS implementation
TEST(GraphicsDemo, NewComponentSystem) {
	Smasher::Engine engine{ 640, 420 };
	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::JobManager& jobManager = engine.GetJobManager();

	// Setup system contexts
	CursorInteractSystem::Initialize(engine.GetRegistry());
	GameLogicSystem::Initialize(engine.GetRegistry());
	Smasher::DrawableSystem::Initialize(engine.GetRegistry());
	Smasher::FrameTimeSystem::Initialize(engine.GetRegistry());

	// Add entities
	for (size_t i = 0; i < 100; ++i) {
		entt::entity entity = engine.GetRegistry().create();
		Smasher::DrawableSystem::AddComponent(engine.GetRegistry(), entity);
	}

	jobManager.SetTickJobProducer([&]() {
		entt::registry& registry = engine.GetRegistry();

		Smasher::Job& startFrameJob = jobManager.AddSyncJob(std::bind(Smasher::FrameTimeSystem::StartFrame, std::ref(registry)), {}).Get();

		Smasher::Job& gameLogicJob = jobManager.AddAsyncJob(std::bind(GameLogicSystem::Update, std::ref(registry)), 
			{ startFrameJob }).Get();
		Smasher::Job& collisionAvoidJob = jobManager.AddAsyncJob(std::bind(CollisionAvoidSystem::Update, std::ref(registry)), 
			{ startFrameJob }).Get();
		Smasher::Job& cursorMagnetJob = jobManager.AddAsyncJob(std::bind(CursorInteractSystem::Update, std::ref(registry)),
			{ startFrameJob, collisionAvoidJob }).Get();
		

		Smasher::Job& drawableJob = jobManager.AddSyncJob(std::bind(Smasher::DrawableSystem::Render, std::ref(registry)),
			{ cursorMagnetJob }).Get();
		Smasher::Job& endFrameJob = jobManager.AddSyncJob(std::bind(Smasher::FrameTimeSystem::EndFrame, std::ref(registry)), {}).Get();
	});

	layer.Activate();
	engine.Run(); // Engine should shutdown after the third update

	CursorInteractSystem::Teardown(engine.GetRegistry());
	GameLogicSystem::Teardown(engine.GetRegistry());
	Smasher::DrawableSystem::Teardown(engine.GetRegistry());
	Smasher::FrameTimeSystem::Teardown(engine.GetRegistry());
}