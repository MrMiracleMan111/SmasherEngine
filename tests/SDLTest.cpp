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
		entt::entity camera;
		entt::entity model;
	};

	struct Component {
		char padding;
	};

	Smasher::ErrorCode Initialize(entt::registry& registry, entt::entity camera, entt::entity model) {
		if (registry.ctx().contains<Context>()) {
			return ERROR_SystemAlreadyInitialized;
		}

		registry.ctx().emplace<Context>(camera, model);
		return ERROR_NoError;
	}

	Smasher::Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity) {
		assert(registry.all_of<Smasher::TransformSystem::Component>(entity) && "StaticMeshSystem::Component requires TransformSystem::Component");

		if (!registry.ctx().contains<Context>()) {
			return Smasher::Expected<std::reference_wrapper<Component>>::Error(ERROR_SystemNotInitialized);
		}

		Component& component = registry.emplace<Component>(entity);
		return std::ref(component);
	}

	Smasher::ErrorCode Update(entt::registry& registry, Smasher::Millisecond ms) {
		//Smasher::Engine& engine = registry.ctx().get<Smasher::EngineSystem::Context>().engineRef;
		//Smasher::EventManager& eventManager = engine.GetEventManager();
		auto& ctx = registry.ctx().get<Context>();
		auto& transform = registry.get<Smasher::TransformSystem::Component>(ctx.model);

		float delta = ms.count() / 1000.f;
		float speed = 1.0f * delta;
		float rotationSpeed = 40.0f * delta;
		glm::vec3 pos = Smasher::TransformSystem::GetPosition(transform);
		glm::vec3 euler = Smasher::TransformSystem::GetEulerDeg(transform);
		int numkeys = 0;
		const bool* state = SDL_GetKeyboardState(&numkeys);
		bool pressedKey = false;
		if (state[SDL_SCANCODE_W]) {
			pos.z -= speed;
			Smasher::TransformSystem::SetPosition(transform, pos);
			pressedKey = true;
		}
		if (state[SDL_SCANCODE_A]) {
			pos.x -= speed;
			Smasher::TransformSystem::SetPosition(transform, pos);
			pressedKey = true;
		}
		if (state[SDL_SCANCODE_S]) {
			pos.z += speed;
			Smasher::TransformSystem::SetPosition(transform, pos);
			pressedKey = true;
		}
		if (state[SDL_SCANCODE_D]) {
			pos.x += speed;
			Smasher::TransformSystem::SetPosition(transform, pos);
			pressedKey = true;
		}
		if (state[SDL_SCANCODE_Q]) {
			euler.y -= rotationSpeed;
			Smasher::TransformSystem::RotateEulerDeg(transform, 0.f, -rotationSpeed, 0.f, true);
			pressedKey = true;
		}
		if (state[SDL_SCANCODE_E]) {
			euler.y += rotationSpeed;
			Smasher::TransformSystem::RotateEulerDeg(transform, 0.f, rotationSpeed, 0.f, true);
			pressedKey = true;
		}

		if (pressedKey) {
			Smasher::TransformSystem::MarkDirty(registry, ctx.model);
			Smasher::TransformSystem::MarkDirty(registry, ctx.camera);
		}

		float radians = glm::radians(rotationSpeed);
		auto view = registry.view<Smasher::TransformSystem::Component, Component>();
		glm::quat rotation = glm::angleAxis(radians, glm::vec3(0, 1, 0));
		for (auto [entity, transform, logic] : view.each()) {
			Smasher::TransformSystem::Rotate(transform, rotation, true);
			Smasher::TransformSystem::MarkDirty(registry, entity);
		}
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
	auto cottageMeshResource = resourceManager.GetOrLoadResource<Smasher::Manifest::Models::cottage, Smasher::StaticMeshResource>(sdlSystemCtx.pGpu);
	auto suzanneMeshResource = resourceManager.GetOrLoadResource<Smasher::Manifest::Models::suzanne, Smasher::StaticMeshResource>(sdlSystemCtx.pGpu);
	auto teapotMeshResource = resourceManager.GetOrLoadResource<Smasher::Manifest::Models::teapot, Smasher::StaticMeshResource>(sdlSystemCtx.pGpu);
	auto material = resourceManager.GetOrLoadResource<Smasher::Manifest::Materials::cottage, Smasher::MaterialResource>(sdlSystemCtx.pGpu);

	Smasher::StaticMeshSystem::Initialize(registry);
	Smasher::PBRSystem::Initialize(registry, sdlSystemCtx.pGpu);
	Smasher::SDLSpriteSystem::Initialize(registry, sdlSystemCtx.pGpu, fragShader, vertShader, sdlSystemCtx.window);

	auto& staticMeshSystemCtx = registry.ctx().get<Smasher::StaticMeshSystem::Context>();
	auto& pbrSystemCtx = registry.ctx().get<Smasher::PBRSystem::Context>();
	auto& cameraSystemCtx = registry.ctx().get<Smasher::CameraSystem::Context>();

	Smasher::MaterialBinding materialBinding = Smasher::PBRSystem::BindMaterial(pbrSystemCtx, material).Get();

	// Add Camera
	int width, height;
	SDL_GetWindowSizeInPixels(sdlSystemCtx.window, &width, &height);
	entt::entity camera = registry.create();
	auto& cameraTransform = Smasher::TransformSystem::AddComponent(registry, camera).Get().get();
	Smasher::TransformSystem::SetPosition(cameraTransform, 0.f, 0.f, 6.f);
	Smasher::TransformSystem::MarkDirty(registry, camera);
	auto& cameraComponent = Smasher::CameraSystem::AddComponent(registry, camera).Get().get();
	Smasher::CameraSystem::SetFOV(cameraComponent, 90.f);
	Smasher::CameraSystem::SetNearClipPlane(cameraComponent, 0.1f);
	Smasher::CameraSystem::SetFarClipPlane(cameraComponent, 100.f);
	Smasher::CameraSystem::SetAspectRatio(cameraComponent, (float)width / (float)height);
	Smasher::CameraSystem::ComputeProjectionMatrix(cameraComponent);
	Smasher::CameraSystem::ComputeViewMatrix(cameraComponent, Smasher::TransformSystem::GetTransform(cameraTransform));

	// Add Teapot Model
	entt::entity interactObject = registry.create();
	auto& teapotTransform = Smasher::TransformSystem::AddComponent(registry, interactObject).Get().get();
	Smasher::TransformSystem::SetPosition(teapotTransform, 0.f, 0.f, 4.f);
	Smasher::TransformSystem::SetScale(teapotTransform, 0.1f, 0.1f, 0.1f);
	auto& teapotMesh = Smasher::StaticMeshSystem::AddComponent(registry, interactObject, cottageMeshResource, pbrSystemCtx.staticMeshBatchPool).Get().get();
	Smasher::StaticMeshSystem::SetMaterial(teapotMesh, materialBinding);
	GameLogic::Initialize(registry, camera, interactObject);

	int dimension = 10;
	for (int i = 0; i < dimension; i++) {
		for (int j = 0; j < dimension; j++) {
			const float inc = 0.1f;
			const float offset = (dimension/2) * inc;
			entt::entity tmp = registry.create();
			auto& tmpTransform = Smasher::TransformSystem::AddComponent(registry, tmp).Get().get();
			Smasher::TransformSystem::SetPosition(tmpTransform, i * inc - offset, j * inc - offset, 3.f);
			Smasher::TransformSystem::SetScale(tmpTransform, 0.1f, 0.1f, 0.1f);
			auto& tmpMesh = Smasher::StaticMeshSystem::AddComponent(registry, tmp, suzanneMeshResource, pbrSystemCtx.staticMeshBatchPool).Get().get();
			GameLogic::AddComponent(registry, tmp);
		}
	}

	jobManager.SetTickJobProducer([&]() {
		entt::registry& registry = engine.GetRegistry();
		auto& engine = registry.ctx().get<Smasher::EngineSystem::Context>().engineRef.get();
		auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(engine.GetTickDelta());
		Smasher::Job& gameLogicUpdate = jobManager.AddSyncJob(std::bind(GameLogic::Update, std::ref(registry), delta),
			{ }, "Game Logic Update Job").Get();
		Smasher::Job& cameraUpdate = jobManager.AddSyncJob(std::bind(Smasher::CameraSystem::SyncCameraViewProjection, std::ref(registry), std::ref(cameraSystemCtx)),
			{ gameLogicUpdate }, "Camera Update Job").Get();

		if (engine.IsRenderTick()) {
			auto& sdlSpriteSystemCtx = registry.ctx().get<Smasher::SDLSpriteSystem::Context>();
			auto& pbrSystemCtx = registry.ctx().get<Smasher::PBRSystem::Context>();
			auto& staticMeshSystemCtx = registry.ctx().get<Smasher::StaticMeshSystem::Context>();

			SDL_GPUDevice* device = sdlSystemCtx.pGpu->Get();
			Smasher::Job& syncModelTransformsJob = jobManager.AddSyncJob(std::bind(Smasher::StaticMeshSystem::SyncStaticMeshTransforms, std::ref(registry), std::ref(staticMeshSystemCtx)),
				{ cameraUpdate }, "Sync Transforms Job").Get();

			Smasher::Job& sdlModelDepthPrePass = jobManager.AddSyncJob(std::bind(Smasher::PBRSystem::DepthPrePass, std::ref(pbrSystemCtx), std::ref(staticMeshSystemCtx), std::ref(resourceManager), cameraComponent),
				{ syncModelTransformsJob }, "Depth Pre Pass Job").Get();
			Smasher::Job& sdlModelMaterialPass = jobManager.AddSyncJob(std::bind(Smasher::PBRSystem::MaterialsPass, std::ref(pbrSystemCtx), cameraComponent),
				{ sdlModelDepthPrePass }, "Material Pass Job").Get();
			//Smasher::Job& sdlCompositionJob = jobManager.AddSyncJob(std::bind(Smasher::SDLSystem::CompositionPass, std::ref(sdlSystemCtx), std::vector<Smasher::SDLSystem::RenderTexture>{ Smasher::SDLSystem::RenderTexture{ sdlSpriteSystemCtx.renderTexture, sdlSpriteSystemCtx.depthTexture } }),
			//	{ sdlDrawTriangleJob }).Get();
			Smasher::Job& sdlCompositionJob = jobManager.AddSyncJob(std::bind(Smasher::SDLSystem::CompositionPass, std::ref(sdlSystemCtx), std::vector<Smasher::SDLSystem::RenderTexture>{ Smasher::SDLSystem::RenderTexture{ pbrSystemCtx.gTriangleNormals, sdlSpriteSystemCtx.depthTexture } }),
				{ sdlModelMaterialPass }, "Composition Job").Get();
			Smasher::Job& sdlCopyToWindowJob = jobManager.AddSyncJob(std::bind(Smasher::SDLSystem::CopyToWindow, std::ref(sdlSystemCtx), pbrSystemCtx.gAlbedo),
				{ sdlCompositionJob }, "Copy to Window Job").Get();
		}

		Smasher::Job& transformLogicJob = jobManager.AddSyncJob(std::bind(Smasher::TransformSystem::ClearDirty, std::ref(registry)),
			{ gameLogicUpdate }, "Clear Transform Dirty Bits Job").Get();
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