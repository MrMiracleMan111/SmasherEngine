#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entity/registry.hpp>
#include "Smasher/Base.h"
#include "Smasher/ErrorCodes.h"
#include "Smasher/Resources.h"

namespace Smasher {
	namespace TextSystem {
		struct Component {
			std::shared_ptr<FontResource> font;
			sf::Text text;
			bool fontLoaded = false;
		};

		SMASHER_API ErrorCode Render(entt::registry& registry);

		SMASHER_API ErrorCode Initialiaze(entt::registry& registry);

		SMASHER_API Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity, std::shared_ptr<FontResource> font);

		SMASHER_API ErrorCode UseDefaults(Component& component);

		template <class ManifestData>
		ErrorCode SetFontAsset(Component& component, entt::registry& registry);

		SMASHER_API ErrorCode SetFontAsset(Component& component, entt::registry& registry, ResourceId resourceId, const ResourcePath* paths, const std::size_t numPaths);

		SMASHER_API ErrorCode SetString(Component& component, const std::string& str);

		SMASHER_API ErrorCode SetFontSize(Component& component, unsigned int size);

		SMASHER_API ErrorCode SetFillColor(Component& component, sf::Color color);

		SMASHER_API ErrorCode SetOutlineThickness(Component& component, float thickness);

		SMASHER_API ErrorCode SetOutlineColor(Component& component, sf::Color color);
	}
}

#include "Smasher/ComponentSystems/TextSystem.inl"