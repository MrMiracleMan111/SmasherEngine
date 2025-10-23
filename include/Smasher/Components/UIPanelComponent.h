#pragma once
#include <functional>
#include "Smasher/Base.h"
#include "Smasher/IComponent.h"
#include "Smasher/ResourceManager.h"
#include "Smasher/EngineConfig.h"
#include "Transform2DWrapper.h"
#include "Smasher/EventManager.h"
#include "Smasher/Events.h"


namespace Smasher {
	class UIPanelComponentManager;

	enum class UIPanelSettings : unsigned char {
		NONE = 0,			// Base state
		BLOCK_MOUSE_MOVE = 1 << 0,
		BLOCK_MOUSE_PRESS = 1 << 1
	};

	inline UIPanelSettings operator &(const UIPanelSettings& lhs, const UIPanelSettings& rhs) {
		return static_cast<UIPanelSettings>(static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs));
	}

	inline UIPanelSettings operator |(const UIPanelSettings& lhs, const UIPanelSettings& rhs) {
		return static_cast<UIPanelSettings>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
	}

	inline UIPanelSettings operator !(const UIPanelSettings& lhs) {
		return static_cast<UIPanelSettings>(!static_cast<unsigned char>(lhs));
	}

	inline UIPanelSettings operator ~(const UIPanelSettings& lhs) {
		return static_cast<UIPanelSettings>(~static_cast<unsigned char>(lhs));
	}

	enum class UIPanelState : unsigned char {
		NONE = 0,			// Base state
		PRESSED = 1 << 0,
		HOVERED = 1 << 1
	};

	inline UIPanelState operator &(const UIPanelState& lhs, const UIPanelState& rhs) {
		return static_cast<UIPanelState>(static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs));
	}

	inline UIPanelState operator |(const UIPanelState& lhs, const UIPanelState& rhs) {
		return static_cast<UIPanelState>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
	}

	inline UIPanelState operator !(const UIPanelState& lhs) {
		return static_cast<UIPanelState>(!static_cast<unsigned char>(lhs));
	}

	inline UIPanelState operator ~(const UIPanelState& lhs) {
		return static_cast<UIPanelState>(~static_cast<unsigned char>(lhs));
	}

	class SMASHER_API UIPanelComponent : public IComponent, public Transform2DWrapper<UIPanelComponent> {
		friend class UIPanelComponentManager;

		SMASHER_USE_COMPONENT_MANAGER(UIPanelComponentManager)
		
	public:
		UIPanelComponent();
		UIPanelComponent(const UIPanelComponent&) = default;
		UIPanelComponent& operator=(const UIPanelComponent&) = default;

		static void StaticRenderComponent(UIPanelComponent& self, sf::RenderWindow& rWindow);

		// Checks if window coordinate intersects the panel
		bool IntersectsPanel(int x, int y);

		UIPanelComponent& SetColor(const sf::Color& color);

		const UIPanelState& GetPanelState() const { return m_PanelState; };
		void SetPanelState(UIPanelState state) { m_PanelState = state; }

		const UIPanelSettings& GetPanelSettings() const { return m_PanelSettings; };
		UIPanelComponent& SetPanelSettings(UIPanelSettings settings) { m_PanelSettings = settings; return *this; }

		UIPanelComponent& SetOnHoverCallback(std::function<void(Events::MouseMoveEvent&)> callback);
		template<class T>
		UIPanelComponent& SetOnHoverCallback(void (T::* method)(Events::MouseMoveEvent&), T* instance);

		UIPanelComponent& SetOnPressCallback(std::function<void(Events::MouseButtonEvent&)> callback);
		template<class T>
		UIPanelComponent& SetOnPressCallback(void (T::* method)(Events::MouseButtonEvent&), T* instance);

	protected:
		void OnHoverEvent(Events::MouseMoveEvent& event);
		void OnPressEvent(Events::MouseButtonEvent& event);

	private:
		UIPanelState m_PanelState = UIPanelState::NONE;
		UIPanelSettings m_PanelSettings = UIPanelSettings::NONE;
		sf::RectangleShape m_DebugSprite;
		sf::Transformable m_Transformable;
		std::function<void(Events::MouseMoveEvent&)> m_MouseMoveCallback;
		std::function<void(Events::MouseButtonEvent&)> m_MousePressCallback;
	};
}

#include "Smasher/Components/UIPanelComponent.inl"