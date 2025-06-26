#ifndef __RLE_H___
#define __RLE_H___

#include "c.h"
#include "common.h"

int rleCompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

int rleDecompress(CUDesc *pDesc, Buffer *pIn, Buffer *pOut);

void rleUT();

#endif
