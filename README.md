实现各类列存压缩算法并进行测评对比，评估压缩性能和压缩效果。

## 压缩算法
实现的压缩算法包括如下:
* rle
* simple8b (zigzag + simple8b)
* bitpacking (zigzag + bitpacking)
* varint (zigzag + varint)
* deltaA=(delta + zigzag + simple8b)
* deltaB=(delta + zigzag + rle)
* deltaC=(delta + zigzag + bitpacking)
* delta2A=(delta2 + zigzag + simple8b)
* delta2B=(delta2 + zigzag + rle)

## 测试数据集
参见 [dataset/README.md](dataset/README.md) 文件

## 测试策略
1. 模拟 CU 6w 行数据，在对数据集进行压缩时，以 6w 条数据分批压缩；
2. 收集压缩解压所需时间、收集压缩前后的压缩比；
3. 同一数据集采用各种不同压缩算法进行压缩测试；

## 使用方式
1. 根目录下执行 make 编译，得到可执行文件 compress
2. `compress test` 执行 UT
3. `compress filepath algorithm datatype` 执行对二进制文件的压缩
