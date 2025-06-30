#include "zigzag.h"
#include "rle.h"
#include "deltaA.h"
#include "deltaB.h"

int deltaBCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
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

    ret = rleCompress(pDesc, pZigzagCompressed, pOut);

l_end:
    if (NULL != pDeltaCalculated) {
        destroyBuffer(pDeltaCalculated);
    }
    if (NULL != pZigzagCompressed) {
        destroyBuffer(pZigzagCompressed);
    }
    return ret;
}

int deltaBDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
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

    ret = rleDecompress(pDesc, pIn, pSimple8bDecompressed);
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
