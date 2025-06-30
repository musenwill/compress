#ifndef __SIMPLE8B_H___
#define __SIMPLE8B_H___

#include "c.h"
#include "common.h"

int simple8bCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int simple8bDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

void simple8bUT();

#endif
