#include "Core.h"
#include "SavedComponent.h"
#include "SerializeLayer.h"
#include "Manifest.h"
#include "ComponentManagers/DrawableComponentManager.h"

int main() {
	Smasher::Engine engine(640, 420);
	Smasher::Layer& state = engine.PushLayer<SerializeLayer>();
	state.Activate();
	engine.Run();
}