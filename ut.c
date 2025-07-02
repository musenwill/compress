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
#include "ut.h"

typedef int (*pfCompressFunc)(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

pfCompressFunc getCompressFunc(const char *pAlgo) {
    pfCompressFunc pf = NULL;

    if (strcmp(pAlgo, "rle") == 0) {
        pf = rleCompress;
    } else if (strcmp(pAlgo, "zigzag") == 0) {
        pf = zigzagCompress;
    } else if (strcmp(pAlgo, "simple8b") == 0) {
        pf = simple8bCompress;
    } else if (strcmp(pAlgo, "deltaA") == 0) {
        pf = deltaACompress;
    } else if (strcmp(pAlgo, "deltaB") == 0) {
        pf = deltaBCompress;
    } else if (strcmp(pAlgo, "deltaC") == 0) {
        pf = deltaCCompress;
    } else if (strcmp(pAlgo, "delta2A") == 0) {
        pf = delta2ACompress;
    } else if (strcmp(pAlgo, "delta2B") == 0) {
        pf = delta2BCompress;
    } else if (strcmp(pAlgo, "bitpacking") == 0) {
        pf = bitPackingCompress;
    } else if (strcmp(pAlgo, "varint") == 0) {
        pf = varintCompress;
    } else {
        LOG_FATAL("compress algorithm %s unsupported yet", pAlgo);
    }

    return pf;
}

pfCompressFunc getDecompressFunc(const char *pAlgo) {
    pfCompressFunc pf = NULL;

    if (strcmp(pAlgo, "rle") == 0) {
        pf = rleDecompress;
    } else if (strcmp(pAlgo, "zigzag") == 0) {
        pf = zigzagDecompress;
    } else if (strcmp(pAlgo, "simple8b") == 0) {
        pf = simple8bDecompress;
    } else if (strcmp(pAlgo, "deltaA") == 0) {
        pf = deltaADecompress;
    } else if (strcmp(pAlgo, "deltaB") == 0) {
        pf = deltaBDecompress;
    } else if (strcmp(pAlgo, "deltaC") == 0) {
        pf = deltaCDecompress;
    }  else if (strcmp(pAlgo, "delta2A") == 0) {
        pf = delta2ADecompress;
    } else if (strcmp(pAlgo, "delta2B") == 0) {
        pf = delta2BDecompress;
    } else if (strcmp(pAlgo, "bitpacking") == 0) {
        pf = bitPackingDecompress;
    } else if (strcmp(pAlgo, "varint") == 0) {
        pf = varintDecompress;
    } else {
        LOG_FATAL("compress algorithm %s unsupported yet", pAlgo);
    }

    return pf;
}

static void collectCUDesc(Buffer *pIn, CUDesc *pDesc, int eachValSize) {
    int64 min, max, pre, delta, minDelta, maxDelta;
    int64 sum = 0;
    int64 count = 0;
    int64 sumldeltal = 0;
    int64 continuity = 0;
    int64 repeats = 0;
    int64 smallNums = 0;
    int preDeltaSign = 0;   // 0 undefined, -1 negative, 1 positive
    bool hasDelta = false;

    for (int i = 0; i < pIn->len / eachValSize; i++) {
        if (pIn->readPos + eachValSize > pIn->len) {
            break;
        }
        int64 val = BufferRead(pIn, eachValSize);

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

    BufferFinishRead(pIn);
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

void runCase(const char *pAlgo, int eachValSize, byte *pOrigin ,int originSize, byte *pExpect, int expectSize) {
    int ret = OK;
    pfCompressFunc pfCmprFunc = getCompressFunc(pAlgo);
    pfCompressFunc pfDecmprFunc = getDecompressFunc(pAlgo);

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
    collectCUDesc(pPlain, &desc, eachValSize);
    ret = pfCmprFunc(&desc, pPlain, pCompressed);
    assert(ret >= 0);
    assert(pCompressed->len == expectSize);
    if (memcmp(pExpect, pCompressed->buf, expectSize) != 0) {
        printf("%s expect compress result: \n", pAlgo);
        dumpHexBuffer(pExpect, expectSize);
        printf("%s actual compress result: \n", pAlgo);
        dumpHexBuffer(pCompressed->buf, pCompressed->len);
    }

    ret = pfDecmprFunc(&desc, pCompressed, pDecompressed);
    assert(ret >= 0);
    assert(pDecompressed->len == originSize);
    if (memcmp(pOrigin, pDecompressed->buf, originSize) != 0) {
        printf("%s expect decompress result: \n", pAlgo);
        dumpHexBuffer(pOrigin, originSize);
        printf("%s actual decompress result: \n", pAlgo);
        dumpHexBuffer(pDecompressed->buf, pDecompressed->len);
    }

    destroyBuffer(pPlain);
    destroyBuffer(pCompressed);
    destroyBuffer(pDecompressed);
}

void Test() {
    {
        byte origin[256] = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 0xFE, 0xFE};
        byte compressed[] = {1, 2, 2, 3, 3, 3, 0xFE, 4, 4, 0xFE, 5, 5, 0xFE, 2, 0xFE, 0x80, 0xEF, 0};
        runCase("rle", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[8] = {1, -2, 3, -4, 5, -6, 7, -8};
        byte compressed[8] = {2, 3, 6, 7, 10, 11, 14, 15};
        runCase("zigzag", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[8] = {0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        byte compressed[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe};
        runCase("zigzag", 8, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[256] = {10, 11, 12, 13, 14, 15};
        byte compressed[] = {0x00, 0x00, 0x00, 0x00, 0x0F, 0xED, 0xCB, 0xA5,    // 10, 11, 12, 13, 14, 15, 0x00 x 9
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // 0x00 * 240
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F};    // 0x00 * 1
        runCase("simple8b", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[] = {0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5,
                         0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5,
                         0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5,
                         0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5, 0x00, 0x0c, 0x7a, 0xd5};
        byte compressed[] = {0xc7, 0xad, 0x5c, 0x7a, 0xd5, 0xc7, 0xad, 0x5d, 0xc7, 0xad, 0x5c, 0x7a, 0xd5, 0xc7, 0xad, 0x5d,    // 817877 x 6
                             0xc7, 0xad, 0x5c, 0x7a, 0xd5, 0xc7, 0xad, 0x5d, 0xc7, 0xad, 0x5c, 0x7a, 0xd5, 0xc7, 0xad, 0x5d,    // 817877 x 6
                             0xc7, 0xad, 0x5c, 0x7a, 0xd5, 0xc7, 0xad, 0x5d,                                                    // 817877 x 3
                             0x00, 0x00, 0x00, 0x00, 0x00, 0xc7, 0xad, 0x5f};                                                   // 817877 x 1
        runCase("simple8b", 4, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[256] = {2, 2, 0, 2, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 9};
        byte compressed[] = {0x00, 0x00, 0x20, 0x00, 0x20, 0x02, 0x02, 0x25,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x95,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f};
        runCase("simple8b", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[256] = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5};
        byte compressed[] = {0x00, 0x00, 0x20, 0x00, 0x20, 0x02, 0x02, 0x25,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x95,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f};
        runCase("deltaA", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[256] = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5};
        byte compressed[] = {0x02, 0x02, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00,
                             0x00, 0x00, 0x02, 0xfe, 0x04, 0x00, 0x09,
                             0xfe, 0x80, 0xf0, 0x00};
        runCase("deltaB", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[] = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5};
        byte compressed[] = {0x01, 0x01, 0x25, 0x02};
        runCase("deltaC", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[256] = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5};
        byte compressed[] = {0x00, 0x01, 0x20, 0x01, 0x20, 0x12, 0x10, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x95,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f};
        runCase("delta2A", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[256] = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5};
        byte compressed[] = {0x02, 0x00, 0x01, 0x02, 0x01, 0x00, 0x02, 0x01,
                             0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x09,
                             0x0a, 0xfe, 0x80, 0xef, 0x00};
        runCase("delta2B", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[256] = {0};
        byte compressed[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00};
        runCase("bitpacking", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[256] = {1};
        byte compressed[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00};
        runCase("bitpacking", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        byte compressed[] = {0x01, 0xff, 0x03};
        runCase("bitpacking", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[] = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1};
        byte compressed[] = {0x02, 0xe4, 0xe4, 0x04};
        runCase("bitpacking", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2};
        byte compressed[] = {0x03, 0x88, 0xc6, 0xfa, 0x88, 0xc6, 0xfa, 0x88, 0x00};
        runCase("bitpacking", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[] = {120, 0, 1, 2, 3, 4, 5, 6, 7, 8};
        byte compressed[] = {0x07, 0x78, 0x40, 0x40, 0x30, 0x20, 0x14, 0x0c, 0x07, 0x04};
        runCase("bitpacking", 1, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[] = {0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        byte compressed[] = {0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01,
                             0x01};
        runCase("varint", 8, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
                         0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
                         0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
                         0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                         0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                         0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        byte compressed[] = {0xff, 0x01,
                             0xff, 0xff, 0x03,
                             0xff, 0xff, 0xff, 0x07,
                             0xff, 0xff, 0xff, 0xff, 0x0f,
                             0xff, 0xff, 0xff, 0xff, 0xff, 0x1f,
                             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f,
                             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
                             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f};
        runCase("varint", 8, origin, sizeof(origin), compressed, sizeof(compressed));
    }
    {
        byte origin[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
                         0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
                         0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
                         0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                         0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                         0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        byte compressed[] = {0xfe, 0x03,
                             0xfe, 0xff, 0x07,
                             0xfe, 0xff, 0xff, 0x0f,
                             0xfe, 0xff, 0xff, 0xff, 0x1f,
                             0xfe, 0xff, 0xff, 0xff, 0xff, 0x3f,
                             0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
                             0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01,
                             0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01,
                             0x01};
        runCase("varint", 8, origin, sizeof(origin), compressed, sizeof(compressed));
    }
}
