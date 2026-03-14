#include <numbers>
#include <cmath>
#include "Smasher/Core.h"
#include "Smasher/ECS.h"
#include "StressTestLayer.h"
#include "Manifest.h"
#include "Smasher/Components/Transform2DComponent.h"
#include "Smasher/Components/TextComponent.h"
#include "Smasher/ComponentManagers/DrawableComponentManager.h"
#include "Smasher/Components/DrawableComponent.h"
#include "Smasher/Components/CameraComponent.h"
#include "Smasher/Resources.h"
#include "BallComponent.h"

Smasher::Entity& StressTestLayer::SpawnBouncingBall(sf::Vector2i position)
{
	const float toRadian = (float)(180.0 / std::numbers::pi);
	int minSpeed = 100;
	int speedVariance = 100;
	float angle = (float)(rand() % 360);
	float speed = (float)(rand() % speedVariance + minSpeed);
	float tmpX = std::cos(angle * toRadian) * speed;
	float tmpY = std::sin(angle * toRadian) * speed;
	float depth = (float)(rand() % 100) / 100.0f;
	Smasher::Entity& image = AddEntity<Smasher::Entity>();
	image.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f((float)position.x, (float)position.y))
		.SetScale(sf::Vector2f(20.0f, 20.0f))
		.SetDepth(depth)
		.GetEntity()
		.AddComponent<BallComponent>()
		.SetVelocity(sf::Vector2f(tmpX, tmpY));
	return image;
}

void StressTestLayer::Init()
{
	m_OnMouseMoveHandle = Subscribe<Smasher::Events::MouseMoveEvent>(&StressTestLayer::OnMouseMove, this);

	Smasher::Entity& rCamera = AddEntity<Smasher::Entity>();
	// Add camera and apply it to window
	rCamera
		.AddComponent<Smasher::CameraComponent>()
			.SetSize(sf::Vector2f((float)GetEngine().GetWindow().getSize().x,
								  (float)GetEngine().GetWindow().getSize().y))
			.SetTarget(GetEngine().GetWindow())
			.ApplyToTarget();

	m_CameraPtr = &rCamera;

	// Half of the entities will have a non-alpha image
	// Half of the entities will have image with alpha pixels
	// The camera will follow the 10th entity
	for (std::size_t i = 0; i < m_NumEntities; ++i) {
		int positionX = int((rand() % 100) * 5);
		int positionY = int((rand() % 100) * 5);
		Smasher::Entity& image = SpawnBouncingBall(sf::Vector2i{ positionX, positionY });

		if (i % 2 == 0) {
			image.GetComponent<Smasher::DrawableComponent>()
				.SetTextureAsset<Smasher::Manifest::Textures::alpha_test>({});
		}
		else {
			image.GetComponent<Smasher::DrawableComponent>()
				.SetTextureAsset<Smasher::Manifest::Textures::small_art>({});
		}

		if (i == 10) {
			image.GetComponent<BallComponent>().SetCamera(rCamera.GetComponent<Smasher::CameraComponent>());
		}

	}
}

void StressTestLayer::Update(Smasher::Millisecond delta) {

}

void StressTestLayer::Render(sf::RenderWindow &window) {
}

// Spawn an entity at mouse position
void StressTestLayer::OnMouseMove(Smasher::Events::MouseMoveEvent &event) {
	sf::Window &window = GetEngine().GetWindow();
	sf::Rect<int> windowRect(0, 0, (int)window.getSize().x, (int)window.getSize().y);
	if (windowRect.contains(event.Position.x, event.Position.y)) {
		Smasher::Entity &ball = SpawnBouncingBall(event.Position);
		ball.GetComponent<Smasher::DrawableComponent>()
			.SetTextureAsset<Smasher::Manifest::Textures::alpha_test>({});
	}
}
