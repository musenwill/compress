
#ifndef __BIT_PACKING_H___
#define __BIT_PACKING_H___

#include "c.h"
#include "common.h"

int bitPackingCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int bitPackingDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

#endif
