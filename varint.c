#include "zigzag.h"
#include "varint.h"

static void varintWriteVal(uint64 val, int bitWidth, Buffer *pOut) {
    int remainBits = bitWidth;
    while (remainBits > 0) {
        byte b = (byte)((val >> (bitWidth - remainBits)) & 0x7F);
        if (remainBits > 7) {
            b |= 0x80;
            remainBits -= 7;
        } else {
            remainBits = 0;
        }
        BufferWrite(pOut, sizeof(b), b);
    }
    assert(remainBits == 0);
}

static uint64 varintReadVal(Buffer *pIn) {
    uint64 val = 0;
    byte b = 0;
    int readBits = 0;
    do {
        b = BufferRead(pIn, sizeof(b));
        val |= ((uint64)(b & 0x7f) << readBits);
        readBits += 7;
    } while (b & 0x80);

    return val;
}

int varintCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Buffer *pZigzagCompressed = NULL;

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

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        uint64 val = (uint64)BufferRead(pIn, pDesc->eachValSize);
        int bitWidth = BIT_WIDTH(val);
        varintWriteVal(val, bitWidth, pOut);
    }
    assert(pIn->readPos == pIn->len);
    BufferFinishWrite(pOut);
    ret = pOut->len;

l_end:
    if (NULL != pZigzagCompressed) {
        destroyBuffer(pZigzagCompressed);
    }
    return ret;
}

int varintDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Buffer *pVarintDecompressed = NULL;
    Buffer *pTmp = NULL;

    if (pDesc->minValue < 0) {
        ret = createBuffer(pOut->bufSize, &pTmp);
        if (ret < 0) {
            goto l_end;
        }
        pVarintDecompressed = pTmp;
    } else {
        pVarintDecompressed = pOut;
    }

    while (pIn->readPos < pIn->len) {
        int64 val = (int64)varintReadVal(pIn);
        BufferWrite(pVarintDecompressed, pDesc->eachValSize, val);
    }
    BufferFinishWrite(pVarintDecompressed);

    if (pDesc->minValue < 0) {
        ret = zigzagDecompress(pDesc, pVarintDecompressed, pOut);
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
