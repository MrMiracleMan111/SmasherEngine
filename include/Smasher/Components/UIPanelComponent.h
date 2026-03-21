#pragma once
#include <functional>
#include <GL/glew.h>
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__linux__)
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#endif
#include "Smasher/Base.h"
#include "Smasher/IComponent.h"
#include "Smasher/ResourceManager.h"
#include "Smasher/EngineConfig.h"
#include "Transform2DWrapper.h"
#include "Smasher/EventManager.h"
#include "Smasher/Events.h"


namespace Smasher {
	class UIPanelComponentManager;

	struct UIPanelData {
		float position_rotation[4] = { 0.f }; //x, y, z
		float scale_borderThickness[3] = { 0.f }; // x, y, z
		Mat3 texTransform = Mat3{};
		uint32_t backgroundColors[4] = { 0 };
		uint32_t borderColors[4] = { 0 };
		float borderRadius[4] = { 0.f }; // top left, top right, bottom right, bottom left
		uint32_t hasTexture = (uint32_t)false;
	};

	enum class UIPanelCorner : unsigned char {
		TOP_LEFT = 1,
		TOP_RIGHT = 1 << 2,
		BOTTOM_RIGHT = 1 << 3,
		BOTTOM_LEFT = 1 << 4,
		BOTTOM = BOTTOM_RIGHT | BOTTOM_LEFT,
		TOP = TOP_RIGHT | TOP_LEFT,
		LEFT = TOP_LEFT | BOTTOM_LEFT,
		RIGHT = TOP_RIGHT | BOTTOM_RIGHT,
		ALL = TOP_RIGHT | TOP_LEFT | BOTTOM_RIGHT | BOTTOM_LEFT
	};

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

		static void StaticRenderComponent(UIPanelComponent &self, sf::RenderWindow &window);

		// Checks if window coordinate intersects the panel
		bool IntersectsPanel(int x, int y);

		UIPanelComponent& SetClipRect(sf::IntRect clipRect);

		UIPanelComponent& SetClipRotation(Degrees angle);

		UIPanelComponent& SetBorderRadius(float radius);

		UIPanelComponent& SetBorderRadius(UIPanelCorner corner, float radius);

		UIPanelComponent& SetBorderThickness(float thickness);

		UIPanelComponent& SetBorderColor(const sf::Color &color);

		UIPanelComponent& SetBorderColor(UIPanelCorner corner, const sf::Color &color);

		UIPanelComponent& SetBackgroundColor(const sf::Color &color);

		UIPanelComponent& SetBackgroundColor(UIPanelCorner corner, const sf::Color &color);

		UIPanelComponent& SetDepth(float depth);

		UIPanelComponent& SetTexture(std::shared_ptr<Smasher::TextureResource> pTexture);

		UIPanelComponent& SetPanelSettings(UIPanelSettings settings) { m_PanelSettings = settings; return *this; }

		UIPanelComponent& SetOnHoverCallback(std::function<void(Events::MouseMoveEvent&)> callback);

		template<class T>
		UIPanelComponent& SetOnHoverCallback(void (T:: *method)(Events::MouseMoveEvent&), T *pInstance);

		UIPanelComponent& SetOnPressCallback(std::function<void(Events::MouseButtonEvent&)> callback);
		
		template<class T>
		UIPanelComponent& SetOnPressCallback(void (T:: *method)(Events::MouseButtonEvent&), T *pInstance);

		void SetPanelState(UIPanelState state) { m_PanelState = state; }

		float GetDepth() const { return m_Depth; }

		float GetBorderRadius(const UIPanelCorner corner) const;

		float GetBorderThickness() const { return m_BorderThickness; }

		sf::Color GetBorderColor(const UIPanelCorner corner);

		sf::Color GetBackgroundColor(const UIPanelCorner corner);

		std::shared_ptr<Smasher::TextureResource> GetTexture() const { return m_TexturePtr; };

		const std::shared_ptr<Smasher::TextureResource>& GetTextureRef() const { return m_TexturePtr; };

		const sf::IntRect& GetClipRect() const { return m_ClipRect; }

		Degrees GetClipRotation() const { return m_ClipRotation; }

		const sf::Transform& GetClipTransform();

		const UIPanelState& GetPanelState() const { return m_PanelState; };

		const UIPanelSettings& GetPanelSettings() const { return m_PanelSettings; };
		
	protected:
		void OnHoverEvent(Events::MouseMoveEvent &event);
		void OnPressEvent(Events::MouseButtonEvent &event);
		void DrawPanel();
		sf::Shape& GetShape() { return m_DebugSprite; };

		GLuint instanceVAO = std::numeric_limits<GLuint>::max();
		GLuint instanceVBO = std::numeric_limits<GLuint>::max();
		GLuint quadVBO = std::numeric_limits<GLuint>::max();
		GLuint quadEBO = std::numeric_limits<GLuint>::max();

		static inline const GLubyte STATIC_INDICES[6]{
			0, 1, 2,   // first triangle
			2, 3, 0    // second triangle
		};

		static inline const float STATIC_VERTICES[24]{
			//   Position       Tex Coord
			   -0.5f, -0.5f,     0.0, 0.0,   // bottom left
				0.5f, -0.5f,     1.0, 0.0,   // bottom right
				0.5f,  0.5f,     1.0, 1.0,   // top right
			   -0.5f,  0.5f,     0.0, 1.0,   // top left
		};

	private:
		void InitGLObjects();
		void UpdateGLBufferData();

		float m_BorderRadius[4] = { 0.f };
		float m_BorderThickness = 0.f;
		float m_Depth = 0.f;
		UIPanelState m_PanelState = UIPanelState::NONE;
		UIPanelData m_PanelRenderData;
		UIPanelSettings m_PanelSettings = UIPanelSettings::NONE;
		sf::RectangleShape m_DebugSprite;
		sf::Transformable m_Transformable;
		sf::IntRect m_ClipRect{ { 0, 0 }, { 0, 0 } };
		sf::Transform m_ClipTransform;
		Degrees m_ClipRotation = 0.f;
		std::function<void(Events::MouseMoveEvent&)> m_MouseMoveCallback;
		std::function<void(Events::MouseButtonEvent&)> m_MousePressCallback;
		std::shared_ptr<Smasher::TextureResource> m_TexturePtr;
		sf::Color m_BackgroundColors[4] = { sf::Color::White, sf::Color::White, sf::Color::White, sf::Color::White };
		sf::Color m_BorderColors[4] = { sf::Color::Transparent, sf::Color::Transparent, sf::Color::Transparent, sf::Color::Transparent };
		bool m_Changed = false;
		bool m_ClipChanged = false;
	};
}

#include "Smasher/Components/UIPanelComponent.inl"