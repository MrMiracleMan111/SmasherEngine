// Vertex Shader Output / Pixel Shader Input
struct VSInput
{
    [[vk::location(0)]] column_major float4x4 model : MODEL;
    [[vk::location(4)]] float3 position : POSITION;
    [[vk::location(5)]] float3 normal : NORMAL;
    [[vk::location(6)]] float2 uv : TEXCOORD0;
};

cbuffer UBO : register(b0, space1)
{
    column_major float4x4 ProjectionMatrix;
    column_major float4x4 ViewMatrix;
};


// Vertex Shader Output / Pixel Shader Input
struct PSInput
{
    [[vk::location(0)]] float3 normal : NORMAL;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};

// -----------------------------------------------
// Vertex Shader
// -----------------------------------------------
PSInput VSMain(VSInput input, out float4 position : SV_POSITION)
{
    PSInput fragInput;
    column_major float4x4 MVP = mul(ProjectionMatrix, mul(ViewMatrix, input.model));
    position = mul(MVP, float4(input.position, 1.f));
    
    fragInput.normal = input.normal;
    fragInput.uv = input.uv;
    
    return fragInput;
}