#include <numbers>
#include <SFML/Window.hpp>
#include "Core.h"
#include "Manifest.h"
#include "Smasher/Components/Transform2DWrapper.h"
#include "Smasher/Components/TextComponent.h"
#include "Smasher/ComponentManagers/DrawableComponentManager.h"
#include "Smasher/Components/DrawableComponent.h"
#include "Smasher/Entity.h"

#include "StressTestLayer.h"
#include "BallComponent.h"

using namespace Smasher;
int main(int argc, char **argv) {
	Smasher::Engine engine(640, 420);
	std::size_t numEntities = 20; // 100000;
	engine.GetResourceManager().SetResourceDirectory(Smasher::Resources::Metadata::RESOURCES_DIRECTORY);


	if (argc >= 2) {
		numEntities = std::size_t{ std::stoull(argv[1]) };
	}

	StressTestLayer& state = engine.PushLayer<StressTestLayer>(numEntities);

	state.Activate();
	engine.Run();
	return 0;
}
