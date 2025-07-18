#ifndef __DELTAD_H___
#define __DELTAD_H___

// deltaD means delta + zigzag + simple8b, but delta means with the min data

#include "c.h"
#include "common.h"

int deltaDCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int deltaDDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

#endif
