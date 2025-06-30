#ifndef __COMPRESS_H___
#define __COMPRESS_H___

#include "c.h"

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

int compressFile(const char *filePath, const char *pAlgo, const char *dataType);

#endif
