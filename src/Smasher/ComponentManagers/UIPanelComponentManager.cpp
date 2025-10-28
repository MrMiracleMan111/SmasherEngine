#include "Smasher/ComponentManagers/UIPanelComponentManager.h"
#include "Smasher/Layer.h"
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

namespace Smasher {
	UIPanelComponentManager::UIPanelComponentManager(Layer& layer) :
		BaseComponentManager<UIPanelComponent>(layer)
	{
		m_MouseMoveSubscription = layer.Subscribe<Events::MouseMoveEvent>(&UIPanelComponentManager::OnMouseMove, this);
		m_MouseButtonSubscription = layer.Subscribe<Events::MouseButtonEvent>(&UIPanelComponentManager::OnMouseButton, this);
	}

	UIPanelComponent& UIPanelComponent::SetOnHoverCallback(std::function<void(Events::MouseMoveEvent&)> callback) {
		m_MouseMoveCallback = callback;
		return *this;
	}

	UIPanelComponent& UIPanelComponent::SetOnPressCallback(std::function<void(Events::MouseButtonEvent&)> callback) {
		m_MousePressCallback = callback;
		return *this;
	}

	void UIPanelComponentManager::RenderComponents(sf::RenderWindow& rWindow)
	{
		/*for (auto& itr : m_Components) {
			itr.m_DebugSprite.setPosition(itr.m_Transformable.getPosition());
			itr.m_DebugSprite.setRotation(itr.m_Transformable.getRotation());
			itr.m_DebugSprite.setScale(itr.m_Transformable.getScale());
			rWindow.draw(itr.GetShape());
		}*/

		GLint currentVAO, currentVBO;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

		sf::Glsl::Mat4 ViewProjectionMatrix(rWindow.getView().getTransform().getMatrix());
		sf::Shader::bind(&m_ShaderResource->GetShader());
		static_assert(std::is_same_v<std::shared_ptr<ShaderResource>, decltype(m_ShaderResource)>, "fail");
		m_ShaderResource->GetShader().setUniform("ViewProjectionMatrix", ViewProjectionMatrix);
		m_ShaderResource->GetShader().setUniform("translucentPass", false);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDepthRange(1.0f, -1.0f); // top = 1, bottom = 0
		glDisable(GL_BLEND);

		for (auto& itr : m_Components) {
			if (itr.GetTextureRef()) {
				sf::Texture::bind(&itr.GetTexture()->GetTexture(), sf::Texture::Pixels);
			}
			itr.DrawPanel();
		}

		m_ShaderResource->GetShader().setUniform("translucentPass", true);
		glDepthMask(GL_FALSE); // Disable depth writes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (auto& itr : m_Components) {
			itr.DrawPanel();
		}

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_ALWAYS);
		glDepthRange(1.0f, -1.0f); // top = 1, bottom = 0

		sf::Texture::bind(NULL);
		sf::Shader::bind(NULL);
		glBindVertexArray(currentVAO);
		glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
		rWindow.resetGLStates();
	}

	void UIPanelComponentManager::OnMouseMove(Events::MouseMoveEvent& event) {
		for (auto& itr : m_Components) {
			UIPanelState prev = itr.GetPanelState();
			if (!event.IsBlocked() && itr.IntersectsPanel(event.Position.x, event.Position.y)) {
				itr.SetPanelState(prev | UIPanelState::HOVERED);
				if ((bool)(itr.GetPanelSettings() & UIPanelSettings::BLOCK_MOUSE_MOVE)) {
					event.Block();
				}
			}
			else {
				itr.SetPanelState(prev & ~UIPanelState::HOVERED);
			}

			if (prev != itr.GetPanelState()) {
				itr.OnHoverEvent(event);
			}
		}
	}

	void UIPanelComponentManager::OnMouseButton(Events::MouseButtonEvent& event) {
		for (auto& itr : m_Components) {
			UIPanelState prev = itr.GetPanelState();
			bool pressed = (event.Type == Mouse::MouseEventType::BUTTON_PRESS && event.ButtonCode == sf::Mouse::Button::Left);
			if (itr.IntersectsPanel(event.Position.x, event.Position.y) && pressed) {
				itr.SetPanelState(prev | UIPanelState::PRESSED);
				if ((bool)(itr.GetPanelSettings() & UIPanelSettings::BLOCK_MOUSE_PRESS)) {
					event.StopPropagate();
				}
			}
			else {
				itr.SetPanelState(prev & ~UIPanelState::PRESSED);
			}

			if (prev != itr.GetPanelState()) {
				itr.OnPressEvent(event);
			}
		}
	}

}
