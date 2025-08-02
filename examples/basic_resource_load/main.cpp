#include <SFML/Window.hpp>
#include "Components/DrawableComponent.h"
#include "Components/Transform2DComponent.h"
#include "Components/TextComponent.h"
#include "Core.h"
#include "Manifest.h"
#include "ExampleResourcesGameState.h"

template <class T>
concept TestPaths = std::is_array_v<decltype(T::PATHS)> && std::same_as<std::decay_t<std::remove_extent_t<decltype(T::PATHS)>>, ResourcePath>;

int main() {
	sf::ContextSettings  settings;
	settings.depthBits = 24; // Request a 24 bits depth buffer
	settings.stencilBits = 8;  // Request a 8 bits stencil buffer
	settings.antialiasingLevel = 2;  // Request 2 levels of antialiasing

	Smasher::Engine engine(640, 420, settings);

	engine.GetResourceManager().SetResourceDirectory(Resources::Metadata::RESOURCES_DIRECTORY);
	ExampleResourcesGameState& state = engine.AddState<ExampleResourcesGameState>();
	Entity& message = state.AddEntity<Smasher::Entity>();
	
	static_assert(HasPathsVariable<Resources::Shaders::basic_texture_shader>, "Test1");
	static_assert(HasPathVariable<Resources::Fonts::arial>, "Test2");

	std::shared_ptr<ShaderResource> shader = engine.GetResourceManager().GetOrLoadResource<Resources::Shaders::basic_texture_clip_shader, ShaderResource>();
	sf::Vector2f windowSize = sf::Vector2f(engine.GetWindow().getSize().x , engine.GetWindow().getSize().y);
	shader->GetShader().setUniform("windowSize", windowSize);

	// Create 10 entites
	// Half will have a non-alpha image
	// Hald will have image iwth alpha pixels
	const size_t numEntities = 10;
	for (size_t i = 0; i < numEntities; ++i) {
		float depth = (float)(rand() % 100) / 100.0;

		int separation = 20;
		Entity& image = state.AddEntity<Smasher::Entity>();
		image.AddComponent<Transform2DComponent>()
			.SetPosition(sf::Vector2f(i * separation, i * separation))
			.SetScale(sf::Vector2f(1.0f, 1.0f));

		image.AddComponent<DrawableComponent>()
			.SetShader(shader)
			.SetClipRect(sf::IntRect{ 0, 0, 30, 20 })
			.SetDepth(depth); // Render in reverse with i=0 being on top

		if (i % 2 == 0) {
			 image.GetComponent<DrawableComponent>().SetTextureAsset<Resources::Textures::alpha_test>();
		}
		else {
			image.GetComponent<DrawableComponent>().SetTextureAsset<Resources::Textures::small_art>();
		}
	}

	// Create a window
	Entity& window1 = state.AddEntity<Smasher::Entity>();
	window1.AddComponent<Transform2DComponent>()
		.SetPosition(sf::Vector2f(400, 200))
		.SetScale(sf::Vector2f(1.0f, 1.0f));

	window1.AddComponent<DrawableComponent>()
		.SetShader(shader)
		.SetTextureAsset<Resources::Textures::window>()
		.SetClipRect(sf::IntRect{ 0, 0, 64, 64 })
		.SetDepth(0.6); // Window on top

	Entity& window2 = state.AddEntity<Smasher::Entity>();
	window2.AddComponent<Transform2DComponent>()
		.SetPosition(sf::Vector2f(420, 220))
		.SetScale(sf::Vector2f(1.0f, 1.0f));

	window2.AddComponent<DrawableComponent>()
		.SetShader(shader)
		.SetTextureAsset<Resources::Textures::window>()
		.SetClipRect(sf::IntRect{ 0, 0, 64, 64 })
		.SetDepth(0.5); // Window on bottom

	Entity& inWindowEntity = state.AddEntity<Smasher::Entity>();
	inWindowEntity.AddComponent<Transform2DComponent>()
		.SetPosition(sf::Vector2f(410, 210))
		.SetScale(sf::Vector2f(1.0f, 1.0f));

	inWindowEntity.AddComponent<DrawableComponent>()
		.SetShader(shader)
		.SetTextureAsset<Resources::Textures::alpha_test>()
		.SetClipRect(sf::IntRect{ 0, 0, 64, 64 })
		.SetDepth(0.65); // Window on bottom


	message.AddComponent<Transform2DComponent>();
	message.AddComponent<TextComponent>()
		.SetString("Hello World")
		.UseDefaults()
		.SetFontAsset<Resources::Fonts::arial>();

 	state.Activate();
	engine.Run();
}