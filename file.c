#include "file.h"

int readFile(const char *filePath, Buffer **ppBuffer) {
    int ret = OK;

    struct stat stat_buf;
    if (stat(filePath, &stat_buf) < 0) {
        LOG_ERROR("Failed stat file %s", filePath);
        return ERR_FILE;
    }

    FILE *pFile = fopen(filePath, "rb");
    if (pFile == NULL) {
        LOG_ERROR("Failed open file %s", filePath);
        ret = ERR_FILE;
        goto l_end;
    }

    Buffer *pBuffer = NULL;
    ret = createBuffer(stat_buf.st_size, &pBuffer);
    if (ret < 0) {
        goto l_end;
    }

    int rd = fread(pBuffer->buf, 1, pBuffer->bufSize, pFile);
    if (rd < pBuffer->len) {
        LOG_ERROR("Failed read file %s, expect read %d bytes, actual got %d", filePath, pBuffer->len, rd);
        ret = ERR_FILE;
        goto l_fail;
    }
    pBuffer->len = rd;
    *ppBuffer = pBuffer;

    goto l_end;

l_fail:
    if (pBuffer != NULL) {
        destroyBuffer(pBuffer);
    }

l_end:
    if (pFile != NULL) {
        fclose(pFile);
    }
    return ret;
}


