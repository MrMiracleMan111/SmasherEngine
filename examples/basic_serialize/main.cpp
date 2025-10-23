#include "Core.h"
#include "SavedComponent.h"
#include "SerializeLayer.h"
#include "Manifest.h"
#include "Smasher/ComponentManagers/DrawableComponentManager.h"

int main() {
	Smasher::Engine engine(640, 420);
	engine.GetResourceManager().SetResourceDirectory(Smasher::Resources::Metadata::RESOURCES_DIRECTORY);

	Smasher::Layer& state = engine.PushLayer<SerializeLayer>();
	state.Activate();
	engine.Run();
}