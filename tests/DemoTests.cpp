#include <fstream>
#include <iostream>
#include <type_traits>
#include <array>
#include <format>
#include "glm/glm.hpp"
#include "Smasher/Core.h"
#include "Smasher/ComponentSystems/EngineSystem.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ComponentSystems/SpriteSystem.h"
#include "Smasher/ComponentSystems/TextSystem.h"
#include "Smasher/Resources.h"
#include "Manifest.h"

class DummyLayer : public Smasher::Layer {
public:
	DummyLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {}
};

namespace BallSystem {
	struct Component {
		glm::vec2 velocity;

	};

	Smasher::Expected<std::reference_wrapper<Component>> AddComponent(entt::registry &registry, entt::entity entity) {
		return std::ref(registry.emplace<Component>(entity));
	}

	void SetVelocity(Component &component, float x, float y) {
		component.velocity.x = x;
		component.velocity.y = y;
	}

	void SetVelocity(Component& component, glm::vec2 velocity) {
		component.velocity = velocity;
	}

	glm::vec2 GetVelocity(Component& component) {
		return component.velocity;
	}

	Smasher::ErrorCode Update(entt::registry& registry) {
		if (!registry.ctx().contains<Smasher::EngineSystem::Context>()) {
			return ERROR_SystemNotInitialized;
		}
		Smasher::Engine& engine = registry.ctx().get<Smasher::EngineSystem::Context>().engineRef.get();
		
		auto view = registry.view<Smasher::TransformSystem::Component, BallSystem::Component>();
		glm::vec2 windowSize{ (float)engine.GetWindow().getSize().x, (float)engine.GetWindow().getSize().y };
		auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(engine.GetTickDelta());


		for (auto [ball, ballTransform, ballLogic] : view.each()) {
			auto& ballTransform = registry.get<Smasher::TransformSystem::Component>(ball);
			glm::vec3 position = Smasher::TransformSystem::GetPosition(ballTransform);

			position += glm::vec3(ballLogic.velocity * (float)delta.count() * 0.001f, 0.f);

			//if (position.x < 0 || position.y < 0 ||
			//	position.x > windowSize.x || position.y > windowSize.y) {
			//	Smasher::Entity &entity = self.GetEntity();
			//	entity.GetGameState().RemoveEntity(entity.GetUUID());
			//	return;
			//}

			if (position.x <= 0) {
				ballLogic.velocity.x = -ballLogic.velocity.x;
				position.x = 0;

				//Smasher::Entity& entity = self.GetEntity();
				//entity.GetGameState().RemoveEntity(entity.GetUUID());
			}
			if (position.y <= 0) {
				ballLogic.velocity.y = -ballLogic.velocity.y;
				position.y = 0;

				//Smasher::Entity& entity = self.GetEntity();
				//entity.GetGameState().RemoveEntity(entity.GetUUID());
			}

			if (position.x >= windowSize.x) {
				ballLogic.velocity.x = -ballLogic.velocity.x;
				position.x = windowSize.x;
			}
			if (position.y >= windowSize.y) {
				ballLogic.velocity.y = -ballLogic.velocity.y;
				position.y = windowSize.y;
			}

			Smasher::TransformSystem::SetPosition(ballTransform, position);
		}
		return ERROR_NoError;
	};
}

namespace CursorInteractSystem {
	struct Context {
		// Mouse velocity
		Smasher::EventSubscriptionHandle onMouseMoveHandle;
	};

	entt::entity SpawnBouncingBall(entt::registry &registry, glm::vec2 position) {
		const float toRadian = (float)(180.0 / std::numbers::pi);
		int minSpeed = 100;
		int speedVariance = 100;
		float angle = (float)(rand() % 360);
		float speed = (float)(rand() % speedVariance + minSpeed);
		float tmpX = std::cos(angle * toRadian) * speed;
		float tmpY = std::sin(angle * toRadian) * speed;
		float depth = (float)(rand() % 100) / 100.0f;
		entt::entity ball = registry.create();
		
		auto &ballTransform = Smasher::TransformSystem::AddComponent(registry, ball).Get().get();
		Smasher::TransformSystem::SetPosition(ballTransform, position.x, position.y, 0.f);
		Smasher::TransformSystem::SetScale(ballTransform, 70.f, 70.f, 1.f);

		auto &ballImage = Smasher::SpriteSystem::AddComponent(registry, ball).Get().get();

		Smasher::SpriteSystem::SetDepth(ballImage, 0.5f);
		Smasher::SpriteSystem::SetTexture<Smasher::Manifest::Textures::alpha_test>(ballImage, registry, {});
		Smasher::SpriteSystem::SetTextureClipSize(ballImage, 40.f, 20.f);
		Smasher::SpriteSystem::SetTextureClipOffset(ballImage, 0.f, 0.f);
		auto& ballLogic = BallSystem::AddComponent(registry, ball).Get().get();
		BallSystem::SetVelocity(ballLogic, tmpX, tmpY);
		return ball;
	}

	Smasher::ErrorCode OnMouseMove(entt::registry &registry, Smasher::Events::MouseMoveEvent& event) {
		if (!registry.ctx().contains<Context>()) {
			return ERROR_SystemNotInitialized;
		}
		if (!registry.ctx().contains<Smasher::EngineSystem::Context>()) {
			return ERROR_SystemNotInitialized;
		}

		Smasher::Engine &engine = registry.ctx().get<Smasher::EngineSystem::Context>().engineRef.get();

		sf::Window& window = engine.GetWindow();
		sf::Rect<int> windowRect({ 0, 0 }, { (int)window.getSize().x, (int)window.getSize().y });
		if (windowRect.contains({ event.Position.x, event.Position.y })) {
			entt::entity ball = SpawnBouncingBall(registry, glm::vec2(event.Position.x, event.Position.y));
		}
		return ERROR_NoError;
	};

	Smasher::ErrorCode Update(entt::registry& registry) {
		if (!registry.ctx().contains<Context>()) {
			return ERROR_SystemNotInitialized;
		}

		Context& ctx = registry.ctx().get<Context>();
		// Make selected ball fly toward cursor
		return ERROR_NoError;
	};

	Smasher::ErrorCode Initialize(entt::registry& registry, Smasher::Layer& layer) {
		if (registry.ctx().contains<Context>()) {
			return ERROR_SystemAlreadyInitialized;
		}
		Context & ctx = registry.ctx().emplace<Context>();
		ctx.onMouseMoveHandle = layer.Subscribe<Smasher::Events::MouseMoveEvent>([&](Smasher::Events::MouseMoveEvent &event) {
			OnMouseMove(registry, event);
		});

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
		if (registry.ctx().contains<Context>()) {
			return ERROR_SystemAlreadyInitialized;
		}
		registry.ctx().emplace<Context>();
		return ERROR_NoError;
	}

	Smasher::ErrorCode Teardown(entt::registry& registry) {
		return ERROR_NoError;
	}
}

namespace GameLogicSystem {
	struct Context {
		int counter = 0;
		std::chrono::microseconds tickTimeSum{ 0 };
		entt::entity frameTimeText;
	};

	Smasher::ErrorCode Initialize(entt::registry& registry) {
		if (registry.ctx().contains<Context>()) {
			return ERROR_SystemAlreadyInitialized;
		}
		if (!registry.ctx().contains<Smasher::EngineSystem::Context>()) {
			return ERROR_SystemNotInitialized;
		}
		Context &ctx = registry.ctx().emplace<Context>();
		auto& engine = registry.ctx().get<Smasher::EngineSystem::Context>().engineRef.get();
		ctx.frameTimeText = registry.create();

		auto pFont = engine.GetResourceManager().GetOrLoadResource<Smasher::Manifest::Fonts::arial, Smasher::FontResource>();
		auto &transform = Smasher::TransformSystem::AddComponent(registry, ctx.frameTimeText).Get().get();
		auto &text = Smasher::TextSystem::AddComponent(registry, ctx.frameTimeText, pFont).Get().get();


		Smasher::TransformSystem::SetScale(transform, 1.f, 1.f, 1.f);
		Smasher::TransformSystem::SetPosition(transform, 20.f, 20.f, 0.f);
		Smasher::TextSystem::SetFontAsset<Smasher::Manifest::Fonts::arial>(text, registry);
		Smasher::TextSystem::SetFillColor(text, sf::Color::White);
		Smasher::TextSystem::SetOutlineThickness(text, 5.0f);
		Smasher::TextSystem::SetOutlineColor(text, sf::Color::Black);
		return ERROR_NoError;
	}

	Smasher::ErrorCode Update(entt::registry& registry) {
		static const int UPDATE_INTERVAL = 10; // Update every 10 ticks
		// When to shutdown game
		if (!registry.ctx().contains<Context>()) {
			return ERROR_SystemNotInitialized;
		}
		if (!registry.ctx().contains<Smasher::EngineSystem::Context>()) {
			return ERROR_SystemNotInitialized;
		}

		Context& ctx = registry.ctx().get<Context>();
		auto& engine = registry.ctx().get<Smasher::EngineSystem::Context>().engineRef.get();
		ctx.tickTimeSum += engine.GetTickDelta();
		ctx.counter++;
		if (ctx.counter < UPDATE_INTERVAL) {
			return ERROR_NoError;
		}
		ctx.counter = 0;

		int ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(ctx.tickTimeSum).count() / UPDATE_INTERVAL;
		auto& textComponent = registry.get<Smasher::TextSystem::Component>(ctx.frameTimeText);
		Smasher::TextSystem::SetString(textComponent, std::format("Update: {}ms", ms));
		ctx.tickTimeSum = std::chrono::microseconds{ 0 };
		return ERROR_NoError;
	};

	Smasher::ErrorCode Teardown(entt::registry & registry) {
		return ERROR_NoError;
	}
}

// Make an interactible graphics demo
// showing off how to use the Smasher ECS implementation
int main() {
	std::cout << "Resource Directory: " << Smasher::Manifest::Metadata::RESOURCES_DIRECTORY  << std::endl;
	Smasher::Engine engine{ 640u, 420u };
	engine.GetResourceManager().SetResourceDirectory(Smasher::Manifest::Metadata::RESOURCES_DIRECTORY);
	Smasher::ResourceManager& resourceManager = engine.GetResourceManager();
	
	auto pShaderResource = resourceManager.GetOrLoadResource<Smasher::Manifest::Shaders::basic_texture_shader, Smasher::ShaderResource>();
	sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(engine.GetWindow().getView().getTransform().getMatrix());
	pShaderResource->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);

	DummyLayer& layer = engine.PushLayer<DummyLayer>();
	Smasher::JobManager& jobManager = engine.GetJobManager();

	// Setup system contexts
	entt::registry& registry = engine.GetRegistry();
	Smasher::TransformSystem::Initialize(registry);
	Smasher::SpriteSystem::Initialize(registry);
	Smasher::TextSystem::Initialiaze(registry);
	GameLogicSystem::Initialize(registry);
	CursorInteractSystem::Initialize(registry, layer);

	auto& ctx = registry.ctx().get<Smasher::SpriteSystem::Context>();
	Smasher::SpriteSystem::SetSystemShader(ctx, pShaderResource);

		// Add entities
		glm::uvec2 bounds{ engine.GetWindow().getSize().x, engine.GetWindow().getSize().y };
		for (size_t i = 0; i < 10; ++i) {
			glm::vec2 position{ rand() % bounds.x, rand() % bounds.y };
			entt::entity entity = registry.create();
		entt::entity ball = CursorInteractSystem::SpawnBouncingBall(engine.GetRegistry(), position);
	}

	jobManager.SetTickJobProducer([&]() {
		entt::registry& registry = engine.GetRegistry();
		auto& engine = registry.ctx().get<Smasher::EngineSystem::Context>().engineRef.get();
		 
		Smasher::Job* pClearWindowJob = nullptr;
		if (engine.IsRenderTick()) {
			pClearWindowJob = &jobManager.AddSyncJob(std::bind(Smasher::EngineSystem::ClearWindow, std::ref(registry)),
				{}).Get().get();
		}

		Smasher::Job& transformLogicJob = jobManager.AddSyncJob(std::bind(Smasher::TransformSystem::Update, std::ref(registry)),
		 	{}).Get();
		Smasher::Job& gameLogicJob = jobManager.AddAsyncJob(std::bind(GameLogicSystem::Update, std::ref(registry)), 
			{ transformLogicJob }).Get();
		Smasher::Job& ballPhysicsJob = jobManager.AddAsyncJob(std::bind(BallSystem::Update, std::ref(registry)),
			{ transformLogicJob }).Get();

		if (engine.IsRenderTick()) {
			assert(pClearWindowJob != nullptr);
			 Smasher::Job& drawableJob = jobManager.AddSyncJob(std::bind(Smasher::SpriteSystem::Render, std::ref(registry)),
				{ *pClearWindowJob, gameLogicJob, ballPhysicsJob }).Get();
			 Smasher::Job& textDrawJob = jobManager.AddSyncJob(std::bind(Smasher::TextSystem::Render, std::ref(registry)),
				{ drawableJob }).Get();
			 Smasher::Job& displayWindowJob = jobManager.AddSyncJob(std::bind(Smasher::EngineSystem::DisplayWindow, std::ref(registry)),
				 { textDrawJob }).Get();

			 GLenum err;
			 while ((err = glGetError()) != GL_NO_ERROR)
			 {
				 std::cout << "GL Error: \"" << gluErrorString(err) << "\" Code: " << err << std::endl;
			 }
		}
	});

	layer.Activate();
	engine.Run(); // Engine should shutdown after the third update

	Smasher::SpriteSystem::Teardown(engine.GetRegistry());
	GameLogicSystem::Teardown(engine.GetRegistry());
	CursorInteractSystem::Teardown(engine.GetRegistry());
	Smasher::TransformSystem::Teardown(engine.GetRegistry());
}