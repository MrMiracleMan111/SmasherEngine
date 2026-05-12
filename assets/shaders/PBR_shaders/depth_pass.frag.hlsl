
// -----------------------------------------------
// Pixel (Fragment) Shader
// -----------------------------------------------
struct PSInput
{
    [[vk::location(0)]] float3 normal : NORMAL;
    [[vk::location(1)]] float3 position : POSITION;
    [[vk::location(2)]] float2 uv : TEXCOORD0;
    [[vk::location(3)]] uint materialID : MATERIALID;
};

cbuffer UBO : register(b0, space1)
{
    row_major float4x4 ProjectionMatrix;
    row_major float4x4 ViewMatrix;
    row_major float4x4 CameraMatrix;
    float2 WindowSize;
    // column_major float4x4 ProjectionMatrix;
    // column_major float4x4 ViewMatrix;
    // column_major float4x4 CameraMatrix;
};

struct PSOutput
{
    float4 gNormal : SV_Target0;
    float2 gUV : SV_Target1;
    uint4 gMaterialIDs : SV_Target2;
};


PSOutput PSMain(PSInput input)
{
    PSOutput output;
    
    output.gNormal = float4(input.normal, 1.f);
    output.gUV = float2(input.uv);
    output.gMaterialIDs = uint4(
        (input.materialID) & 0xFF,
        (input.materialID >> 8) & 0xFF,
        (input.materialID >> 16) & 0xFF,
        (input.materialID >> 24) & 0xFF
    ); // input.materialID;
    
    return output;
}