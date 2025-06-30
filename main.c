#include "c.h"
#include "common.h"
#include "rle.h"
#include "zigzag.h"
#include "simple8b.h"
#include "compress.h"

void helper() {
    printf("usage: compress filepath algorithm datatype\n");
}

int main(int argc, char **ppArgv) {
    if (argc == 2 && strcmp(ppArgv[1], "test") == 0) {
        rleUT();
        zigzagUT();
        simple8bUT();
        return OK;
    }

    if (argc != 4) {
        helper();
        return ERR;
    }
    char *filepath = ppArgv[1];
    char *algo = ppArgv[2];
    char *dataType = ppArgv[3];

    return compressFile(filepath, algo, dataType);
}
