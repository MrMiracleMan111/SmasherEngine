
// -----------------------------------------------
// Pixel (Fragment) Shader
// -----------------------------------------------
struct PSInput
{
    [[vk::location(0)]] float3 position : POSITION;
};

cbuffer UBO : register(b0, space1)
{
    row_major float4x4 ProjectionMatrix;
    row_major float4x4 ViewMatrix;
    row_major float4x4 CameraMatrix;
    float2 WindowSize;
    int padding0;
    int padding1;
    // column_major float4x4 ProjectionMatrix;
    // column_major float4x4 ViewMatrix;
    // column_major float4x4 CameraMatrix;
};

//struct PSOutput
//{
//    float4 gNormal : SV_Target0;
//    float2 gUV : SV_Target1;
//    int gMaterialIDs : SV_Target2;
//};


void PSMain(PSInput input)
{
    return;
}