# 测试数据集

测试数据集包含两部分，分别为从网络下载的真实时序数据和采用工具生成的数据。

### 真实时序数据
真实时序数据存放路径为 dataset/bitcoin(来源于 timescaledb 官网) 和 dataset/warehouse(来源于阿里天池)。

真实时序数据下载的原始文件都是 csv，不是压缩算法所需要的二进制数据，因此需要对 csv 文件的数据按列进行提取。提取工具用 golang 编写，进入 datagen 目录 make 编译即可，命令为 `datagen extract`，可通过 `datagen extract -h` 查看使用帮助。

bitcoin 和 warehouse 两个目录下是已经提取好了的数据，csv 中提取第 n 列会生成三个文件(n.raw, n.txt, n.desc)，其中 n 为列的顺序，n.raw 为数据的二进制，n.txt 为数据的文本形式，n.desc 为数据的特征描述信息，n.desc 各字段介绍如下:
```
{
    min:            // 数据的最小值
    max:            // 数据的最大值
    count:          // 数据的个数
    sum:            // 数据求和
    average:        // 数据平均值
    attLen:         // 数据的字节数，在提取时会根据数据的 [min, max] 确定所需的字节数(1, 2, 4, 8)
    avgldeltal:     // delta 值绝对值的平均数，用于评估数据的平稳性
    continuity:     // 评估数据的连续性，数据 delta 值的正负号变化越频繁连续性越差
    repeats:        // 数据重复度，即有多少个数据与前一个数据相同，或 delta 值为 0 的个数
    smallNums:      // (-256, 256) 之间小整数的个数
    hasDesc:        // 该 desc 是否有效，目前只有整数数值类型的列才会生成有效的 .desc 文件
}
```

### 生成数据
真实时序数据无法按测试需要调整数据特征，因此还需要能够根据需求生成具备某些特征的数据。生成数据的命令可查看帮助 `datagen h`，如 `datagen random` 就是生成完全随机的数据，`datagen repeat` 就是生成具有一定重复度的数据。

若 datagen 目录下执行 `make generate` 命令，会在 dataset/gen 目录下生成系列数据，这些数据量比较大，已被 .gitignore 忽略，想要用的时候随时生成即可。
