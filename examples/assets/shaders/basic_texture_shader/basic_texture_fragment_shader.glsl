#version 330 core
precision mediump float;
precision highp int;

in vec4 vertexColor;
in vec2 texCoord;
flat in uint hasTextureUint;

out vec4 FragColor;

uniform sampler2D texture;
uniform bool translucentPass;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = vec4(1.f);
    
    if (hasTextureUint != 0u) {
        // Tex coord needs to be in bounds
        if (clamp(texCoord.x, 0.0, 1.0) != texCoord.x ||
            clamp(texCoord.y, 0.0, 1.0) != texCoord.y) {
            FragColor = vec4(0.0);
            //discard;
            return;
        }

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
