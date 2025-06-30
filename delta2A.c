#include "zigzag.h"
#include "simple8b.h"
#include "delta2A.h"

int delta2Calculate(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int64 preVal = 0;
    int64 preDelta = 0;

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 val = BufferRead(pIn, pDesc->eachValSize);
        int64 delta = val - preVal;
        int64 delta2 = delta - preDelta;

        preVal = val;
        BufferWrite(pOut, pDesc->eachValSize, delta);
    }
    BufferFinishWrite(pOut);

    return pOut->len;
}

int delta2Recover(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int64 preVal = 0;
    int64 preDelta = 0;

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 delta2 = BufferRead(pIn, pDesc->eachValSize);
        int64 delta = preDelta + delta2;
        preDelta = delta;
        int64 val = preVal + delta;
        preVal = val;
        BufferWrite(pOut, pDesc->eachValSize, val);
    }
    BufferFinishWrite(pOut);

    return pOut->len;
}

int delta2ACompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {

}

int delta2ADecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {

}
