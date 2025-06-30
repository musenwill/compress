#ifndef __DELTAA_H___
#define __DELTAA_H___

#include "c.h"
#include "common.h"

int deltaCalculate(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int deltaACompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int deltaADecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

#endif
