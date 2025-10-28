#version 330 core
precision highp int;
layout(location = 0) in vec4 aPos_TexCoord; // position xy, texCoord zw
layout(location = 1) in vec4 aWorldPos_WorldRotation; // World Position xyz, World Rotation w
layout(location = 2) in vec3 aWorldScale_BorderThickness; // Word Scale (xy) Border Thickness (z)
layout(location = 3) in mat3 texMatrix;
layout(location = 6) in uint aColorCode;
layout(location = 7) in uint aBorderColorCode;
layout(location = 8) in vec4 aBorderRadius;
layout(location = 9) in uint aHasTextureUint;


out vec4 vertexColor;
out vec4 texCoord_spriteCoord;
out vec3 dimensions_borderThickness;
out vec4 borderRadius;
out vec4 borderColor;
flat out uint hasTextureUint;

uniform mat4 ViewProjectionMatrix;
uniform ivec2 textureSize;

// Copied from SFML Transformable::getTransform() defition
mat4 GetTransform(vec3 position, vec2 scale, float rotation) {
    float c = cos(rotation);
    float s = sin(rotation);

    return mat4(
        scale.x * c, scale.x * s, 0.0, 0.0,    // First column
        -scale.y * s, scale.y * c, 0.0, 0.0,    // Second column
        0.0, 0.0, 1.0, 0.0,    // Third column
        position.x, position.y, position.z, 1.0     // Fourth column (translation)
    );
}

vec4 GetColorFromCode(uint code) {
    return vec4(
        float((code >> 24) & 255u) / 255.0, // R from highest byte
        float((code >> 16) & 255u) / 255.0, // G
        float((code >> 8) & 255u) / 255.0,  // B
        float((code >> 0) & 255u) / 255.0   // A from lowest byte
    );
}

void main()
{
    vec2 pos = vec2(aPos_TexCoord.xy);
    vec2 tmpTexCoord = vec2(aPos_TexCoord.zw);
    vec2 worldScale = vec2(aWorldScale_BorderThickness.xy);
    vec3 worldPos = vec3(aWorldPos_WorldRotation.xyz);

    hasTextureUint = aHasTextureUint;
    borderRadius = aBorderRadius;
    dimensions_borderThickness.z = aWorldScale_BorderThickness.z;
    float worldRotation = aWorldPos_WorldRotation.w;
    mat4 model = GetTransform(worldPos, worldScale, worldRotation);

    dimensions_borderThickness.xy = worldScale;

    vertexColor = GetColorFromCode(aColorCode);
    borderColor = GetColorFromCode(aBorderColorCode);

    // transform the vertex position
    gl_Position = ViewProjectionMatrix * model * vec4(pos, 0.0, 1.0);

    vec3 texTmp = texMatrix * vec3(tmpTexCoord, 1.0);
    tmpTexCoord = texTmp.xy;

    texCoord_spriteCoord.xy = vec2(tmpTexCoord);
    texCoord_spriteCoord.zw = vec2(aPos_TexCoord.zw);
}