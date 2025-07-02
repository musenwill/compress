#include "common.h"

int createBuffer(int size, Buffer **ppBuffer) {
    size_t mallocSize = sizeof(Buffer) + size;
    Buffer *pBuffer = (Buffer *)malloc(mallocSize);
    if (NULL == pBuffer) {
        LOG_ERROR("Failed malloc Buffer");
        return ERR_MEM;
    }
    memset(pBuffer, 0, mallocSize);
    pBuffer->bufSize = size;
    *ppBuffer = pBuffer;
    return OK;
}

void destroyBuffer(Buffer *pBuffer) {
    if (NULL != pBuffer) {
        free(pBuffer);
    }
}

// must read as signed value
int64 BufferRead(Buffer *pBuffer, int datasize)
{
    assert(pBuffer->readPos + datasize <= pBuffer->len);
    byte* psrc = pBuffer->buf + pBuffer->readPos;
    int64 val = 0;

    if (IS_LITTLE_ENDIAN) {
        if (datasize == 1) {
            int8 v = (int8)(*psrc);
            val = v;
        } else if (datasize == 2) {
            int16 v = 0;
            byte *pTmp = (byte*)(&v);
            for (int i = 0; i < datasize; i++) {
                pTmp[datasize-i-1] = psrc[i];
            }
            val = (int64)v;
        } else if (datasize == 4) {
            int32 v = 0;
            byte *pTmp = (byte*)(&v);
            for (int i = 0; i < datasize; i++) {
                pTmp[datasize-i-1] = psrc[i];
            }
            val = (int64)v;
        } else if (datasize == 8) {
            int64 v = 0;
            byte *pTmp = (byte*)(&v);
            for (int i = 0; i < datasize; i++) {
                pTmp[datasize-i-1] = psrc[i];
            }
            val = (int64)v;
        } else {
            byte *pTmp = (byte*)(&val);
            for (int i = 0; i < datasize; i++) {
                pTmp[datasize-i-1] = psrc[i];
            }
        }
    } else {
        if (datasize == 1) {
            int8 v = (int8)(*psrc);
            val = v;
        } else if (datasize == 2) {
            int16 v = 0;
            byte *pTmp = (byte*)(&v);
            for (int i = 0; i < datasize; i++) {
                pTmp[i] = psrc[i];
            }
            val = (int64)v;
        } else if (datasize == 4) {
            int32 v = 0;
            byte *pTmp = (byte*)(&v);
            for (int i = 0; i < datasize; i++) {
                pTmp[i] = psrc[i];
            }
            val = (int64)v;
        } else if (datasize == 8) {
            int64 v = 0;
            byte *pTmp = (byte*)(&v);
            for (int i = 0; i < datasize; i++) {
                pTmp[i] = psrc[i];
            }
            val = (int64)v;
        } else {
            byte *pTmp = (byte*)(&val);
            for (int i = 0; i < datasize; i++) {
                pTmp[i] = psrc[i];
            }
        }
    }

    pBuffer->readPos += datasize;
    pBuffer->readBits = pBuffer->readPos * 8;
    return val;
}

void BufferWrite(Buffer *pBuffer, int datasize, int64 data)
{
    assert(pBuffer->writePos + datasize <= pBuffer->bufSize);
    byte* pdst = pBuffer->buf + pBuffer->writePos;
    byte *pTmp = (byte*)(&data);

    if (IS_LITTLE_ENDIAN) {
        for (int i = 0; i < datasize; i++) {
            pdst[datasize-i-1] = pTmp[i];
        }
    } else {
        for (int i = 0; i < datasize; i++) {
            pdst[i] = pTmp[i];
        }
    }

    pBuffer->writePos += datasize;
    pBuffer->writeBits = pBuffer->writePos * 8;
}

void BufferFinishWrite(Buffer *pBuffer) {
    pBuffer->len = pBuffer->writePos;
    pBuffer->writePos = 0;
    pBuffer->readPos = 0;
}

void BufferFinishRead(Buffer *pBuffer) {
    pBuffer->readPos = 0;
}

void BufferWriteBits(Buffer *pBuffer, uint64 bits, int bitNum) {
    assert(((pBuffer->writeBits + bitNum + 7) >> 3) <= pBuffer->bufSize);

    int remainBits = bitNum;
    while (remainBits > 0) {
        byte *pByte = pBuffer->buf + (pBuffer->writeBits >> 3);
        int offset = pBuffer->writeBits & 0x07;
        *pByte |= (byte)(((bits >> (bitNum - remainBits)) << offset) & 0xff);

        if (8 - offset < remainBits) {
            remainBits -= (8 - offset);
            pBuffer->writeBits += (8 - offset);
        } else {
            pBuffer->writeBits += remainBits;
            remainBits = 0;
        }
    }
    assert(remainBits == 0);
}

uint64 BufferReadBits(Buffer *pBuffer, int bitNum) {
    assert(((pBuffer->readBits + bitNum + 7) >> 3) <= pBuffer->len);

    uint64 val = 0;
    int remainBits = bitNum;
    while (remainBits > 0) {
        byte *pByte = pBuffer->buf + (pBuffer->readBits >> 3);
        int offset = pBuffer->readBits & 0x07;
        val |= (((uint64)(*pByte) >> offset) & NBITS(remainBits)) << (bitNum - remainBits);

        if (8 - offset < remainBits) {
            remainBits -= (8 - offset);
            pBuffer->readBits += (8 - offset);
        } else {
            pBuffer->readBits += remainBits;
            remainBits = 0;
        }
    }
    assert(remainBits == 0);
    return val;
}

void BufferFinishWriteBits(Buffer *pBuffer) {
    pBuffer->len = (pBuffer->writeBits + 7) >> 3;
    pBuffer->writeBits = 0;
    pBuffer->readBits = 0;
}

void supportedDataType(const char *dataType) {
    static const char* gSupportedTypes[8] = {
        "int8", "int16", "int32", "int64", "float32", "float64", "bool", "string",
    };
    bool support = false;
    for (int i = 0; i < sizeof(gSupportedTypes) / sizeof(gSupportedTypes[0]); i++) {
        if (strcmp(dataType, gSupportedTypes[i]) == 0) {
            support = true;
            break;
        }
    }
    if (!support) {
        LOG_FATAL("unsupported data type %s", dataType);
    }
}

int dataTypeIsInteger(const char *dataType) {
    static const char* gIntegerTypes[4] = {
        "int8", "int16", "int32", "int64",
    };
    bool isInteger = false;
    for (int i = 0; i < sizeof(gIntegerTypes) / sizeof(gIntegerTypes[0]); i++) {
        if (strcmp(dataType, gIntegerTypes[i]) == 0) {
            isInteger = true;
            break;
        }
    }
    return isInteger;
}

int dataTypeSize(const char *dataType) {
    int size = -1;

    if (strcmp(dataType, "int8") == 0) {
        size = 1;
    } else if (strcmp(dataType, "int16") == 0) {
        size = 2;
    }else if (strcmp(dataType, "int32") == 0) {
        size = 4;
    } else if (strcmp(dataType, "int64") == 0) {
        size = 8;
    } else if (strcmp(dataType, "float32") == 0) {
        size = 4;
    } else if (strcmp(dataType, "float64") == 0) {
        size = 8;
    } else if (strcmp(dataType, "bool") == 0) {
        size = 1;
    } else if (strcmp(dataType, "string") == 0) {
        size = -1;
    } else {
        LOG_FATAL("unsupported data type %s", dataType);
    }

    return size;
}

void CUDescDumpHeader() {
    printf("%4s %16s %6s %12s %12s %12s %14s %5s %12s %12s %12s %10s %10s %10s\n",
        "id", 
        "compressedSize", 
        "attlen", 
        "minValue", 
        "maxValue", 
        "average", 
        "sum", 
        "count", 
        "minDelta", 
        "maxDelta", 
        "avgldeltal", 
        "continuity", 
        "repeats", 
        "smallNums");
}

int CUDescDump(CUDesc *pDesc, byte *pBuf) {
    return sprintf((char *)pBuf, "%6d %12ld %12ld %12ld %14ld "
        "%5ld %12ld %12ld %12ld %10ld %10ld %10ld",
        pDesc->eachValSize, pDesc->minValue, pDesc->maxValue, pDesc->average, pDesc->sum, pDesc->count,
        pDesc->minDelta, pDesc->maxDelta, pDesc->avgldeltal, pDesc->continuity, pDesc->repeats, pDesc->smallNums);
}

void dumpHexBuffer(const byte *buf, int len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}
