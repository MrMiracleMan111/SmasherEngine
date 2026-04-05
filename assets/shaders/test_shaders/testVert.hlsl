// Vertex Shader Output / Pixel Shader Input
struct VSInput
{
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float3 color : COLOR;
};

struct PSInput
{
    [[vk::location(0)]] float4 position : SV_POSITION;
    [[vk::location(1)]] float3 color : COLOR;
};

// -----------------------------------------------
// Vertex Shader
// -----------------------------------------------
PSInput VSMain(VSInput input)
{
    PSInput result;
    result.position = float4(input.position, 1.0f);
    result.color = input.color;
    return result;
}