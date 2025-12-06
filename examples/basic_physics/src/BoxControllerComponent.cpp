#include "BoxControllerComponent.h"
#include "Smasher/Drawable.h"
#include "Smasher/Physics.h"

void BoxControllerComponent::StaticUpdateComponent(BoxControllerComponent& self, const Smasher::Millisecond& delta) {
	Smasher::PhysicsComponent& physicsComp = self.GetSiblingComponent<Smasher::PhysicsComponent>();
	Smasher::DrawableComponent& drawableComp = self.GetSiblingComponent<Smasher::DrawableComponent>();
	drawableComp.SetPosition(physicsComp.GetPosition());
	drawableComp.SetRotation(physicsComp.GetRotation());
}