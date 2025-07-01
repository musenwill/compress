
#include "zigzag.h"
#include "bitpacking.h"

static int calculateBitWidth(CUDesc *pDesc) {
    int bitWidth = 1;

    if (pDesc->minValue == 0 && pDesc->maxValue == 0) {
        return bitWidth;
    }

    if (pDesc->minValue < 0) {
        int m = BIT_WIDTH((uint64)(((0 - pDesc->minValue) << 1) - 1));    // will be zigzaged
        if (pDesc->maxValue < 0) {
            bitWidth = m;
        } else {
            int n = BIT_WIDTH((uint64)(pDesc->maxValue << 1));
            bitWidth = m > n? m : n;
        }
    } else {
        bitWidth = BIT_WIDTH((uint64)(pDesc->maxValue));        // won't zigzag
    }
    return bitWidth;
}

int bitPackingCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Buffer *pZigzagCompressed = NULL;
    int bitWidth = calculateBitWidth(pDesc);

    if (pDesc->minValue < 0) {
        ret = createBuffer(pIn->len, &pZigzagCompressed);
        if (ret < 0) {
            goto l_end;
        }
        ret = zigzagCompress(pDesc, pIn, pZigzagCompressed);
        if (ret < 0) {
            goto l_end;
        }
        pIn = pZigzagCompressed;
    }

    BufferWriteBits(pOut, (uint64)bitWidth, 8);     // writer 1 byte header, mark bitwidth
    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 val = BufferRead(pIn, pDesc->eachValSize);
        BufferWriteBits(pOut, (uint64)val, bitWidth);
    }
    assert(pIn->readPos == pIn->len);
    BufferFinishWriteBits(pOut);
    ret = pOut->len;

l_end:
    if (NULL != pZigzagCompressed) {
        destroyBuffer(pZigzagCompressed);
    }
    return ret;
}

int bitPackingDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Buffer *pBitPackDecompressed = NULL;
    Buffer *pTmp = NULL;

    if (pDesc->minValue < 0) {
        ret = createBuffer(pOut->bufSize, &pTmp);
        if (ret < 0) {
            goto l_end;
        }
        pBitPackDecompressed = pTmp;
    } else {
        pBitPackDecompressed = pOut;
    }

    int bitWidth = BufferReadBits(pIn, 8);
    for (int i = 0; i < pDesc->count; i++) {
        int64 val = (int64)BufferReadBits(pIn, bitWidth);
        BufferWrite(pBitPackDecompressed, pDesc->eachValSize, val);
    }
    BufferFinishWrite(pBitPackDecompressed);

    if (pDesc->minValue < 0) {
        ret = zigzagDecompress(pDesc, pBitPackDecompressed, pOut);
        if (ret < 0) {
            goto l_end;
        }
    }
    ret = pOut->len;

l_end:
    if (NULL != pTmp) {
        destroyBuffer(pTmp);
    }
    return ret;
}
