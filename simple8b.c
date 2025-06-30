#include "zigzag.h"
#include "simple8b.h"

#define SELECTOR_NUM    (16)
#define NBITS(bitWidth) ((0xFFFFFFFFFFFFFFFF << (bitWidth)) ^ 0xFFFFFFFFFFFFFFFF)
#define SIMPLE8B_MAX_SUPPORT_VAL    (0x0FFFFFFFFFFFFFFFUL)

typedef struct {
    int len;
    int readPos;
    int writePos;
    uint64 vals[COMPRESS_BATCHSIZE];
} Simple8bArray;

static inline void Simple8bArrayReadPosAdd(Simple8bArray *pArray, int delta) {
    assert(pArray->readPos + delta <= pArray->len);
    pArray->readPos += delta;
}

typedef uint64 (*simple8bPack)(Simple8bArray *pArray);

typedef void (*simple8bUnpack)(uint64 block, Simple8bArray *pArray);

typedef struct {
    int bitWidth;
    simple8bPack pfPack;
    simple8bUnpack pfUnpack;
} Packing;

static inline uint64 packn(Simple8bArray *pArray, int selectorID, int bitWidth) {
    uint64 block = selectorID;
    int valNum = 60 / bitWidth;

    for (int i = 0; i < valNum; i++) {
        block |= (pArray->vals[pArray->readPos+i] << (i * bitWidth + 4));
    }

    Simple8bArrayReadPosAdd(pArray, valNum);
    return block;
}

/* encode 240 number of 1 or 0*/
static inline uint64 pack0(Simple8bArray *pArray) {
    uint64 block = 0;

    if (pArray->vals[pArray->readPos] == 1) {
        block |= 0x10;
    }

    Simple8bArrayReadPosAdd(pArray, 240);
    return block;
}

/* encode 120 number of 1 or 0*/
static inline uint64 pack1(Simple8bArray *pArray) {
    uint64 block = 1;

    if (pArray->vals[pArray->readPos] == 1) {
        block |= 0x10;
    }

    Simple8bArrayReadPosAdd(pArray, 120);
    return block;
}

/* encode 60 number of 1 and 0*/
static inline uint64 pack2(Simple8bArray *pArray) {
    return packn(pArray, 2, 1);
}

static inline uint64 pack3(Simple8bArray *pArray) {
    return packn(pArray, 3, 2);
}

static inline uint64 pack4(Simple8bArray *pArray) {
    return packn(pArray, 4, 3);
}

static inline uint64 pack5(Simple8bArray *pArray) {
    return packn(pArray, 5, 4);
}

static inline uint64 pack6(Simple8bArray *pArray) {
    return packn(pArray, 6, 5);
}

static inline uint64 pack7(Simple8bArray *pArray) {
    return packn(pArray, 7, 6);
}

static inline uint64 pack8(Simple8bArray *pArray) {
    return packn(pArray, 8, 7);
}

static inline uint64 pack9(Simple8bArray *pArray) {
    return packn(pArray, 9, 8);
}

static inline uint64 pack10(Simple8bArray *pArray) {
    return packn(pArray, 10, 10);
}

static inline uint64 pack11(Simple8bArray *pArray) {
    return packn(pArray, 11, 12);
}

static inline uint64 pack12(Simple8bArray *pArray) {
    return packn(pArray, 12, 15);
}

static inline uint64 pack13(Simple8bArray *pArray) {
    return packn(pArray, 13, 20);
}

static inline uint64 pack14(Simple8bArray *pArray) {
    return packn(pArray, 14, 30);
}

static inline uint64 pack15(Simple8bArray *pArray) {
    return packn(pArray, 15, 60);
}

static inline void unpackn(uint64 block, Simple8bArray *pArray, int bitWidth) {
    int valNum = 60 / bitWidth;

    // if (IS_LITTLE_ENDIAN) {
    //     for (int i = 0; i < valNum; i++) {
    //         pArray->vals[pArray->writePos + i] = (block >> (i * bitWidth + 4)) & NBITS(bitWidth);
    //     }
    // } else {
    //     for (int i = 0; i < valNum; i++) {
    //         pArray->vals[pArray->writePos + i] = (block >> ((valNum - i) * bitWidth)) & NBITS(bitWidth);
    //     }
    // }

    for (int i = 0; i < valNum; i++) {
        pArray->vals[pArray->writePos + i] = (block >> (i * bitWidth + 4)) & NBITS(bitWidth);
    }

    pArray->writePos += valNum;
    assert(pArray->writePos <= COMPRESS_BATCHSIZE);
}

static inline void unpack0(uint64 block, Simple8bArray *pArray) {
    if ((block & 0x10) == 0x10) {
        for (int i = 0; i < 240; i++) {
            pArray->vals[pArray->writePos + i] = 1;
        }
    } else {
        for (int i = 0; i < 240; i++) {
            pArray->vals[pArray->writePos + i] = 0;
        }
    }

    pArray->writePos += 240;
    assert(pArray->writePos <= COMPRESS_BATCHSIZE);
}

static inline void unpack1(uint64 block, Simple8bArray *pArray) {

    if ((block & 0x10) == 0x10) {
        for (int i = 0; i < 120; i++) {
            pArray->vals[pArray->writePos + i] = 1;
        }
    } else {
        for (int i = 0; i < 120; i++) {
            pArray->vals[pArray->writePos + i] = 0;
        }
    }

    pArray->writePos += 120;
    assert(pArray->writePos <= COMPRESS_BATCHSIZE);
}

static inline void unpack2(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 1);
}

static inline void unpack3(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 2);
}

static inline void unpack4(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 3);
}

static inline void unpack5(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 4);
}

static inline void unpack6(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 5);
}

static inline void unpack7(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 6);
}

static inline void unpack8(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 7);
}

static inline void unpack9(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 8);
}

static inline void unpack10(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 10);
}

static inline void unpack11(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 12);
}

static inline void unpack12(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 15);
}

static inline void unpack13(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 20);
}

static inline void unpack14(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 30);
}

static inline void unpack15(uint64 block, Simple8bArray *pArray) {
    unpackn(block, pArray, 60);
}

static Packing gSelectors[SELECTOR_NUM] = {
	{0, pack0, unpack0},
	{0, pack1, unpack1},
	{1, pack2, unpack2},
	{2, pack3, unpack3},
	{3, pack4, unpack4},
	{4, pack5, unpack5},
	{5, pack6, unpack6},
	{6, pack7, unpack7},
	{7, pack8, unpack8},
	{8, pack9, unpack9},
	{10, pack10, unpack10},
	{12, pack11, unpack11},
	{15, pack12, unpack12},
	{20, pack13, unpack13},
	{30, pack14, unpack14},
	{60, pack15, unpack15},
};

static inline bool canPack(Simple8bArray *pArray, int selectorID) {
    assert(selectorID >= 0 && selectorID < SELECTOR_NUM);
    Packing *pPacker = &gSelectors[selectorID];
    int valNum;

    if (selectorID == 0) {
        valNum = 240;
    } else if (selectorID == 1) {
        valNum = 120;
    } else {
        valNum = 60 / pPacker->bitWidth;
    }

    if (pArray->len - pArray->readPos < valNum) {
        return false;
    }

    if (selectorID < 2) {
        uint64 runsVal = pArray->vals[pArray->readPos];
        if (runsVal > 1) {
            return false;
        }
        for (int i = 0; i < valNum; i++) {
            if (pArray->vals[pArray->readPos + i] != runsVal) {
                return false;
            }
        }
        return true;
    } else {
        uint64 maxVal = (1UL << pPacker->bitWidth) - 1;
        for (int i = 0; i < valNum; i++) {
            if (pArray->vals[pArray->readPos + i] > maxVal) {
                return false;
            }
        }
        return true;
    }
}

static void Simple8bArrayRead(Simple8bArray *pOut, CUDesc *pDesc, Buffer *pIn) {
    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        uint64 val = (uint64)BufferRead(pIn, pDesc->eachValSize);
        assert(val <= SIMPLE8B_MAX_SUPPORT_VAL);
        pOut->vals[pOut->writePos++] = val;
        assert(pOut->writePos <= COMPRESS_BATCHSIZE);
    }
    pOut->len = pOut->writePos;
    pOut->writePos = 0;
}

static void Simple8bArrayWrite(Simple8bArray *pIn, CUDesc *pDesc, Buffer *pOut) {
    for (int i = 0; i < pIn->len; i++) {
        BufferWrite(pOut, pDesc->eachValSize, pIn->vals[pIn->readPos + i]);
    }
    BufferFinishWrite(pOut);
    Simple8bArrayReadPosAdd(pIn, pIn->len);
}

int simple8bCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Simple8bArray array = {0};
    Buffer *pZigzagCompressed = NULL;

    if (pDesc->minValue < 0) {
        assert(pDesc->maxValue * 2 <= SIMPLE8B_MAX_SUPPORT_VAL);
    } else {
        assert(pDesc->maxValue <= SIMPLE8B_MAX_SUPPORT_VAL);
    }

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

    Simple8bArrayRead(&array, pDesc, pIn);
    while (array.readPos < array.len) {
        int i;
        for (i = 0; i < SELECTOR_NUM; i++) {
            if (canPack(&array, i)) {
                Packing *pPacker = &gSelectors[i];
                uint64 block = pPacker->pfPack(&array);
                BufferWrite(pOut, sizeof(block), block);
                break;
            }
        }
        assert(array.readPos <= array.len);
    }
    BufferFinishWrite(pOut);
    ret = pOut->len;

l_end:
    if (NULL != pZigzagCompressed) {
        destroyBuffer(pZigzagCompressed);
    }
    return ret;
}

int simple8bDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut) {
    int ret = OK;
    assert(pDesc->eachValSize > 0);
    Simple8bArray array = {0};
    Buffer *pZigzagDecompressed = NULL;

    while (pIn->readPos < pIn->len) {
        uint64 block = BufferRead(pIn, sizeof(block));
        byte header = block & 0x0F;
        Packing *pPacker = &gSelectors[header];
        pPacker->pfUnpack(block, &array);
    }
    array.len = array.writePos;
    array.readPos = 0;
    array.writePos = 0;

    if (pDesc->minValue < 0) {
        ret = createBuffer(pIn->len, &pZigzagDecompressed);
        if (ret < 0) {
            goto l_end;
        }
        Simple8bArrayWrite(&array, pDesc, pZigzagDecompressed);
        ret = zigzagDecompress(pDesc, pZigzagDecompressed, pOut);
        if (ret < 0) {
            goto l_end;
        }
    }
    else {
        Simple8bArrayWrite(&array, pDesc, pOut);
    }

    ret = pOut->len;

l_end:
    if (NULL != pZigzagDecompressed) {
        destroyBuffer(pZigzagDecompressed);
    }
    return ret;
}

void simple8bUTCase(int eachValSize, byte *pOrigin ,int originSize, byte *pExpect, int expectSize) {
    int ret = OK;

    Buffer *pPlain = NULL;
    Buffer *pCompressed = NULL;
    Buffer *pDecompressed = NULL;
    ret = createBuffer(originSize, &pPlain);
    assert(ret >= 0);
    ret = createBuffer(expectSize, &pCompressed);
    assert(ret >= 0);
    ret = createBuffer(originSize, &pDecompressed);
    assert(ret >= 0);

    memcpy(pPlain->buf, pOrigin, originSize);
    pPlain->len = originSize;

    CUDesc desc = {0};
    desc.eachValSize = eachValSize;
    ret = simple8bCompress(&desc, pPlain, pCompressed);
    assert(ret >= 0);
    assert(pCompressed->len == expectSize);
    if (memcmp(pExpect, pCompressed->buf, expectSize) != 0) {
        printf("simple8b expect compress result: \n");
        dumpHexBuffer(pExpect, expectSize);
        printf("simple8b actual compress result: \n");
        dumpHexBuffer(pCompressed->buf, pCompressed->len);
    }

    ret = simple8bDecompress(&desc, pCompressed, pDecompressed);
    assert(ret >= 0);
    assert(pDecompressed->len == originSize);
    if (memcmp(pOrigin, pDecompressed->buf, originSize) != 0) {
        printf("simple8b expect decompress result: \n");
        dumpHexBuffer(pOrigin, originSize);
        printf("simple8b actual decompress result: \n");
        dumpHexBuffer(pDecompressed->buf, pDecompressed->len);
    }

    destroyBuffer(pPlain);
    destroyBuffer(pCompressed);
    destroyBuffer(pDecompressed);
}
