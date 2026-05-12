// Vertex Shader Output / Pixel Shader Input
struct VSInput
{
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};


// Vertex Shader Output / Pixel Shader Input
struct PSInput
{
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};

// -----------------------------------------------
// Vertex Shader
// -----------------------------------------------
PSInput VSMain(VSInput input, out float4 position : SV_POSITION)
{
    PSInput fragInput;
    fragInput.uv = input.uv;
    fragInput.position = input.position;
    position = float4(input.position, 1.f);
    return fragInput;
}