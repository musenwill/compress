#ifndef __VARINT_H___
#define __VARINT_H___

#include "c.h"
#include "common.h"

int varintCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int varintDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

#endif
