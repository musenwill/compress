#ifndef __DELTAB_H___
#define __DELTAB_H___

// deltaB means delta + zigzag + rle

#include "c.h"
#include "common.h"

int deltaBCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int deltaBDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

#endif
