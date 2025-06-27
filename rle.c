#include "rle.h"

#define RLE_MIN_REPEATS     4
#define RLE_MAX_REPEATS     0x7fff

// different m_eachValSize, different marker used, just only repeating 0xFE.
// m_eachValSize is the index of this array.
const int64 gRleMarker[] = {(int64)0,
    (int64)0x00000000000000FE,
    (int64)0x000000000000FEFE,
    (int64)0x0000000000FEFEFE,
    (int64)0x00000000FEFEFEFE,
    (int64)0x000000FEFEFEFEFE,
    (int64)0x0000FEFEFEFEFEFE,
    (int64)0x00FEFEFEFEFEFEFE,
    (int64)0xFEFEFEFEFEFEFEFE
};

static bool isSymbolEqualsRleMarker(int64 symbol, int eachValSize) {
    int64 marker = gRleMarker[eachValSize];
    int64 mask = 0;
    byte *pTmp = (byte*)(&mask);
    for (int i = 0; i < eachValSize; i++) {
        pTmp[i] = 0xFF;
    }

    symbol &= mask;
    return marker == symbol;
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
    int64 data1 = 0;
    int64 data2 = 0;
    int count = 0;

    data1 = BufferRead(pIn, pDesc->eachValSize);
    count = 1;
    if (likely(pIn->len >= (2 * pDesc->eachValSize))) {
        data2 = BufferRead(pIn, pDesc->eachValSize);
        count = 2;

        // Main compression loop
        do {
            if (data1 == data2) {
                // scan a sequence of identical block data, until another exception happens
                while ((data1 == data2) && ((pIn->readPos + pDesc->eachValSize) <= pIn->len) &&
                       (count < RLE_MAX_REPEATS)) {
                    data2 = BufferRead(pIn, pDesc->eachValSize);
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
                        data1 = BufferRead(pIn, pDesc->eachValSize);
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
                data2 = BufferRead(pIn, pDesc->eachValSize);
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
    pOut->len = pOut->writePos;
    assert(pIn->readPos == pIn->len);
    return pOut->writePos;
}

int rleDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int outcnt = 0;

    // Main decompression loop
    do {
        int64 symbol = BufferRead(pIn, pDesc->eachValSize);

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

                symbol = BufferRead(pIn, pDesc->eachValSize);
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
    pOut->len = pOut->writePos;
    return (pDesc->eachValSize * outcnt);
}

void rleUT() {
    int ret = OK;
    byte origin[256] = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 0xFE, 0xFE};
    byte compressed[] = {1, 2, 2, 3, 3, 3, 0xFE, 4, 4, 0xFE, 5, 5, 0xFE, 2, 0xFE, 0x80, 0xEF, 0};

    Buffer *pPlain = NULL;
    Buffer *pCompressed = NULL;
    Buffer *pDecompressed = NULL;
    ret = createBuffer(sizeof(origin), &pPlain);
    assert(ret >= 0);
    ret = createBuffer(sizeof(compressed), &pCompressed);
    assert(ret >= 0);
    ret = createBuffer(sizeof(origin), &pDecompressed);
    assert(ret >= 0);

    memcpy(pPlain->buf, origin, sizeof(origin));
    pPlain->len = sizeof(origin);

    CUDesc desc = {0};
    desc.eachValSize = 1;
    ret = rleCompress(&desc, pPlain, pCompressed);
    assert(ret >= 0);
    if (memcmp(compressed, pCompressed->buf, sizeof(compressed)) != 0) {
        printf("rle expect compress result: \n");
        dumpHexBuffer(compressed, sizeof(compressed));
        printf("rle actual compress result: \n");
        dumpHexBuffer(pCompressed->buf, pCompressed->len);
    }

    ret = rleDecompress(&desc, pCompressed, pDecompressed);
    assert(ret >= 0);
    if (memcmp(origin, pDecompressed->buf, sizeof(origin)) != 0) {
        printf("rle expect decompress result: \n");
        dumpHexBuffer(origin, sizeof(origin));
        printf("rle actual decompress result: \n");
        dumpHexBuffer(pDecompressed->buf, pDecompressed->len);
    }

    destroyBuffer(pPlain);
    destroyBuffer(pCompressed);
    destroyBuffer(pDecompressed);
}
