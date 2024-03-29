//===============================================================================
// Copyright (c) 2007-2016  Advanced Micro Devices, Inc. All rights reserved.
// Copyright (c) 2004-2006 ATI Technologies Inc.
//===============================================================================
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//
//  BC7_Decode.cpp : A reference decoder for BC7
//

#include "bc7_definitions.h"
#include "bc7_partitions.h"
#include "bc7_decode.h"
#include "bc7_utils.h"

//
// Bit reader - reads one bit from a buffer at the current bit offset
//              and increments the offset
//

CMP_DWORD BC7BlockDecoder::ReadBit(const CMP_BYTE base[]) {
    int             byteLocation;
    int             remainder;
    CMP_DWORD bit = 0;
    byteLocation = m_bitPosition/8;
    remainder = m_bitPosition % 8;

    bit = base[byteLocation];
    bit >>= remainder;
    bit &= 0x1;
    // Increment bit position
    m_bitPosition++;
    return (bit);
}


void BC7BlockDecoder::DecompressDualIndexBlock(double  out[MAX_SUBSET_SIZE][MAX_DIMENSION_BIG],
        CMP_BYTE   in[COMPRESSED_BLOCK_SIZE],
        CMP_DWORD  endpoint[2][MAX_DIMENSION_BIG]) {
    CMP_DWORD i, j, k;

    double  ramp[MAX_DIMENSION_BIG][1<<MAX_INDEX_BITS];
    CMP_DWORD   blockIndices[2][MAX_SUBSET_SIZE];

    CMP_DWORD   clusters[2];
    clusters[0] = 1 << bti_cpu[m_blockMode].indexBits[0];
    clusters[1] = 1 << bti_cpu[m_blockMode].indexBits[1];
    if(m_indexSwap) {
        CMP_DWORD   temp = clusters[0];
        clusters[0] = clusters[1];
        clusters[1] = temp;
    }

    GetRamp(endpoint,
            ramp,
            clusters,
            m_componentBits);

    // Extract the indices
    for(i=0; i<2; i++) {
        for(j=0; j<MAX_SUBSET_SIZE; j++) {
            blockIndices[i][j] = 0;
            // If this is a fixup index then clear the implicit bit
            if(j==0) {
                blockIndices[i][j] &= ~(1 << (bti_cpu[m_blockMode].indexBits[i]-1));
                for(k=0; k<bti_cpu[m_blockMode].indexBits[i]-1; k++) {
                    blockIndices[i][j] |= (CMP_DWORD)ReadBit(in) << k;
                }
            } else {
                for(k=0; k<bti_cpu[m_blockMode].indexBits[i]; k++) {
                    blockIndices[i][j] |= (CMP_DWORD)ReadBit(in) << k;
                }
            }
        }
    }

    // Generate block colours
    for(i=0; i<MAX_SUBSET_SIZE; i++) {
        out[i][COMP_ALPHA] = ramp[COMP_ALPHA][blockIndices[m_indexSwap^1][i]];
        out[i][COMP_RED] = ramp[COMP_RED][blockIndices[m_indexSwap][i]];
        out[i][COMP_GREEN] = ramp[COMP_GREEN][blockIndices[m_indexSwap][i]];
        out[i][COMP_BLUE] = ramp[COMP_BLUE][blockIndices[m_indexSwap][i]];
    }

    // Resolve the component rotation
    double swap;
    for(i=0; i<MAX_SUBSET_SIZE; i++) {
        switch(m_rotation) {
        case    0:
            // Do nothing
            break;
        case    1:
            // Swap A and R
            swap = out[i][COMP_ALPHA];
            out[i][COMP_ALPHA] = out[i][COMP_RED];
            out[i][COMP_RED] = swap;
            break;
        case    2:
            // Swap A and G
            swap = out[i][COMP_ALPHA];
            out[i][COMP_ALPHA] = out[i][COMP_GREEN];
            out[i][COMP_GREEN] = swap;
            break;
        case    3:
            // Swap A and B
            swap = out[i][COMP_ALPHA];
            out[i][COMP_ALPHA] = out[i][COMP_BLUE];
            out[i][COMP_BLUE] = swap;
            break;
        }
    }
}



void BC7BlockDecoder::DecompressBlock(double  out[MAX_SUBSET_SIZE][MAX_DIMENSION_BIG],
                                      CMP_BYTE   in[COMPRESSED_BLOCK_SIZE]) {
    CMP_DWORD           i,j;
    CMP_DWORD           *partitionTable;
    CMP_DWORD           blockIndices[MAX_SUBSET_SIZE];
    // Endpoints
    CMP_DWORD           endpoint[MAX_SUBSETS][2][MAX_DIMENSION_BIG];

    m_blockMode = 0;
    m_partition = 0;
    m_rotation = 0;
    m_indexSwap = 0;

    // Position the read pointer at the LSB of the block
    m_bitPosition = 0;

    while(!ReadBit(in) && (m_blockMode < 8)) {
        m_blockMode++;
    }

    if(m_blockMode > 7) {
        // Something really bad happened...
        return;
    }

    for(i=0; i<bti_cpu[m_blockMode].rotationBits; i++) {
        m_rotation |= ReadBit(in) << i;
    }
    for(i=0; i<bti_cpu[m_blockMode].indexModeBits; i++) {
        m_indexSwap |= ReadBit(in) << i;
    }

    for(i=0; i<bti_cpu[m_blockMode].partitionBits; i++) {
        m_partition |= ReadBit(in) << i;
    }



    if(bti_cpu[m_blockMode].encodingType == NO_ALPHA) {
        m_componentBits[COMP_ALPHA] = 0;
        m_componentBits[COMP_RED]   =
            m_componentBits[COMP_GREEN] =
                m_componentBits[COMP_BLUE]  = bti_cpu[m_blockMode].vectorBits / 3;
    } else if(bti_cpu[m_blockMode].encodingType == COMBINED_ALPHA) {
        m_componentBits[COMP_ALPHA] =
            m_componentBits[COMP_RED]   =
                m_componentBits[COMP_GREEN] =
                    m_componentBits[COMP_BLUE]  = bti_cpu[m_blockMode].vectorBits / 4;
    } else if(bti_cpu[m_blockMode].encodingType == SEPARATE_ALPHA) {
        m_componentBits[COMP_ALPHA] = bti_cpu[m_blockMode].scalarBits;
        m_componentBits[COMP_RED]   =
            m_componentBits[COMP_GREEN] =
                m_componentBits[COMP_BLUE]  = bti_cpu[m_blockMode].vectorBits / 3;
    }

    CMP_DWORD   subset, ep, component;
    // Endpoints are stored in the following order RRRR GGGG BBBB (AAAA) (PPPP)
    // i.e. components are packed together
    // Loop over components
    for(component=0; component < MAX_DIMENSION_BIG; component++) {
        // loop over subsets
        for(subset=0; subset<(int)bti_cpu[m_blockMode].subsetCount; subset++) {
            // Loop over endpoints
            for(ep=0; ep<2; ep++) {
                endpoint[subset][ep][component] = 0;
                for(j=0; j<m_componentBits[component]; j++) {
                    endpoint[subset][ep][component] |= ReadBit(in) << j;
                }
            }
        }
    }


    // Now get any parity bits
    if(bti_cpu[m_blockMode].pBitType != NO_PBIT) {
        for(subset=0; subset<(int)bti_cpu[m_blockMode].subsetCount; subset++) {
            CMP_DWORD   pBit[2];
            if(bti_cpu[m_blockMode].pBitType == ONE_PBIT) {
                pBit[0] = ReadBit(in);
                pBit[1] = pBit[0];
            } else if(bti_cpu[m_blockMode].pBitType == TWO_PBIT) {
                pBit[0] = ReadBit(in);
                pBit[1] = ReadBit(in);
            }

            for(component=0; component < MAX_DIMENSION_BIG; component++) {
                if(m_componentBits[component]) {
                    endpoint[subset][0][component] <<= 1;
                    endpoint[subset][1][component] <<= 1;
                    endpoint[subset][0][component] |= pBit[0];
                    endpoint[subset][1][component] |= pBit[1];
                }
            }
        }
    }

    if(bti_cpu[m_blockMode].pBitType != NO_PBIT) {
        // Now that we've unpacked the parity bits, update the component size information
        // for the ramp generator
        for(j=0; j<MAX_DIMENSION_BIG; j++) {
            if(m_componentBits[j]) {
                m_componentBits[j] += 1;
            }
        }
    }

    // If this block has two independent sets of indices then put it to that decoder
    if(bti_cpu[m_blockMode].encodingType == SEPARATE_ALPHA) {
        DecompressDualIndexBlock(out, in, endpoint[0]);
        return;
    }

    CMP_DWORD   fixup[MAX_SUBSETS] = {0, 0, 0};
    switch(bti_cpu[m_blockMode].subsetCount) {
    case    3:
        fixup[1] = BC7_FIXUPINDICES[2][m_partition][1];
        fixup[2] = BC7_FIXUPINDICES[2][m_partition][2];
        break;
    case    2:
        fixup[1] = BC7_FIXUPINDICES[1][m_partition][1];
        break;
    default:
        break;
    }

    partitionTable = (CMP_DWORD*)BC7_PARTITIONS_CPU[bti_cpu[m_blockMode].subsetCount-1][m_partition];

    // Extract index bits
    for(i=0; i < MAX_SUBSET_SIZE; i++) {
        CMP_DWORD   p = partitionTable[i];
        blockIndices[i] = 0;
        CMP_DWORD   bitsToRead = bti_cpu[m_blockMode].indexBits[0];

        // If this is a fixup index then set the implicit bit
        if(i==fixup[p]) {
            blockIndices[i] &= ~(1 << (bitsToRead-1));
            bitsToRead--;
        }

        for(j=0; j<bitsToRead; j++) {
            blockIndices[i] |= ReadBit(in) << j;
        }
    }

    // Get the ramps
    CMP_DWORD   clusters[2];
    clusters[0] = clusters[1] = 1 << bti_cpu[m_blockMode].indexBits[0];

    // Colour Ramps
    double          c[MAX_SUBSETS][MAX_DIMENSION_BIG][1<<MAX_INDEX_BITS];

    for(i=0; i<(int)bti_cpu[m_blockMode].subsetCount; i++) {
        // Unpack the colours
        GetRamp(endpoint[i],
                c[i],
                clusters,
                m_componentBits);
    }

    // Generate the block colours.
    for(i=0; i<MAX_SUBSET_SIZE; i++) {
        for(j=0; j < MAX_DIMENSION_BIG; j++) {
            out[i][j] = c[partitionTable[i]][j][blockIndices[i]];
        }
    }
}
