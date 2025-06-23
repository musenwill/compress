#ifndef __COMMON_H
#define __COMMON_H

#include "c.h"

typedef struct {
    int64 minValue;
    int64 maxValue;
    int64 average;
    int64 sum;
    int rowCount;
    int eachValSize;
} CUDesc;

typedef struct {
    char *buf;
    int sz;
    int16 mode;
} CompressionIn;

typedef struct {
    char *buf;
    int sz;
    int16 mode;
} CompressionOut;

#endif
