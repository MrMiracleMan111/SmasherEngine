#include "MidLayer.h"
#include "Smasher/Components/TextComponent.h"
#include "Manifest.h"
#include "Smasher/UI.h"

MidLayer::~MidLayer() {

}

void MidLayer::Init()
{
	std::shared_ptr<Smasher::ShaderResource> pShader = GetEngine().GetResourceManager().GetOrLoadResource<Smasher::Manifest::Shaders::basic_ui_shader, Smasher::ShaderResource>();
	Smasher::UIPanelComponentManager& rUIManager = static_cast<Smasher::UIPanelComponentManager&>(GetComponentManager<Smasher::UIPanelComponent>());
	sf::Vector2f windowSize = sf::Vector2f((float)GetEngine().GetWindow().getSize().x, (float)GetEngine().GetWindow().getSize().y);
	sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(GetEngine().GetWindow().getView().getTransform().getMatrix());
	pShader->GetShader().setUniform("windowSize", windowSize);
	pShader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);
	rUIManager.SetShaderResource(pShader);

	Smasher::Entity& entity = AddEntity();

	entity.AddComponent<Smasher::TextComponent>()
		.UseDefaults()
		.SetString("Mid Layer")
		.SetFillColor(sf::Color::Magenta)
		.SetPosition(130.0f, 200.0f)
		.SetFontAsset<Smasher::Manifest::Fonts::arial>();

	Smasher::Entity& uiPanel = AddEntity();

	std::shared_ptr<Smasher::TextureResource> pTexture = GetEngine().GetResourceManager().GetOrLoadResource<Smasher::Manifest::Textures::small_art, Smasher::TextureResource>();

	Smasher::UIPanelComponent& panel = uiPanel.AddComponent<Smasher::UIPanelComponent>()
		.SetColor(sf::Color::Yellow)
		.SetBorderRadius(Smasher::UIPanelCorner::LEFT, 40.0f)
		.SetBorderColor(sf::Color::Red)
		.SetTexture(pTexture)
		.SetBorderThickness(10.0f)
		.SetPosition(100.0f, 60.0f)
		.SetScale(100.0f, 200.0f)
		//.SetClipRect(sf::IntRect{0, 0, 100, 100})
		//.SetClipRotation(Smasher::Degrees{ 10.0f })
		.SetDepth(0.4f);
	panel.SetOnHoverCallback([&panel](Smasher::Events::MouseMoveEvent& event) {
		if ((bool)(panel.GetPanelState() & Smasher::UIPanelState::HOVERED)) {
			sf::Color tmp = sf::Color::Cyan;
			tmp.a = 40;
			panel.SetColor(tmp);
		}
		else {
			sf::Color tmp = sf::Color::Magenta;
			tmp.a = 40;
			panel.SetColor(tmp);
		}
	});

	m_OnMouseMove = Subscribe<Smasher::Events::MouseMoveEvent>([&panel](Smasher::Events::MouseMoveEvent& event) {
		//panel.SetPosition(sf::Vector2f(event.Position));
		panel.SetClipRotation(panel.GetClipRotation() + 1.0f);
		panel.SetRotation(panel.GetRotation() + 1.0f);
	});
}
