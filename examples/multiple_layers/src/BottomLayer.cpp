#include "BottomLayer.h"
#include "Smasher/Components/TextComponent.h"
#include "Manifest.h"
BottomLayer::~BottomLayer()
{
}

void BottomLayer::Init()
{
	Smasher::Entity& entity = AddEntity();

	entity.AddComponent<Smasher::TextComponent>()
		.UseDefaults()
		.SetString("Bottom Layer")
		.SetFillColor(sf::Color::White)
		.SetPosition(100.0f, 200.0f)
		.SetFontAsset<Smasher::Manifest::Fonts::arial>();
}
