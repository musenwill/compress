#include "simple8b.h"

#define NBITS(bitWidth) ((0xFFFFFFFFFFFFFFFF << (bitWidth)) ^ 0xFFFFFFFFFFFFFFFF)

typedef struct {
    int len;
    int readPos;
    int writePos;
    uint64 vals[COMPRESS_BATCHSIZE];
} Simple8bArray;

typedef uint64 (*simple8bPack)(Simple8bArray *pArray);

typedef uint64 (*simple8bUnpack)(uint64 block, Simple8bArray *pArray);

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

    pArray->readPos += valNum;
    return block;
}

/* encode 240 number of 1 or 0*/
static inline uint64 pack0(Simple8bArray *pArray) {
    uint64 block = 0;

    if (pArray->vals[0] == 1) {
        byte *pTmp = (byte*)&block;
        *pTmp |= (byte)0x10;
    }

    return block;
}

/* encode 120 number of 1 or 0*/
static inline uint64 pack1(Simple8bArray *pArray) {
    uint64 block = 1;

    if (pArray->vals[0] == 1) {
        byte *pTmp = (byte*)&block;
        *pTmp |= 0x10;
    }

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

    for (int i = 0; i < valNum; i++) {
        pArray->vals[pArray->writePos + i] = (block >> (i * bitWidth + 4)) & NBITS(bitWidth);
    }

    pArray->writePos += valNum;
}

static inline void unpack0(uint64 block, Simple8bArray *pArray) {
    byte tmp = *(byte*)&block;

    if (tmp & 0x10 == 0x10) {
        for (int i = 0; i < 240; i++) {
            pArray->vals[pArray->writePos + i] = 1;
        }
    } else if (tmp & 0x10 == 0) {
        for (int i = 0; i < 240; i++) {
            pArray->vals[pArray->writePos + i] = 0;
        }
    } else {
        assert(false);
    }

    pArray->writePos += 240;
}

static inline void unpack1(uint64 block, Simple8bArray *pArray) {
    byte tmp = *(byte*)&block;

    if (tmp & 0x10 == 0x10) {
        for (int i = 0; i < 120; i++) {
            pArray->vals[pArray->writePos + i] = 1;
        }
    } else if (tmp & 0x10 == 0) {
        for (int i = 0; i < 120; i++) {
            pArray->vals[pArray->writePos + i] = 0;
        }
    } else {
        assert(false);
    }

    pArray->writePos += 120;
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

static Packing gSelectors[16] = {
	{0, unpack0, pack0},
	{0, unpack1, pack1},
	{1, unpack2, pack2},
	{2, unpack3, pack3},
	{3, unpack4, pack4},
	{4, unpack5, pack5},
	{5, unpack6, pack6},
	{6, unpack7, pack7},
	{7, unpack8, pack8},
	{8, unpack8, pack9},
	{10, unpack10, pack10},
	{12, unpack11, pack11},
	{15, unpack12, pack12},
	{20, unpack13, pack13},
	{30, unpack14, pack14},
	{60, unpack15, pack15},
};

static void readSimple8bArray(CUDesc *pDesc, Buffer *pIn, Simple8bArray *pOut) {
    while (pIn->readPos + pDesc->eachValSize <= pIn->len) {
        uint64 val = (uint64)BufferRead(pIn, pDesc->eachValSize);
        pOut->vals[pOut->writePos++] = val;
        assert(pOut->writePos > COMPRESS_BATCHSIZE);
    }
}


