#include "zigzag.h"
#include "simple8b.h"

static int deltaCalculate(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 val = BufferReadSigned(pIn, pDesc->eachValSize);
        int64 delta = val - pDesc->minValue;
        BufferWrite(pOut, pDesc->eachValSize, delta);
    }
    BufferFinishWrite(pOut);

    return pOut->len;
}

static int deltaRecover(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 delta = BufferReadSigned(pIn, pDesc->eachValSize);
        int64 val = pDesc->minValue + delta;
        BufferWrite(pOut, pDesc->eachValSize, val);
    }
    BufferFinishWrite(pOut);

    return pOut->len;
}

int deltaDCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
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

    CUDesc simple8bDesc = {};
    simple8bDesc.eachValSize = pDesc->eachValSize;
    simple8bDesc.count = pDesc->count;
    simple8bDesc.minValue = 1;
    ret = simple8bCompress(&simple8bDesc, pDeltaCalculated, pOut);

l_end:
    if (NULL != pDeltaCalculated) {
        destroyBuffer(pDeltaCalculated);
    }
    return ret;
}

int deltaDDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Buffer *pSimple8bDecompressed = NULL;

    ret = createBuffer(pOut->bufSize, &pSimple8bDecompressed);
    if (ret < 0) {
        goto l_end;
    }

    CUDesc simple8bDesc = {};
    simple8bDesc.eachValSize = pDesc->eachValSize;
    simple8bDesc.count = pDesc->count;
    simple8bDesc.minValue = 1;
    ret = simple8bDecompress(&simple8bDesc, pIn, pSimple8bDecompressed);
    if (ret < 0) {
        goto l_end;
    }
    return deltaRecover(pDesc, pSimple8bDecompressed, pOut);

l_end:
    if (NULL != pSimple8bDecompressed) {
        destroyBuffer(pSimple8bDecompressed);
    }
    return ret;
}
