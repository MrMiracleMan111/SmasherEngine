#pragma once
#include "Smasher/Base.h"
#include "IComponent.h"
#include "ResourceManager.h"
#include "Smasher/EngineConfig.h"
#include "Transform2DWrapper.h"

namespace Smasher {
	class SMASHER_API TextComponent : public IComponent, public Transform2DWrapper<TextComponent> {
	public:
		TextComponent() : IComponent(), Transform2DWrapper(*this, m_Text) {
			m_Text.setCharacterSize(EngineConfig::DEFAULT_FONT_SIZE);
			m_Text.setFillColor(EngineConfig::DEFAULT_FONT_COLOR);
		}
		TextComponent(const TextComponent&) = default;
		TextComponent& operator=(const TextComponent&) = default;


		static void StaticRenderComponent(TextComponent& self, sf::RenderWindow& rWindow);

		void SetEntity(Entity& rEntity) override;

		TextComponent& UseDefaults() {
			m_Text.setCharacterSize(30);
			return *this;
		}

		template <class T>
		TextComponent& SetFontAsset() {
			auto& rResourceManager = GetEntity().GetEngine().GetResourceManager();
			m_Font = rResourceManager.template GetOrLoadResource<T, FontResource>();
			m_Text.setFont(m_Font->GetFont());
			m_FontLoaded = true;
			return *this;
		}

		TextComponent& SetString(const std::string& str);

		TextComponent& SetFontSize(unsigned int size);

		TextComponent& SetFillColor(sf::Color color);

		TextComponent& SetOutlineThickness(float thickness);

		TextComponent& SetOutlineColor(sf::Color color);


	private:
		std::shared_ptr<FontResource> m_Font;
		sf::Text m_Text;
		bool m_FontLoaded = false;
	};
}