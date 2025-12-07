#include "TopLayer.h"
#include "Smasher/Components/TextComponent.h"
#include "Smasher/UI.h"
#include "Smasher/Drawable.h"
#include "Manifest.h"

TopLayer::~TopLayer()
{
}

void TopLayer::Init()
{
	Smasher::DrawableComponentManager &drawableMgr = static_cast<Smasher::DrawableComponentManager&>(GetComponentManager<Smasher::DrawableComponent>());
	std::shared_ptr<Smasher::ShaderResource> pShader = GetEngine().GetResourceManager().GetOrLoadResource<Smasher::Manifest::Shaders::basic_texture_shader, Smasher::ShaderResource>();
	sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(GetEngine().GetWindow().getView().getTransform().getMatrix());
	pShader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);
	drawableMgr.SetShaderResource(pShader);

	Smasher::Entity &entity = AddEntity();
	Smasher::Entity &uiPanel = AddEntity();

	entity.AddComponent<Smasher::TextComponent>()
		.UseDefaults()
		.SetString("Top Layer")
		.SetFillColor(sf::Color::Red)
		.SetPosition(160.0f, 200.0f)
		.SetFontAsset<Smasher::Manifest::Fonts::arial>();


	AddEntity().AddComponent<Smasher::DrawableComponent>()
		.SetScale(sf::Vector2f(100.0f, 200.0f))
		.SetOrigin(0.0f, 1.f)
		.SetPosition(sf::Vector2f(50.f, 370.f))
		//.SetPosition(sf::Vector2f(0.f, 0.f))
		.SetColor(sf::Color::Magenta)
		.SetTextureAsset<Smasher::Manifest::Textures::small_art>({})
		.SetDepth(0.25f);


	Smasher::UIPanelComponent &panel = uiPanel.AddComponent<Smasher::UIPanelComponent>()
		.SetBackgroundColor(sf::Color::Yellow)
		.SetPosition(160.0f, 100.0f)
		.SetScale(100.0f, 100.0f)
		.SetDepth(0.5f)
		.SetPanelSettings(Smasher::UIPanelSettings::BLOCK_MOUSE_MOVE);
	panel.SetOnHoverCallback([&panel](Smasher::Events::MouseMoveEvent &event) {
		if ((bool)(panel.GetPanelState() & Smasher::UIPanelState::HOVERED)) {
			panel.SetBackgroundColor(sf::Color::Cyan);
		}
		else {
			panel.SetBackgroundColor(sf::Color::Magenta);
		}
	});
}
