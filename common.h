#ifndef __COMMON_H
#define __COMMON_H

#include "c.h"

#define OK          0
#define ERR         -1
#define ERR_MEM     -10
#define ERR_FILE    -11

enum LOG_LEVEL
{
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
};

static inline void print_log(enum LOG_LEVEL level, const char* filename, const char* func_name, int line, const char* fmt, ...)
{
    char* pclevel;
    switch (level)
    {
    case LOG_LEVEL_DEBUG:
        pclevel = "[DEBUG]";
        break;
    case LOG_LEVEL_INFO:
        pclevel = "[INFO]";
        break;
    case LOG_LEVEL_WARN:
        pclevel = "[WARNING]";
        break;
    case LOG_LEVEL_ERROR:
        pclevel = "[ERROR]";
        break;
    default:
        pclevel = "INFO";
        break;
    }

    time_t rawtime;
    time(&rawtime);

    fprintf(stderr, "%s %s, %s, %d | %s", pclevel, filename, func_name, line, ctime(&rawtime));
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    printf("\n");
    va_end(ap);
    fflush(stderr);
}

#define LOG_DEBUG(...) print_log(LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define LOG_INFO(...) print_log(LOG_LEVEL_INFO, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define LOG_WARN(...) print_log(LOG_LEVEL_WARN, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define LOG_ERROR(...) print_log(LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define LOG_FATAL(...) print_log(LOG_LEVEL_FATAL, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

static inline int64 systimeus(struct rusage start, struct rusage end) {
    int64 endus = end.ru_stime.tv_sec * 1000000 + end.ru_stime.tv_usec;
    int64 startus = start.ru_stime.tv_sec * 1000000 + start.ru_stime.tv_usec;
    return endus - startus;
}

static inline int64 usertimeus(struct rusage start, struct rusage end) {
    int64 endus = end.ru_utime.tv_sec * 1000000 + end.ru_utime.tv_usec;
    int64 startus = start.ru_utime.tv_sec * 1000000 + start.ru_utime.tv_usec;
    return endus - startus;
}

typedef struct {
    int len;
    int readPos;
    int writePos;
    int bufSize;
    byte buf[0];
} Buffer;

typedef struct {
    int64 minValue;
    int64 maxValue;
    int64 average;
    int64 sum;
    int64 count;
    int64 minDelta;
    int64 maxDelta;
    int64 avgldeltal;
    int64 continuity;
    int64 repeats;
    int64 smallNums;
    int eachValSize;
} CUDesc;

int64 BufferRead(Buffer *pBuffer, int datasize);

void BufferWrite(Buffer *pBuffer, int datasize, int64 data);

int createBuffer(int size, Buffer **ppBuffer);

void destroyBuffer(Buffer *pBuffer);

void supportedDataType(const char *dataType);

int dataTypeIsInteger(const char *dataType);

int dataTypeSize(const char *dataType);

void CUDescDumpHeader();

int CUDescDump(CUDesc *pDesc, byte *pBuf);

void dumpHexBuffer(const byte *buf, int len);

#define COMPRESS_BATCHSIZE  60000

#endif
