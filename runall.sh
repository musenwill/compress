#!/bin/sh

cd datagen && make generate && cd -
make clean && make

set -e

./compress test

./compress ./dataset/bitcoin/1.raw int32 rle
./compress ./dataset/bitcoin/1.raw int32 simple8b
./compress ./dataset/bitcoin/1.raw int32 bitpacking
./compress ./dataset/bitcoin/1.raw int32 varint
./compress ./dataset/bitcoin/1.raw int32 deltaA
./compress ./dataset/bitcoin/1.raw int32 deltaB
./compress ./dataset/bitcoin/1.raw int32 deltaC
./compress ./dataset/bitcoin/1.raw int32 delta2A
./compress ./dataset/bitcoin/1.raw int32 delta2B
./compress ./dataset/bitcoin/1.raw int32

./compress ./dataset/bitcoin/3.raw int32 rle
./compress ./dataset/bitcoin/3.raw int32 simple8b
./compress ./dataset/bitcoin/3.raw int32 bitpacking
./compress ./dataset/bitcoin/3.raw int32 varint
./compress ./dataset/bitcoin/3.raw int32 deltaA
./compress ./dataset/bitcoin/3.raw int32 deltaB
./compress ./dataset/bitcoin/3.raw int32 deltaC
./compress ./dataset/bitcoin/3.raw int32 delta2A
./compress ./dataset/bitcoin/3.raw int32 delta2B
./compress ./dataset/bitcoin/3.raw int32

./compress ./dataset/bitcoin/4.raw int32 rle
./compress ./dataset/bitcoin/4.raw int32 simple8b
./compress ./dataset/bitcoin/4.raw int32 bitpacking
./compress ./dataset/bitcoin/4.raw int32 varint
./compress ./dataset/bitcoin/4.raw int32 deltaA
./compress ./dataset/bitcoin/4.raw int32 deltaB
./compress ./dataset/bitcoin/4.raw int32 deltaC
./compress ./dataset/bitcoin/4.raw int32 delta2A
./compress ./dataset/bitcoin/4.raw int32 delta2B
./compress ./dataset/bitcoin/4.raw int32

./compress ./dataset/bitcoin/6.raw int64 rle
./compress ./dataset/bitcoin/6.raw int64 simple8b
./compress ./dataset/bitcoin/6.raw int64 bitpacking
./compress ./dataset/bitcoin/6.raw int64 varint
./compress ./dataset/bitcoin/6.raw int64 deltaA
./compress ./dataset/bitcoin/6.raw int64 deltaB
./compress ./dataset/bitcoin/6.raw int64 deltaC
./compress ./dataset/bitcoin/6.raw int64 delta2A
./compress ./dataset/bitcoin/6.raw int64 delta2B
./compress ./dataset/bitcoin/6.raw int64

./compress ./dataset/bitcoin/8.raw int32 rle
./compress ./dataset/bitcoin/8.raw int32 simple8b
./compress ./dataset/bitcoin/8.raw int32 bitpacking
./compress ./dataset/bitcoin/8.raw int32 varint
./compress ./dataset/bitcoin/8.raw int32 deltaA
./compress ./dataset/bitcoin/8.raw int32 deltaB
./compress ./dataset/bitcoin/8.raw int32 deltaC
./compress ./dataset/bitcoin/8.raw int32 delta2A
./compress ./dataset/bitcoin/8.raw int32 delta2B
./compress ./dataset/bitcoin/8.raw int32

./compress ./dataset/warehouse/0.raw int16 rle
./compress ./dataset/warehouse/0.raw int16 simple8b
./compress ./dataset/warehouse/0.raw int16 bitpacking
./compress ./dataset/warehouse/0.raw int16 varint
./compress ./dataset/warehouse/0.raw int16 deltaA
./compress ./dataset/warehouse/0.raw int16 deltaB
./compress ./dataset/warehouse/0.raw int16 deltaC
./compress ./dataset/warehouse/0.raw int16 delta2A
./compress ./dataset/warehouse/0.raw int16 delta2B
./compress ./dataset/warehouse/0.raw int16

./compress ./dataset/warehouse/1.raw int8 rle
./compress ./dataset/warehouse/1.raw int8 simple8b
./compress ./dataset/warehouse/1.raw int8 bitpacking
./compress ./dataset/warehouse/1.raw int8 varint
./compress ./dataset/warehouse/1.raw int8 deltaA
./compress ./dataset/warehouse/1.raw int8 deltaB
./compress ./dataset/warehouse/1.raw int8 deltaC
./compress ./dataset/warehouse/1.raw int8 delta2A
./compress ./dataset/warehouse/1.raw int8 delta2B
./compress ./dataset/warehouse/1.raw int8
