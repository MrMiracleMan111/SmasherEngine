#include "MidLayer.h"
#include "Smasher/Components/TextComponent.h"
#include "Manifest.h"
#include "Smasher/UI.h"

MidLayer::~MidLayer() {

}

void MidLayer::Init()
{
	Smasher::Entity& entity = AddEntity();

	entity.AddComponent<Smasher::TextComponent>()
		.UseDefaults()
		.SetString("Mid Layer")
		.SetFillColor(sf::Color::Magenta)
		.SetPosition(130.0f, 200.0f)
		.SetFontAsset<Smasher::Resources::Fonts::arial>();

	Smasher::Entity& uiPanel = AddEntity();

	Smasher::UIPanelComponent& panel = uiPanel.AddComponent<Smasher::UIPanelComponent>()
		.SetColor(sf::Color::Yellow)
		.SetPosition(100.0f, 60.0f)
		.SetScale(100.0f, 100.0f);
	panel.SetOnHoverCallback([&panel](Smasher::Events::MouseMoveEvent& event) {
		if ((bool)(panel.GetPanelState() & Smasher::UIPanelState::HOVERED)) {
			panel.SetColor(sf::Color::Cyan);
		}
		else {
			panel.SetColor(sf::Color::Magenta);
		}
	});
}
