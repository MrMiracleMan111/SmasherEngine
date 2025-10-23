#include "TopLayer.h"
#include "Components/TextComponent.h"
#include "Smasher/UI.h"
#include "Manifest.h"

TopLayer::~TopLayer()
{
}

void TopLayer::Init()
{
	Smasher::Entity& entity = AddEntity();
	Smasher::Entity& uiPanel = AddEntity();

	entity.AddComponent<Smasher::TextComponent>()
		.UseDefaults()
		.SetString("Top Layer")
		.SetFillColor(sf::Color::Red)
		.SetPosition(160.0f, 200.0f)
		.SetFontAsset<Smasher::Resources::Fonts::arial>();


	Smasher::UIPanelComponent& panel = uiPanel.AddComponent<Smasher::UIPanelComponent>()
		.SetColor(sf::Color::Yellow)
		.SetPosition(160.0f, 100.0f)
		.SetScale(100.0f, 100.0f)
		.SetPanelSettings(Smasher::UIPanelSettings::BLOCK_MOUSE_MOVE);
	panel.SetOnHoverCallback([&panel](Smasher::Events::MouseMoveEvent& event) {
		if ((bool)(panel.GetPanelState() & Smasher::UIPanelState::HOVERED)) {
			panel.SetColor(sf::Color::Cyan);
		}
		else {
			panel.SetColor(sf::Color::Magenta);
		}
	});
}
