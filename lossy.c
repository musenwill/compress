#include "lossy.h"

#define LOSSY_CMPR_INFINITE_RATE    10000
#define CMPR_RATE_ADJUST_FACTOR      0.05   // 调整系数

typedef struct {
    uint64 timestampUs;
    float64 value;
    char originLine[256];
} DataPoint;

typedef struct {

} SDTFilter;

/**
 * 死区压缩不用怎么考虑时间乱序的问题
 */
typedef struct {
    float64 targetRatio;        // 目标压缩比 R（如 10）
    float64 targetRatioLowerBound;
    float64 targetRatioUpperBound;
    float64 currentDeviation;   // 当前死区阈值
    float64 adjustFactor;       // 调整系数（默认 0.05）

    CircularBitmap bitmap;      // 记录最近 10R 个数据的过滤情况, 1 为保留，0 为丢弃
    float64 lastSavedVal;       // 上一个存储点
} DeadZoneFilter;

void DeadZoneFilterInit(
    DeadZoneFilter* filter, 
    float64 targetRatio,
    float64 init_deviation
) {
    Assert(targetRatio >= 5);
    Assert(init_deviation > 0);
    memset(filter, 0, sizeof(*filter));
    filter->targetRatio = targetRatio;
    filter->currentDeviation = init_deviation;
    filter->adjustFactor = CMPR_RATE_ADJUST_FACTOR; // 默认每次调整±5%
    filter->targetRatioLowerBound = targetRatio * (1 - filter->adjustFactor);
    filter->targetRatioUpperBound = targetRatio * (1 + filter->adjustFactor);
    CircularBitmapInit(&filter->bitmap, (int)(10 * targetRatio));
}

void DeadZoneFilterFini(DeadZoneFilter* filter) {
    CircularBitmapFini(&filter->bitmap);
}

bool DeadZoneFilterIsEmpty(DeadZoneFilter* filter) {
    return CircularBitmapIsEmpty(&filter->bitmap);
}

bool DeadZoneFilterCanCalcCmprRate(DeadZoneFilter* filter) {
    return 10 * filter->bitmap.len >= filter->bitmap.capacity;
}

float64 DeadZoneFilterCurrentCmprRate(DeadZoneFilter* filter) {
    if (filter->bitmap.bitCount <= 0) {
        return LOSSY_CMPR_INFINITE_RATE;
    } else {
        return (float64)filter->bitmap.len / (float64)filter->bitmap.bitCount;
    }
}

void DeadZoneFilterAdjustDeviation(DeadZoneFilter* filter) {
    if (!DeadZoneFilterCanCalcCmprRate(filter)) {
        return;
    }

     // 提升压缩率时慢点增大阈值，降低压缩率时快点减少阈值
     // 可用性优先级高于压缩率
    float64 current_ratio = DeadZoneFilterCurrentCmprRate(filter);
    if (current_ratio < filter->targetRatioLowerBound) {
        // 压缩不足：降低精度（增大阈值）
        filter->currentDeviation *= (1.0 + filter->adjustFactor);
    } else if (current_ratio > filter->targetRatioUpperBound) {
        // 压缩过度：提高精度（减小阈值）
        filter->currentDeviation *= pow((1.0 - filter->adjustFactor), current_ratio / filter->targetRatio);
    }
}

bool DeadZoneFilterShouldKeepPoint(DeadZoneFilter* filter, float64 val) {
    bool shouldKeep = false;

    // the first point must keep
    if (DeadZoneFilterIsEmpty(filter)) {
        shouldKeep = true;
    } else {
        double value_diff = fabs(val - filter->lastSavedVal);
        // 死区判断：值变化超过阈值则存储
        if (value_diff > filter->currentDeviation) {
            shouldKeep = true;
        }
    }

    CircularBitmapPut(&filter->bitmap, shouldKeep);
    if (shouldKeep) {
        filter->lastSavedVal = val; // 更新最后存储点
    }

    // 动态调整阈值
    DeadZoneFilterAdjustDeviation(filter);

    return shouldKeep;
}

bool isLossyAlgorithm(const char *pAlgo) {
    if ((strcmp(pAlgo, "deadzone") != 0) && (strcmp(pAlgo, "dst") != 0)) {
        return false;
    }
    return true;
}

void trim_trailing(char *str) {
    if (str == NULL || *str == '\0') return;
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
}

int readDataPointFromFile(FILE *pFile, DataPoint *dp) {
    dp->originLine[0] = '\0';
    char *pLine = fgets(dp->originLine, sizeof(dp->originLine), pFile);
    if (pLine == NULL) {
        return ERR_EOF;
    }
    trim_trailing(pLine);
    sscanf(pLine, "%ld    %lf", &dp->timestampUs, &dp->value);
    return OK;
}

int lossyCompressFile(const char *filePath, const char *pAlgo, float rate) {
    int rc = OK;

    FILE *pFile = fopen(filePath, "r");
    if (pFile == NULL) {
        LOG_ERROR("Failed open file %s", filePath);
        rc = ERR_FILE;
        goto l_end;
    }

    if (strcmp(pAlgo, "deadzone") == 0) {
        DataPoint dp = { };
        DeadZoneFilter filter = {};
        DeadZoneFilterInit(&filter, rate, 1.0);

        while (readDataPointFromFile(pFile, &dp) >= 0) {
            if (DeadZoneFilterShouldKeepPoint(&filter, dp.value)) {
                printf("%s\n", dp.originLine);
            }
        }

        DeadZoneFilterFini(&filter);
    } else if (strcmp(pAlgo, "sdt") == 0) {

    } else {
        LOG_FATAL("lossy compress algorithm %s unsupported yet", pAlgo);
    }

l_end:
    if (pFile != NULL) {
        fclose(pFile);
    }
    return rc;
}
