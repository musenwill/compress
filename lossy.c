#include "lossy.h"

typedef struct {
    float64 timestampUs;
    float64 value;
} DataPoint;

typedef struct {

} SDTFilter;

#define TARGET_LOWER_BOUND 0.95 // 压缩比下限（0.95R）
#define TARGET_UPPER_BOUND 1.05 // 压缩比上限（1.05R）

typedef struct {
    float64 target_ratio;    // 目标压缩比R（如10）
    float64 current_deviation; // 当前死区阈值
    float64 adjust_factor;   // 调整系数（默认0.05）
    
    // 滑动窗口统计
    int window_size;        // 窗口大小 = R
    int processed_count;    // 已处理点数
    int saved_count;        // 已存储点数
    DataPoint last_saved;   // 上一个存储点
} DeadZoneFilter;

static void initDeadZoneFilter(
    DeadZoneFilter* filter, 
    float64 target_ratio, 
    float64 init_deviation
) {
    memset(filter, 0, sizeof(*filter));
    filter->target_ratio = target_ratio;
    filter->current_deviation = init_deviation;
    filter->adjust_factor = 0.05; // 默认每次调整±5%
    filter->window_size = (int)target_ratio;
    filter->processed_count = 0;
    filter->saved_count = 0;
}

void adjust_deviation(DeadZoneFilter* comp) {
    if (comp->processed_count < comp->window_size) return;

    double current_ratio = (double)comp->processed_count / comp->saved_count;
    double lower_bound = comp->target_ratio * TARGET_LOWER_BOUND;
    double upper_bound = comp->target_ratio * TARGET_UPPER_BOUND;

    if (current_ratio < lower_bound) {
        // 压缩不足：提高精度（减小阈值）
        comp->current_deviation *= (1.0 - comp->adjust_factor);
    } else if (current_ratio > upper_bound) {
        // 压缩过度：降低精度（增大阈值）
        comp->current_deviation *= (1.0 + comp->adjust_factor);
    }

    // 重置窗口统计
    comp->processed_count = 0;
    comp->saved_count = 0;
}

int compress_point(DeadZoneFilter* comp, DataPoint point) {
    comp->processed_count++;
    int should_save = 0;

    // 首点必存
    if (isnan(comp->last_saved.value)) {
        should_save = 1;
    } else {
        double value_diff = fabs(point.value - comp->last_saved.value);
        double time_diff = point.timestampUs - comp->last_saved.timestampUs;
        
        // 死区判断：值变化超过阈值则存储[6,8](@ref)
        if (value_diff > comp->current_deviation) {
            should_save = 1;
        }
    }

    if (should_save) {
        comp->last_saved = point; // 更新最后存储点
        comp->saved_count++;
    }

    // 每处理R个点后调整阈值
    if (comp->processed_count % comp->window_size == 0) {
        adjust_deviation(comp);
    }

    return should_save;
}

int lossyCompressFile(const char *filePath, const char *pAlgo, const char *dataType) {

}
