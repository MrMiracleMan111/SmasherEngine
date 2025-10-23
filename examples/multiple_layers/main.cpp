#include "Core.h"
#include "TopLayer.h"
#include "MidLayer.h"
#include "BottomLayer.h"
#include "Manifest.h"
#include "Smasher/ComponentManagers/DrawableComponentManager.h"

int main() {
	Smasher::Engine engine(640, 420);

	engine.GetResourceManager().SetResourceDirectory(Smasher::Resources::Metadata::RESOURCES_DIRECTORY);

	BottomLayer& bottomLayer = engine.PushLayer<BottomLayer>();
	MidLayer& midLayer = engine.PushLayer<MidLayer>();
	TopLayer& topLayer = engine.PushLayer<TopLayer>();
	topLayer.Activate();
	midLayer.Activate();
	bottomLayer.Activate();

	engine.Run();
}