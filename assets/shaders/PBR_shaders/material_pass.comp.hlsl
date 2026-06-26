Texture2DArray<float4> materialsArray : register(t0, space2);
SamplerState samplerMaterial : register(s0, space2);

Texture2D<float4> normalsTexture : register(t1, space2);
SamplerState samplerNormals : register(s1, space2);

Texture2D<float2> uvTexture : register(t2, space2);
SamplerState samplerUV : register(s2, space2);

[[vk::image_format("r32ui")]]
RWTexture2D<uint> materialIDTexture : register(u3, space2);

// -----------------------------------------------
// Pixel (Fragment) Shader
// -----------------------------------------------
struct CSInput
{
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};

cbuffer UBO : register(b0, space1)
{
    row_major float4x4 ProjectionMatrix;
    row_major float4x4 ViewMatrix;
    row_major float4x4 CameraMatrix;
    float2 WindowSize;
    int padding0;
    int padding1;
};


struct PSOutput
{
    float4 gNormal : SV_Target0;
    float4 gAlbedo : SV_Target1;
};

[numthreads(8, 8, 1)]
PSOutput CSMain(CSInput input)
{
    PSOutput output;
        
    float w, h;
    materialIDTexture.GetDimensions(w, h);
    w = 512;
    h = 512;
    int2 texelPos = int2(float2(w, h) * input.uv);

    
    float2 materialUV = uvTexture.Sample(samplerUV, input.uv);
    materialUV = float2(0.4f, 0.5f);
    uint materialId = materialIDTexture.Load(int3(texelPos, 0));
    float4 albedo = materialsArray.Sample(samplerMaterial, float3(materialUV, materialId));
    float4 normal = normalsTexture.Sample(samplerNormals, input.uv);
    output.gNormal = normal;
    output.gAlbedo = albedo;
    
    return output;
}