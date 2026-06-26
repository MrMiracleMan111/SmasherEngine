
struct BVHInternalNode
{
    float3 minAABB;
    int childCount;
    float3 maxAABB;
    uint flags;
};

struct BVHLeafNode
{
    float3 minAABB;
    int parentNode;
    float3 maxAABB;
    int instanceIndex;
};

// Vertex Shader Output / Pixel Shader Input
struct VSInput
{
    [[vk::location(0)]] float3 position : POSITION;
};

struct PSInput
{
    bool isInternalNode : INTERNAL_NODE;
    bool ignore : IGNORE;
    float3 color : COLOR0;
};

StructuredBuffer<BVHInternalNode> bvhInternalBuffer : register(t0, space0);
StructuredBuffer<BVHLeafNode> bvhLeafBuffer : register(t1, space0);

cbuffer UBO : register(b0, space1)
{
    row_major float4x4 ProjectionMatrix;
    row_major float4x4 ViewMatrix;
    row_major float4x4 CameraMatrix;
    float2 WindowSize;
    int numLeaves;
    int minLevel;
    int maxLevel;
    // column_major float4x4 ProjectionMatrix;
    // column_major float4x4 ViewMatrix;
    // column_major float4x4 CameraMatrix;
};

// -----------------------------------------------
// Vertex Shader
// -----------------------------------------------
static const int BRANCHING_FACTOR = 4;
PSInput VSMain(VSInput input, uint instanceID : SV_InstanceID, out float4 position : SV_POSITION)
{
    PSInput psInput;
    psInput.isInternalNode = true;
    // BVHInternalNode node = bvhInternalBuffer[instanceID];
    float3 minAABB;
    float3 maxAABB;
    int level = (log2((float) (instanceID - numLeaves)) / log2(BRANCHING_FACTOR)) * (instanceID - numLeaves != 0);
    if (instanceID < numLeaves)
    {
        level = log2((float) numLeaves) / log2(BRANCHING_FACTOR);
        BVHLeafNode node = bvhLeafBuffer[instanceID];
        minAABB = node.minAABB;
        maxAABB = node.maxAABB;
        psInput.isInternalNode = false;
        psInput.color = float3(1.f, 0.f, 1.f);
        if (instanceID < 4)
        {
            psInput.color = float3(1.f, 1.f, 1.f);
        }
    }
    else
    {
        BVHInternalNode node = bvhInternalBuffer[instanceID - numLeaves];
        minAABB = node.minAABB;
        maxAABB = node.maxAABB;
        psInput.color = float3(1.f, 0.f, 0.f);
    }
    minAABB = minAABB * (int) (level >= minLevel && level <= maxLevel);
    maxAABB = maxAABB * (int) (level >= minLevel && level <= maxLevel);
    
    psInput.ignore = all(maxAABB == minAABB);
    
    float3 s = (maxAABB - minAABB) / 2.f;
    float3 o = (maxAABB + minAABB) / 2.f;

    row_major float4x4 model = float4x4(
        s.x, 0.f, 0.f, 0.f,
        0.f, s.y, 0.f, 0.f,
        0.f, 0.f, s.z, 0.f,
        o.x, o.y, o.z, 1.f
    );
    row_major float4x4 MVP = mul(mul(model, ViewMatrix), ProjectionMatrix);    
    
    // column_major float4x4 instanceModel = input.model;
    // column_major float4x4 MVP = mul(ProjectionMatrix, mul(ViewMatrix, instanceModel));
    // position = mul(float4(input.position, 1.f), MVP);
    position = mul(float4(input.position, 1.f), MVP);
    return psInput;
}