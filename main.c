#include "c.h"
#include "common.h"
#include "compress.h"
#include "lossy.h"
#include "ut.h"

void helper() {
    printf("usage: compress filepath datatype [algorithm]\n");
}

int main(int argc, char **ppArgv) {
    if (argc == 2 && strcmp(ppArgv[1], "test") == 0) {
        Test();
        return OK;
    }

    if (argc < 3) {
        helper();
        return ERR;
    }
    char *filepath = ppArgv[1];
    char *dataType = ppArgv[2];
    char *algo = NULL;
    if (argc >= 4) {
        algo = ppArgv[3];
    }
    bool adaptive = false;
    if (argc >= 5 && strcmp(ppArgv[4], "adaptive") == 0) {
        adaptive = true;
    }

    if (isLossyAlgorithm(algo)) {
        int rate = atoi(dataType);
        return lossyCompressFile(filepath, algo, (float64)rate, adaptive);
    } else {
        return compressFile(filepath, algo, dataType);
    }
}
