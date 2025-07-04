#include "zigzag.h"

int zigzagCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    assert(pDesc->eachValSize > 0);

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 val = BufferReadSigned(pIn, pDesc->eachValSize);
        uint64 encoded = (val << 1) ^ (val >> (sizeof(val) * 8 - 1));
        BufferWrite(pOut, pDesc->eachValSize, encoded);
    }

    assert(pIn->readPos == pIn->len);
    BufferFinishWrite(pOut);
    return pOut->len;
}

int zigzagDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    assert(pDesc->eachValSize > 0);

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        uint64 val = BufferReadUnsigned(pIn, pDesc->eachValSize);
        val = (val >> 1) ^ -(val & 1);
        BufferWrite(pOut, pDesc->eachValSize, val);
    }

    assert(pIn->readPos == pIn->len);
    BufferFinishWrite(pOut);
    return pOut->len;
}
