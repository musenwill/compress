#ifndef __DELTA2A_H___
#define __DELTA2A_H___

#include "c.h"
#include "common.h"

int delta2Calculate(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int delta2Recover(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int delta2ACompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int delta2ADecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

#endif
