#include "Core.h"
#include "TopLayer.h"
#include "MidLayer.h"
#include "BottomLayer.h"
#include "Manifest.h"
#include "DrawableComponentManager.h"

int main() {
	Smasher::Engine engine(640, 420);

	engine.GetResourceManager().SetResourceDirectory(Smasher::Resources::Metadata::RESOURCES_DIRECTORY);

	std::shared_ptr<Smasher::ShaderResource> shader = engine.GetResourceManager().GetOrLoadResource<Smasher::Resources::Shaders::basic_texture_shader, Smasher::ShaderResource>();
	sf::Vector2f windowSize = sf::Vector2f((float)engine.GetWindow().getSize().x, (float)engine.GetWindow().getSize().y);
	sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(engine.GetWindow().getView().getTransform().getMatrix());
	shader->GetShader().setUniform("windowSize", windowSize);
	shader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);


	// TODO: FIND A BETTER SOLUTION
	// I shouldn't have to worry about loading DrawableComponentManager after drawable component
	/*Smasher::DrawableComponentManager& rCompManager = static_cast<Smasher::DrawableComponentManager&>(layer.GetComponentManager<Smasher::DrawableComponent>());
	rCompManager.SetShaderResource(shader);*/

	BottomLayer& bottomLayer = engine.PushLayer<BottomLayer>();
	MidLayer& midLayer = engine.PushLayer<MidLayer>();
	TopLayer& topLayer = engine.PushLayer<TopLayer>();
	topLayer.Activate();
	midLayer.Activate();
	bottomLayer.Activate();

	engine.Run();
}