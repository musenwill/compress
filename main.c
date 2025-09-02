#include "c.h"
#include "common.h"
#include "compress.h"
#include "lossy.h"
#include "ut.h"

void helper() {
    printf("usage: compress filepath datatype [algorithm]\n");
    printf("usage: compress filepath rate algorithm delta [adaptive]\n");
}

int lossyCompress(int argc, char **ppArgv) {
    char *filepath = ppArgv[1];
    int rate = atoi(ppArgv[2]);
    char *algo = ppArgv[3];
    float64 delta = atof(ppArgv[4]);
    bool adaptive = false;
    if (argc >= 6 && strcmp(ppArgv[5], "adaptive") == 0) {
        adaptive = true;
    }

    return lossyCompressFile(filepath, algo, (float64)rate, delta, adaptive);
}

int losslessCompress(int argc, char **ppArgv) {
    char *filepath = ppArgv[1];
    char *datatype = ppArgv[2];
    char *algo = ppArgv[3];

    return compressFile(filepath, algo, datatype);
}

int main(int argc, char **ppArgv) {
    if (argc == 2 && strcmp(ppArgv[1], "test") == 0) {
        Test();
        return OK;
    }

    if (argc < 4) {
        helper();
        return ERR;
    }

    char *algo = ppArgv[3];

    if (isLossyAlgorithm(algo)) {
        return lossyCompress(argc, ppArgv);
    } else {
        return losslessCompress(argc, ppArgv);
    }

}
