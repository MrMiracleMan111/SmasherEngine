#include <fstream>
#include <iostream>
#include <type_traits>
#include <array>
#include <format>
#include "glm/glm.hpp"
#include "Smasher/Core.h"
#include "Smasher/Base.h"
#include "Smasher/ComponentSystems/EngineSystem.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ComponentSystems/SDLSystem.h"
#include "Smasher/ComponentSystems/SDLSpriteSystem.h"
#include "Smasher/Resources.h"
#include "Manifest.h"

class DummyLayer : public Smasher::Layer {
public:
	DummyLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {}
};


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
	Smasher::SDLSystem::Initialize(registry, Smasher::WindowOptions { "Hello World", 640, 480 });


	auto &sdlSystemCtx = registry.ctx().get<Smasher::SDLSystem::Context>();
	// Same shader file, different entry points used (PSMain, VSMain)
	auto fragShader = resourceManager.GetOrLoadResource<Smasher::Manifest::Shaders::test_frag_shader, Smasher::SDLGraphicShaderResource>(sdlSystemCtx.pGpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT);
	auto vertShader = resourceManager.GetOrLoadResource<Smasher::Manifest::Shaders::test_vert_shader, Smasher::SDLGraphicShaderResource>(sdlSystemCtx.pGpu, SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX);

	// a list of vertices
	static Vertex vertices[]
	{
		{0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},     // top vertex
		{-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f},   // bottom left vertex
		{0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f}     // bottom right vertex
	};

	// Create Command Buffer
	SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(sdlSystemCtx.pGpu->Get());

	// Create Vertex Buffer
	SDL_GPUBufferCreateInfo bufferInfo{};
	bufferInfo.size = sizeof(vertices);
	bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	SDL_GPUBuffer* vertexBuffer = SDL_CreateGPUBuffer(sdlSystemCtx.pGpu->Get(), &bufferInfo);

	// Create Transfer Buffer
	SDL_GPUTransferBufferCreateInfo transferInfo{};
	transferInfo.size = sizeof(vertices);
	transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(sdlSystemCtx.pGpu->Get(), &transferInfo);

	// Copy from Transfer Buffer to Vertex Buffer
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

	// Upload to Transfer Buffer
	Vertex* data = (Vertex*)SDL_MapGPUTransferBuffer(sdlSystemCtx.pGpu->Get(), transferBuffer, false);
	SDL_memcpy(data, vertices, sizeof(vertices));
	SDL_UnmapGPUTransferBuffer(sdlSystemCtx.pGpu->Get(), transferBuffer);

	SDL_GPUTransferBufferLocation location{};
	location.transfer_buffer = transferBuffer;
	location.offset = 0;

	SDL_GPUBufferRegion region{};
	region.buffer = vertexBuffer;
	region.size = sizeof(vertices);
	region.offset = 0;

	SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(commandBuffer);

	SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.vertex_shader = vertShader->GetShader();
	pipelineInfo.fragment_shader = fragShader->GetShader();

	pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

	SDL_GPUVertexBufferDescription vertexBufferDescriptions[1];
	vertexBufferDescriptions[0].slot = 0;
	vertexBufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	vertexBufferDescriptions[0].instance_step_rate = 0;
	vertexBufferDescriptions[0].pitch = sizeof(Vertex);

	SDL_GPUVertexAttribute vertexAttributeDescriptions[2];
	// a_position
	vertexAttributeDescriptions[0].buffer_slot = 0;								// fetch data from buffer at slot 0
	vertexAttributeDescriptions[0].location = 0;								// layout (location = 0)
	vertexAttributeDescriptions[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; // vec3
	vertexAttributeDescriptions[0].offset = 0;									// 

	// a_color
	vertexAttributeDescriptions[1].buffer_slot = 0;								// fetch data from buffer at slot 1
	vertexAttributeDescriptions[1].location = 1;								// layout (location = 1)
	vertexAttributeDescriptions[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4; // vec4
	vertexAttributeDescriptions[1].offset = sizeof(float) * 3;					// 

	SDL_GPUColorTargetDescription colorTargetDescriptions[1];
	colorTargetDescriptions[0] = {};
	colorTargetDescriptions[0].blend_state.enable_blend = true;
	colorTargetDescriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
	colorTargetDescriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
	colorTargetDescriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	colorTargetDescriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	colorTargetDescriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	colorTargetDescriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(sdlSystemCtx.pGpu->Get(), sdlSystemCtx.pWindow);

	pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
	pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDescriptions;
	pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
	pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributeDescriptions;
	pipelineInfo.target_info.num_color_targets = 1;
	pipelineInfo.target_info.color_target_descriptions = colorTargetDescriptions;

	SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(sdlSystemCtx.pGpu->Get(), &pipelineInfo);



	// Add entities
	glm::uvec2 bounds{ 640u, 420u };
	for (size_t i = 0; i < 10; ++i) {
		glm::vec2 position{ rand() % bounds.x, rand() % bounds.y };
		entt::entity entity = registry.create();
		entt::entity ball = SpawnBouncingBall(engine.GetRegistry(), position);
	}

	jobManager.SetTickJobProducer([&]() {
		entt::registry& registry = engine.GetRegistry();
		auto& engine = registry.ctx().get<Smasher::EngineSystem::Context>().engineRef.get();

		Smasher::Job& transformLogicJob = jobManager.AddSyncJob(std::bind(Smasher::TransformSystem::Update, std::ref(registry)),
			{}).Get();

		if (engine.IsRenderTick()) {
			Smasher::Job& sdlBeginFrameJob = jobManager.AddSyncJob(std::bind(Smasher::SDLSystem::BeginFrame, std::ref(registry)),
				{}).Get();
			Smasher::Job& sdlDrawTriangleJob = jobManager.AddSyncJob(std::bind(Smasher::SDLSystem::DrawTrianglePass, std::ref(registry), fragShader, vertShader, pipeline, vertexBuffer),
				{ sdlBeginFrameJob }).Get();
			Smasher::Job& sdlEndFrameJob = jobManager.AddSyncJob(std::bind(Smasher::SDLSystem::EndFrame, std::ref(registry)),
				{ sdlDrawTriangleJob }).Get();
		}
		});

	layer.Activate();
	engine.Run(); // Engine should shutdown after the third update

	SDL_ReleaseGPUTransferBuffer(sdlSystemCtx.pGpu->Get(), transferBuffer);
	SDL_ReleaseGPUBuffer(sdlSystemCtx.pGpu->Get(), vertexBuffer);
	Smasher::TransformSystem::Teardown(engine.GetRegistry());
}