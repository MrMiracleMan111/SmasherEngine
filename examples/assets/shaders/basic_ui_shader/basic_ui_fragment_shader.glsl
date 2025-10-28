#version 330 core
precision mediump float;
precision highp int;

in vec3 dimensions_borderThickness;
in vec4 vertexColor;
in vec4 texCoord_spriteCoord;
in vec4 borderRadius;
in vec4 borderColor;
flat in uint hasTextureUint;


out vec4 FragColor;

//uniform float depth;
uniform sampler2D texture;
uniform bool translucentPass;
//uniform vec2 windowSize;

// Used to blend two alpha colors
vec4 blend(vec4 base, vec4 overlay) {
    float finalAlpha = overlay.a + base.a * (1.0 - overlay.a);
    vec3 finalColor = (overlay.rgb * overlay.a + base.rgb * base.a * (1.0 - overlay.a)) / finalAlpha;
    return vec4(finalColor, finalAlpha);
}

// The MIT License
// Copyright © 2015 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// https://www.youtube.com/c/InigoQuilez
// https://iquilezles.org
// https://www.shadertoy.com/view/4llXD7
// Signed distance to a 2D rounded box. Tutorials explaining
// how it works: 
//
// https://www.youtube.com/watch?v=62-pRVZuS5c
// https://www.youtube.com/watch?v=s5NGeUV2EyU
// p.x = frag coordinate relative to shape
// p.y = frag coordinate relative to shape
// b.x = half width
// b.y = half height
// r.x = roundness top-right  
// r.y = roundness boottom-right
// r.z = roundness top-left
// r.w = roundness bottom-left
float sdRoundBox(vec2 p, vec2 b, vec4 r)
{
    r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x = (p.y > 0.0) ? r.x : r.y;
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

void main()
{
    float AADist = 2.0; // size of Anti Aliasing region
    float dist = 0.f;

    bool hasTexture = (hasTextureUint != 0u);
    vec2 dimensions = vec2(dimensions_borderThickness.xy);
    vec2 texCoord = texCoord_spriteCoord.xy;
    vec2 spriteCoord = texCoord_spriteCoord.zw;
    float borderThickness = dimensions_borderThickness.z;

    vec4 innerColor = vertexColor;
    if (hasTexture) {
        // lookup the pixel in the texture
        vec4 pixel = texture2D(texture, texCoord);
        innerColor = blend(pixel, vertexColor);
        //innerColor = vec4(spriteCoord.xy, 0.0, 1.0);
    }

    // Create distance field for border
    vec2 pixelPos = spriteCoord.xy * dimensions.xy;

    dist = sdRoundBox(pixelPos - dimensions/2.0, dimensions / 2.0, borderRadius);

    // Draw the border
    if (dist > -borderThickness && dist <= 0) {

        // Inisde Edge
        if (dist <= -borderThickness / 2.0) {
            float tmp = borderThickness + clamp(dist, -borderThickness, -borderThickness + AADist); // between [0, AADist]
            float alpha = clamp(abs(tmp / AADist), 0.0, 1.0); // clamped between [0, 1]
            FragColor = blend(innerColor, vec4(borderColor.xyz, alpha));
        }
        // Outside Edge
        else {
            float tmp = AADist - (AADist + clamp(dist, -AADist, 0)); // between [0, AADist]
            float alpha = clamp(abs(tmp / AADist), 0.0, 1.0); // clamped between [0, 1]
            FragColor = vec4(borderColor.xyz, alpha);
        }
    }
    // Draw interior
    else if (dist <= -borderThickness) {
        FragColor = innerColor;
    }
    else {
        discard;
        return;
    }

    // Discard transparent stuff during Opaque pass 
    if (!translucentPass && (FragColor.a <= 0.95f)) {
        discard;
        return;
    }

    // Discard opaque stuff during transparent pass
    if (translucentPass && (FragColor.a > 0.95f)) {
        discard;
        return;
    }
}
