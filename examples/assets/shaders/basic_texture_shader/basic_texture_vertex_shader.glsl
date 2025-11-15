#version 330 core
precision highp int;
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in mat3 vertMatrix;
layout(location = 5) in mat3 texMatrix;
layout(location = 8) in uint aColorCode;
layout(location = 9) in float aDepth;

out vec4 vertexColor;
out vec2 texCoord;
flat out uint hasTextureUint;

uniform mat4 ViewProjectionMatrix;
uniform ivec2 textureSize;
uniform bool hasTexture;

// Copied from SFML Transformable::getTransform() defition
mat4 GetTransform(vec3 position, vec2 scale, float rotation) {
    float c = cos(rotation);
    float s = sin(rotation);

    return mat4(
        scale.x * c, scale.x * s, 0.0, 0.0,
        -scale.y * s, scale.y * c, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        position.x, position.y, position.z, 1.0
    );
}

// Convert 2D Mat3x3 transformation into 3D Mat4x4 transformation (on XY Plane)
mat4 Mat3ToMat4(mat3 matrix) {
    return mat4(
        matrix[0][0], matrix[0][1],     0.0,          0.0,
        matrix[1][0], matrix[1][1],     0.0,          0.0,
                 0.0,          0.0,     1.0,          0.0,
        matrix[2][0], matrix[2][1],     aDepth,       1.0
    );
}

void main()
{
    hasTextureUint = 0u;
    if (hasTexture) {
        hasTextureUint = 1u;
    }

    //mat4 model = GetTransform(vec3(0.0), vec2(100.0), 0.0);
    mat4 model = Mat3ToMat4(vertMatrix);

    vertexColor = vec4(
        // R is the highest Byte, A is the lowest byte
        float((aColorCode >> 24) & 255u) / 255.0,  // R
        float((aColorCode >> 16) & 255u) / 255.0,  // G
        float((aColorCode >> 8)  & 255u) / 255.0,  // B
        float((aColorCode >> 0)  & 255u) / 255.0   // A
    );

    // transform the vertex position
    gl_Position = ViewProjectionMatrix * model * vec4(aPos, 0.0, 1.0);

    vec3 texTmp = texMatrix * vec3(aTexCoord, 1.0);
    texCoord = texTmp.xy;
}