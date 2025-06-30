#ifndef __DELTA2_H___
#define __DELTA2_H___

#include "c.h"
#include "common.h"

int delta2Compress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int delta2Decompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

#endif
