// Vertex Shader Output / Pixel Shader Input
struct VSInput
{
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(1)]] float2 uv       : TEXCOORD0;
};

struct PSInput
{
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};

// -----------------------------------------------
// Vertex Shader
// -----------------------------------------------
PSInput VSMain(VSInput input, out float4 pos : SV_Position)
{
    PSInput output;
    pos = float4(input.position, 1.f);
    output.uv = input.uv;
    return output;
}