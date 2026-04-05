//[[vk::combinedImageSampler]][[vk::binding(0, 2)]]
Texture2D<float4> sourceColorImage : register(t0, space2);
//[[vk::combinedImageSampler]][[vk::binding(0, 2)]]
SamplerState sourceColorSampler : register(s0, space2);

//[[vk::combinedImageSampler]][[vk::binding(1, 2)]]
Texture2D<float> sourceDepthImage : register(t1, space2);
//[[vk::combinedImageSampler]][[vk::binding(1, 2)]]
SamplerState sourceDepthSampler : register(s1, space2);

// Vertex Shader Output / Pixel Shader Input
struct PSInput
{
    [[vk::location(0)]] float2 uv : TEXCOORD0;
};

struct PSOutput
{
    float4 gColor : SV_Target0;
    float gDepth : SV_Depth;
};

// -----------------------------------------------
// Pixel (Fragment) Shader
// -----------------------------------------------
PSOutput PSMain(PSInput input)
{
    PSOutput output;
    float sourceDepth = sourceDepthImage.Sample(sourceDepthSampler, input.uv);
    float4 sourceColor = sourceColorImage.Sample(sourceColorSampler, input.uv);
    
    // Resolve depth color assignment without branching
    output.gDepth = sourceDepth;
    output.gColor = sourceColor;
    return output;
}