#include "common.h"

int64 readData(int datasize, char* inbuf, unsigned int* inpos)
{
    char* psrc = inbuf + *inpos;
    int64 val = *(int64*)psrc;

    if (IS_LITTLE_ENDIAN) {
        switch (datasize) {
            case sizeof(char):
                val = val & 0xFF;
                break;

            case sizeof(int16):
                val = val & 0xFFFF;
                break;

            case sizeof(int32):
                val = val & 0xFFFFFFFF;
                break;

            case sizeof(int64):
                val = val;
                break;

            case 3:
                val = val & 0xFFFFFF;
                break;

            case 5:
                val = val & 0xFFFFFFFFFF;
                break;

            case 6:
                val = val & 0xFFFFFFFFFFFF;
                break;

            case 7:
                val = val & 0xFFFFFFFFFFFFFF;
                break;

            default:
                assert(false);
                break;
        }
    } else {
        switch (datasize) {
            case sizeof(char):
                val = val >> 56;
                break;

            case sizeof(int16):
                val = val >> 48;
                break;

            case sizeof(int32):
                val = val >> 32;
                break;

            case sizeof(int64):
                val = val;
                break;

            case 3:
                val = val >> 40;
                break;

            case 5:
                val = val >> 24;
                break;

            case 6:
                val = val >> 16;
                break;

            case 7:
                val = val >> 8;
                break;

            default:
                assert(false);
                break;
        }
    }

    *inpos += datasize;
    return val;
}

void writeData(int datasize, char* out, unsigned int* outpos, int64 data)
{
    char* pdst = out + *outpos;
    char* data_pos = (char*)&data;

    if (IS_LITTLE_ENDIAN) {
        switch (datasize) {
            case sizeof(char):
                *(char*)pdst = (char)(data);
                break;

            case sizeof(int16):
                *(int16*)pdst = (int16)(data);
                break;

            case sizeof(int32):
                *(int32*)pdst = (int32)(data);
                break;

            case sizeof(int64):
                *(int64*)pdst = (int64)(data);
                break;
            case 3:
                *(int16*)pdst = *(int16*)data_pos;
                pdst = pdst + 2;
                data_pos = data_pos + 2;
                *(char*)pdst = *data_pos;
                break;

            case 5:
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(char*)pdst = *data_pos;
                break;

            case 6:
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(int16*)pdst = *(int16*)data_pos;
                break;

            case 7:
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(int16*)pdst = *(int16*)data_pos;
                pdst = pdst + 2;
                data_pos = data_pos + 2;
                *(char*)pdst = *data_pos;
                break;

            default:
                assert(false);
                break;
        }
    } else {
        switch (datasize) {
            case sizeof(char):
                *(char*)pdst = (char)(data);
                break;

            case sizeof(int16):
                *(int16*)pdst = (int16)(data);
                break;

            case sizeof(int32):
                *(int32*)pdst = (int32)(data);
                break;

            case sizeof(int64):
                *(int64*)pdst = (int64)(data);
                break;
            case 3:
                data_pos = data_pos + 5;
                *(int16*)pdst = *(int16*)data_pos;
                pdst = pdst + 2;
                data_pos = data_pos + 2;
                *(char*)pdst = *data_pos;
                break;

            case 5:
                data_pos = data_pos + 3;
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(char*)pdst = *data_pos;
                break;

            case 6:
                data_pos = data_pos + 2;
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(int16*)pdst = *(int16*)data_pos;
                break;

            case 7:
                data_pos = data_pos + 1;
                *(int32*)pdst = *(int32*)data_pos;
                pdst = pdst + 4;
                data_pos = data_pos + 4;
                *(int16*)pdst = *(int16*)data_pos;
                pdst = pdst + 2;
                data_pos = data_pos + 2;
                *(char*)pdst = *data_pos;
                break;

            default:
                assert(false);
        }
    }

    *outpos += datasize;
}
