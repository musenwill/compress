#ifndef __ZIGZAG_H___
#define __ZIGZAG_H___

#include "c.h"
#include "common.h"

int zigzagCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int zigzagDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

#endif
