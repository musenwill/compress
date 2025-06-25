#include "common.h"
#include "compress.h"
#include "rle.h"

void cleanCompressResult(CompressResult *pCmprResult) {
    for (int i = 0; i < pCmprResult->len; i++) {
        if (NULL != pCmprResult->pBufs[i]) {
            destroyBuffer(pCmprResult->pBufs[i]);
            pCmprResult->pBufs[i] = NULL;
        }
    }
    pCmprResult->len = 0;
}

int readFile(const char *filePath, Buffer **ppBuffer) {
    int ret = OK;

    struct stat stat_buf;
    if (stat(filePath, &stat_buf) < 0) {
        LOG_ERROR("Failed stat file %s", filePath);
        return ERR_FILE;
    }

    FILE *pFile = fopen(filePath, "rb");
    if (pFile == NULL) {
        LOG_ERROR("Failed open file %s", filePath);
        ret = ERR_FILE;
        goto l_end;
    }

    Buffer *pBuffer = NULL;
    ret = createBuffer(stat_buf.st_size, &pBuffer);
    if (ret < 0) {
        goto l_end;
    }

    int rd = fread(pBuffer->buf, 1, pBuffer->bufSize, pFile);
    if (rd < pBuffer->len) {
        LOG_ERROR("Failed read file %s, expect read %d bytes, actual got %d", filePath, pBuffer->len, rd);
        ret = ERR_FILE;
        goto l_fail;
    }
    pBuffer->len = rd;

    goto l_end;

l_fail:
    if (pBuffer != NULL) {
        destroyBuffer(pBuffer);
    }

l_end:
    if (pFile != NULL) {
        fclose(pFile);
    }
    return ret;
}

void collectIntegerCU(Buffer *pIn, const char *dataType, Buffer *pOut, CUDesc *pDesc) {
    int64 min, max, pre, delta;
    int64 sum = 0;
    int64 count = 0;
    int64 sumldeltal = 0;
    int64 continuity = 0;
    int64 repeats = 0;
    int64 smallNums = 0;
    int preDeltaSign = 0;   // 0 undefined, -1 negative, 1 positive
    int eachValSize = dataTypeSize(dataType);

    for (int i = 0; i < COMPRESS_BATCHSIZE; i++) {
        if (pIn->readPos + eachValSize > pIn->len) {
            break;
        }
        int64 val = BufferRead(pIn, eachValSize);
        BufferWrite(pOut, eachValSize, val);

        count++;
        if (i == 0) {
            min = val;
            max = val;
        } else {
            if (val == pre) {
                repeats++;
            }

            if (val < min) {
                min = val;
            }
            if (val > max) {
                max = val;
            }
            delta = val - pre;

            if (delta == 0) {
                continuity++;
                preDeltaSign = 0;
            } else if (delta > 0 && preDeltaSign >= 0) {
                continuity++;
            } else if (delta < 0 && preDeltaSign <= 0) {
                continuity++;
            }
            if (delta > 0) {
                preDeltaSign = 1;
                sumldeltal += delta;
            } else if (delta < 0) {
                preDeltaSign = -1;
                sumldeltal += (delta * -1);
            }
        }
        if (val < 256 && val*-1 < 256) {
            smallNums++;
        }
        sum += val;
        pre = val;
    }

    pDesc->eachValSize = eachValSize;
    pDesc->count = count;
    pDesc->sum = sum;
    pDesc->minValue = min;
    pDesc->maxValue = max;
    pDesc->average = sum / count;
    pDesc->avgldeltal = sumldeltal / (count - 1);
    pDesc->continuity = continuity;
    pDesc->repeats = repeats;
    pDesc->smallNums = smallNums;
}

int compressCU(CUDesc *pDesc, Buffer *pIn, Buffer *pOut, const char *pAlgo) {
    int ret = OK;

    if (strcmp(pAlgo, "rle") == 0) {
        ret = rleCompress(pDesc, pIn, pOut);
    } else {
        LOG_FATAL("compress algorithm %s unsupported yet", pAlgo);
    }

    return ret;
}

int compressFile(const char *filePath, const char *pAlgo, const char *dataType) {
    int ret = OK;
    Buffer *pOrigin = NULL;
    CompressResult compressResult = {0};
    CompressResult decompressResult = {0};
    Buffer *pCurBuf = NULL;

    ret = createBuffer(1024 * 1024 * 10, &pCurBuf);
    if (ret < 0) {
        goto l_end;
    }

    ret = readFile(filePath, &pOrigin);
    if (ret < 0) {
        goto l_end;
    }

    if (dataTypeIsInteger(dataType)) {
        while (pOrigin->readPos < pOrigin->len) {
            pCurBuf->readPos = 0;
            pCurBuf->writePos = 0;
            CUDesc desc = {0};
            collectIntegerCU(pOrigin, dataType, pCurBuf, &desc);
            if (desc.count > 0) {
                Buffer *pCompressedBuf = NULL;
                ret = createBuffer(pCurBuf->bufSize, &pCompressedBuf);
                if (ret < 0) {
                    goto l_end;
                }
                compressResult.pBufs[compressResult.len] = pCompressedBuf;
                compressResult.len++;
                ret = compressCU(&desc, pCurBuf, pCompressedBuf, pAlgo);
                if (ret < 0) {
                    goto l_end;
                }
                pCompressedBuf->len = pCompressedBuf->writePos;
                // print CUDesc
                // statistics
                // decompress
            }
        }
    } else {
        LOG_FATAL("data type %s unsupported yet", dataType);
    }

l_end:
    if (pCurBuf != NULL) {
        destroyBuffer(pCurBuf);
    }
    if (pOrigin != NULL) {
        destroyBuffer(pOrigin);
    }
    cleanCompressResult(&compressResult);
    cleanCompressResult(&decompressResult);
    return ret;
}