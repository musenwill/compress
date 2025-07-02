#ifndef __DELTAC_H___
#define __DELTAC_H___

// deltaB means delta + zigzag + bitpacking

#include "c.h"
#include "common.h"

int deltaCCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int deltaCDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

#endif
