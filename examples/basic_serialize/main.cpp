#include "Core.h"
#include "SavedComponent.h"
#include "SerializeGameState.h"
#include "Manifest.h"
#include "ComponentManagers\DrawableComponentManager.h"

int main() {
	Smasher::Engine engine(640, 420);
	Smasher::GameState& state = engine.AddState<SerializeGameState>();
	state.Activate();
	engine.Run();
}