#include "radix_util.hlsl"

cbuffer UBO : register(b0, space2)
{
    int numEntries;
    int splitTableIndex; // 0 1 2 3 4 5 6 7 8
    uint padding1;
    uint padding2;
};

// 32 SECTIONS
//      - 1 SPLITS
//          - 16 SPLIT ROW PREFIX SUMS
StructuredBuffer<RadixEntry> inputBuffer : register(t0, space0);
RWStructuredBuffer<SplitTable> countBuffer : register(u0, space1);
RWStructuredBuffer<SplitTable> prefixCountBuffer : register(u1, space1);
RWStructuredBuffer<RadixEntry> outputBuffer : register(u2, space1);

// 32 Sections x 16 threads (operate on one split and each row in that split table)

// Dispatch (NUM_SECTIONS, 1, 1)

[numthreads(ROWS_PER_SPLIT_TABLE, 1, 1)]
void CSMain(uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{        
    const int SECTION_SIZE = max(ceil((float) numEntries / (float) NUM_SECTIONS), 32);

    int sectionIndex = GroupId.x;
    int splitTableRowIndex = GroupThreadID.x;
    
    // Global Offsets of for the current section's split
    int globalOffset = 0;

    // Calculate Global Prefix Offset
    // for all rows of the current split
    {
        int offsetSum = 0;
        for (int i = 0; i < NUM_SECTIONS; ++i)
        {
            offsetSum += prefixCountBuffer[i].row[splitTableRowIndex];
        }

        for (int j = 0; j < sectionIndex; ++j)
        {
            offsetSum += countBuffer[j].row[splitTableRowIndex];
        }
        globalOffset = offsetSum;
    }
    GroupMemoryBarrierWithGroupSync();

    // Reorder
    {
        // 32 Sections x 16 threads (operate on one split and each row in that split table)
        // Calculate Offset from prior 
        
        // Scatter based on inputs
        int subkeyOffset = BITS_PER_SPLIT * splitTableIndex; // 0 4 8 12 16 20 24 28
        
        // Scatter if input subkey matches splitTableRowIndex
        int count = 0;
        for (int i = 0; i < SECTION_SIZE; ++i)
        {
            int inputIndex = (sectionIndex * SECTION_SIZE) + i;
            if (inputIndex >= numEntries)
            {
                break;
            }
            int tmp = globalOffset + count;
            RadixEntry entry = inputBuffer[inputIndex];
            uint subkey = GetSubkey(entry.key, subkeyOffset);
            if (subkey == splitTableRowIndex)
            {
                outputBuffer[tmp] = entry;
                count++;
            }
        }
    }
}