#include "rle.h"

#define RLE_MIN_REPEATS     4
#define RLE_MAX_REPEATS     0x7fff

// different m_eachValSize, different marker used, just only repeating 0xFE.
// m_eachValSize is the index of this array.
const uint64 gRleMarker[] = {(uint64)0,
    (uint64)0x00000000000000FE,
    (uint64)0x000000000000FEFE,
    (uint64)0x0000000000FEFEFE,
    (uint64)0x00000000FEFEFEFE,
    (uint64)0x000000FEFEFEFEFE,
    (uint64)0x0000FEFEFEFEFEFE,
    (uint64)0x00FEFEFEFEFEFEFE,
    (uint64)0xFEFEFEFEFEFEFEFE
};

static bool isSymbolEqualsRleMarker(uint64 symbol, int eachValSize) {
    return symbol == gRleMarker[eachValSize];
    // int64 mask = 0;
    // byte *pTmp = (byte*)(&mask);
    // for (int i = 0; i < eachValSize; i++) {
    //     pTmp[i] = 0xFF;
    // }

    // symbol &= mask;
    // return marker == symbol;
}

// static int rleNonRunsNeedSpace(CUDesc *pDesc, bool equalToMarker, int repeats)
// {
//     assert(repeats > 0 && repeats < RLE_MIN_REPEATS);
//     return equalToMarker ? (pDesc->eachValSize + 1) : (pDesc->eachValSize  * repeats);
// }

// static int rleRunsNeedSpace(CUDesc *pDesc, int repeats)
// {
//     assert(repeats >= RLE_MIN_REPEATS && repeats <= RLE_MAX_REPEATS);
//     return (pDesc->eachValSize + (repeats < 128 ? 1 : 2) + pDesc->eachValSize);
// }

static void rleWriteRuns(CUDesc *pDesc, int64 symbol, int repeat, Buffer *pOut)
{
    assert(repeat >= RLE_MIN_REPEATS && repeat <= RLE_MAX_REPEATS);
    BufferWrite(pOut, pDesc->eachValSize, gRleMarker[pDesc->eachValSize]);
    if (repeat >= 128) {
        *(uint8*)(pOut->buf + pOut->writePos) = (uint8)((repeat | 0x8000) >> 8);
        *(uint8*)(pOut->buf + pOut->writePos + 1) = (uint8)(repeat & 0xff);
        pOut->writePos += sizeof(int16);
    } else {
        BufferWrite(pOut, sizeof(int8), repeat);
    }
    BufferWrite(pOut, pDesc->eachValSize, symbol);
}

static void rleWriteNonRuns(CUDesc *pDesc, int64 symbol, int repeat, Buffer *pOut)
{
    assert(repeat < RLE_MIN_REPEATS);
    if (likely(!isSymbolEqualsRleMarker(symbol, pDesc->eachValSize))) {
        // in normal case Non-Runs will be written plainly into out-buffer.
        for (int i = 0; i < repeat; i++) {
            BufferWrite(pOut, pDesc->eachValSize, symbol);
        }
    } else {
        // Special case: this symbol is the same to marker. the format is
        //      MARKER + REPEAT
        // where REPEAT is in [1, RleMinRepeats - 1] and only holds one byte.
        // In normal runs, the storage format is:
        //      MARKER + REPEAT + SYMBOL
        // where REPEAT is equal to or greater than RleMinRepeats.
        // it's safe to convert repeat to CHAR type.
        BufferWrite(pOut, pDesc->eachValSize, symbol);
        BufferWrite(pOut, 1, repeat);
    }
}

int rleCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    assert(pDesc->eachValSize > 0);
    uint64 data1 = 0;
    uint64 data2 = 0;
    int count = 0;

    data1 = BufferReadUnsigned(pIn, pDesc->eachValSize);
    count = 1;
    if (likely(pIn->len >= (2 * pDesc->eachValSize))) {
        data2 = BufferReadUnsigned(pIn, pDesc->eachValSize);
        count = 2;

        // Main compression loop
        do {
            if (data1 == data2) {
                // scan a sequence of identical block data, until another exception happens
                while ((data1 == data2) && ((pIn->readPos + pDesc->eachValSize) <= pIn->len) &&
                       (count < RLE_MAX_REPEATS)) {
                    data2 = BufferReadUnsigned(pIn, pDesc->eachValSize);
                    ++count;
                }

                if (data1 == data2) {
                    if (count >= RLE_MIN_REPEATS)
                        rleWriteRuns(pDesc, data1, count, pOut);
                    else
                        rleWriteNonRuns(pDesc, data1, count, pOut);

                    // repeating data have been written, and then read the new data1;
                    count = 0;
                    if ((pIn->readPos + pDesc->eachValSize) <= pIn->len) {
                        data1 = BufferReadUnsigned(pIn, pDesc->eachValSize);
                        count = 1;
                    }
                } else {
                    --count;
                    assert(count > 0);
                    if (count >= RLE_MIN_REPEATS)
                        rleWriteRuns(pDesc, data1, count, pOut);
                    else
                        rleWriteNonRuns(pDesc, data1, count, pOut);
                    data1 = data2;
                    count = 1;
                }
            } else {
                assert(count == 2);
                rleWriteNonRuns(pDesc, data1, 1, pOut);
                data1 = data2;
                count = 1;
            }

            /* read the new data2 to continue to compress; */
            if ((pIn->readPos + pDesc->eachValSize) <= pIn->len) {
                data2 = BufferReadUnsigned(pIn, pDesc->eachValSize);
                assert(count == 1);
                count = 2;
            }

            /* make sure there is no input data when count is either 1 or 0; */
            assert(count == 2 || count == 1 || count == 0);
            assert((count >= 2) || ((pIn->readPos + pDesc->eachValSize) > pIn->len));
        } while (count >= 2);
    }

    /*
     * make sure all input-data are handled and compressed.
     * And at most one data is left out.
     */
    assert(count == 0 || count == 1);
    if (count == 1)
        rleWriteNonRuns(pDesc, data1, 1, pOut);
    BufferFinishWrite(pOut);
    assert(pIn->readPos == pIn->len);
    return pOut->writePos;
}

int rleDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int outcnt = 0;

    // Main decompression loop
    do {
        uint64 symbol = BufferReadUnsigned(pIn, pDesc->eachValSize);

        // maybe We had a marker data, check it first
        if (isSymbolEqualsRleMarker(symbol, pDesc->eachValSize)) {
            uint8 markerCount = *(uint8*)(pIn->buf + pIn->readPos);

            if (markerCount >= RLE_MIN_REPEATS) {
                // [RLE_MIN_REPEATS, RLE_MAX_REPEATS] indicates the compressed Runs data.
                uint16 symbolCount = 0;

                // the first bit represents whether tow bytes are
                // used to store the length info.
                if (markerCount & 0x80) {
                    symbolCount = ((uint16)(markerCount << 8) + *(uint8*)(pIn->buf + pIn->readPos + 1)) & 0x7fff;
                    pIn->readPos += sizeof(uint16);
                } else {
                    symbolCount = markerCount;
                    pIn->readPos += sizeof(int8);
                }
                assert(symbolCount >= RLE_MIN_REPEATS && symbolCount <= RLE_MAX_REPEATS);

                symbol = BufferReadUnsigned(pIn, pDesc->eachValSize);
                for (uint16 i = 0; i < symbolCount; ++i) {
                    BufferWrite(pOut, pDesc->eachValSize, symbol);
                }
                outcnt += symbolCount;
            } else {
                // [1, RleMinRepeats - 1] indicates that symbol is the same to marker itself,
                // and repeat time is markerCount.
                pIn->readPos += sizeof(int8);
                assert(markerCount > 0);
                for (uint8 i = 0; i < markerCount; ++i) {
                    BufferWrite(pOut, pDesc->eachValSize, symbol);
                }
                outcnt += markerCount;
            }
        } else {
            // No marker, copy the plain data
            BufferWrite(pOut, pDesc->eachValSize, symbol);
            ++outcnt;
        }
    } while (likely((pIn->readPos + pDesc->eachValSize) <= pIn->len));

    assert(pIn->readPos == pIn->len);
    BufferFinishWrite(pOut);
    return (pDesc->eachValSize * outcnt);
}

void rleDumpCompressed(CUDesc *pDesc, Buffer *pIn) {
    int outcnt = 0;

    do {
        uint64 symbol = BufferReadUnsigned(pIn, pDesc->eachValSize);

        // maybe We had a marker data, check it first
        if (isSymbolEqualsRleMarker(symbol, pDesc->eachValSize)) {
            uint8 markerCount = *(uint8*)(pIn->buf + pIn->readPos);

            if (markerCount >= RLE_MIN_REPEATS) {
                // [RLE_MIN_REPEATS, RLE_MAX_REPEATS] indicates the compressed Runs data.
                uint16 symbolCount = 0;

                // the first bit represents whether tow bytes are
                // used to store the length info.
                if (markerCount & 0x80) {
                    symbolCount = ((uint16)(markerCount << 8) + *(uint8*)(pIn->buf + pIn->readPos + 1)) & 0x7fff;
                    pIn->readPos += sizeof(uint16);
                } else {
                    symbolCount = markerCount;
                    pIn->readPos += sizeof(int8);
                }
                assert(symbolCount >= RLE_MIN_REPEATS && symbolCount <= RLE_MAX_REPEATS);

                symbol = BufferReadUnsigned(pIn, pDesc->eachValSize);
                printf("%5d %ld\n", symbolCount, symbol);
                outcnt += symbolCount;
            } else {
                // [1, RleMinRepeats - 1] indicates that symbol is the same to marker itself,
                // and repeat time is markerCount.
                pIn->readPos += sizeof(int8);
                assert(markerCount > 0);
                printf("%5d %ld\n", markerCount, symbol);
                outcnt += markerCount;
            }
        } else {
            // No marker, copy the plain data
            printf("%5d %ld\n", 1, symbol);
            ++outcnt;
        }
    } while (likely((pIn->readPos + pDesc->eachValSize) <= pIn->len));

    assert(pIn->readPos == pIn->len);
    assert(outcnt == pDesc->count);
}
