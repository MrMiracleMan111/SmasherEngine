#include <numbers>
#include <SFML/Window.hpp>
#include "Smasher/Core.h"
#include "Smasher/ECS.h"
#include "Manifest.h"
#include "Smasher/Resources.h"
#include "Smasher/Components/Transform2DWrapper.h"
#include "Smasher/Components/TextComponent.h"
#include "Smasher/ComponentManagers/DrawableComponentManager.h"
#include "Smasher/Components/DrawableComponent.h"

#include "StressTestLayer.h"
#include "StatisticsLayer.h"
#include "BallComponent.h"


using namespace Smasher;
int main(int argc, char **argv) {
	std::cout << "Resource Directory: " << Smasher::Manifest::Metadata::RESOURCES_DIRECTORY << std::endl;

	Smasher::Engine engine{ 640, 420 };
	std::size_t numEntities = 100000;// 20; // 10000;
	engine.GetResourceManager().SetResourceDirectory(Smasher::Manifest::Metadata::RESOURCES_DIRECTORY);
	Smasher::ResourceManager& resourceManager = engine.GetResourceManager();
	auto pShaderResource = resourceManager.GetOrLoadResource<Smasher::Manifest::Shaders::basic_texture_shader, Smasher::ShaderResource>();
	sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(engine.GetWindow().getView().getTransform().getMatrix());
	pShaderResource->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);

	if (argc >= 2) {
		numEntities = std::size_t{ std::stoull(argv[1]) };
	}

	StressTestLayer& state = engine.PushLayer<StressTestLayer>(numEntities);
	StatisticsLayer& statistics = engine.PushLayer<StatisticsLayer>(state);

	static_cast<DrawableComponentManager&>(state.GetComponentManager<DrawableComponent>()).SetShaderResource(pShaderResource);

	state.Activate();
	statistics.Activate();
	engine.Run();
	return 0;
}
