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
#include "Smasher/ComponentManagers/UIPanelComponentManager.h"
#include "Smasher/Layer.h"
#include "Smasher/Engine.h"

namespace Smasher {
	UIPanelComponentManager::UIPanelComponentManager(Layer &layer) :
		BaseComponentManager<UIPanelComponent>(layer)
	{
		Engine &engine = GetLayer().GetEngine();

		m_MouseMoveSubscription = layer.Subscribe<Events::MouseMoveEvent>(&UIPanelComponentManager::OnMouseMove, this);
		m_MouseButtonSubscription = layer.Subscribe<Events::MouseButtonEvent>(&UIPanelComponentManager::OnMouseButton, this);
	
		// Load the basic shader 
		static_assert(Smasher::HasRenderCapability<Smasher::UIPanelComponentManager>, "UIPanelComponentManager should have the render capability");

		std::shared_ptr<Smasher::ShaderResource> pShader = engine.GetResourceManager().LoadVertFragShaderResource(EngineConfig::UI_PANEL_COMPONENT_VERT_SHADER, EngineConfig::UI_PANEL_COMPONENT_FRAG_SHADER);
		sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(engine.GetWindow().getView().getTransform().getMatrix());
		pShader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);
		SetShaderResource(pShader);
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
		/*for (auto &itr : m_Components) {
			itr.m_DebugSprite.setPosition(itr.m_Transformable.getPosition());
			itr.m_DebugSprite.setRotation(itr.m_Transformable.getRotation());
			itr.m_DebugSprite.setScale(itr.m_Transformable.getScale());
			rWindow.draw(itr.GetShape());
		}*/

		GLint currentVAO, currentVBO;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

		sf::Glsl::Mat4 ViewProjectionMatrix(rWindow.getView().getTransform().getMatrix());
		sf::Shader::bind(&m_ShaderResourcePtr->GetShader());
		static_assert(std::is_same_v<std::shared_ptr<ShaderResource>, decltype(m_ShaderResourcePtr)>, "fail");
		m_ShaderResourcePtr->GetShader().setUniform("ViewProjectionMatrix", ViewProjectionMatrix);
		m_ShaderResourcePtr->GetShader().setUniform("translucentPass", false);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDepthRange(1.f, -1.f); // top = 1, bottom = 0
		glDisable(GL_BLEND);

		for (auto &itr : m_Components) {
			if (itr.GetTextureRef()) {
				sf::Texture::bind(&itr.GetTexture()->GetTexture(), sf::Texture::Pixels);
			}
			itr.DrawPanel();
		}

		m_ShaderResourcePtr->GetShader().setUniform("translucentPass", true);
		glDepthMask(GL_FALSE); // Disable depth writes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (auto &itr : m_Components) {
			itr.DrawPanel();
		}

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_ALWAYS);
		glDepthRange(1.f, -1.f); // top = 1, bottom = 0

		sf::Texture::bind(NULL);
		sf::Shader::bind(NULL);
		glBindVertexArray(currentVAO);
		glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
		rWindow.resetGLStates();
	}

	void UIPanelComponentManager::OnMouseMove(Events::MouseMoveEvent &event) {
		for (auto &itr : m_Components) {
			UIPanelState prevPanelState = itr.GetPanelState();
			if (!event.IsBlocked() && itr.IntersectsPanel(event.Position.x, event.Position.y)) {
				itr.SetPanelState(prevPanelState | UIPanelState::HOVERED);
				if ((bool)(itr.GetPanelSettings() & UIPanelSettings::BLOCK_MOUSE_MOVE)) {
					event.Block();
				}
			}
			else {
				itr.SetPanelState(prevPanelState & ~UIPanelState::HOVERED);
			}

			if (prevPanelState != itr.GetPanelState()) {
				itr.OnHoverEvent(event);
			}
		}
	}

	void UIPanelComponentManager::OnMouseButton(Events::MouseButtonEvent &event) {
		for (auto &itr : m_Components) {
			UIPanelState prevPanelState = itr.GetPanelState();
			bool pressed = (event.Type == Mouse::MouseEventType::BUTTON_PRESS && event.ButtonCode == sf::Mouse::Button::Left);
			if (itr.IntersectsPanel(event.Position.x, event.Position.y) && pressed) {
				itr.SetPanelState(prevPanelState | UIPanelState::PRESSED);
				if ((bool)(itr.GetPanelSettings() & UIPanelSettings::BLOCK_MOUSE_PRESS)) {
					event.StopPropagate();
				}
			}
			else {
				itr.SetPanelState(prevPanelState & ~UIPanelState::PRESSED);
			}

			if (prevPanelState != itr.GetPanelState()) {
				itr.OnPressEvent(event);
			}
		}
	}

}
