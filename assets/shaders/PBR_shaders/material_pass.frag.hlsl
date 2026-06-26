Texture2DArray<float4> materialsArray : register(t0, space2);
SamplerState samplerMaterial : register(s0, space2);

Texture2D<float4> normalsTexture : register(t1, space2);
SamplerState samplerNormals : register(s1, space2);

Texture2D<float2> uvTexture : register(t2, space2);
SamplerState samplerUV : register(s2, space2);

Texture2D<int> materialIDTexture : register(t3, space2);

struct MaterialProps
{
    float4 albedo; // float 16 align
    float4 specular; // float 16 align
    int albedoTextureIndex; // -1 = invalid
    int padding0;
    int padding1;
    int padding2;
};

StructuredBuffer<MaterialProps> materialPropsBuff : register(t4, space2);

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
    float padding0;
    float padding1;
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
    int materialId = materialIDTexture.Load(int3(texelPos, 0));
    if (materialId == -1)
    {
        discard;
    }
    
    MaterialProps matProps = materialPropsBuff[materialId];

    float4 sampleAlbedo = materialsArray.Sample(samplerMaterial, float3(materialUV, matProps.albedoTextureIndex));
    float4 albedo = matProps.albedo * sampleAlbedo;
    float4 normal = normalsTexture.Sample(samplerNormals, input.uv);
    // normal = float4(materialUV.xy, 0.f, 0.f);
    output.gNormal = normal;
    output.gAlbedo = albedo;
    
    return output;
}