#version 120
precision mediump float;

uniform ivec2 textureSize;
uniform ivec2 clipCoords;
uniform ivec2 clipSize;
uniform float depth;



/**

All texture clipping can be represented as two operations
Scale -> Transform


Needs to map clipCoords to texture coords

clipCoords (uses textureSize)

    (0, 0) --------- (width, 0)
      |                 |
      |                 |
      |                 |
      |                 |
  (0, height) ------ (width, height)


  texCoords

     (0, 1) -------- (1, 1)
      |                 |
      |                 |
      |                 |
      |                 |
    (0, 0) ---------- (1, 0)

*/

// Takes texCoord input and spits out texture coord after clipping operation
vec4 getTextureClipCoords(vec4 _texCoord, ivec2 _clipCoords, ivec2 _clipSize, ivec2 _textureSize) {
    vec2 scale = vec2(_clipSize) / vec2(_textureSize);
    vec2 offset = vec2(_clipCoords) / vec2(_textureSize);
    _texCoord.xy = (_texCoord.xy * scale) + offset;
    return _texCoord;
}

void main()
{
    // transform the vertex position
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_Position.z = depth;

    // transform the texture coordinates
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_TexCoord[0] = getTextureClipCoords(gl_TexCoord[0], clipCoords, clipSize, textureSize);
    // forward the vertex color
    gl_FrontColor = gl_Color;
}