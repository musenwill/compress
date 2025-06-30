#ifndef __DELTA2B_H___
#define __DELTA2B_H___

// deltaA means delta2 + zigzag + rle

#include "c.h"
#include "common.h"

int delta2BCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int delta2BDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

#endif
