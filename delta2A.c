#include "zigzag.h"
#include "simple8b.h"
#include "delta2A.h"

int delta2Calculate(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int64 preVal = 0;
    int64 preDelta = 0;

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 val = BufferReadSigned(pIn, pDesc->eachValSize);
        int64 delta = val - preVal;
        int64 delta2 = delta - preDelta;
        preDelta = delta;
        preVal = val;
        BufferWrite(pOut, pDesc->eachValSize, delta2);
    }
    BufferFinishWrite(pOut);

    return pOut->len;
}

int delta2Recover(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int64 preVal = 0;
    int64 preDelta = 0;

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 delta2 = BufferReadSigned(pIn, pDesc->eachValSize);
        int64 delta = preDelta + delta2;
        int64 val = preVal + delta;
        preDelta = delta;
        preVal = val;
        BufferWrite(pOut, pDesc->eachValSize, val);
    }
    BufferFinishWrite(pOut);

    return pOut->len;
}

int delta2ACompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Buffer *pDelta2Calculated = NULL;
    Buffer *pZigzagCompressed = NULL;

    ret = createBuffer(pIn->len, &pDelta2Calculated);
    if (ret < 0) {
        goto l_end;
    }
    ret = createBuffer(pIn->len, &pZigzagCompressed);
    if (ret < 0) {
        goto l_end;
    }

    // calculate delta first
    ret = delta2Calculate(pDesc, pIn, pDelta2Calculated);
    if (ret < 0) {
        goto l_end;
    }

    // zigzag
    ret = zigzagCompress(pDesc, pDelta2Calculated, pZigzagCompressed);
    if (ret < 0) {
        goto l_end;
    }

    ret = simple8bCompress(pDesc, pZigzagCompressed, pOut);

l_end:
    if (NULL != pDelta2Calculated) {
        destroyBuffer(pDelta2Calculated);
    }
    if (NULL != pZigzagCompressed) {
        destroyBuffer(pZigzagCompressed);
    }
    return ret;
}

int delta2ADecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
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
    return delta2Recover(pDesc, pZigzagDecompressed, pOut);

l_end:
    if (NULL != pSimple8bDecompressed) {
        destroyBuffer(pSimple8bDecompressed);
    }
    if (NULL != pZigzagDecompressed) {
        destroyBuffer(pZigzagDecompressed);
    }
    return ret;
}
