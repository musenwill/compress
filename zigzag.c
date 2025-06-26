#include "zigzag.h"

int zigzagCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    assert(pDesc->eachValSize > 0);

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 val = BufferRead(pIn, pDesc->eachValSize);
        val = (val << 1) ^ (val >> (sizeof(val) * 8 - 1));
        BufferWrite(pOut, pDesc->eachValSize, val);
    }

    assert(pIn->readPos == pIn->len);
    pOut->len = pOut->writePos;
    return OK;
}

int zigzagDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    assert(pDesc->eachValSize > 0);

    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        int64 val = BufferRead(pIn, pDesc->eachValSize);
        val = (val >> 1) ^ -(val & 1);
        BufferWrite(pOut, pDesc->eachValSize, val);
    }

    assert(pIn->readPos == pIn->len);
    pOut->len = pOut->writePos;
    return OK;
}

void zigzagUT() {
    int ret = OK;
    byte origin[8] = {1, -2, 3, -4, 5, -6, 7, -8};
    byte compressed[8] = {2, 3, 6, 7, 10, 11, 14, 15};

    Buffer *pPlain = NULL;
    Buffer *pCompressed = NULL;
    Buffer *pDecompressed = NULL;
    ret = createBuffer(sizeof(origin), &pPlain);
    assert(ret >= 0);
    ret = createBuffer(sizeof(compressed), &pCompressed);
    assert(ret >= 0);
    ret = createBuffer(sizeof(origin), &pDecompressed);
    assert(ret >= 0);

    memcpy(pPlain->buf, origin, sizeof(origin));
    pPlain->len = sizeof(origin);

    CUDesc desc = {0};
    desc.eachValSize = 1;
    ret = zigzagCompress(&desc, pPlain, pCompressed);
    assert(ret >= 0);
    if (memcmp(compressed, pCompressed->buf, sizeof(compressed)) != 0) {
        printf("zigzag expect compress result: \n");
        dumpHexBuffer(compressed, sizeof(compressed));
        printf("zigzag actual compress result: \n");
        dumpHexBuffer(pCompressed->buf, pCompressed->len);
    }

    ret = zigzagDecompress(&desc, pCompressed, pDecompressed);
    assert(ret >= 0);
    if (memcmp(origin, pDecompressed->buf, sizeof(origin)) != 0) {
        printf("zigzag expect decompress result: \n");
        dumpHexBuffer(origin, sizeof(origin));
        printf("zigzag actual decompress result: \n");
        dumpHexBuffer(pDecompressed->buf, pDecompressed->len);
    }

    destroyBuffer(pPlain);
    destroyBuffer(pCompressed);
    destroyBuffer(pDecompressed);
}
