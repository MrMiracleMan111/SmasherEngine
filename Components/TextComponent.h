#pragma once
#include "Base.h"
#include "IComponent.h"
#include "Transform2DComponent.h"
#include "ResourceManager.h"
#include "EngineConfig.h"

namespace Smasher {
	class SMASHER_API TextComponent : public IComponent {
	public:
		TextComponent() : IComponent() {
			m_Text.setCharacterSize(EngineConfig::DEFAULT_FONT_SIZE);
			m_Text.setFillColor(EngineConfig::DEFAULT_FONT_COLOR);
		}
		TextComponent(const TextComponent&) = default;
		TextComponent& operator=(const TextComponent&) = default;


		static void StaticRenderComponent(TextComponent& self, sf::RenderWindow& rWindow);

		TextComponent& UseDefaults() {
			m_FontLoaded = true;
			return *this;
		}

		template <class T>
		TextComponent& SetFontAsset() {
			auto& rResourceManager = GetEntity().GetEngine().GetResourceManager();
			m_Font = rResourceManager.GetOrLoadResource<T, FontResource>();
			m_Text.setFont(m_Font->GetFont());
			m_FontLoaded = true;
			return *this;
		}

		TextComponent& SetString(const std::string& str) {
			m_Text.setString(str);
			return *this;
		}

		TextComponent& SetFontSize(unsigned int size) {
			m_Text.setCharacterSize(size);
			return *this;
		}


	private:
		std::shared_ptr<FontResource> m_Font;
		sf::Text m_Text;
		bool m_FontLoaded = false;
	};
}