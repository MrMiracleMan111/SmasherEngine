
// -----------------------------------------------
// Pixel (Fragment) Shader
// -----------------------------------------------

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

struct PSInput
{
    bool isInternalNode : INTERNAL_NODE;
    bool ignore : IGNORE;
    float3 color : COLOR0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

PSOutput PSMain(PSInput input)
{
    PSOutput output;
    if (input.ignore)
    {
        discard;
    }
    
    output.color = float4(input.color.rgb, 1.f);
    return output;
}