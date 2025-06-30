#ifndef __DELTA_H___
#define __DELTA_H___

#include "c.h"
#include "common.h"

int deltaCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int deltaDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

#endif
