#include <SFML/Window.hpp>
#include "Core.h"
#include "Manifest.h"
#include "ExampleResourcesGameState.h"
#include "Components/Transform2DComponent.h"
#include "Components/TextComponent.h"
#include "ComponentManagers/DrawableComponentManager.h"
#include "Components/DrawableComponent.h"
#include "Entity.h"

using namespace Smasher;
int main() {
	Smasher::Engine engine(640, 420);

	static_assert(HasRenderCapability<DrawableComponentManager>, "Failed");

	engine.GetResourceManager().SetResourceDirectory(Resources::Metadata::RESOURCES_DIRECTORY);
	ExampleResourcesGameState& state = engine.AddState<ExampleResourcesGameState>();

	std::shared_ptr<ShaderResource> shader = engine.GetResourceManager().GetOrLoadResource<Resources::Shaders::basic_texture_shader, ShaderResource>();
	sf::Vector2f windowSize = sf::Vector2f(engine.GetWindow().getSize().x , engine.GetWindow().getSize().y);
	sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(engine.GetWindow().getView().getTransform().getMatrix());
	shader->GetShader().setUniform("windowSize", windowSize);
	shader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);
	
	// TODO: FIND A BETTER SOLUTION
// I shouldn't have to worry about loading DrawableComponentManager after drawable component
	DrawableComponentManager& rCompManager = static_cast<DrawableComponentManager&>(state.GetComponentManager<DrawableComponent>());
	rCompManager.SetShaderResource(shader);

	Entity& image = state.AddEntity<Smasher::Entity>();
	image.AddComponent<Transform2DComponent>()
		.SetPosition(sf::Vector2f(300.0f, 300.0f))
		.SetScale(sf::Vector2f(400.0f, 400.0f));
	image.AddComponent<DrawableComponent>()
		.SetShader(shader)
		.SetTextureAsset<Smasher::Resources::Textures::small_art>()
		.SetClipRect(sf::IntRect{ 0, 0, 400, 400 })
		.SetDepth(0.2f)
		.PushToGPU(); // Render in reverse with i=0 being on top
	
	// Create 10 entites
	// Half will have a non-alpha image
	// Half will have image with alpha pixels
	const std::size_t numEntities = 10;
	for (int i = 0; i < numEntities; ++i) {
		float depth = (float)(rand() % 100) / 100.0f;

		int separation = 20;
		int offset = 0;
		Entity& image = state.AddEntity<Smasher::Entity>();
		image.AddComponent<Transform2DComponent>()
			.SetPosition(sf::Vector2f((float)(i * separation + offset), (float)(i * separation + offset)))
			.SetScale(sf::Vector2f(100.0f, 100.0f));

		image.AddComponent<DrawableComponent>()
			.SetShader(shader)
			.SetClipRect(sf::IntRect{ 0, 0, 30, 20 })
			.SetDepth(depth);

		if (i % 2 == 0) {
			 image.GetComponent<DrawableComponent>()
				 .SetTextureAsset<Smasher::Resources::Textures::alpha_test>()
				 .PushToGPU();
		}
		else {
			image.GetComponent<DrawableComponent>()
				.SetTextureAsset<Smasher::Resources::Textures::small_art>()
				.PushToGPU();
		}

	}

	// Create a window
	Entity& window1 = state.AddEntity<Smasher::Entity>();
	window1.AddComponent<Transform2DComponent>()
		.SetPosition(sf::Vector2f(400, 200))
		.SetScale(sf::Vector2f(100.0f, 100.0f));

	window1.AddComponent<DrawableComponent>()
		.SetShader(shader)
		.SetTextureAsset<Smasher::Resources::Textures::window>(true)
		.SetClipRect(sf::IntRect{ 0, 0, 64, 64 })
		.SetDepth(0.6)
		.PushToGPU(); // Window on top

	Entity& window2 = state.AddEntity<Smasher::Entity>();
	window2.AddComponent<Transform2DComponent>()
		.SetPosition(sf::Vector2f(420, 220))
		.SetScale(sf::Vector2f(100.0f, 100.0f));

	window2.AddComponent<DrawableComponent>()
		.SetShader(shader)
		.SetTextureAsset<Smasher::Resources::Textures::window>(true)
		.SetClipRect(sf::IntRect{ 0, 0, 64, 64 })
		.SetDepth(0.5)
		.PushToGPU(); // Window on bottom

	Entity& inWindowEntity = state.AddEntity<Smasher::Entity>();
	inWindowEntity.AddComponent<Transform2DComponent>()
		.SetPosition(sf::Vector2f(410, 210))
		.SetScale(sf::Vector2f(100.0f, 100.0f));

	inWindowEntity.AddComponent<DrawableComponent>()
		.SetShader(shader)
		.SetTextureAsset<Smasher::Resources::Textures::alpha_test>()
		.SetClipRect(sf::IntRect{ 0, 0, 64, 64 })
		.SetDepth(0.55)
		.PushToGPU(); // Window on bottom


	Entity& message = state.AddEntity<Smasher::Entity>();
	message.AddComponent<Transform2DComponent>();
	message.AddComponent<TextComponent>()
		.SetString("Hello World")
		.UseDefaults()
		.SetFontAsset<Smasher::Resources::Fonts::arial>();

	

 	state.Activate();
	engine.Run();
	return 0;
}