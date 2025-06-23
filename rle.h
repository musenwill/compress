#ifndef __RLE_H___
#define __RLE_H___

#include "c.h"

int rleCompress(CUDesc *pDesc, CompressionIn *pIn, CompressionOut *pOut);

int rleDecompress(CUDesc *pDesc, CompressionIn *pIn, CompressionOut *pOut);

#endif
