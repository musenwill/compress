#include "common.h"

int createBuffer(int size, Buffer **ppBuffer) {
    Buffer *pBuffer = (Buffer *)malloc(sizeof(Buffer) + size);
    if (NULL == pBuffer) {
        LOG_ERROR("Failed malloc Buffer");
        return ERR_MEM;
    }
    memset(pBuffer, 0, sizeof(*pBuffer));
    pBuffer->bufSize = size;
    *ppBuffer = pBuffer;
    return OK;
}

void destroyBuffer(Buffer *pBuffer) {
    if (NULL != pBuffer) {
        free(pBuffer);
    }
}

int64 BufferRead(Buffer *pBuffer, int datasize)
{
    assert(pBuffer->readPos + datasize <= pBuffer->len);
    byte* psrc = pBuffer->buf + pBuffer->readPos;
    int64 val = 0;
    byte *pTmp = (byte*)(&val);

    if (IS_LITTLE_ENDIAN) {
        for (int i = 0; i < datasize; i++) {
            pTmp[datasize-i-1] = psrc[i];
        }
    } else {
        for (int i = 0; i < datasize; i++) {
            pTmp[i] = psrc[i];
        }
    }

    pBuffer->readPos += datasize;
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

int CUDescDump(CUDesc *pDesc, byte *pBuf) {
    return sprintf((char *)pBuf, "attlen=%d, minValue=%ld, maxValue=%ld, average=%ld, sum=%ld, count=%ld, "
        "avgldeltal=%ld, continuity=%ld, repeats=%ld, smallNums=%ld",
        pDesc->eachValSize, pDesc->minValue, pDesc->maxValue, pDesc->average, pDesc->sum, pDesc->count,
        pDesc->average, pDesc->continuity, pDesc->repeats, pDesc->smallNums);
}

void dumpHexBuffer(const byte *buf, int len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}
