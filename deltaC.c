#include "zigzag.h"
#include "bitpacking.h"
#include "deltaA.h"
#include "deltaC.h"

int deltaCCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Buffer *pDeltaCalculated = NULL;

    ret = createBuffer(pIn->len, &pDeltaCalculated);
    if (ret < 0) {
        goto l_end;
    }

    // calculate delta first
    ret = deltaCalculate(pDesc, pIn, pDeltaCalculated);
    if (ret < 0) {
        goto l_end;
    }

    int64 val = BufferRead(pDeltaCalculated, pDesc->eachValSize);
    BufferWrite(pOut, pDesc->eachValSize, val);
    CUDesc bitPackingDesc = {0};
    bitPackingDesc.eachValSize = pDesc->eachValSize;
    bitPackingDesc.minValue = pDesc->minDelta;
    bitPackingDesc.maxValue = pDesc->maxDelta;
    bitPackingDesc.count = pDesc->count - 1;
    ret = bitPackingCompress(&bitPackingDesc, pDeltaCalculated, pOut);
    if (ret < 0) {
        goto l_end;
    }

l_end:
    if (NULL != pDeltaCalculated) {
        destroyBuffer(pDeltaCalculated);
    }
    return ret;
}

int deltaCDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Buffer *pBitPackingDecompressed = NULL;

    ret = createBuffer(pOut->bufSize, &pBitPackingDecompressed);
    if (ret < 0) {
        goto l_end;
    }

    int64 val = BufferRead(pIn, pDesc->eachValSize);
    BufferWrite(pBitPackingDecompressed, pDesc->eachValSize, val);

    CUDesc bitPackingDesc = {0};
    bitPackingDesc.eachValSize = pDesc->eachValSize;
    bitPackingDesc.minValue = pDesc->minDelta;
    bitPackingDesc.maxValue = pDesc->maxDelta;
    bitPackingDesc.count = pDesc->count - 1;
    ret = bitPackingDecompress(&bitPackingDesc, pIn, pBitPackingDecompressed);
    if (ret < 0) {
        goto l_end;
    }
    return deltaRecover(pDesc, pBitPackingDecompressed, pOut);

l_end:
    if (NULL != pBitPackingDecompressed) {
        destroyBuffer(pBitPackingDecompressed);
    }
    return ret;
}
