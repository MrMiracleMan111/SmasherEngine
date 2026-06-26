struct LightData
{
    float3 direction;
    int type;
    float3 position;
    float spreadAngle;
    float3 color;
    float falloff;
};

StructuredBuffer<LightData> lightsBuffer : register(t0, space0);

Texture2DArray<float4> materialsArray : register(t0, space2);
SamplerState samplerMaterial : register(s0, space2);

Texture2D<float4> normalsTexture : register(t1, space2);
SamplerState samplerNormals : register(s1, space2);

//[[vk::image_format("r32ui")]]
Texture2D<uint> materialIDTexture : register(t3, space2);

// Unorm
RWTexture2D<float4> lightingTexture : register(u0, space2);


cbuffer UBO : register(b0, space1)
{
    row_major float4x4 ProjectionMatrix;
    row_major float4x4 ViewMatrix;
    row_major float4x4 CameraMatrix;
    int2 WindowSize;
    int numLights;
    int padding1;
};

static const int KERNEL_SIZE = 8;

[numthreads(KERNEL_SIZE, KERNEL_SIZE, 1)]
void CSMain(uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{
    int2 pixelPos = GroupId.xy * int2(KERNEL_SIZE, KERNEL_SIZE) + GroupThreadID.xy;
    float3 output = float3(0.f, 0.f, 0.f);
    // 8x8 kernel
    uint materialId = materialIDTexture[pixelPos];
    if (materialId == 0)
    {
        lightingTexture[pixelPos] = float4(output, 1.f);
        return;
    }
    
    if (pixelPos.x >= WindowSize.x || pixelPos.y >= WindowSize.y)
    {
        return;
    }
    
    float3 normal = normalsTexture.Sample(samplerNormals, (float2) pixelPos).xyz;

    
    // Scan all lights
    {
        int i = 0;
        LightData light = lightsBuffer[i];
        float intensity = dot(normal, light.direction);
        output = intensity * light.color;
    }
    
    lightingTexture[pixelPos] = float4(output, 1.f);
}