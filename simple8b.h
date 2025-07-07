#ifndef __SIMPLE8B_H___
#define __SIMPLE8B_H___

#include "c.h"
#include "common.h"

#define SIMPLE8B_PACKING_BITS       (60)

int simple8bCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int simple8bDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

float32 simple8bEstimate(CUDesc *pDesc);

#endif
