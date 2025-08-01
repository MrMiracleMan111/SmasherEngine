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

	std::shared_ptr<ShaderResource> shader = engine.GetResourceManager().GetOrLoadResource<Resources::Shaders::basic_texture_shader, ShaderResource>();
	sf::Vector2f windowSize = sf::Vector2f(engine.GetWindow().getSize().x , engine.GetWindow().getSize().y);
	shader->GetShader().setUniform("windowSize", windowSize);

	for (size_t i = 0; i < 10; ++i) {
		Entity& image = state.AddEntity<Smasher::Entity>();
		image.AddComponent<Transform2DComponent>()
			.SetPosition(sf::Vector2f(i * 10, i * 10))
			.SetScale(sf::Vector2f(1.0f, 1.0f));
		image.AddComponent<DrawableComponent>()
			.SetShader(shader)
			.SetTextureAsset<Resources::Textures::small_art>()
			.SetDepth(1.0f - ((float)(i) / 10.0f)); // Render in reverse with i=0 being on top
	}
	
	/*message.AddComponent<Transform2DComponent>();
	message.AddComponent<TextComponent>()
		.SetString("Hello World")
		.UseDefaults()
		.SetFontAsset<Resources::Fonts::arial>();*/

 	state.Activate();
	engine.Run();
}