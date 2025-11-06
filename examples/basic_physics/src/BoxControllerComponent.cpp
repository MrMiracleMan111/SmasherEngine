#include "BoxControllerComponent.h"
#include "Smasher/AABBPhysics.h"
#include "Smasher/Drawable.h"

void BoxControllerComponent::StaticUpdateComponent(BoxControllerComponent& self, Smasher::Millisecond& delta) {
	Smasher::AABBPhysicsComponent& physicsComponent = self.GetEntity().GetComponent<Smasher::AABBPhysicsComponent>();
	Smasher::DrawableComponent& drawableComponent = self.GetEntity().GetComponent<Smasher::DrawableComponent>();

	sf::RenderWindow& rWindow = self.GetEntity().GetLayer().GetEngine().GetWindow();
	sf::Vector2f position(sf::Mouse::getPosition(rWindow));
	sf::Vector2f direction = position - self.GetEntity().GetComponent<Smasher::AABBPhysicsComponent>().GetPosition();
	float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	direction /= length;
	direction *= self.velocity * ((float)delta.count() / 1000.f);
	physicsComponent.Move(direction);
}

void BoxControllerComponent::StaticRenderComponent(BoxControllerComponent& self, sf::RenderWindow& rWindow) {
	Smasher::AABBPhysicsComponent& physicsComponent = self.GetEntity().GetComponent<Smasher::AABBPhysicsComponent>();
	Smasher::DrawableComponent& drawableComponent = self.GetEntity().GetComponent<Smasher::DrawableComponent>();

	//sf::Vector2f pos = physicsComponent.GetOldPosition();
	sf::Vector2f pos = physicsComponent.GetPosition();
	//std::cout << "Drawn at position X: " << pos.x << " Y: " << pos.y << std::endl;
	drawableComponent.SetPosition(pos);
}
