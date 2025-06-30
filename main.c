#include "c.h"
#include "common.h"
#include "compress.h"
#include "ut.h"

void helper() {
    printf("usage: compress filepath algorithm datatype\n");
}

int main(int argc, char **ppArgv) {
    if (argc == 2 && strcmp(ppArgv[1], "test") == 0) {
        Test();
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
