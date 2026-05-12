Texture2DArray<float4> materialsArray : register(t0, space2);
SamplerState samplerMaterial : register(s0, space2);

Texture2D<float4> normalsTexture : register(t1, space2);
SamplerState samplerNormals : register(s1, space2);

Texture2D<float2> uvTexture : register(t2, space2);
SamplerState samplerUV : register(s2, space2);

Texture2D<uint> materialIDTexture : register(t3, space2);

// -----------------------------------------------
// Pixel (Fragment) Shader
// -----------------------------------------------
struct PSInput
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
};


struct PSOutput
{
    float4 gNormal : SV_Target0;
    float4 gAlbedo : SV_Target1;
};


PSOutput PSMain(PSInput input)
{
    PSOutput output;
        
    float w, h;
    materialIDTexture.GetDimensions(w, h);
    int2 texelPos = int2(float2(w, h) * input.uv);
    
    float2 materialUV = uvTexture.Sample(samplerUV, input.uv);
    uint materialId = materialIDTexture.Load(int3(texelPos, 0));
    float4 albedo = materialsArray.Sample(samplerMaterial, float3(materialUV, materialId));
    float4 normal = normalsTexture.Sample(samplerNormals, input.uv);
    // normal = float4(materialUV.xy, 0.f, 0.f);
    output.gNormal = normal;
    output.gAlbedo = albedo;
    
    return output;
}