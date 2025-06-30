#include "rle.h"
#include "zigzag.h"
#include "simple8b.h"
#include "deltaA.h"
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
    } else {
        LOG_FATAL("compress algorithm %s unsupported yet", pAlgo);
    }

    return pf;
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
    desc.eachValSize = eachValSize;
    ret = pfCmprFunc(&desc, pPlain, pCompressed);
    assert(ret >= 0);
    assert(pCompressed->len == expectSize);
    if (memcmp(pExpect, pCompressed->buf, expectSize) != 0) {
        printf("simple8b expect compress result: \n");
        dumpHexBuffer(pExpect, expectSize);
        printf("simple8b actual compress result: \n");
        dumpHexBuffer(pCompressed->buf, pCompressed->len);
    }

    ret = pfDecmprFunc(&desc, pCompressed, pDecompressed);
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
}
