#include "zigzag.h"
#include "rle.h"
#include "delta2A.h"
#include "delta2B.h"

int delta2BCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
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

    ret = rleCompress(pDesc, pZigzagCompressed, pOut);

l_end:
    if (NULL != pDelta2Calculated) {
        destroyBuffer(pDelta2Calculated);
    }
    if (NULL != pZigzagCompressed) {
        destroyBuffer(pZigzagCompressed);
    }
    return ret;
}

int delta2BDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Buffer *pRleDecompressed = NULL;
    Buffer *pZigzagDecompressed = NULL;

    ret = createBuffer(pOut->bufSize, &pRleDecompressed);
    if (ret < 0) {
        goto l_end;
    }
    ret = createBuffer(pOut->bufSize, &pZigzagDecompressed);
    if (ret < 0) {
        goto l_end;
    }

    ret = rleDecompress(pDesc, pIn, pRleDecompressed);
    if (ret < 0) {
        goto l_end;
    }
    ret = zigzagDecompress(pDesc, pRleDecompressed, pZigzagDecompressed);
    if (ret < 0) {
        goto l_end;
    }
    return delta2Recover(pDesc, pZigzagDecompressed, pOut);

l_end:
    if (NULL != pRleDecompressed) {
        destroyBuffer(pRleDecompressed);
    }
    if (NULL != pZigzagDecompressed) {
        destroyBuffer(pZigzagDecompressed);
    }
    return ret;
}
