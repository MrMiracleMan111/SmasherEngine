#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__linux__)
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#endif
#include "DrawableComponent.h"
#include "Entity.h"

namespace Smasher {
	void DrawableComponent::StaticRenderComponent(DrawableComponent& self, sf::RenderWindow& rWindow) {
		Transform2DComponent& rTransform = self.GetEntity().GetComponent<Transform2DComponent>();
		
		sf::RenderTarget& target = rWindow;
        if (self.m_TextureResource.get() == nullptr) {
            return;
        }

		if (self.m_ShaderResource.get() == nullptr) {
			//rWindow.draw(self.m_Vertices, &self.m_ShaderResource->GetShader());
            self.draw(rWindow);
        }
        else {
            self.m_ShaderResource->GetShader().setUniform("depth", self.m_Depth);
            self.m_ShaderResource->GetShader().setUniform("depth", self.m_Depth);
            self.m_ShaderResource->GetShader().setUniform("textureSize", sf::Glsl::Ivec2(self.m_TextureResource->GetTexture().getSize()));
            self.m_ShaderResource->GetShader().setUniform("clipCoords", self.m_ClipRect.getPosition());
            self.m_ShaderResource->GetShader().setUniform("clipSize", self.m_ClipRect.getSize());

            //rWindow.draw(self.m_Vertices, self.m_RenderState);
            self.draw(rWindow);
        }
    }

    // Draws to render target
    void DrawableComponent::draw(sf::RenderTarget& rTarget) const {
        // Get vertices data
        const QuadVertex* vertices = &m_Vertices[0];

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDepthRange(1.0f, 0.0f); // top = 1, bottom = 0
        glAlphaFunc(GL_GREATER, 0.0f);


        // Apply the transform
        glLoadMatrixf(m_TransformRef->getMatrix());

        // Apply the view
        applyCurrentView(rTarget);

        // Apply the blend mode
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Apply the texture
        sf::Texture::bind(m_RenderState.texture, sf::Texture::Pixels);

        // Apply the shader
        if (m_RenderState.shader)
            sf::Shader::bind(m_RenderState.shader);

        // Enable client state arrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        // Setup the pointers to the vertices' components
        const char* data = reinterpret_cast<const char*>(m_Vertices);
        glVertexPointer(3, GL_FLOAT, sizeof(QuadVertex), data + offsetof(QuadVertex, position));
        glColorPointer(4, GL_FLOAT, sizeof(QuadVertex), data + offsetof(QuadVertex, color));
        glTexCoordPointer(2, GL_FLOAT, sizeof(QuadVertex), data + offsetof(QuadVertex, texCoords));

        // Draw the primitives
        glDrawArrays(GL_QUADS, 0, 4);

        // Unbind the shader, if any
        if (m_RenderState.shader)
            sf::Shader::bind(NULL);
	}

    void DrawableComponent::applyCurrentView(sf::RenderTarget& target) const
    {
        // Set the viewport
        sf::IntRect viewport = getViewport(target, target.getView());
        int top = target.getSize().y - (viewport.top + viewport.height);
        (glViewport(viewport.left, top, viewport.width, viewport.height));

        // Set the projection matrix
        (glMatrixMode(GL_PROJECTION));
        (glLoadMatrixf(target.getView().getTransform().getMatrix()));

        // Go back to model-view mode
        (glMatrixMode(GL_MODELVIEW));
    }

    sf::IntRect DrawableComponent::getViewport(sf::RenderTarget& target, const sf::View& view) const
    {
        float width = static_cast<float>(target.getSize().x);
        float height = static_cast<float>(target.getSize().y);
        const sf::FloatRect& viewport = view.getViewport();

        return sf::IntRect(
            static_cast<int>(0.5f + width * viewport.left),
            static_cast<int>(0.5f + height * viewport.top),
            static_cast<int>(width * viewport.width),
            static_cast<int>(height * viewport.height));
    }
}