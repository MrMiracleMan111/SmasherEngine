#version 330 core
precision highp int;
layout(location = 0) in vec2 aPos; // the position variable has attribute position 0
layout(location = 1) in vec2 aTexCoord; // the color variable has attribute position 1
layout(location = 2) in mat4 model;
layout(location = 6) in mat3 texMatrix;
layout(location = 9) in uint aColorCode; // the color variable has attribute position 1

out vec4 vertexColor;
out vec2 texCoord;

uniform mat4 ViewProjectionMatrix;
uniform ivec2 textureSize;

void main()
{
    vertexColor = vec4(
        float((aColorCode >> 0) & 255u) / 255.0f,
        float((aColorCode >> 8) & 255u) / 255.0f,
        float((aColorCode >> 16) & 255u) / 255.0f,
        float((aColorCode >> 24) & 255u) / 255.0f
    );

    //// transform the vertex position
    //gl_Position = model * vec4(aPos, 0.0, 1.0);
    gl_Position = ViewProjectionMatrix * model * vec4(aPos, 0.0, 1.0);

    texCoord = aTexCoord;
}