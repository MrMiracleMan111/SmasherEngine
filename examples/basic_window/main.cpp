#include "Smasher/Core.h"
#include "ExampleWindowLayer.h"
#include "Smasher/Components/Transform2DComponent.h"
int main() {
	Smasher::Engine engine{ 640, 420 };
	ExampleWindowLayer &state = engine.PushLayer<ExampleWindowLayer>();
	Smasher::Entity &entity = state.AddEntity<Smasher::Entity>();
	entity.AddComponent<Smasher::Transform2DComponent>();
	state.Activate();
	engine.Run();
}