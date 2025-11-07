#include <cmath>
#include <chrono>
#include "Smasher/Components/DrawableComponent.h"
#include "Smasher/IComponent.h"
#include "Smasher/Entity.h"
#include "Smasher/ComponentManagers/DrawableComponentManager.h"

namespace Smasher {
    DrawableComponent::DrawableComponent() : IComponent(), Transform2DWrapper(*this, m_Transformable),
        m_OpaqueBatchContext() /* Invalid State */,
        m_TranslucentBatchContext() /* Invalid State */ {
        SetScale(sf::Vector2f(100.0f, 100.0f));
    }

    DrawableComponent::DrawableComponent(
        std::shared_ptr<TextureResource> texturePtr,
        std::shared_ptr<ShaderResource> shaderPtr) : IComponent(), Transform2DWrapper(*this, m_Transformable),
        m_ShaderResource(shaderPtr),
        m_TextureResource(texturePtr),
        m_OpaqueBatchContext() /* Invalid State */,
        m_TranslucentBatchContext() /* Invalid State */ {
        SetScale(sf::Vector2f(100.0f, 100.0f));

        auto& rCompManager = static_cast<DrawableComponentManager&>(GetManager());
        SetShader(rCompManager.GetDefaultShader());
    }

    DrawableComponent::~DrawableComponent() {
        auto& rCompManager = static_cast<DrawableComponentManager&>(GetManager());
        rCompManager.OnComponentDelete(*this);
    }

    void DrawableComponent::OnAddComponent() {
        auto& rCompManager = static_cast<DrawableComponentManager&>(GetManager());
        rCompManager.OnComponentSetTexture(*this, DrawableComponentManager::EMPTY_TEXTURE_ID, true);
    }

    void DrawableComponent::SetEntity(Entity& pEntity) {
        IComponent::SetEntity(pEntity);
        m_TransformChanged = true;
    }

    DrawableComponent& DrawableComponent::SetDepth(float depth) {
        m_TransformChanged = true;
        m_Depth = depth;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetColor(sf::Color color) {
        m_TransformChanged = true;
        m_Color = color;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetShader(std::shared_ptr<ShaderResource> pShader) {
        m_TransformChanged = true;
        m_ShaderResource = pShader;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetClipRect(sf::IntRect clipRect) {
        m_ClipChanged = true;
        m_ClipRect = clipRect;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetClipRotation(Degrees angle) {
        m_ClipChanged = true;
        m_ClipRotation = angle;
        return *this;
    }

    const sf::Transform& DrawableComponent::GetClipTransform()
    {
        //assert(m_TextureLoaded);
        if (!m_ClipChanged) {
            return m_ClipTransform;
        }

        m_ClipTransform = sf::Transform::Identity;
        sf::Texture& pTexture = m_TextureResource->GetTexture();
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

}