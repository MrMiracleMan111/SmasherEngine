#include "Core.h"
#include "Manifest.h"
#include "ExampleResourcesGameState.h"
#include "Components/DrawableComponent.h"
#include "Components/Transform2DComponent.h"
#include "Components/TextComponent.h"

int main() {
	Smasher::Engine engine(640, 420);
	engine.GetResourceManager().SetResourceDirectory(Resources::Metadata::RESOURCES_DIRECTORY);
	ExampleResourcesGameState& state = engine.AddState<ExampleResourcesGameState>();
	Entity& texture1 = state.AddEntity<Smasher::Entity>();
	Entity& message = state.AddEntity<Smasher::Entity>();

	texture1.AddComponent<Transform2DComponent>();
	texture1.AddComponent<DrawableComponent>().SetTextureAsset<Resources::Textures::small_art>();

	message.AddComponent<Transform2DComponent>();
	message.AddComponent<TextComponent>()
		.SetFontAsset<Resources::Fonts::arial>()
		.SetString("Hello World").UseDefaults();

	state.Activate();
	engine.Run();
}