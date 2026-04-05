// Vertex Shader Output / Pixel Shader Input
struct VSInput
{
    [[vk::location(0)]] column_major float4x4 model : MODEL;
    [[vk::location(4)]] float3 position : POSITION;
    [[vk::location(5)]] float3 normal : NORMAL;
};

cbuffer UBO : register(b0, space1)
{
    column_major float4x4 ProjectionMatrix;
    column_major float4x4 ViewMatrix;
};

// -----------------------------------------------
// Vertex Shader
// -----------------------------------------------
float4 VSMain(VSInput input) : SV_POSITION
{
    column_major float4x4 MVP = mul(ProjectionMatrix, mul(ViewMatrix, input.model));
    float4 output = mul(MVP, float4(input.position, 1.f));
    return output;
}