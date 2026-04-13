#define GLM_ENABLE_EXPERIMENTAL
#include <fstream>
#include <iostream>
#include <type_traits>
#include <array>
#include <format>
#include <SDL3/SDL.h>
#include <Windows.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "Smasher/Core.h"
#include "Smasher/Base.h"
#include "Smasher/ComponentSystems/EngineSystem.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ComponentSystems/SDLSystem.h"
#include "Smasher/ComponentSystems/SDLSpriteSystem.h"
#include "Smasher/ComponentSystems/StaticMeshSystem.h"
#include "Smasher/ComponentSystems/CameraSystem.h"
#include "Smasher/ComponentSystems/PBRSystem.h"
#include "Smasher/Resources.h"
#include "Smasher/Exceptions.h"
#include "Manifest.h"

class DummyLayer : public Smasher::Layer {
public:
	DummyLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {}
};

namespace GameLogic {
	struct Context {
		Smasher::TransformSystem::Component& cameraTransform;
	};

	Smasher::ErrorCode Initialize(entt::registry& registry, Smasher::TransformSystem::Component& cameraTransform) {
		if (registry.ctx().contains<Context>()) {
			return ERROR_SystemAlreadyInitialized;
		}

		registry.ctx().emplace<Context>(cameraTransform);
	}

	Smasher::ErrorCode Update(entt::registry& registry) {
		auto& ctx = registry.ctx().get<Context>();
		Smasher::Engine& engine = registry.ctx().get<Smasher::EngineSystem::Context>().engineRef;
		Smasher::EventManager& eventManager = engine.GetEventManager();


		float speed = 0.01f;
		glm::vec3 pos = Smasher::TransformSystem::GetPosition(ctx.cameraTransform);
		int numkeys = 0;
		const bool* state = SDL_GetKeyboardState(&numkeys);
		if (state[SDL_SCANCODE_W]) {
			pos.z -= speed;
			Smasher::TransformSystem::SetPosition(ctx.cameraTransform, pos);
		}
		if (state[SDL_SCANCODE_A]) {
			pos.x -= speed;
			Smasher::TransformSystem::SetPosition(ctx.cameraTransform, pos);
		}
		if (state[SDL_SCANCODE_S]) {
			pos.z += speed;
			Smasher::TransformSystem::SetPosition(ctx.cameraTransform, pos);
		}
		if (state[SDL_SCANCODE_D]) {
			pos.x += speed;
			Smasher::TransformSystem::SetPosition(ctx.cameraTransform, pos);
		}
		std::cout << "Z Pos: " << pos.z << std::endl;

		return ERROR_NoError;
	}
}


entt::entity SpawnBouncingBall(entt::registry& registry, glm::vec2 position) {
	const float toRadian = (float)(180.0 / std::numbers::pi);
	int minSpeed = 100;
	int speedVariance = 100;
	float angle = (float)(rand() % 360);
	float speed = (float)(rand() % speedVariance + minSpeed);
	float tmpX = std::cos(angle * toRadian) * speed;
	float tmpY = std::sin(angle * toRadian) * speed;
	float depth = (float)(rand() % 100) / 100.0f;
	entt::entity ball = registry.create();

	auto& ballTransform = Smasher::TransformSystem::AddComponent(registry, ball).Get().get();
	Smasher::TransformSystem::SetPosition(ballTransform, position.x, position.y, 0.f);
	Smasher::TransformSystem::SetScale(ballTransform, 70.f, 70.f, 1.f);

	return ball;
}

// the vertex input layout
struct Vertex
{
	float x, y, z;      //vec3 position
	float r, g, b, a;   //vec4 color
};

// Make an interactible graphics demo
// showing off how to use the Smasher ECS implementation
int main() {
	std::cout << "Resource Directory: " << Smasher::Manifest::Metadata::RESOURCES_DIRECTORY << std::endl;
	Smasher::Engine engine{ 640u, 420u };
	engine.GetResourceManager().SetResourceDirectory(Smasher::Manifest::Metadata::RESOURCES_DIRECTORY);
	Smasher::JobManager& jobManager = engine.GetJobManager();
	Smasher::ResourceManager& resourceManager = engine.GetResourceManager();
	DummyLayer& layer = engine.PushLayer<DummyLayer>();

	// Setup system contexts
	entt::registry& registry = engine.GetRegistry();
	Smasher::TransformSystem::Initialize(registry);
	Smasher::CameraSystem::Initialize(registry);
	Smasher::SDLSystem::Initialize(registry, Smasher::WindowOptions { "Hello World", 640, 480 });
	auto &sdlSystemCtx = registry.ctx().get<Smasher::SDLSystem::Context>();

	// Same shader file, different entry points used (PSMain, VSMain)
	auto fragShader = resourceManager.GetOrLoadResource<Smasher::Manifest::Shaders::test_frag_shader, Smasher::SDLGraphicShaderResource>(sdlSystemCtx.pGpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT);
	auto vertShader = resourceManager.GetOrLoadResource<Smasher::Manifest::Shaders::test_vert_shader, Smasher::SDLGraphicShaderResource>(sdlSystemCtx.pGpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX);
	auto teapotMeshResource = resourceManager.GetOrLoadResource<Smasher::Manifest::Models::teapot, Smasher::StaticMeshResource>(sdlSystemCtx.pGpu);

	Smasher::StaticMeshSystem::Initialize(registry);
	Smasher::PBRSystem::Initialize(registry, sdlSystemCtx.pGpu);
	Smasher::SDLSpriteSystem::Initialize(registry, sdlSystemCtx.pGpu, fragShader, vertShader, sdlSystemCtx.window);

	// Add entities
	glm::uvec2 bounds{ 640u, 420u };
	for (size_t i = 0; i < 10; ++i) {
		glm::vec2 position{ rand() % bounds.x, rand() % bounds.y };
		entt::entity entity = registry.create();
		entt::entity ball = SpawnBouncingBall(engine.GetRegistry(), position);
	}
	// Add Camera
	int width, height;
	SDL_GetWindowSizeInPixels(sdlSystemCtx.window, &width, &height);
	entt::entity camera = registry.create();
	auto& cameraTransform = Smasher::TransformSystem::AddComponent(registry, camera).Get().get();
	Smasher::TransformSystem::SetPosition(cameraTransform, 0.f, 0.f, 5.f);
	auto& cameraComponent = Smasher::CameraSystem::AddComponent(registry, camera).Get().get();
	Smasher::CameraSystem::SetFOV(cameraComponent, 90.f);
	Smasher::CameraSystem::SetNearClipPlane(cameraComponent, 1.f);
	Smasher::CameraSystem::SetFarClipPlane(cameraComponent, 1000.f);
	Smasher::CameraSystem::SetAspectRatio(cameraComponent, width / height);
	Smasher::CameraSystem::ComputeProjectionMatrix(cameraComponent);
	Smasher::CameraSystem::ComputeViewMatrix(cameraComponent, Smasher::TransformSystem::GetTransform(cameraTransform));


	GameLogic::Initialize(registry, cameraTransform);

	std::cout << "Projection Matrix" << glm::to_string(cameraComponent._projectionMatrix) << std::endl;
	std::cout << "Camera Transform" << glm::to_string(Smasher::TransformSystem::GetTransform(cameraTransform)) << std::endl;
	std::cout << "View Matrix" << glm::to_string(cameraComponent._viewMatrix) << std::endl;


	// Add Teapot Model
	entt::entity teapot = registry.create();
	auto& teapotTransform = Smasher::TransformSystem::AddComponent(registry, teapot).Get().get();
	Smasher::TransformSystem::SetPosition(teapotTransform, 0.f, 0.f, 0.f);
	Smasher::TransformSystem::SetScale(teapotTransform, 0.1f, 0.1f, 0.1f);
	auto& teapotMesh = Smasher::StaticMeshSystem::AddComponent(registry, teapot, teapotMeshResource).Get().get();

	jobManager.SetTickJobProducer([&]() {
		entt::registry& registry = engine.GetRegistry();
		auto& engine = registry.ctx().get<Smasher::EngineSystem::Context>().engineRef.get();

		Smasher::Job& gameLogicUpdate = jobManager.AddSyncJob(std::bind(GameLogic::Update, std::ref(registry)),
			{ }).Get();
		Smasher::Job& cameraUpdate = jobManager.AddSyncJob(std::bind(Smasher::CameraSystem::Update, std::ref(registry)),
			{ gameLogicUpdate }).Get();

		if (engine.IsRenderTick()) {
			auto& sdlSpriteSystemCtx = registry.ctx().get<Smasher::SDLSpriteSystem::Context>();
			auto& pbrSystemCtx = registry.ctx().get<Smasher::PBRSystem::Context>();
			SDL_GPUDevice* device = sdlSystemCtx.pGpu->Get();
			Smasher::Job& sdlModelDepthPrePass = jobManager.AddSyncJob(std::bind(Smasher::PBRSystem::DepthPrePass, pbrSystemCtx, device, cameraComponent),
				{ cameraUpdate }).Get();
			Smasher::Job& sdlDrawTriangleJob = jobManager.AddSyncJob(std::bind(Smasher::SDLSpriteSystem::DrawTriangle, sdlSpriteSystemCtx),
				{ sdlModelDepthPrePass }).Get();
			Smasher::Job& sdlCompositionJob = jobManager.AddSyncJob(std::bind(Smasher::SDLSystem::CompositionPass, sdlSystemCtx, std::vector<Smasher::SDLSystem::RenderTexture>{ Smasher::SDLSystem::RenderTexture{ sdlSpriteSystemCtx.renderTexture, sdlSpriteSystemCtx.depthTexture } }),
				{ sdlDrawTriangleJob }).Get();
			Smasher::Job& sdlCopyToWindowJob = jobManager.AddSyncJob(std::bind(Smasher::SDLSystem::CopyToWindow, sdlSystemCtx, pbrSystemCtx.gNormals),
				{ sdlCompositionJob }).Get();
		}

		Smasher::Job& transformLogicJob = jobManager.AddSyncJob(std::bind(Smasher::TransformSystem::Update, std::ref(registry)),
			{ gameLogicUpdate }).Get();
		});

	layer.Activate();
	engine.Run(); // Engine should shutdown after the third update

	Smasher::SDLSpriteSystem::Teardown(registry);
	Smasher::PBRSystem::Teardown(registry);
	Smasher::StaticMeshSystem::Teardown(registry);
	Smasher::SDLSystem::Teardown(registry);
	Smasher::CameraSystem::Teardown(registry);
	Smasher::TransformSystem::Teardown(registry);
}