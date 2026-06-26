#include "radix_util.hlsl"
#include "bvh_util.hlsl"


cbuffer UBO : register(b0, space2)
{
    int numEntries;
};

StructuredBuffer<RadixEntry> inputBuffer : register(t0, space0);
StructuredBuffer<StaticMeshData> instancesBuffer : register(t1, space0);
StructuredBuffer<MeshProps> meshPropsBuffer : register(t2, space0);
RWStructuredBuffer<BVHInternalNode> bvhInternalBuffer : register(u0, space1);
RWStructuredBuffer<BVHLeafNode> bvhLeafBuffer : register(u1, space1);


BoundingBox GetRadixEntryAABB(RadixEntry entry)
{
    BoundingBox box;
    StaticMeshData instanceData = instancesBuffer[entry.data];
    uint meshPropIndex = (instanceData.materialIdMeshId >> 16) & 0x0000FFFF;
    MeshProps meshData = meshPropsBuffer[meshPropIndex];
        
    // Convert AABB to center (midpoint of box) and extent (half width + half height + half depth)
    float3 center = (meshData.aabbMax + meshData.aabbMin) / 2.f;
    float3 extend = (meshData.aabbMax - meshData.aabbMin) / 2.f;
    
    float4 newCenter = mul(float4(center.xyz, 1.f), instanceData.transform);
    float4 newExtend = mul(float4(abs(extend.xyz), 0.f), abs(instanceData.transform)); // Just rotation + scale
    box.max = newCenter.xyz + newExtend.xyz;
    box.min = newCenter.xyz - newExtend.xyz;

    return box; 
}

// One thread per BRANCHING_FACTOR number of leaf nodes
static const int THREADS_PER_GROUP = 32;
[numthreads(THREADS_PER_GROUP, 1, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    // For each internal layer construct internal node
    // only thread % by BRANCHING_FACTOR^layerNum should be selected
    const int internalLayerCount = max(ceil(logBaseN((int) BRANCHING_FACTOR, numEntries)), 1);
    // If layer 0, offset should be 0
    const int isNotLayer0 = (int) ((internalLayerCount - 1) != 0);
    const int offset = max(0, seriesExponentSum(BRANCHING_FACTOR, (internalLayerCount) - 2)) * isNotLayer0;
    // Construct from leaves up and add first layer of internal nodes
    {
        bool internalInBounds = false;

        int internalNodeIndex = offset + (int) (DispatchThreadID.x);
        BoundingBox groupBox;
        BVHInternalNode internalNode;
        internalNode.childCount = -1;
        internalNode.flags = 0;
        internalNode.flags |= FLAG_HAS_LEAF_NODES;
        internalNode.maxAABB = float3(-1.f, 1.f, -1.f);
        internalNode.minAABB = float3(-1.f, 1.f, -1.f);
        for (int i = 0; i < BRANCHING_FACTOR; i++)
        {
            BVHLeafNode leafNode;
            leafNode.parentNode = -1;
            int instanceIndex = ((int) (DispatchThreadID.x) * BRANCHING_FACTOR) + i;
            if (instanceIndex >= numEntries)
            {
                break;
            }
            RadixEntry entry = inputBuffer[instanceIndex];
            BoundingBox box = GetRadixEntryAABB(entry);
            leafNode.minAABB = box.min;
            leafNode.maxAABB = box.max;
            leafNode.instanceIndex = entry.data;
            leafNode.parentNode = internalNodeIndex;

            if (i == 0)
            {
                internalNode.childCount = 0;
                internalNode.minAABB = leafNode.minAABB - SMALL_OFFSET;
                internalNode.maxAABB = leafNode.maxAABB + SMALL_OFFSET;
            }
            else
            {
                internalNode.minAABB = min(leafNode.minAABB - SMALL_OFFSET, internalNode.minAABB);
                internalNode.maxAABB = max(leafNode.maxAABB + SMALL_OFFSET, internalNode.maxAABB);
            }

            internalNode.childCount++;
            bvhLeafBuffer[instanceIndex] = leafNode;
        }
        bvhInternalBuffer[internalNodeIndex] = internalNode;
    }
}