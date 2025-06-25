#ifndef __COMPRESS_H___
#define __COMPRESS_H___

#include "c.h"

#define COMPRESS_BATCHSIZE  60000

typedef struct {
    Buffer *pBufs[1024];
    int len;
} CompressResult;

typedef struct {
    int64 minValue;
    int64 maxValue;
    int64 average;
    int64 sum;
    int64 count;
    int64 avgldeltal;
    int64 continuity;
    int64 repeats;
    int64 smallNums;
    int eachValSize;
} CUDesc;

int readFile(const char *filePath, Buffer **ppBuffer);

#endif
