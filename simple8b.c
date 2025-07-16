#include "zigzag.h"
#include "simple8b.h"

#define SIMPLE8B_SELECTOR_NUM       (16)
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
} Simple8bPacker;

static inline uint64 simple8bPackn(Simple8bArray *pArray, int selectorID, int bitWidth) {
    uint64 block = selectorID;
    int valNum = SIMPLE8B_PACKING_BITS / bitWidth;

    for (int i = 0; i < valNum; i++) {
        block |= (pArray->vals[pArray->readPos+i] << (i * bitWidth + 4));
    }

    Simple8bArrayReadPosAdd(pArray, valNum);
    return block;
}

/* encode 240 number of 1 or 0*/
static inline uint64 simple8bPack0(Simple8bArray *pArray) {
    uint64 block = 0;

    if (pArray->vals[pArray->readPos] == 1) {
        block |= 0x10;
    }

    Simple8bArrayReadPosAdd(pArray, 240);
    return block;
}

/* encode 120 number of 1 or 0*/
static inline uint64 simple8bPack1(Simple8bArray *pArray) {
    uint64 block = 1;

    if (pArray->vals[pArray->readPos] == 1) {
        block |= 0x10;
    }

    Simple8bArrayReadPosAdd(pArray, 120);
    return block;
}

/* encode 60 number of 1 and 0*/
static inline uint64 simple8bPack2(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 2, 1);
}

static inline uint64 simple8bPack3(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 3, 2);
}

static inline uint64 simple8bPack4(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 4, 3);
}

static inline uint64 simple8bPack5(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 5, 4);
}

static inline uint64 simple8bPack6(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 6, 5);
}

static inline uint64 simple8bPack7(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 7, 6);
}

static inline uint64 simple8bPack8(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 8, 7);
}

static inline uint64 simple8bPack9(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 9, 8);
}

static inline uint64 simple8bPack10(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 10, 10);
}

static inline uint64 simple8bPack11(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 11, 12);
}

static inline uint64 simple8bPack12(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 12, 15);
}

static inline uint64 simple8bPack13(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 13, 20);
}

static inline uint64 simple8bPack14(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 14, 30);
}

static inline uint64 simple8bPack15(Simple8bArray *pArray) {
    return simple8bPackn(pArray, 15, 60);
}

static inline void simple8bUnpackn(uint64 block, Simple8bArray *pArray, int bitWidth) {
    int valNum = SIMPLE8B_PACKING_BITS / bitWidth;

    for (int i = 0; i < valNum; i++) {
        pArray->vals[pArray->writePos + i] = (block >> (i * bitWidth + 4)) & NBITS(bitWidth);
    }

    pArray->writePos += valNum;
    assert(pArray->writePos <= COMPRESS_BATCHSIZE);
}

static inline void simple8bUnpack0(uint64 block, Simple8bArray *pArray) {
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

static inline void simple8bUnpack1(uint64 block, Simple8bArray *pArray) {

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

static inline void simple8bUnpack2(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 1);
}

static inline void simple8bUnpack3(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 2);
}

static inline void simple8bUnpack4(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 3);
}

static inline void simple8bUnpack5(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 4);
}

static inline void simple8bUnpack6(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 5);
}

static inline void simple8bUnpack7(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 6);
}

static inline void simple8bUnpack8(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 7);
}

static inline void simple8bUnpack9(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 8);
}

static inline void simple8bUnpack10(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 10);
}

static inline void simple8bUnpack11(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 12);
}

static inline void simple8bUnpack12(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 15);
}

static inline void simple8bUnpack13(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 20);
}

static inline void simple8bUnpack14(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 30);
}

static inline void simple8bUnpack15(uint64 block, Simple8bArray *pArray) {
    simple8bUnpackn(block, pArray, 60);
}

static Simple8bPacker gSelectors[SIMPLE8B_SELECTOR_NUM] = {
	{0, simple8bPack0, simple8bUnpack0},
	{0, simple8bPack1, simple8bUnpack1},
	{1, simple8bPack2, simple8bUnpack2},
	{2, simple8bPack3, simple8bUnpack3},
	{3, simple8bPack4, simple8bUnpack4},
	{4, simple8bPack5, simple8bUnpack5},
	{5, simple8bPack6, simple8bUnpack6},
	{6, simple8bPack7, simple8bUnpack7},
	{7, simple8bPack8, simple8bUnpack8},
	{8, simple8bPack9, simple8bUnpack9},
	{10, simple8bPack10, simple8bUnpack10},
	{12, simple8bPack11, simple8bUnpack11},
	{15, simple8bPack12, simple8bUnpack12},
	{20, simple8bPack13, simple8bUnpack13},
	{30, simple8bPack14, simple8bUnpack14},
	{60, simple8bPack15, simple8bUnpack15},
};

static inline bool canPack(Simple8bArray *pArray, int selectorID) {
    assert(selectorID >= 0 && selectorID < SIMPLE8B_SELECTOR_NUM);
    Simple8bPacker *pPacker = &gSelectors[selectorID];
    int valNum;

    if (selectorID == 0) {
        valNum = 240;
    } else if (selectorID == 1) {
        valNum = 120;
    } else {
        valNum = SIMPLE8B_PACKING_BITS / pPacker->bitWidth;
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
    assert((pIn->len - pIn->readPos) / pDesc->eachValSize <= COMPRESS_BATCHSIZE);
    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        uint64 val = BufferReadUnsigned(pIn, pDesc->eachValSize);
        assert(val <= SIMPLE8B_MAX_SUPPORT_VAL);
        pOut->vals[pOut->writePos++] = val;

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
        for (int i = 0; i < SIMPLE8B_SELECTOR_NUM; i++) {
            if (canPack(&array, i)) {
                Simple8bPacker *pPacker = &gSelectors[i];
                uint64 block = pPacker->pfPack(&array);
                BufferWrite(pOut, sizeof(block), block);
                break;
            }
        }
    }
    assert(array.readPos == array.len);
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
        uint64 block = BufferReadUnsigned(pIn, sizeof(block));
        byte header = block & 0x0F;
        Simple8bPacker *pPacker = &gSelectors[header];
        pPacker->pfUnpack(block, &array);
    }
    array.len = array.writePos;
    array.readPos = 0;
    array.writePos = 0;

    if (pDesc->minValue < 0) {
        ret = createBuffer(pOut->bufSize, &pZigzagDecompressed);
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

float32 simple8bEstimate(CUDesc *pDesc) {
    float32 bitwidth;

    if (pDesc->minValue >= 0) {
        bitwidth = (float32)BIT_WIDTH(pDesc->average);
    } else {
        bitwidth = (float32)BIT_WIDTH(2 * labs(pDesc->average));
    }

    return pDesc->eachValSize * SIMPLE8B_PACKING_BITS / ( 8.0 * bitwidth);
}
