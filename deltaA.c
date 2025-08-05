#include "zigzag.h"
#include "simple8b.h"
#include "deltaA.h"

int deltaCalculate(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int64 preVal = 0;

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 val = BufferReadSigned(pIn, pDesc->eachValSize);
        int64 delta = val - preVal;
        preVal = val;
        BufferWrite(pOut, pDesc->eachValSize, delta);
    }
    BufferFinishWrite(pOut);

    return pOut->len;
}

int deltaRecover(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int64 preVal = 0;

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 delta = BufferReadSigned(pIn, pDesc->eachValSize);
        int64 val = preVal + delta;
        preVal = val;
        BufferWrite(pOut, pDesc->eachValSize, val);
    }
    BufferFinishWrite(pOut);

    return pOut->len;
}

int deltaACompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Buffer *pDeltaCalculated = NULL;
    Buffer *pZigzagCompressed = NULL;

    ret = createBuffer(pIn->len, &pDeltaCalculated);
    if (ret < 0) {
        goto l_end;
    }
    ret = createBuffer(pIn->len, &pZigzagCompressed);
    if (ret < 0) {
        goto l_end;
    }

    // calculate delta first
    ret = deltaCalculate(pDesc, pIn, pDeltaCalculated);
    if (ret < 0) {
        goto l_end;
    }

    // zigzag
    ret = zigzagCompress(pDesc, pDeltaCalculated, pZigzagCompressed);
    if (ret < 0) {
        goto l_end;
    }

    CUDesc simple8bDesc = {};
    simple8bDesc.eachValSize = pDesc->eachValSize;
    simple8bDesc.count = pDesc->count;
    simple8bDesc.minValue = 1;
    ret = simple8bCompress(&simple8bDesc, pZigzagCompressed, pOut);

l_end:
    if (NULL != pDeltaCalculated) {
        destroyBuffer(pDeltaCalculated);
    }
    if (NULL != pZigzagCompressed) {
        destroyBuffer(pZigzagCompressed);
    }
    return ret;
}

int deltaADecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Buffer *pSimple8bDecompressed = NULL;
    Buffer *pZigzagDecompressed = NULL;

    ret = createBuffer(pOut->bufSize, &pSimple8bDecompressed);
    if (ret < 0) {
        goto l_end;
    }
    ret = createBuffer(pOut->bufSize, &pZigzagDecompressed);
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
    ret = zigzagDecompress(pDesc, pSimple8bDecompressed, pZigzagDecompressed);
    if (ret < 0) {
        goto l_end;
    }
    return deltaRecover(pDesc, pZigzagDecompressed, pOut);

l_end:
    if (NULL != pSimple8bDecompressed) {
        destroyBuffer(pSimple8bDecompressed);
    }
    if (NULL != pZigzagDecompressed) {
        destroyBuffer(pZigzagDecompressed);
    }
    return ret;
}

float32 deltaAEstimate(CUDesc *pDesc) {
    return (float32)SIMPLE8B_PACKING_BITS * (float32)pDesc->eachValSize / 
            (8.0 * BIT_WIDTH(2 * pDesc->avgAbsDelta));
}
