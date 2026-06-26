#include "radix_util.hlsl"
#include "bvh_util.hlsl"

cbuffer UBO : register(b0, space2)
{
    int numEntries;
};


// Sorted by Morton Code Key
StructuredBuffer<StaticMeshData> instancesBuffer : register(t0, space0);
RWStructuredBuffer<BVHInternalNode> bvhInternalBuffer : register(u0, space1);
RWStructuredBuffer<BVHLeafNode> bvhLeafBuffer : register(u1, space1);


static const int THREADS_PER_GROUP = 256;
[numthreads(THREADS_PER_GROUP, 1, 1)]
void CSMain(uint3 GroupThreadId : SV_GroupThreadID)
{
    // Consturct bounding boxes for internal nodes
    // from bottom up
    
    // For each internal layer construct internal node
    // only thread % by BRANCHING_FACTOR^layerNum should be selected
    const int internalLayerCount = max(ceil(logBaseN(BRANCHING_FACTOR, numEntries)), 1);
    // Internal layer above leaf nodes was already handled in prior pass
    if (internalLayerCount < 2)
    {
        return;
    }
        
    // So far we've added, leaf nodes, and lowest internal node layer
    // now we need to add all other internal node layers.
    // Since tree is left balanced, all internal node layers (outside of
    // the ones containing leaf nodes) should have BRANCHING_FACTOR 
    // number of children per internal node.
    
    // Add tree internal nodes
    {   
        // We already added the parents for all leaf nodes
        // (layerCount is range [1 ... N])
        // (layer is range [0 ... N-1])
        
        int layer = (internalLayerCount - 1) - 1;
        while (layer >= 0)
        {
            int layerSize = pow(BRANCHING_FACTOR, layer);
            int iterations = ceil((float) layerSize / (float) THREADS_PER_GROUP);
            int offset = max(1, seriesExponentSum(BRANCHING_FACTOR, (layer - 1)));
            // If layer 0, offset should be 0
            int isNotLayer0 = (int)(layer != 0);
            offset = offset * isNotLayer0;

            // Split up nodes in layer amongst the threads
            for (int i = 0; i < iterations; i++)
            {
                // index of internal node in layer
                int indexInLayer = (THREADS_PER_GROUP * i) + GroupThreadId.x;
                int globalBVHIndex = offset + indexInLayer;
                if (indexInLayer >= layerSize)
                {
                    break;
                }
                
                BVHInternalNode parentNode;
                parentNode.childCount = BRANCHING_FACTOR;
                parentNode.flags = 0;
                // Process internal node children
                for (int j = 0; j < BRANCHING_FACTOR; j++)
                {                    
                    int childIndex = BVHNodeGetChildIndex(globalBVHIndex, j);
                    BVHInternalNode childNode = bvhInternalBuffer[childIndex];
                    if (childNode.childCount > 0)
                    {
                        if (j == 0)
                        {
                            parentNode.maxAABB = childNode.maxAABB + SMALL_OFFSET;
                            parentNode.minAABB = childNode.minAABB - SMALL_OFFSET;
                        }
                        parentNode.maxAABB = max(childNode.maxAABB + SMALL_OFFSET, parentNode.maxAABB);
                        parentNode.minAABB = min(childNode.minAABB - SMALL_OFFSET, parentNode.minAABB);
                    }
                    else if (j == 0)
                    {
                        parentNode.childCount = -1;
                        parentNode.maxAABB = float3(0.f, 0.f, 0.f);
                        parentNode.minAABB = float3(0.f, 0.f, 0.f);
                        continue;
                    }
                }
                bvhInternalBuffer[globalBVHIndex] = parentNode;
                }
            
            layer--;
            AllMemoryBarrierWithGroupSync();
        }
    }
}