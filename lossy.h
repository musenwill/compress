#ifndef __LOSSY_H___
#define __LOSSY_H___

#include "c.h"
#include "common.h"

bool isLossyAlgorithm(const char *pAlgo);

int lossyCompressFile(const char *filePath, const char *pAlgo, float rate, bool adaptive);

#endif
