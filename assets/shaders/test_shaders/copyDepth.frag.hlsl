
[[vk::combinedImageSampler]][[vk::binding(0, 2)]]
Texture2D<float> depthImage;

[[vk::combinedImageSampler]][[vk::binding(0, 2)]]
SamplerState depthImageSampler;

struct PSInput
{
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};

// -----------------------------------------------
// Pixel (Fragment) Shader
// -----------------------------------------------
float4 PSMain(
    in PSInput input
) : SV_TARGET0
{
    float depth = depthImage.Sample(depthImageSampler, input.uv);
    return float4(depth, 1.0f, 1.0f, 1.0f);
}