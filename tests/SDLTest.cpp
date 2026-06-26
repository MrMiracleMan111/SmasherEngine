#define GLM_ENABLE_EXPERIMENTAL
#include <fstream>
#include <iostream>
#include <type_traits>
#include <array>
#include <format>
#include <random>
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
		auto& transform = registry.get<Smasher::TransformSystem::Component>(ctx.camera);

		float delta = ms.count() / 1000.f;
		float speed = 1.0f * delta;
		float rotationSpeed = 40.0f * delta;
		glm::vec3 posDelta { 0.f, 0.f, 0.f };
		int numkeys = 0;
		const bool* state = SDL_GetKeyboardState(&numkeys);
		bool pressedKey = false;
		if (state[SDL_SCANCODE_W]) {
			posDelta.z -= speed;
			pressedKey = true;
		}
		if (state[SDL_SCANCODE_A]) {
			posDelta.x -= speed;
			pressedKey = true;
		}
		if (state[SDL_SCANCODE_S]) {
			posDelta.z += speed;
			pressedKey = true;
		}
		if (state[SDL_SCANCODE_D]) {
			posDelta.x += speed;
			pressedKey = true;
		}
		if (state[SDL_SCANCODE_Q]) {
			Smasher::TransformSystem::RotateEulerDeg(transform, 0.f, rotationSpeed, 0.f, true);
			pressedKey = true;
		}
		if (state[SDL_SCANCODE_E]) {
			Smasher::TransformSystem::RotateEulerDeg(transform, 0.f, -rotationSpeed, 0.f, true);
			pressedKey = true;
		}

		glm::vec3 pos = Smasher::TransformSystem::GetPosition(transform);
		glm::vec4 tmp{ pos.x, pos.y, pos.z, 1.f };
		pos += Smasher::TransformSystem::GetRotation(transform) * posDelta;
		Smasher::TransformSystem::SetPosition(transform, pos);

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
	Smasher::SDLSystem::Initialize(registry, Smasher::WindowOptions{ "Hello World", 640, 480 });
	auto& sdlSystemCtx = registry.ctx().get<Smasher::SDLSystem::Context>();

	// Same shader file, different entry points used (PSMain, VSMain)
	auto fragShader = resourceManager.GetOrLoadResource<Smasher::Manifest::Shaders::test_frag_shader, Smasher::SDLGraphicShaderResource>(sdlSystemCtx.pGpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT);
	auto vertShader = resourceManager.GetOrLoadResource<Smasher::Manifest::Shaders::test_vert_shader, Smasher::SDLGraphicShaderResource>(sdlSystemCtx.pGpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX);
	auto cottageMeshResource = resourceManager.GetOrLoadResource<Smasher::Manifest::Models::cottage, Smasher::StaticMeshResource>(sdlSystemCtx.pGpu);
	auto suzanneMeshResource = resourceManager.GetOrLoadResource<Smasher::Manifest::Models::suzanne, Smasher::StaticMeshResource>(sdlSystemCtx.pGpu);
	auto teapotMeshResource = resourceManager.GetOrLoadResource<Smasher::Manifest::Models::teapot, Smasher::StaticMeshResource>(sdlSystemCtx.pGpu);
	auto cottageMaterial = resourceManager.GetOrLoadResource<Smasher::Manifest::Materials::cottage, Smasher::MaterialResource>(sdlSystemCtx.pGpu);
	auto wallMaterial = resourceManager.GetOrLoadResource<Smasher::Manifest::Materials::wall, Smasher::MaterialResource>(sdlSystemCtx.pGpu);

	Smasher::StaticMeshSystem::Initialize(registry);
	Smasher::PBRSystem::Initialize(registry, sdlSystemCtx.pGpu);
	Smasher::SDLSpriteSystem::Initialize(registry, sdlSystemCtx.pGpu, fragShader, vertShader, sdlSystemCtx.window);

	auto& staticMeshSystemCtx = registry.ctx().get<Smasher::StaticMeshSystem::Context>();
	auto& pbrSystemCtx = registry.ctx().get<Smasher::PBRSystem::Context>();
	auto& cameraSystemCtx = registry.ctx().get<Smasher::CameraSystem::Context>();

	Smasher::MaterialBinding cottageMaterialBinding = Smasher::PBRSystem::BindMaterial(pbrSystemCtx, cottageMaterial).Get();
	Smasher::MaterialBinding wallMaterialBinding = Smasher::PBRSystem::BindMaterial(pbrSystemCtx, wallMaterial).Get();
	Smasher::StaticMeshBinding suzanneMeshBinding = Smasher::PBRSystem::BindStaticMesh(pbrSystemCtx, suzanneMeshResource).Get();
	Smasher::StaticMeshBinding cottageMeshBinding = Smasher::PBRSystem::BindStaticMesh(pbrSystemCtx, cottageMeshResource).Get();

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
	Smasher::TransformSystem::SetScale(teapotTransform, 0.001f, 0.001f, 0.001f);
	auto& testMesh = Smasher::StaticMeshSystem::AddComponent(registry, interactObject, cottageMeshBinding, pbrSystemCtx.staticMeshBatchPool).Get().get();
	Smasher::StaticMeshSystem::SetMaterial(testMesh, cottageMaterialBinding);
	GameLogic::Initialize(registry, camera, interactObject);

	const int dimension = 5;
	std::uniform_real_distribution<float> distribution{ -3.f, 3.f };
	std::default_random_engine gen;
	for (int k = 0; k < 1; ++k) {
		for (int i = 0; i < dimension; i++) {
			for (int j = 0; j < dimension; j++) {
				float inc = 0.2f;
				const float offset = ((float)(dimension) / 2.f) * inc;
				const float x = distribution(gen);//j * inc;
				const float y = distribution(gen);//i * inc;
				const float z = 1.f * distribution(gen);
				entt::entity tmp = registry.create();
				auto& tmpTransform = Smasher::TransformSystem::AddComponent(registry, tmp).Get().get();
				Smasher::TransformSystem::SetPosition(tmpTransform, x - offset, y - offset, /*0.8f +*/ z - offset);
				Smasher::TransformSystem::SetScale(tmpTransform, 0.1f, 0.1f, 0.1f);
				auto& tmpMesh = Smasher::StaticMeshSystem::AddComponent(registry, tmp, suzanneMeshBinding, pbrSystemCtx.staticMeshBatchPool).Get().get();
				Smasher::StaticMeshSystem::SetMaterial(tmpMesh, wallMaterialBinding);
				GameLogic::AddComponent(registry, tmp);
			}
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
			int numBHVLeaves = staticMeshSystemCtx.totalNumInstances;

			SDL_GPUDevice* device = sdlSystemCtx.pGpu->Get();
			Smasher::Job& syncModelTransformsJob = jobManager.AddSyncJob(std::bind(Smasher::StaticMeshSystem::SyncStaticMeshTransforms, std::ref(registry), std::ref(staticMeshSystemCtx)),
				{ cameraUpdate }, "Sync Transforms Job").Get();
			Smasher::Job& sdlSyncStaticMeshInstances = jobManager.AddSyncJob(std::bind(Smasher::PBRSystem::SyncStaticMeshInstances, std::ref(pbrSystemCtx), std::ref(staticMeshSystemCtx)),
				{ syncModelTransformsJob }, "Sync Static Mesh GPU Instances Job").Get();
			Smasher::Job& constructBVH = jobManager.AddSyncJob(std::bind(Smasher::PBRSystem::ConstructBVH, std::ref(pbrSystemCtx), std::ref(staticMeshSystemCtx)),
				{ sdlSyncStaticMeshInstances }, "Construct BVH").Get();
			Smasher::Job& sdlModelDepthPrePass = jobManager.AddSyncJob(std::bind(Smasher::PBRSystem::DepthPrePass, std::ref(pbrSystemCtx), std::ref(staticMeshSystemCtx), std::ref(resourceManager), cameraComponent),
				{ constructBVH }, "Depth Pre Pass Job").Get();
			Smasher::Job& sdlModelMaterialPass = jobManager.AddSyncJob(std::bind(Smasher::PBRSystem::MaterialsPass, std::ref(pbrSystemCtx), cameraComponent),
				{ sdlModelDepthPrePass }, "Material Pass Job").Get();
			Smasher::Job& sdlShadowPass = jobManager.AddSyncJob(std::bind(Smasher::PBRSystem::ShadowPass, std::ref(pbrSystemCtx), std::ref(staticMeshSystemCtx), std::ref(resourceManager), cameraComponent),
				{ sdlModelMaterialPass }, "Shadow Pass Job").Get();
			Smasher::Job& sdlLightingPass = jobManager.AddSyncJob(std::bind(Smasher::PBRSystem::LightingPass, std::ref(pbrSystemCtx), std::ref(staticMeshSystemCtx), std::ref(resourceManager), cameraComponent),
				{ sdlShadowPass }, "Lighting Pass Job").Get();
			Smasher::Job& debugDrawBVH = jobManager.AddSyncJob(std::bind(Smasher::PBRSystem::DebugDrawBVH, std::ref(pbrSystemCtx), pbrSystemCtx.BVHInternalNodes, pbrSystemCtx.gDepthPrePass, pbrSystemCtx.gAlbedo, cameraComponent, numBHVLeaves),
				{ sdlLightingPass }, "Debug Draw BVH Nodes").Get();
			Smasher::Job& sdlCompositionJob = jobManager.AddSyncJob(std::bind(Smasher::SDLSystem::CompositionPass, std::ref(sdlSystemCtx), std::vector<Smasher::SDLSystem::RenderTexture>{ Smasher::SDLSystem::RenderTexture{ pbrSystemCtx.gTriangleNormals, sdlSpriteSystemCtx.depthTexture } }),
				{ debugDrawBVH }, "Composition Job").Get();
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