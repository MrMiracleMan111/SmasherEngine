#include <cmath>
#include <numbers>
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
#include "Smasher/Components/UIPanelComponent.h"
#include "Smasher/Base.h"
#include "Smasher/Entity.h"
#include "Smasher/Events.h"

namespace Smasher {
	// The MIT License
// Copyright © 2015 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// https://www.youtube.com/c/InigoQuilez
// https://iquilezles.org
// https://www.shadertoy.com/view/4llXD7
// Signed distance to a 2D rounded box. Tutorials explaining
// how it works: 
//
// https://www.youtube.com/watch?v=62-pRVZuS5c
// https://www.youtube.com/watch?v=s5NGeUV2EyU
// p.x = frag coordinate relative to shape
// p.y = frag coordinate relative to shape
// b.x = half width
// b.y = half height
// r.x = roundness top-right  
// r.y = roundness boottom-right
// r.z = roundness top-left
// r.w = roundness bottom-left
	float sdRoundBox(sf::Vector2f p, sf::Vector2f b, std::array<float, 4> borderRadius)
	{
		float r[4] = {
			borderRadius[3], // Bottom left corner
			borderRadius[0], // top left corner
			borderRadius[2], // Bottom right corner
			borderRadius[1], // Top right corner
		};
		if (p.x <= 0.0f) { r[0] = r[2]; r[1] = r[3]; }
		if (p.y <= 0.0f) { r[0] = r[1]; }
		sf::Vector2f q = sf::Vector2f{ std::abs(p.x), std::abs(p.y) } - b + sf::Vector2f{ r[0], r[0] };
		sf::Vector2f t = sf::Vector2f(std::max(q.x, 0.0f), std::max(q.y, 0.0f));
		return std::min(std::max(q.x, q.y), 0.0f) + std::sqrt(t.x*t.x + t.y*t.y) - r[0];

		//r.xy = (p.x > 0.0) ? r.xy : r.zw;
		//r.x = (p.y > 0.0) ? r.x : r.y;
		//vec2 q = abs(p) - b + r.x;
		//return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
	}

	UIPanelComponent::UIPanelComponent() : m_DebugSprite(sf::Vector2f(1.0f, 1.0f)), IComponent(), Transform2DWrapper(*this, m_Transformable)
	{
		m_DebugSprite.setFillColor(sf::Color::White);
		m_DebugSprite.setOrigin(0.5f, 0.5f);
	}

	void UIPanelComponent::StaticRenderComponent(UIPanelComponent& self, sf::RenderWindow& rWindow)
	{
		self.m_DebugSprite.setPosition(self.m_Transformable.getPosition());
		self.m_DebugSprite.setRotation(self.m_Transformable.getRotation());
		self.m_DebugSprite.setScale(self.m_Transformable.getScale());
		rWindow.draw(self.m_DebugSprite);
	}

	bool UIPanelComponent::IntersectsPanel(int x, int y)
	{
		float width = m_Transformable.getScale().x;
		float height = m_Transformable.getScale().y;
		sf::Vector2f pos((float)x, (float)y);
		pos = m_Transformable.getPosition() - pos;
		Smasher::Radians radians = std::atan2(pos.y, pos.x);
		float dist = std::sqrt(pos.x * pos.x + pos.y * pos.y);
		Smasher::Radians inversePanelRadians = -m_Transformable.getRotation() * ((float)std::numbers::pi / 180.f);
		Smasher::Radians finalRadians = radians + inversePanelRadians;
		pos.x = std::cos(finalRadians) * dist;
		pos.y = std::sin(finalRadians) * dist;
		//return (std::abs(pos.x) <= (width / 2)) && (std::abs(pos.y) <= (height / 2));
		return (sdRoundBox(pos, sf::Vector2f{ width / 2.0f, height / 2.0f }, std::to_array(m_BorderRadius)) < 0.0f);

	}

	UIPanelComponent& UIPanelComponent::SetTexture(std::shared_ptr<Smasher::TextureResource> pTexture) {
		m_Changed = true;
		m_ClipChanged = true;
		m_TexturePtr = pTexture;

		if (m_TexturePtr) {
			sf::Vector2u size = m_TexturePtr->GetTexture().getSize();
			SetClipRotation(Degrees{ 0 });
;			SetClipRect(sf::IntRect{ 0, 0, (int)size.x, (int)size.y });
		}
		return *this;
	}

	UIPanelComponent& UIPanelComponent::SetClipRect(sf::IntRect clipRect) {
		m_Changed = true;
		m_ClipChanged = true;
		m_ClipRect = clipRect;
		return *this;
	}

	UIPanelComponent& UIPanelComponent::SetClipRotation(Degrees angle) {
		m_Changed = true;
		m_ClipChanged = true;
		m_ClipRotation = angle;
		return *this;
	}

	const sf::Transform& UIPanelComponent::GetClipTransform()
	{
		if (!m_TexturePtr) {
			return m_ClipTransform;
		}

		m_ClipTransform = sf::Transform::Identity;
		sf::Texture& pTexture = m_TexturePtr->GetTexture();
		sf::Vector2f dimensions = sf::Vector2f((float)pTexture.getSize().x, (float)pTexture.getSize().y);

		float initialX = 0.5f * m_ClipRect.width;
		float initialY = 0.5f * m_ClipRect.height;

		float left = m_ClipRect.left - initialX; // +std::copysign(1.0f, m_ClipRect.width) * -0.5f;
		float top = m_ClipRect.top - initialY; //+std::copysign(1.0f, m_ClipRect.height) * -0.5f;


		m_ClipTransform
			.translate(initialX / dimensions.x, initialY / dimensions.y) // Center the clip at 0,0
			.rotate((float)-m_ClipRotation)
			.scale(m_ClipRect.width / dimensions.x, m_ClipRect.height / dimensions.y)
			.translate(left / dimensions.x, top / dimensions.y);

		m_ClipChanged = false;
		return m_ClipTransform;
	}

	UIPanelComponent& UIPanelComponent::SetBorderRadius(float radius)
	{
		m_Changed = true;
		std::fill(m_BorderRadius, m_BorderRadius + 4, radius);
		return *this;
	}

	float UIPanelComponent::GetBorderRadius(const UIPanelCorner corner) const {
		switch (corner) {
		case UIPanelCorner::TOP_LEFT:
			return m_BorderRadius[0];
		case UIPanelCorner::TOP_RIGHT:
			return m_BorderRadius[1];
		case UIPanelCorner::BOTTOM_RIGHT:
			return m_BorderRadius[2];
		case UIPanelCorner::BOTTOM_LEFT:
			return m_BorderRadius[3];
		default:
			assert(false); // Invalid UIPanelCorner type in GetBorderRadius, must use specific corner (ie. TOP_LEFT)
			return -1.0f;
		}
	}


	UIPanelComponent& UIPanelComponent::SetBorderRadius(UIPanelCorner corner, float radius) {
		m_Changed = true;
		char tmp = (char)corner;
		if (tmp & (char)UIPanelCorner::BOTTOM_RIGHT) {
			m_BorderRadius[1] = radius;
		}
		if (tmp & (char)UIPanelCorner::BOTTOM_LEFT) {
			m_BorderRadius[0] = radius;
		}
		if (tmp & (char)UIPanelCorner::TOP_LEFT) {
			m_BorderRadius[3] = radius; //
		}
		if (tmp & (char)UIPanelCorner::TOP_RIGHT) {
			m_BorderRadius[2] = radius; //
		}
		return *this;
	}

	UIPanelComponent& UIPanelComponent::SetBorderThickness(float thickness) {
		m_Changed = true;
		m_BorderThickness = thickness;
		return *this;
	}

	UIPanelComponent& UIPanelComponent::SetBorderColor(const sf::Color& color) {
		m_Changed = true;
		return SetBorderColor(UIPanelCorner::ALL, color);
	}

	UIPanelComponent& UIPanelComponent::SetBorderColor(UIPanelCorner corner, const sf::Color& color) {
		m_Changed = true;
		char tmp = (char)corner;
		if (tmp & (char)UIPanelCorner::BOTTOM_RIGHT) {
			m_BorderColors[0] = color;
		}
		if (tmp & (char)UIPanelCorner::BOTTOM_LEFT) {
			m_BorderColors[1] = color;
		}
		if (tmp & (char)UIPanelCorner::TOP_LEFT) {
			m_BorderColors[2] = color;
		}
		if (tmp & (char)UIPanelCorner::TOP_RIGHT) {
			m_BorderColors[3] = color;
		}
		return *this;
	}

	sf::Color UIPanelComponent::GetBorderColor(const UIPanelCorner corner) {
		switch (corner) {
		case UIPanelCorner::TOP_LEFT:
			return m_BorderColors[0];
		case UIPanelCorner::TOP_RIGHT:
			return m_BorderColors[1];
		case UIPanelCorner::BOTTOM_RIGHT:
			return m_BorderColors[2];
		case UIPanelCorner::BOTTOM_LEFT:
			return m_BorderColors[3];
		default:
			assert(false); // Invalid UIPanelCorner type in GetBorderRadius, must use specific corner (ie. TOP_LEFT)
			return sf::Color::Transparent;
		}
	}

	UIPanelComponent& UIPanelComponent::SetBackgroundColor(const sf::Color& color) {
		m_Changed = true;
		return SetBackgroundColor(UIPanelCorner::ALL, color);
	}

	UIPanelComponent& UIPanelComponent::SetBackgroundColor(UIPanelCorner corner, const sf::Color& color) {
		m_Changed = true;
		char tmp = (char)corner;
		if (tmp & (char)UIPanelCorner::BOTTOM_RIGHT) {
			m_BackgroundColors[0] = color;
		}
		if (tmp & (char)UIPanelCorner::BOTTOM_LEFT) {
			m_BackgroundColors[1] = color;
		}
		if (tmp & (char)UIPanelCorner::TOP_LEFT) {
			m_BackgroundColors[2] = color;
		}
		if (tmp & (char)UIPanelCorner::TOP_RIGHT) {
			m_BackgroundColors[3] = color;
		}
		return *this;
	}

	sf::Color UIPanelComponent::GetBackgroundColor(const UIPanelCorner corner) {
		switch (corner) {
		case UIPanelCorner::TOP_LEFT:
			return m_BackgroundColors[0];
		case UIPanelCorner::TOP_RIGHT:
			return m_BackgroundColors[1];
		case UIPanelCorner::BOTTOM_RIGHT:
			return m_BackgroundColors[2];
		case UIPanelCorner::BOTTOM_LEFT:
			return m_BackgroundColors[3];
		default:
			assert(false); // Invalid UIPanelCorner type in GetBorderRadius, must use specific corner (ie. TOP_LEFT)
			return sf::Color::Transparent;
		}
	}


	UIPanelComponent& UIPanelComponent::SetDepth(float depth) {
		m_Changed = true;
		m_Depth = depth;
		return *this;
	}

	void UIPanelComponent::OnHoverEvent(Events::MouseMoveEvent& event)
	{
		if (m_MouseMoveCallback) {
			m_MouseMoveCallback(event);
		}
	}

	void UIPanelComponent::OnPressEvent(Events::MouseButtonEvent& event)
	{
		if (m_MousePressCallback) {
			m_MousePressCallback(event);
		}
	}

	void UIPanelComponent::UpdateGLBufferData() {
		if (!m_TransformChanged && !m_Changed) {
			return;
		}

		// UIPanelData border radius format
		// r.x = roundness top-right  
		// r.y = roundness bottom-right
		// r.z = roundness top-left
		// r.w = roundness bottom-left

		UIPanelData data{
			{ GetPosition().x, GetPosition().y, GetDepth(), (float)(GetRotation() * ((float)std::numbers::pi / 180.f))}, // position xyz, rotation w (vec4)
			{ GetScale().x, GetScale().y,  GetBorderThickness()}, // scale (vec2), border thickness (float)
			Mat3{ GetClipTransform() }, // texTransform (Mat3)
			// 0 bottom left
			// 1 bottom right
			// 2 top right
			// 3 top left
			{	(uint32_t)GetBackgroundColor(UIPanelCorner::BOTTOM_RIGHT).toInteger(),
				(uint32_t)GetBackgroundColor(UIPanelCorner::BOTTOM_LEFT).toInteger(),
				(uint32_t)GetBackgroundColor(UIPanelCorner::TOP_LEFT).toInteger(),
				(uint32_t)GetBackgroundColor(UIPanelCorner::TOP_RIGHT).toInteger()
			},

			// 0 bottom left
			// 1 bottom right
			// 2 top right
			// 3 top left
			{	(uint32_t)GetBorderColor(UIPanelCorner::BOTTOM_RIGHT).toInteger(),
				(uint32_t)GetBorderColor(UIPanelCorner::BOTTOM_LEFT).toInteger(),
				(uint32_t)GetBorderColor(UIPanelCorner::TOP_LEFT).toInteger(),
				(uint32_t)GetBorderColor(UIPanelCorner::TOP_RIGHT).toInteger()
			},
			{
				m_BorderRadius[1], // Top right corner
				m_BorderRadius[2], // Bottom right corner
				m_BorderRadius[0], // top left corner
				m_BorderRadius[3]  // Bottom left corner
			},

			(uint32_t)((bool)m_TexturePtr)
		};

		memcpy(&m_PanelRenderData, &data, sizeof(UIPanelData));

		glBindVertexArray(instanceVAO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 1 * sizeof(Smasher::UIPanelData), &m_PanelRenderData);
		glBindVertexArray(0);
		m_TransformChanged = false;
		m_Changed = false;
	}

	void UIPanelComponent::DrawPanel() {
		UpdateGLBufferData();

		glBindVertexArray(instanceVAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, UIPanelComponent::StaticIndices);
		glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_BYTE, (GLvoid*)0, (GLsizei)1);

		glBindVertexArray(0);
	}

	void UIPanelComponent::InitGLObjects() {
		if (GetManager().GetLayer().GetEngine().IsHeadless()) {
			return;
		}

		// Cache the current GL_VERTEX_ARRAY_BINDING, GL_ARRAY_BUFFER_BINDING values
		// so that they can be restored after, InitGLObjects
		GLint currentVAO, currentVBO;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

		glGenVertexArrays(1, &instanceVAO);
		glGenBuffers(1, &instanceVBO);
		glGenBuffers(1, &quadVBO);
		glGenBuffers(1, &quadEBO);

		glBindVertexArray(instanceVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(UIPanelComponent::StaticVertices), UIPanelComponent::StaticVertices, GL_STATIC_DRAW);

		// The following two refer to instance data
		// Vertex position (xy) & Tex Coord (zw)
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(UIPanelComponent::StaticIndices), UIPanelComponent::StaticIndices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Smasher::UIPanelData), &m_PanelRenderData, GL_DYNAMIC_DRAW);

		// Instance Position (xyz) & Rotation (w) Vec3
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(UIPanelData), (GLvoid*)(offsetof(UIPanelData, position_rotation)));

		// Instance Scale (xy) & Border Thickness (z) Vec 3
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(UIPanelData), (GLvoid*)(offsetof(UIPanelData, scale_borderThickness)));

		// Instance Tex Transform Matrix
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(UIPanelData), (GLvoid*)(offsetof(UIPanelData, texTransform) + (0 * sizeof(float))));
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(UIPanelData), (GLvoid*)(offsetof(UIPanelData, texTransform) + (3 * sizeof(float))));
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(UIPanelData), (GLvoid*)(offsetof(UIPanelData, texTransform) + (6 * sizeof(float))));

		// Instance Color Code
		glVertexAttribIPointer(6, 4, GL_UNSIGNED_INT, sizeof(UIPanelData), (GLvoid*)(offsetof(UIPanelData, backgroundColors)));

		// Border Color Code
		glVertexAttribIPointer(7, 4, GL_UNSIGNED_INT, sizeof(UIPanelData), (GLvoid*)(offsetof(UIPanelData, borderColors)));

		// Border Radius Vec4 (4 corners)
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(UIPanelData), (GLvoid*)(offsetof(UIPanelData, borderRadius)));
		
		// Has Texture (bool)
		glVertexAttribIPointer(9, 1, GL_UNSIGNED_INT, sizeof(UIPanelData), (GLvoid*)(offsetof(UIPanelData, hasTexture)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glEnableVertexAttribArray(7);
		glEnableVertexAttribArray(8);
		glEnableVertexAttribArray(9);

		glVertexAttribDivisor(1, 1);  // Instance attribute
		glVertexAttribDivisor(2, 1);  // Instance attribute
		glVertexAttribDivisor(3, 1);  // Instance attribute
		glVertexAttribDivisor(4, 1);  // Instance attribute
		glVertexAttribDivisor(5, 1);  // Instance attribute
		glVertexAttribDivisor(6, 1);  // Instance attribute
		glVertexAttribDivisor(7, 1);  // Instance attribute (should be vertex attribute)
		glVertexAttribDivisor(8, 1);  // Instance attribute
		glVertexAttribDivisor(9, 1);  // Instance attribute

		glBindVertexArray(0);
		glBindVertexArray(currentVAO);
		glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
	}
}