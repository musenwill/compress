#ifndef __COMPRESS_H___
#define __COMPRESS_H___

#include "c.h"

#define COMPRESS_BATCHSIZE  60000

typedef struct {
    Buffer *pBufs[1024];
    int len;
} CompressResult;

int readFile(const char *filePath, Buffer **ppBuffer);

#endif
