#include "Smasher/Core.h"
#include "ExamplePhysicsLayer.h"
#include "Manifest.h"

int main() {
	Smasher::Engine engine{ 640, 420 };
	engine.GetResourceManager().SetResourceDirectory(Smasher::Manifest::Metadata::RESOURCES_DIRECTORY);

	ExamplePhysicsLayer &state = engine.PushLayer<ExamplePhysicsLayer>();

	state.Activate();
	engine.Run();
}