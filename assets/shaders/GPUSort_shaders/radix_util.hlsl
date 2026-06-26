// A Split Table is count
// of appearance of 0-15

static const int NUM_SECTIONS = 32;
static const int BITS_PER_SPLIT = 4;
static const int NUM_REORDER_PASSES = 32 / BITS_PER_SPLIT; // 8
static const int NUM_SPLIT_TABLES_PER_SECTION = 32 / BITS_PER_SPLIT; // 8
static const int ROWS_PER_SPLIT_TABLE = 1U << BITS_PER_SPLIT; // 16 possible values for 4 bit splits
static const int SIZE_SECTION = ROWS_PER_SPLIT_TABLE; // 8 * 16 int entries per section
static const int THREADS_PER_SPLIT_TABLE_ROW = 32;

struct SplitTable
{
    int row[ROWS_PER_SPLIT_TABLE];
};

/**

struct SplitTable
{
    int row1;int row2;int row3;int row4;int row5;int row6;int row7;int row8;
    int row9;int row10;int row11;int row12;int row13;int row14;int row15;int row16;
};

*/

struct RadixEntry
{
    uint key;
    int data;
    uint padding1;
    uint padding2;
};

struct StaticMeshData
{
    row_major float4x4 transform;
			// First 16 bits materialId (O) (index in materialProps array)
			// Second 16 bits MeshId (X)
            // XXXXXXXX XXXXXXXX OOOOOOOO OOOOOOOO
    uint materialIdMeshId;
    uint padding1;
    uint padding2;
    uint padding3;
};

struct MeshProps
{
    float3 aabbMin;
    uint resourceId;
    float3 aabbMax;
    uint padding;
};

struct MortonMeshData
{
    float3 aabbMin;
    uint mortonCode;
    float3 aabbMax;
    uint meshId;
};


// Retrieve 4 bit subkey at offset
uint GetSubkey(uint key, int offset)
{
    return 0x0F & (key >> offset);
}