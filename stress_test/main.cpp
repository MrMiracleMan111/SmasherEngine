#include <numbers>
#include <SFML/Window.hpp>
#include "Core.h"
#include "Manifest.h"
#include "Components/Transform2DComponent.h"
#include "Components/TextComponent.h"
#include "ComponentManagers/DrawableComponentManager.h"
#include "Components/DrawableComponent.h"
#include "Entity.h"

#include "StressTestGameState.h"
#include "BallComponent.h"

using namespace Smasher;
int main() {
	Smasher::Engine engine(640, 420);

	engine.GetResourceManager().SetResourceDirectory(Resources::Metadata::RESOURCES_DIRECTORY);
	StressTestGameState& state = engine.AddState<StressTestGameState>();

	std::shared_ptr<ShaderResource> shader = engine.GetResourceManager().GetOrLoadResource<Resources::Shaders::basic_texture_shader, ShaderResource>();
	sf::Vector2f windowSize = sf::Vector2f(engine.GetWindow().getSize().x, engine.GetWindow().getSize().y);
	sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(engine.GetWindow().getView().getTransform().getMatrix());
	shader->GetShader().setUniform("windowSize", windowSize);
	shader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);

	// TODO: FIND A BETTER SOLUTION
// I shouldn't have to worry about loading DrawableComponentManager after drawable component
	DrawableComponentManager& rCompManager = static_cast<DrawableComponentManager&>(state.GetComponentManager<DrawableComponent>());
	rCompManager.SetShaderResource(shader);

	// Create 10 entites
	// Half will have a non-alpha image
	// Half will have image with alpha pixels
	const std::size_t numEntities = 20000;
	const float toRadian = (float)(180.0 / std::numbers::pi);
	for (int i = 0; i < numEntities; ++i) {
		float angle = (float)(rand() % 360);
		float speed = (float)(rand() % 100 + 100);
		float tmpX = cos(angle * toRadian) * speed;
		float tmpY = sin(angle * toRadian) * speed;
		float depth = (float)(rand() % 100) / 100.0f;
		float positionX = (float)((rand() % 100) * 5);
		float positionY = (float)((rand() % 100) * 5);
		Entity& image = state.AddEntity<Smasher::Entity>();
		image.AddComponent<Transform2DComponent>()
			.SetPosition(sf::Vector2f(positionX, positionY))
			.SetScale(sf::Vector2f(20.0f, 20.0f));

		image.AddComponent<DrawableComponent>()
			.SetShader(shader)
			.SetDepth(depth);

		image.AddComponent<BallComponent>()
			.SetVelocity(sf::Vector2f(tmpX, tmpY));

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