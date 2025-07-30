#include "Core.h"
#include "ExampleWindowGameState.h"
#include "Components/Transform2DComponent.h"
int main() {
	Smasher::Engine engine(640, 420);
	ExampleWindowGameState& state = engine.AddState<ExampleWindowGameState>();
	Smasher::Entity& entity = state.AddEntity<Smasher::Entity>();
	entity.AddComponent<Smasher::Transform2DComponent>();
	state.Activate();
	engine.Run();
}