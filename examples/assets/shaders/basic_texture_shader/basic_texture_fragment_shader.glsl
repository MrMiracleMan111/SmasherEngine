#version 330 core
precision mediump float;
precision highp int;

in vec4 vertexColor;
in vec2 texCoord;
flat in uint hasTextureUint;

out vec4 FragColor;

//uniform float depth;
uniform sampler2D texture;
uniform bool translucentPass;
//uniform vec2 windowSize;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = vec4(1.f);
    
    if (hasTextureUint != 0u) {
        pixel = texture2D(texture, texCoord);
    }

    // Discard transparent stuff during Opaque pass 
    if (!translucentPass && (pixel.a <= 0.95f)) {
        discard;
    }

    // Discard opaque stuff during transparent pass
    if (translucentPass && (pixel.a > 0.95f)) {
        discard;
    }


    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    FragColor = vertexColor * pixel;
}
