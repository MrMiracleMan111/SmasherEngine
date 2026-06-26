#include "radix_util.hlsl"

// 32 SECTIONS
//      - 1 SPLITS
//          - 16 SPLIT ROW PREFIX SUMS
StructuredBuffer<RadixEntry> inputBuffer : register(t0, space0);
RWStructuredBuffer<SplitTable> countBuffer : register(u0, space1);
RWStructuredBuffer<SplitTable> prefixCountBuffer : register(u1, space1);
RWStructuredBuffer<RadixEntry> outputBuffer : register(u2, space1);

cbuffer UBO : register(b0, space2)
{
    int numEntries;
    int splitTableIndex; // 0 1 2 3 4 5 6 7 8
    uint padding1;
    uint padding2;
};


// Dispatch(NUM_SECTIONS, ROWS_PER_SPLIT_TABLE, 1)
// 1 thread group per table row per section
// Single Threadgroup (16 x 1 x 1)
//   - 16 threads will read each split table cell however each
//         thread will be checking for different value (ex. first
//         thread will check for value == 0, second thread value == 1).
//      
[numthreads(THREADS_PER_SPLIT_TABLE_ROW, 1, 1)]
void CSMain(uint3 GroupThreadId : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{
    const int SECTION_SIZE = max(ceil((float) numEntries / (float) NUM_SECTIONS), 32);
    int sectionIndex = GroupId.x; // 0 1 2 3 ... NUM_SECTIONS
    int rowIndex = GroupId.y; // 0 1 2 3 ... 13 14 15 (which possible split value)
    int threadOffset = GroupThreadId.x; // 0 1 2 3 4 ... THREADS_PER_SPLIT_TABLE_ROW
    int subkeyOffset = BITS_PER_SPLIT * splitTableIndex; // 0 4 8 12 16 20 24 28

    // Clear Count Buffer;
    if (threadOffset == 0)
    {
        countBuffer[sectionIndex].row[rowIndex] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    int iterations = ceil((float) SECTION_SIZE / (float) THREADS_PER_SPLIT_TABLE_ROW);
    int count = 0;
    for (int i = 0; i < iterations; ++i)
    {
        int indexInSection = (i * THREADS_PER_SPLIT_TABLE_ROW) + threadOffset;
        int inputBuffIndex = (sectionIndex * SECTION_SIZE) + (i * THREADS_PER_SPLIT_TABLE_ROW) + threadOffset;

        // Out of bounds check
        if (inputBuffIndex >= numEntries || indexInSection >= SECTION_SIZE)
        {
            break;
        }
        
        uint key = inputBuffer[inputBuffIndex].key;
        uint subkey = GetSubkey(key, subkeyOffset);
        count += (int)(subkey == rowIndex);
    }
    InterlockedAdd(countBuffer[sectionIndex].row[rowIndex], count);
}