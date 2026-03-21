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
        SetScale(sf::Vector2f{ 100.f, 100.f });
    }

    DrawableComponent::DrawableComponent(
        std::shared_ptr<TextureResource> texturePtr,
        std::shared_ptr<ShaderResource> shaderPtr) : IComponent(), Transform2DWrapper(*this, m_Transformable),
        m_ShaderResourcePtr(shaderPtr),
        m_TextureResourcePtr(texturePtr),
        m_OpaqueBatchContext() /* Invalid State */,
        m_TranslucentBatchContext() /* Invalid State */ {
        SetScale(sf::Vector2f{ 100.f, 100.f });

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
        m_ShaderResourcePtr = pShader;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetClipRect(sf::IntRect clipRect) {
        assert(m_TextureLoaded);
        m_ClipChanged = true;
        m_ClipRect = clipRect;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetClipRotation(Degrees angle) {
        assert(m_TextureLoaded);
        m_ClipChanged = true;
        m_ClipRotation = angle;
        return *this;
    }

    const sf::Transform& DrawableComponent::GetClipTransform()
    {
        assert(m_TextureLoaded);
        if (!m_ClipChanged) {
            return m_ClipTransform;
        }

        m_ClipTransform = sf::Transform::Identity;
        sf::Texture& pTexture = m_TextureResourcePtr->GetTexture();
        sf::Vector2f dimensions = sf::Vector2f{ (float)pTexture.getSize().x, (float)pTexture.getSize().y };
        
        sf::Vector2f offset = sf::Vector2f{ m_ClipRect.position };
        sf::Vector2f scale = sf::Vector2f{ m_ClipRect.size }.componentWiseDiv(dimensions);

        m_ClipTransform
            .scale(scale)
            .rotate(sf::degrees(-m_ClipRotation))
            .translate(sf::Vector2f{ m_ClipRect.position }.componentWiseDiv(dimensions.componentWiseMul(scale))) // Center the clip at 0,0
            ;

        m_ClipChanged = false;
        return m_ClipTransform;
    }

}