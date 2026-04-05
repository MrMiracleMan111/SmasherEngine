//[[vk::combinedImageSampler]][[vk::binding(0, 2)]]
Texture2D<float4> sourceColorImage : register(t0, space2);
//[[vk::combinedImageSampler]][[vk::binding(0, 2)]]
SamplerState sourceColorSampler : register(s0, space2);

//[[vk::combinedImageSampler]][[vk::binding(1, 2)]]
Texture2D<float> sourceDepthImage : register(t1, space2);
//[[vk::combinedImageSampler]][[vk::binding(1, 2)]]
SamplerState sourceDepthSampler : register(s1, space2);

//[[vk::combinedImageSampler]][[vk::binding(2, 2)]]
Texture2D<float4> gColorImage : register(t2, space2);
//[[vk::combinedImageSampler]][[vk::binding(2, 2)]]
SamplerState gColorSampler : register(s2, space2);

//[[vk::combinedImageSampler]][[vk::binding(3, 2)]]
Texture2D<float> gDepthImage : register(t3, space2);
//[[vk::combinedImageSampler]][[vk::binding(3, 2)]]
SamplerState gDepthSampler : register(s3, space2);

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
    float gDepth = gDepthImage.Sample(gDepthSampler, input.uv);
    float sourceDepth = sourceDepthImage.Sample(sourceDepthSampler, input.uv);

    float4 gColor = gColorImage.Sample(gColorSampler, input.uv);
    float4 sourceColor = sourceColorImage.Sample(sourceColorSampler, input.uv);
    
    PSOutput output;
    
    // Resolve depth color assignment without branching
    output.gColor.a = 1.f;
    output.gColor.rgb = (sourceDepth < gDepth) ? sourceColor.rgb : gColor.rgb;
    output.gDepth = min(sourceDepth, gDepth);
    return output;
}