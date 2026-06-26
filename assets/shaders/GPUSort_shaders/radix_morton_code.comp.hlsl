#include "radix_util.hlsl"

StructuredBuffer<StaticMeshData> inputBuffer : register(t0, space0);
RWStructuredBuffer<RadixEntry> outputBuffer : register(u0, space1);

cbuffer UBO : register(b0, space2)
{
    float3 minAABB;
    int radixOffset; // Offset within radix entries list
    float3 maxAABB;
    int instanceOffset; // Offset within static mesh entries list
    int count; // Number of entries
};

// Evenly spaces each bit of x with 2 bits (only spreads first 10 bits remaining are dropped)
//  (input)    (output)
// 00000111 -> 01001001
// Implementation based off of
// https://graphics.stanford.edu/~seander/bithacks.html
uint spread3(uint x)
{
    x &= 0x000003ff; // x = -------- -------- ------98 76543210
    x = (x | x << 16) & 0x030000FF; // x = ------98 -------- -------- 76543210
    x = (x | x << 8)  & 0x0300F00F; // x = ------98 -------- 7654---- ----3210
    x = (x | x << 4)  & 0x030C30C3; // x = ------98 ----76-- --54---- 32----10
    x = (x | x << 2)  & 0x09249249; // x = ----9--8 --7--6-- 5--4--3- -2--1--0
    return x;
}

// Dispatch(NUM_ENTITIES/256, 1, 1)
[numthreads(256, 1, 1)]
void CSMain(uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID, uint DispatchThreadId : SV_DispatchThreadID)
{
    int instanceIndex = DispatchThreadId + instanceOffset;
    int radixIndex = DispatchThreadId + radixOffset;
    if (DispatchThreadId > count)
    {
        return;
    }
    StaticMeshData meshInstance = inputBuffer[instanceIndex];
    uint meshIndex = (meshInstance.materialIdMeshId >> 16) & 0xFFFF;
    if (meshIndex == 0)
    {
        // Invalid
        return;
    }
    // Compute Morton Code
    // 10 bits per axis
    uint mortonCode = 0;
    uint axisPrecision = (1U << 10) - 1U;
    
    // position relative to global AABB
    row_major float4x4 meshTransform = meshInstance.transform;
    float3 globalPosition = mul(float4(0.f, 0.f, 0.f, 1.f), meshTransform).xyz;
    double3 relPosition = globalPosition - minAABB.xyz;
    double3 increments = ((double3) maxAABB - (double3) minAABB) / double(axisPrecision);
    double3 mortPosition = relPosition / increments;
    mortPosition = max(double3(0, 0, 0), mortPosition);
    mortPosition = min(double3(double(axisPrecision), double(axisPrecision), double(axisPrecision)), mortPosition);

    uint3 tmp = uint3(mortPosition);
    uint sp1 = spread3(tmp.x);
    uint sp2 = spread3(tmp.y);
    uint sp3 = spread3(tmp.z);
    mortonCode = sp1 | sp2 << 1 | sp3 << 2;

    RadixEntry output;
    output.key = mortonCode;
    output.data = instanceIndex;
    outputBuffer[radixIndex] = output;
}