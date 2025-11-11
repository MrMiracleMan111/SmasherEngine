#include "TopLayer.h"
#include "Smasher/Components/TextComponent.h"
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
		.SetFontAsset<Smasher::Manifest::Fonts::arial>();


	Smasher::UIPanelComponent& panel = uiPanel.AddComponent<Smasher::UIPanelComponent>()
		.SetBackgroundColor(sf::Color::Yellow)
		.SetPosition(160.0f, 100.0f)
		.SetScale(100.0f, 100.0f)
		.SetDepth(0.5f)
		.SetPanelSettings(Smasher::UIPanelSettings::BLOCK_MOUSE_MOVE);
	panel.SetOnHoverCallback([&panel](Smasher::Events::MouseMoveEvent& event) {
		if ((bool)(panel.GetPanelState() & Smasher::UIPanelState::HOVERED)) {
			panel.SetBackgroundColor(sf::Color::Cyan);
		}
		else {
			panel.SetBackgroundColor(sf::Color::Magenta);
		}
	});
}
