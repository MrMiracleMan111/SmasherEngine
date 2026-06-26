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

// INPUT BUFFER
//     ___________________________________
//    |    1   |    2   |   3    |   4    |
//    |________|________|________|________|
//         |        |       |        |
//         |        |       |        |
//         V        V       V        V
//    Table (0) Table (0) Table (0) Table (0)      [Split tables for first 4 bits]
//

// Dispatch(1, 1, 1)
// 8 threadgroup per section
// Single Threadgroup (16 x 1 x 1)
//   - 16 threads will read each split table cell however each
//         thread will be checking for different value (ex. first
//         thread will check for value == 0, second thread value == 1).
//      
[numthreads(NUM_SECTIONS, 1, 1)]
void CSMain(uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{
    // Prefix Count
    {
        // Prefix Count Memory Structure
        // 32 SECTIONS
        //      - 1 SPLIT_TABLE
        //          - 16 SPLIT_TABLE_ROWS
    
        // Only ONE thread group is dispatched
        // ONE thread will perform prefix sum per table
        int prev = 0;
        int sum = 0;
        int sectionIndex = GroupThreadID.x;
        for (int i = 0; i < ROWS_PER_SPLIT_TABLE; ++i)
        {
            prev = countBuffer[sectionIndex].row[i];
            prefixCountBuffer[sectionIndex].row[i] = sum;
            sum += prev;
        }
    }
}