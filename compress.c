#include "common.h"
#include "compress.h"
#include "rle.h"
#include "zigzag.h"
#include "simple8b.h"
#include "deltaA.h"
#include "deltaB.h"
#include "deltaC.h"
#include "delta2A.h"
#include "delta2B.h"
#include "bitpacking.h"
#include "varint.h"

void CompressStatsPrint(CompressStats *pStats) {
    printf("compress rate:      %.2f\n", (float)(pStats->plainSize) / (float)(pStats->compressedSize));
    printf("plain size:         %ld\n", pStats->plainSize);
    printf("compressed size:    %ld\n", pStats->compressedSize);
    printf("compress user time(us):     %ld\n", pStats->compressTimeUserUs);
    printf("compress sys time(us):      %ld\n", pStats->compressTimeSysUs);
    printf("decompress user time(us):   %ld\n", pStats->decompressTimeUserUs);
    printf("decompress sys time(us):    %ld\n", pStats->decompressTimeSysUs);
}

int CompressResultCreate(CompressResult **pResult) {
    CompressResult *pR = (CompressResult *)malloc(sizeof(CompressResult));
    if (NULL == pR) {
        LOG_ERROR("Failed malloc CompressResult");
        return ERR_MEM;
    }
    memset(pR, 0, sizeof(*pR));
    *pResult = pR;
    return OK;
}

void CompressResultDestroy(CompressResult *pCmprResult) {
    for (int i = 0; i < pCmprResult->len; i++) {
        if (NULL != pCmprResult->pBufs[i]) {
            destroyBuffer(pCmprResult->pBufs[i]);
            pCmprResult->pBufs[i] = NULL;
        }
    }
    free(pCmprResult);
}

int64 CompressResultTotalSize(CompressResult *pCmprResult) {
    int64 totalSize = 0;
    for (int i = 0; i < pCmprResult->len; i++) {
        if (NULL != pCmprResult->pBufs[i]) {
            totalSize += pCmprResult->pBufs[i]->len;
        }
    }
    return totalSize;
}

void CompressResultPrint(CompressResult *pCmprResult) {
    byte line[1024] = {0};
    CUDescDumpHeader();
    for (int i = 0; i < pCmprResult->len; i++) {
        if (NULL != pCmprResult->pBufs[i]) {
            CUDescDump(&pCmprResult->descs[i], line);
            printf("%4d %16d %s\n", i, pCmprResult->pBufs[i]->len, line);
        }
    }
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
    *ppBuffer = pBuffer;

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

void DecompressResultCheck(int eachValSize, CompressResult *pDecmprResult, Buffer *pOrigin) {
    int64 decompressedSize = 0;
    for (int i = 0; i < pDecmprResult->len; i++) {
        decompressedSize += pDecmprResult->pBufs[i]->len;
    }
    if (decompressedSize != pOrigin->len) {
        LOG_ERROR("decompressed size, exp=%ld, act=%ld", pOrigin->len, decompressedSize);
    }
    assert(decompressedSize == pOrigin->len);

    int pos = 0;
    for (int i = 0; i < pDecmprResult->len; i++) {
        if (memcmp(pOrigin->buf + pos, pDecmprResult->pBufs[i]->buf, pDecmprResult->pBufs[i]->len) != 0) {
            printf("origin data\n");
            dumpHexBuffer(pOrigin->buf + pos, pDecmprResult->pBufs[i]->len);
            printf("decompressed data\n");
            dumpHexBuffer(pDecmprResult->pBufs[i]->buf, pDecmprResult->pBufs[i]->len);
        }
        assert(memcmp(pOrigin->buf + pos, pDecmprResult->pBufs[i]->buf, pDecmprResult->pBufs[i]->len) == 0);
        pos += pDecmprResult->pBufs[i]->len;
    }
}

void collectIntegerCU(Buffer *pIn, const char *dataType, Buffer *pOut, CUDesc *pDesc) {
    int64 min, max, pre, delta, minDelta, maxDelta;
    int64 sum = 0;
    int64 count = 0;
    int64 sumldeltal = 0;
    int64 continuity = 0;
    int64 repeats = 0;
    int64 smallNums = 0;
    int preDeltaSign = 0;   // 0 undefined, -1 negative, 1 positive
    int eachValSize = dataTypeSize(dataType);
    bool hasDelta = false;

    for (int i = 0; i < COMPRESS_BATCHSIZE; i++) {
        if (pIn->readPos + eachValSize > pIn->len) {
            break;
        }
        int64 val = BufferReadSigned(pIn, eachValSize);
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
            if (!hasDelta) {
                hasDelta = true;
                minDelta = delta;
                maxDelta = delta;
            } else {
                if (delta < minDelta) {
                    minDelta = delta;
                }
                if (delta > maxDelta) {
                    maxDelta = delta;
                }
            }

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
    if (count > 0) {
        pDesc->average = sum / count;
    } else {
        pDesc->average = 0;
    }
    if (count > 1) {
        pDesc->avgldeltal = sumldeltal / (count - 1);
    } else {
        pDesc->avgldeltal = 0;
    }
    pDesc->minDelta = minDelta;
    pDesc->maxDelta = maxDelta;
    pDesc->continuity = continuity;
    pDesc->repeats = repeats;
    pDesc->smallNums = smallNums;
}

int compressCU(CUDesc *pDesc, Buffer *pIn, Buffer *pOut, const char *pAlgo) {
    int ret = OK;

    if (strcmp(pAlgo, "rle") == 0) {
        ret = rleCompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "zigzag") == 0) {
        ret = zigzagCompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "simple8b") == 0) {
        ret = simple8bCompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "deltaA") == 0) {
        ret = deltaACompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "deltaB") == 0) {
        ret = deltaBCompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "deltaC") == 0) {
        ret = deltaCCompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "delta2A") == 0) {
        ret = delta2ACompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "delta2B") == 0) {
        ret = delta2BCompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "bitpacking") == 0) {
        ret = bitPackingCompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "varint") == 0) {
        ret = varintCompress(pDesc, pIn, pOut);
    } else {
        LOG_FATAL("compress algorithm %s unsupported yet", pAlgo);
    }

    return ret;
}

int decompressCU(CUDesc *pDesc, Buffer *pIn, Buffer *pOut, const char *pAlgo) {
    int ret = OK;

    if (strcmp(pAlgo, "rle") == 0) {
        ret = rleDecompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "zigzag") == 0) {
        ret = zigzagDecompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "simple8b") == 0) {
        ret = simple8bDecompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "deltaA") == 0) {
        ret = deltaADecompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "deltaB") == 0) {
        ret = deltaBDecompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "deltaC") == 0) {
        ret = deltaCDecompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "delta2A") == 0) {
        ret = delta2ADecompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "delta2B") == 0) {
        ret = delta2BDecompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "bitpacking") == 0) {
        ret = bitPackingDecompress(pDesc, pIn, pOut);
    } else if (strcmp(pAlgo, "varint") == 0) {
        ret = varintDecompress(pDesc, pIn, pOut);
    } else {
        LOG_FATAL("compress algorithm %s unsupported yet", pAlgo);
    }

    return ret;
}

int compressFile(const char *filePath, const char *pAlgo, const char *dataType) {
    int ret = OK;
    Buffer *pOrigin = NULL;
    CompressResult *pCompressResult = NULL;
    CompressResult *pDecompressResult = NULL;
    Buffer *pCurBuf = NULL;
    CompressStats stats = {0};
    struct rusage start = {0};
    struct rusage end = {0};

    ret = CompressResultCreate(&pCompressResult);
    if (ret < 0) {
        goto l_end;
    }
    ret = CompressResultCreate(&pDecompressResult);
    if (ret < 0) {
        goto l_end;
    }

    ret = createBuffer(1024 * 1024, &pCurBuf);
    if (ret < 0) {
        goto l_end;
    }

    ret = readFile(filePath, &pOrigin);
    if (ret < 0) {
        goto l_end;
    }

    getrusage(RUSAGE_SELF, &start);
    if (dataTypeIsInteger(dataType)) {
        while (pOrigin->readPos < pOrigin->len) {
            pCurBuf->readPos = 0;
            pCurBuf->writePos = 0;
            CUDesc *pDesc = &pCompressResult->descs[pCompressResult->len];
            collectIntegerCU(pOrigin, dataType, pCurBuf, pDesc);
            BufferFinishWrite(pCurBuf);
            if (pDesc->count > 0) {
                Buffer *pCompressedBuf = NULL;
                ret = createBuffer(pCurBuf->bufSize, &pCompressedBuf);
                if (ret < 0) {
                    goto l_end;
                }
                pCompressResult->pBufs[pCompressResult->len] = pCompressedBuf;
                pCompressResult->len++;
                ret = compressCU(pDesc, pCurBuf, pCompressedBuf, pAlgo);
                if (ret < 0) {
                    goto l_end;
                }
            }
        }
    } else {
        LOG_FATAL("data type %s unsupported yet", dataType);
    }
    getrusage(RUSAGE_SELF, &end);
    stats.compressTimeSysUs = systimeus(start, end);
    stats.compressTimeUserUs = usertimeus(start, end);

    getrusage(RUSAGE_SELF, &start);
    for (int i = 0; i < pCompressResult->len; i++) {
        Buffer *pDecompressBuf = NULL;
        ret = createBuffer(pCurBuf->bufSize, &pDecompressBuf);
        if (ret < 0) {
            goto l_end;
        }
        pDecompressResult->pBufs[pDecompressResult->len] = pDecompressBuf;
        pDecompressResult->len++;
        ret = decompressCU(&pCompressResult->descs[i], pCompressResult->pBufs[i], pDecompressBuf, pAlgo);
        if (ret < 0) {
            goto l_end;
        }
    }
    getrusage(RUSAGE_SELF, &end);
    stats.decompressTimeSysUs = systimeus(start, end);
    stats.decompressTimeUserUs = usertimeus(start, end);
    stats.plainSize = pOrigin->len;
    stats.compressedSize = CompressResultTotalSize(pCompressResult);

    DecompressResultCheck(dataTypeSize(dataType), pDecompressResult, pOrigin);
    CompressStatsPrint(&stats);
    CompressResultPrint(pCompressResult);

l_end:
    if (pCurBuf != NULL) {
        destroyBuffer(pCurBuf);
    }
    if (pOrigin != NULL) {
        destroyBuffer(pOrigin);
    }
    if (NULL != pCompressResult) {
        CompressResultDestroy(pCompressResult);
    }
    if (NULL != pDecompressResult) {
        CompressResultDestroy(pDecompressResult);
    }
    return ret;
}
