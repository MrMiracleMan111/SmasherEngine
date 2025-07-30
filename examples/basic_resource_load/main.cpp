#include "Components/DrawableComponent.h"
#include "Components/Transform2DComponent.h"
#include "Components/TextComponent.h"
#include "Core.h"
#include "Manifest.h"
#include "ExampleResourcesGameState.h"


int main() {
	Smasher::Engine engine(640, 420);
	engine.GetResourceManager().SetResourceDirectory(Resources::Metadata::RESOURCES_DIRECTORY);
	ExampleResourcesGameState& state = engine.AddState<ExampleResourcesGameState>();
	Entity& message = state.AddEntity<Smasher::Entity>();

	for (size_t i = 0; i < 10; ++i) {
		Entity& image = state.AddEntity<Smasher::Entity>();
		image.AddComponent<Transform2DComponent>().SetPosition(sf::Vector2f(i * 10, i * 10));
		image.AddComponent<DrawableComponent>()
			.SetTextureAsset<Resources::Textures::small_art>()
			.SetDepth(i / 10); // Render in reverse with i=0 being on top
	}
	
	message.AddComponent<Transform2DComponent>();
	message.AddComponent<TextComponent>()
		.SetString("Hello World")
		.UseDefaults()
		.SetFontAsset<Resources::Fonts::arial>();

 	state.Activate();
	engine.Run();
}