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

typedef struct {
    int len;
    int readPos;
    int writePos;
    int bufSize;
    char buf[0];
} Buffer;

int64 BufferRead(Buffer *pBuffer, int datasize);

void BufferWrite(Buffer *pBuffer, int datasize, int64 data);

int createBuffer(int size, Buffer **ppBuffer);

void destroyBuffer(Buffer *pBuffer);

void supportedDataType(const char *dataType);

int dataTypeIsInteger(const char *dataType);

int dataTypeSize(const char *dataType);

#endif
