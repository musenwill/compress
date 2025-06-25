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
    char* psrc = pBuffer->buf + pBuffer->readPos;
    int64 val = *(int64*)psrc;

    if (IS_LITTLE_ENDIAN) {
        switch (datasize) {
            case sizeof(char):
                val = val & 0xFF;
                break;
            case sizeof(int16):
                val = val & 0xFFFF;
                break;
            case sizeof(int32):
                val = val & 0xFFFFFFFF;
                break;
            case sizeof(int64):
                val = val;
                break;
            case 3:
                val = val & 0xFFFFFF;
                break;
            case 5:
                val = val & 0xFFFFFFFFFF;
                break;
            case 6:
                val = val & 0xFFFFFFFFFFFF;
                break;
            case 7:
                val = val & 0xFFFFFFFFFFFFFF;
                break;
            default:
                assert(false);
                break;
        }
    } else {
        switch (datasize) {
            case sizeof(char):
                val = val >> 56;
                break;
            case sizeof(int16):
                val = val >> 48;
                break;
            case sizeof(int32):
                val = val >> 32;
                break;
            case sizeof(int64):
                val = val;
                break;
            case 3:
                val = val >> 40;
                break;
            case 5:
                val = val >> 24;
                break;
            case 6:
                val = val >> 16;
                break;
            case 7:
                val = val >> 8;
                break;
            default:
                assert(false);
                break;
        }
    }

    pBuffer->readPos += datasize;
    return val;
}

void BufferWrite(Buffer *pBuffer, int datasize, int64 data)
{
    assert(pBuffer->writePos + datasize <= pBuffer->len);
    char* pdst = pBuffer->buf + pBuffer->writePos;
    char* data_pos = (char*)&data;

    if (IS_LITTLE_ENDIAN) {
        switch (datasize) {
            case sizeof(char):
                *(char*)pdst = (char)(data);
                break;
            case sizeof(int16):
                *(int16*)pdst = (int16)(data);
                break;
            case sizeof(int32):
                *(int32*)pdst = (int32)(data);
                break;
            case sizeof(int64):
                *(int64*)pdst = (int64)(data);
                break;
            case 3:
                *(int16*)pdst = *(int16*)data_pos;
                pdst = pdst + 2;
                data_pos = data_pos + 2;
                *(char*)pdst = *data_pos;
                break;
            case 5:
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(char*)pdst = *data_pos;
                break;
            case 6:
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(int16*)pdst = *(int16*)data_pos;
                break;
            case 7:
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(int16*)pdst = *(int16*)data_pos;
                pdst = pdst + 2;
                data_pos = data_pos + 2;
                *(char*)pdst = *data_pos;
                break;
            default:
                assert(false);
                break;
        }
    } else {
        switch (datasize) {
            case sizeof(char):
                *(char*)pdst = (char)(data);
                break;
            case sizeof(int16):
                *(int16*)pdst = (int16)(data);
                break;
            case sizeof(int32):
                *(int32*)pdst = (int32)(data);
                break;
            case sizeof(int64):
                *(int64*)pdst = (int64)(data);
                break;
            case 3:
                data_pos = data_pos + 5;
                *(int16*)pdst = *(int16*)data_pos;
                pdst = pdst + 2;
                data_pos = data_pos + 2;
                *(char*)pdst = *data_pos;
                break;
            case 5:
                data_pos = data_pos + 3;
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(char*)pdst = *data_pos;
                break;
            case 6:
                data_pos = data_pos + 2;
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(int16*)pdst = *(int16*)data_pos;
                break;
            case 7:
                data_pos = data_pos + 1;
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(int16*)pdst = *(int16*)data_pos;
                pdst = pdst + 2;
                data_pos = data_pos + 2;
                *(char*)pdst = *data_pos;
                break;

            default:
                assert(false);
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
