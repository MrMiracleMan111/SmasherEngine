#include <cstdlib>
#include <SFML/Window.hpp>
#include "Core.h"
#include "Manifest.h"
#include "ExampleResourcesGameState.h"
#include "Components/Transform2DComponent.h"
#include "Components/TextComponent.h"
#include "ComponentManagers/DrawableComponentManager.h"
#include "Components/DrawableComponent.h"

int main() {
	Smasher::Engine engine(640, 420);

	static_assert(Smasher::HasRenderCapability<Smasher::DrawableComponentManager>, "Failed");

	engine.GetResourceManager().SetResourceDirectory(Smasher::Resources::Metadata::RESOURCES_DIRECTORY);
	ExampleResourcesGameState& state = engine.AddState<ExampleResourcesGameState>();

	std::shared_ptr<Smasher::ShaderResource> shader = engine.GetResourceManager().GetOrLoadResource<Smasher::Resources::Shaders::basic_texture_shader, Smasher::ShaderResource>();
	sf::Vector2f windowSize = sf::Vector2f((float)engine.GetWindow().getSize().x , (float)engine.GetWindow().getSize().y);
	sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(engine.GetWindow().getView().getTransform().getMatrix());
	shader->GetShader().setUniform("windowSize", windowSize);
	shader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);
	

	// TODO: FIND A BETTER SOLUTION
	// I shouldn't have to worry about loading DrawableComponentManager after drawable component
	Smasher::DrawableComponentManager& rCompManager = static_cast<Smasher::DrawableComponentManager&>(state.GetComponentManager<Smasher::DrawableComponent>());
	rCompManager.SetShaderResource(shader);

	Smasher::Entity& image = state.AddEntity<Smasher::Entity>();
	image.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(500.0f, 300.0f))
		.SetScale(sf::Vector2f(400.0f, 400.0f))
		.SetShader(shader)
		.SetTextureAsset<Smasher::Resources::Textures::small_art>({}) // Default texture options
		//.SetClipRect(sf::IntRect{ 0, 0, 64, 64 })
		.SetClipRotation(Smasher::Degrees{ 30 })
		.SetDepth(0.2f);

	// Create 10 entites
	// Half will have a non-alpha image
	// Half will have image with alpha pixels
	const std::size_t numEntities = 10;
	for (int i = 0; i < numEntities; ++i) {
		float depth = (float)(std::rand() % 100) / 100.0f;

		int separation = 20;
		int offset = 100;
		Smasher::Entity& image = state.AddEntity<Smasher::Entity>();
		image.AddComponent<Smasher::DrawableComponent>()
			.SetPosition(sf::Vector2f((float)(i * separation + offset), (float)(i * separation + offset)))
			.SetScale(sf::Vector2f(100.0f, 100.0f))
			.SetShader(shader)
			.SetClipRect(sf::IntRect{ 0, 0, 30, 20 })
			.SetDepth(depth);

		if (i % 2 == 0) {
			image.GetComponent<Smasher::DrawableComponent>()
				.SetTextureAsset<Smasher::Resources::Textures::alpha_test>({});
		}
		else {
			image.GetComponent<Smasher::DrawableComponent>()
				.SetTextureAsset<Smasher::Resources::Textures::small_art>({});
		}

	}

	// Create a window
	Smasher::Entity& window1 = state.AddEntity<Smasher::Entity>();
	window1.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(400, 200))
		.SetScale(sf::Vector2f(150.0f, 150.0f))
		.SetShader(shader)
		.SetTextureAsset<Smasher::Resources::Textures::window>({.transluscent = true})
		.SetClipRect(sf::IntRect{ 10, 0, 64, 64 })
		.SetClipRotation(Smasher::Degrees{ 30 })
		.SetDepth(0.6f);

	Smasher::Entity& window2 = state.AddEntity<Smasher::Entity>();
	window2.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(420, 220))
		.SetScale(sf::Vector2f(150.0f, 150.0f))
		.SetShader(shader)
		.SetTextureAsset<Smasher::Resources::Textures::window>({.transluscent = true})
		//.SetClipRect(sf::IntRect{ 0, 0, 64, 64 })
		.SetDepth(0.5);

	Smasher::Entity& inWindowEntity = state.AddEntity<Smasher::Entity>();
	inWindowEntity.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(410, 210))
		.SetScale(sf::Vector2f(100.0f, 100.0f))
		.SetShader(shader)
		.SetTextureAsset<Smasher::Resources::Textures::alpha_test>({.transluscent = true})
		.SetClipRotation(Smasher::Degrees{ 30 })
		.SetDepth(0.55f);



	Smasher::Entity& message = state.AddEntity<Smasher::Entity>();
	message.AddComponent<Smasher::TextComponent>()
		.SetString("Hello World")
		.UseDefaults()
		.SetFontAsset<Smasher::Resources::Fonts::arial>();

 	state.Activate();
	engine.Run();
	return 0;
}