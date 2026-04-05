// Vertex Shader Output / Pixel Shader Input
struct PSInput
{
    [[vk::location(0)]] float4 position : SV_POSITION;
    [[vk::location(1)]] float3 color : COLOR;
};

struct PSOutput
{
    float4 color : SV_Target0;
    float depth : SV_Depth;
};

// -----------------------------------------------
// Pixel (Fragment) Shader
// -----------------------------------------------
PSOutput PSMain(PSInput input)
{
    PSOutput output;
    output.color = float4(input.color, 1.0f);
    output.depth = 0.2f;
    return output;
}