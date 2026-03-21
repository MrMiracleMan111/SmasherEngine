#include "Smasher/Base.h"
#include "Smasher/ComponentSystems/TextSystem.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ComponentSystems/EngineSystem.h"
#include "Smasher/Resources.h"
#include "Smasher/Engine.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	namespace TextSystem {
		ErrorCode Render(entt::registry& registry) {
			if (!registry.ctx().contains<EngineSystem::Context>()) {
				return ERROR_SystemNotInitialized;
			}

			Engine& engine = registry.ctx().get<EngineSystem::Context>().engineRef;
			auto& window = engine.GetWindow();
			auto view = registry.view<TransformSystem::Component, Component>();
			for (auto [entity, textTransform, textImage] : view.each()) {
				if (textImage.fontLoaded) {
					if (TransformSystem::HasChanged(textTransform)) {
						textImage.text.setPosition({ textTransform._position.x, textTransform._position.y });
					}
					window.draw(textImage.text);
				}
			}

			return ERROR_NoError;
		}

		ErrorCode Initialiaze(entt::registry& registry) {
			return ERROR_NoError;
		}

		Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity, std::shared_ptr<FontResource> font) {
			assert(registry.all_of<TransformSystem::Component>(entity) && "TextSystem::Component requires TransformSystem::Component");
			Component& component = registry.emplace<Component>(entity, font, font->GetFont(), false);
			UseDefaults(component);
			return std::ref(component);
		}

		ErrorCode UseDefaults(Component& component) {
			component.text.setCharacterSize(30);
			return ERROR_NoError;
		}

		ErrorCode SetFontAsset(Component& component, entt::registry& registry, ResourceId resourceId, const ResourcePath* paths, const std::size_t numPaths) {
			if (!registry.ctx().contains<EngineSystem::Context>()) {
				return ERROR_SystemNotInitialized;
			}
			Engine& engine = registry.ctx().get<EngineSystem::Context>().engineRef;
			ResourceManager& resourceManager = engine.GetResourceManager();
			component.font = resourceManager.template GetOrLoadResource<FontResource>(resourceId, paths, numPaths);
			component.text.setFont(component.font->GetFont());
			component.fontLoaded = true;
			return ERROR_NoError;
		}

		ErrorCode SetString(Component& component, const std::string& str) {
			component.text.setString(str);
			return ERROR_NoError;
		}

		ErrorCode SetFontSize(Component& component, unsigned int size) {
			component.text.setCharacterSize(size);
			return ERROR_NoError;
		}

		ErrorCode SetFillColor(Component& component, sf::Color color) {
			component.text.setFillColor(color);
			return ERROR_NoError;
		}

		ErrorCode SetOutlineThickness(Component& component, float thickness) {
			component.text.setOutlineThickness(thickness);
			return ERROR_NoError;
		}

		ErrorCode SetOutlineColor(Component& component, sf::Color color) {
			component.text.setOutlineColor(color);
			return ERROR_NoError;
		}
	}
}