Texture2D<uint4> sourceColorImage : register(t0, space0);
Texture2D<uint4> sourceDepthImage : register(t1, space0);
RWTexture2D<uint4> gColorImage : register(u0, space1);
RWTexture2D<uint4> gDepthImage : register(u1, space1);

// -----------------------------------------------
// Compute Shader
// -----------------------------------------------
[numthreads(8, 8, 1)]
void CSMain(uint3 GlobalInvocationID : SV_DispatchThreadID)
{
    int2 coord = int2(GlobalInvocationID.xy);
    float4 sourceColor = sourceColorImage[coord];
    gColorImage[coord] = sourceColor;
}