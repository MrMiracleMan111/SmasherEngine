#include <cstdlib>
#include <SFML/Window.hpp>
#include "Smasher/Core.h"
#include "Manifest.h"
#include "ExampleResourcesLayer.h"
#include "Smasher/Components/Transform2DComponent.h"
#include "Smasher/Components/TextComponent.h"
#include "Smasher/ComponentManagers/DrawableComponentManager.h"
#include "Smasher/Components/DrawableComponent.h"

int main() {
	Smasher::Engine engine(640, 420);
	engine.GetResourceManager().SetResourceDirectory(Smasher::Manifest::Metadata::RESOURCES_DIRECTORY);

	ExampleResourcesLayer& exampleLayer = engine.PushLayer<ExampleResourcesLayer>();

	Smasher::Entity& image = exampleLayer.AddEntity<Smasher::Entity>();
	image.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(500.0f, 300.0f))
		.SetScale(sf::Vector2f(400.0f, 400.0f))
		.SetTextureAsset<Smasher::Manifest::Textures::small_art>({}) // Default texture options
		//.SetClipRect(sf::IntRect{ 0, 0, 64, 64 })
		.SetClipRotation(Smasher::Degrees{ 30 })
		.SetDepth(0.2f);

	Smasher::Entity& clipTest = exampleLayer.AddEntity<Smasher::Entity>();
	clipTest.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(50.0f, 200.0f))
		.SetScale(sf::Vector2f(100.0f, 75.0f))
		.SetTextureAsset<Smasher::Manifest::Textures::clip_test>({}) // Default texture options
		.SetClipRect(sf::IntRect{ 0, 0, 24, 18 })
		//.SetClipRotation(Smasher::Degrees{ 30 })
		.SetDepth(0.1f);

	// Create drawable component instance without texture
	Smasher::Entity& shape = exampleLayer.AddEntity<Smasher::Entity>();
	shape.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(50.0f, 300.0f))
		.SetScale(sf::Vector2f(100.0f, 100.0f))
		//.SetClipRect(sf::IntRect{ 0, 0, 64, 64 })
		.SetColor(sf::Color::Magenta)
		//.SetClipRotation(Smasher::Degrees{ 30 })
		.SetDepth(0.25f);

	// Create 10 entites
	// Half will have a non-alpha image
	// Half will have image with alpha pixels
	const std::size_t numEntities = 10;
	for (int i = 0; i < numEntities; ++i) {
		float depth = (float)(std::rand() % 100) / 100.0f;

		int separation = 20;
		int offset = 100;
		Smasher::Entity& image = exampleLayer.AddEntity<Smasher::Entity>();
		image.AddComponent<Smasher::DrawableComponent>()
			.SetPosition(sf::Vector2f((float)(i * separation + offset), (float)(i * separation + offset)))
			.SetScale(sf::Vector2f(100.0f, 100.0f))
			.SetDepth(depth);

		if (i % 2 == 0) {
			image.GetComponent<Smasher::DrawableComponent>()
				.SetTextureAsset<Smasher::Manifest::Textures::alpha_test>({})
				.SetClipRect(sf::IntRect{ 0, 0, 30, 20 });
		}
		else {
			image.GetComponent<Smasher::DrawableComponent>()
				.SetTextureAsset<Smasher::Manifest::Textures::small_art>({})
				.SetClipRect(sf::IntRect{ 0, 0, 30, 20 });
		}

	}

	// Create a window
	Smasher::Entity& window1 = exampleLayer.AddEntity<Smasher::Entity>();
	window1.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(400, 200))
		.SetScale(sf::Vector2f(150.0f, 150.0f))
		.SetTextureAsset<Smasher::Manifest::Textures::window>({.transluscent = true})
		.SetClipRect(sf::IntRect{ 10, 0, 64, 64 })
		.SetClipRotation(Smasher::Degrees{ 30 })
		.SetDepth(0.6f);

	Smasher::Entity& window2 = exampleLayer.AddEntity<Smasher::Entity>();
	window2.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(420, 220))
		.SetScale(sf::Vector2f(150.0f, 150.0f))
		.SetTextureAsset<Smasher::Manifest::Textures::window>({.transluscent = true})
		//.SetClipRect(sf::IntRect{ 0, 0, 64, 64 })
		.SetDepth(0.5);

	Smasher::Entity& inWindowEntity = exampleLayer.AddEntity<Smasher::Entity>();
	inWindowEntity.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(410, 210))
		.SetScale(sf::Vector2f(100.0f, 100.0f))
		.SetTextureAsset<Smasher::Manifest::Textures::alpha_test>({.transluscent = true})
		.SetClipRotation(Smasher::Degrees{ 30 })
		.SetDepth(0.55f);



	Smasher::Entity& message = exampleLayer.AddEntity<Smasher::Entity>();
	message.AddComponent<Smasher::TextComponent>()
		.SetString("Hello World")
		.UseDefaults()
		.SetFontAsset<Smasher::Manifest::Fonts::arial>();

 	exampleLayer.Activate();
	engine.Run();
	return 0;
}