// Vertex Shader Output / Pixel Shader Input
struct VSInput
{
    [[vk::location(0)]] float4x4 model : MODEL;
    [[vk::location(4)]] float3 position : POSITION;
    [[vk::location(5)]] float3 normal : NORMAL;
    [[vk::location(6)]] float2 uv : TEXCOORD0;
    [[vk::location(7)]] int materialID : MATERIALID;
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


// Vertex Shader Output / Pixel Shader Input
struct PSInput
{
    [[vk::location(0)]] float3 normal : NORMAL;
    [[vk::location(1)]] float4 position : POSITION;
    [[vk::location(2)]] float2 uv : TEXCOORD0;
    [[vk::location(3)]] uint materialID : MATERIALID;
};

// -----------------------------------------------
// Vertex Shader
// -----------------------------------------------
PSInput VSMain(VSInput input, out float4 position : SV_POSITION)
{
    PSInput fragInput;
    row_major float4x4 instanceModel = input.model;
    row_major float4x4 MVP = mul(mul(instanceModel, ViewMatrix), ProjectionMatrix);
    position = mul(float4(input.position, 1.f), MVP);
    
    // column_major float4x4 instanceModel = input.model;
    // column_major float4x4 MVP = mul(ProjectionMatrix, mul(ViewMatrix, instanceModel));
    // position = mul(float4(input.position, 1.f), MVP);
    
    fragInput.normal = input.normal;
    fragInput.uv = input.uv;
    fragInput.materialID = input.materialID;
    fragInput.position = position;
    
    return fragInput;
}