#ifndef __COMPRESS_H___
#define __COMPRESS_H___

#include "c.h"

typedef enum {
    CMPR_RLE            = 1,
    CMPR_SIMPLE8B       = 2,
    CMPR_BIT_PACKING    = 3,
    CMPR_VARINT         = 4,
    CMPR_DELTA          = 5,
    CMPR_DELTA2         = 6
} CompressType;

static inline const char *CompressTypeName(CompressType e) {
    static const char *names[] = {
        [1] = "rle",
        [2] = "simple8b",
        [3] = "bitpacking",
        [4] = "varint",
        [5] = "delta",
        [6] = "delta2",
    };

    if (NULL == names[e]) {
        return "unknown";
    } else {
        return names[e];
    }
}

typedef struct {
    CUDesc descs[1024 * 1024];
    Buffer *pBufs[1024 * 1024];
    int len;
} CompressResult;

typedef struct {
    int64 plainSize;
    int64 compressedSize;

    int64 compressTimeSysUs;
    int64 compressTimeUserUs;
    int64 decompressTimeSysUs;
    int64 decompressTimeUserUs;
} CompressStats;

void collectIntegerCU(Buffer *pIn, const char *dataType, Buffer *pOut, CUDesc *pDesc);

int compressFile(const char *filePath, const char *pAlgo, const char *dataType);

#endif
