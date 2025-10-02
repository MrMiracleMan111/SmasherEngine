#include <numbers>
#include <SFML/Window.hpp>
#include "Core.h"
#include "Manifest.h"
#include "Components/Transform2DComponent.h"
#include "Components/TextComponent.h"
#include "ComponentManagers/DrawableComponentManager.h"
#include "Components/DrawableComponent.h"
#include "Entity.h"

#include "StressTestLayer.h"
#include "BallComponent.h"

using namespace Smasher;
int main(int argc, char **argv) {
	Smasher::Engine engine(640, 420);
	std::size_t numEntities = 20;// 100000;

	if (argc >= 2) {
		numEntities = std::size_t{ std::stoull(argv[1]) };
	}

	engine.GetResourceManager().SetResourceDirectory(Resources::Metadata::RESOURCES_DIRECTORY);
	StressTestLayer& state = engine.PushLayer<StressTestLayer>(numEntities);

	state.Activate();
	engine.Run();
	return 0;
}