
// -----------------------------------------------
// Pixel (Fragment) Shader
// -----------------------------------------------
struct PSInput
{
    [[vk::location(0)]] float3 normal : NORMAL;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};


struct PSOutput
{
    float4 gNormal : SV_Target0;
    float2 gUV : SV_Target1;
};


PSOutput PSMain(PSInput input)
{
    PSOutput output;
    
    output.gNormal = float4(input.normal, 1.f);
    output.gUV = float2(input.uv);
    
    return output;
}