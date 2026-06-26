static const int FLAG_HAS_LEAF_NODES = 1;
static const int FLAG_COMPLETED = 2;
static const uint BRANCHING_FACTOR = 4;
static const float3 SMALL_OFFSET = float3(0.01f, 0.01f, 0.01f);


struct BoundingBox
{
    float3 min;
    uint padding1;
    float3 max;
    uint padding2;
};

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

int BVHNodeParentIndex(int nodeIndex)
{
    return max(0, (nodeIndex - 1) / BRANCHING_FACTOR);
}

// Get Nth child of parent
int BVHNodeGetChildIndex(int parentIndex, int child)
{
    return max(0, (parentIndex * BRANCHING_FACTOR) + 1 + child);
}

// Returns a sum of all integers between a and b
// ex a = 2, b = 5
// output 2 + 3 + 4 + 5 = 14
int seriesSum(int a, int b)
{
    int diff = max(b - a, 0);
    return diff * ((a + b) / 2);
}

// Returns sum b^0 + b^1 + b^2 ... + b^n
int seriesExponentSum(int b, int n)
{
    n = max(0, n);
    return (1 - (pow(b, n + 1))) / (1 - b);
}

float logBaseN(int b, float x)
{
    return log2(x) / log2(b);
}