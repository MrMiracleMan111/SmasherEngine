#version 120
precision mediump float;

uniform float depth;
uniform sampler2D texture;
uniform vec2 windowSize;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

    vec2 uv = gl_FragCoord.xy / windowSize;
    uv = gl_TexCoord[0].xy;

    // multiply it by the color
    gl_FragColor = gl_Color * pixel;
    gl_FragColor = vec4(uv, 0.0, 1.0);
}
