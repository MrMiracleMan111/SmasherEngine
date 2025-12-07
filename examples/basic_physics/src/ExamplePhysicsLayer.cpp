#include <cmath>
#include "ExamplePhysicsLayer.h"
#include "Smasher/Physics.h"
#include "Smasher/Drawable.h"
#include "BoxControllerComponent.h"
#include "Manifest.h"

ExamplePhysicsLayer::~ExamplePhysicsLayer()
{
	m_KeyPressSubscription.Unsubscribe();
}

void ExamplePhysicsLayer::Init() {
	GetEngine().GetPhysicsManager().Initialize();
	m_KeyPressSubscription = Subscribe<Smasher::Events::KeyboardEvent>(&ExamplePhysicsLayer::OnKeyPress, this);
	
	Smasher::Entity &box = AddEntity();

	box.AddComponent<Smasher::PhysicsComponent>()
		.SetPhysicsType(Smasher::PhysicsType::STATIC)
		.SetPosition(sf::Vector2f{ 200.0f, 300.0f })
		.UseRectCollider(100.f, 100.f)
		.SetOnCollisionCallback([](Smasher::PhysicsCollision collision) {
			std::cout << "Static Box hit something (this message should not appear)" << std::endl;
		})
		;
	box.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(box.GetComponent<Smasher::PhysicsComponent>().GetPosition())
		.SetTextureAsset<Smasher::Manifest::Textures::small_art>({})
		.SetScale(sf::Vector2f{ 100.f, 100.f })
		.SetColor(sf::Color::Red);

	Smasher::Entity &moveBox = AddEntity();
	moveBox.AddComponent<Smasher::PhysicsComponent>()
		.SetPhysicsType(Smasher::PhysicsType::DYNAMIC)
		.UseRectCollider(100.f, 100.f)
		.SetPosition(sf::Vector2f{ 400.0f, 250.0f })
		.SetOnCollisionCallback([](Smasher::PhysicsCollision collision) {
			std::cout << "Dynamic Box hit something" << std::endl;
		});
	moveBox.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(box.GetComponent<Smasher::PhysicsComponent>().GetPosition())
		.SetColor(sf::Color::Yellow)
		.SetScale(sf::Vector2f{ 100.f, 100.f })
		.SetTextureAsset<Smasher::Manifest::Textures::small_art>({});
	moveBox.AddComponent<BoxControllerComponent>();

	m_MousePressSubscription = Subscribe<Smasher::Events::MouseButtonEvent>(
		[&moveBox](Smasher::Events::MouseButtonEvent &event) {
			sf::Vector2f pos = moveBox.GetComponent<Smasher::PhysicsComponent>().GetPosition();

			moveBox.GetComponent<Smasher::PhysicsComponent>()
				.SetPosition(sf::Vector2f{ event.Position });
			
			std::cout << "Mouse Click X: " << event.Position.x << " Y: " << event.Position.y << std::endl;
		});

	m_MouseMoveSubscription = Subscribe<Smasher::Events::MouseMoveEvent>(
		[&moveBox](Smasher::Events::MouseMoveEvent &event) {
			sf::Vector2f pos = moveBox.GetComponent<Smasher::PhysicsComponent>().GetPosition();
			sf::Vector2f mousePos{ event.Position };

			float length = (float)std::sqrt(std::pow((mousePos.x - pos.x), 2) + std::pow((mousePos.y - pos.y), 2));
			sf::Vector2f direction = (mousePos - pos) / length;
			moveBox.GetComponent<Smasher::PhysicsComponent>()
				.SetVelocity(BoxControllerComponent::VELOCITY * direction);
		});
}

void ExamplePhysicsLayer::Reset() {

}

void ExamplePhysicsLayer::Render(sf::RenderWindow &window) {

}

void ExamplePhysicsLayer::OnKeyPress(Smasher::Events::KeyboardEvent &event) {
	std::string type = "NA";
	switch (event.Type) {
		case Smasher::Keyboard::KeyboardEventType::KEY_PRESS:
			type = "KEY PRESS";
		break;
		case Smasher::Keyboard::KeyboardEventType::KEY_RELEASE:
			type = "KEY RELEASE";
		break;
		case Smasher::Keyboard::KeyboardEventType::KEY_HOLD:
			type = "KEY HOLD";
		break;
	}
	std::cout << "Key Event: " << type << " Key Code: " << event.KeyCode << std::endl;
}