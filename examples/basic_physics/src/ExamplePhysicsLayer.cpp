#include "ExamplePhysicsLayer.h"
#include "Smasher/AABBPhysics.h"
#include "Smasher/Drawable.h"
#include "BoxControllerComponent.h"
#include "Manifest.h"

ExamplePhysicsLayer::~ExamplePhysicsLayer()
{
	m_KeyPressSubscription.Unsubscribe();
}

void ExamplePhysicsLayer::Init() {
	m_KeyPressSubscription = Subscribe<Smasher::Events::KeyboardEvent>(&ExamplePhysicsLayer::OnKeyPress, this);

	Smasher::Entity& box = AddEntity();

	box.AddComponent<Smasher::AABBPhysicsComponent>()
		.MakeStatic()
		.SetPosition(sf::Vector2f(200.0f, 300.0f))
		.SetScale(sf::Vector2f(100.0f, 100.0f));
	box.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(box.GetComponent<Smasher::AABBPhysicsComponent>().GetPosition())
		.SetScale(box.GetComponent<Smasher::AABBPhysicsComponent>().GetScale())
		.SetTextureAsset<Smasher::Manifest::Textures::small_art>({})
		.SetColor(sf::Color::Red);

	Smasher::Entity& moveBox = AddEntity();
	moveBox.AddComponent<Smasher::AABBPhysicsComponent>()
		.MakeDynamic()
		.SetPosition(sf::Vector2f(400.0f, 250.0f))
		.SetScale(sf::Vector2f(100.0f, 100.0f))
		.SetAcceleration(sf::Vector2f(-50.0f, 0.0f));
	moveBox.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(box.GetComponent<Smasher::AABBPhysicsComponent>().GetPosition())
		.SetScale(box.GetComponent<Smasher::AABBPhysicsComponent>().GetScale())
		.SetColor(sf::Color::Yellow)
		.SetTextureAsset<Smasher::Manifest::Textures::small_art>({});
	moveBox.AddComponent<BoxControllerComponent>();

	m_MousePressSubscription = Subscribe<Smasher::Events::MouseButtonEvent>(
		[&moveBox](Smasher::Events::MouseButtonEvent& event) {
			sf::Vector2f pos = moveBox.GetComponent<Smasher::AABBPhysicsComponent>().GetPosition();
			/*moveBox.GetComponent<Smasher::AABBPhysicsComponent>()
				.Move(sf::Vector2f(event.Position) - pos);*/

			moveBox.GetComponent<Smasher::AABBPhysicsComponent>()
				.MoveTo(sf::Vector2f(event.Position));
			
			std::cout << "Mouse Click X: " << event.Position.x << " Y: " << event.Position.y << std::endl;
			/*moveBox.GetComponent<Smasher::DrawableComponent>()
				.SetPosition(sf::Vector2f(event.Position));*/
		});
}

void ExamplePhysicsLayer::Reset() {

}

void ExamplePhysicsLayer::Render(sf::RenderWindow& window) {

}

void ExamplePhysicsLayer::OnKeyPress(Smasher::Events::KeyboardEvent& e) {
	std::string type = "NA";
	switch (e.Type) {
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
	std::cout << "Key Event: " << type << " Key Code: " << e.KeyCode << std::endl;
}