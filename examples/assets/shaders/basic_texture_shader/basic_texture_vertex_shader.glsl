#version 330 core
precision highp int;
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aWorldPos;
layout(location = 3) in vec2 aWorldScale;
layout(location = 4) in mat3 texMatrix;
layout(location = 7) in uint aColorCode;
layout(location = 8) in float aWorldRotation;

out vec4 vertexColor;
out vec2 texCoord;

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

void main()
{
    mat4 model = GetTransform(aWorldPos, aWorldScale, aWorldRotation);

    vertexColor = vec4(
        float((aColorCode >> 24) & 255u) / 255.0,  // R from highest byte
        float((aColorCode >> 16) & 255u) / 255.0,  // G
        float((aColorCode >> 8)  & 255u) / 255.0,  // B
        float((aColorCode >> 0)  & 255u) / 255.0   // A from lowest byte
    );

    // transform the vertex position
    gl_Position = ViewProjectionMatrix * model * vec4(aPos, 0.0, 1.0);

    vec3 texTmp = texMatrix * vec3(aTexCoord, 1.0);
    texCoord = texTmp.xy;
}