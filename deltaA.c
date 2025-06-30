#include "zigzag.h"
#include "simple8b.h"
#include "deltaA.h"

int deltaCalculate(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int64 preVal = 0;

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 val = BufferRead(pIn, pDesc->eachValSize);
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
        int64 delta = BufferRead(pIn, pDesc->eachValSize);
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

    ret = simple8bCompress(pDesc, pZigzagCompressed, pOut);

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

    ret = simple8bDecompress(pDesc, pIn, pSimple8bDecompressed);
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
