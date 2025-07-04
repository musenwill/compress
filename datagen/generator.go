package main

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"log"
	"math"
	"math/rand"
	"os"
	"path/filepath"
	"strings"

	"github.com/urfave/cli"
)

/**
 * 1. 散点数据，即在 [min, max] 之间随机分布，参数 random{num, min, max}
 * 2. 重复数据，即数据有很高的重复度，参数 repeat{num, min, max, repeat}
 * 3. 规律数据，即数据随时间规律变化，参数 series{num, start, func}
 * 4. 聚簇数据，即数据集中分布在某小范围内，参数 cluster{num, min, max, sub_num, sub_min, sub_max}
 */

type GenWriter struct {
	rawFilePath  string
	txtFilePath  string
	descFilePath string
	rawFile      *os.File
	txtFile      *os.File
	descFile     *os.File
	rawWriter    *bufio.Writer
	txtWriter    *bufio.Writer
	descWriter   *bufio.Writer
}

func GenWriterCreate(outFile string) *GenWriter {
	dirPath := filepath.Dir(outFile)
	fileName := filepath.Base(outFile)
	fileName = strings.TrimSuffix(fileName, filepath.Ext(fileName))
	txtFilePath := filepath.Join(dirPath, fmt.Sprintf("%s.txt", fileName))
	descFilePath := filepath.Join(dirPath, fmt.Sprintf("%s.desc", fileName))

	rawFile, err := os.Create(outFile)
	if err != nil {
		log.Fatalf("failed create file %s, %s", outFile, err)
	}
	txtFile, err := os.Create(txtFilePath)
	if err != nil {
		log.Fatalf("failed create file %s, %s", txtFilePath, err)
	}
	descFile, err := os.Create(descFilePath)
	if err != nil {
		log.Fatalf("failed create file %s, %s", descFilePath, err)
	}

	return &GenWriter{
		rawFilePath:  outFile,
		txtFilePath:  txtFilePath,
		descFilePath: descFilePath,
		rawFile:      rawFile,
		txtFile:      txtFile,
		descFile:     descFile,
		rawWriter:    bufio.NewWriter(rawFile),
		txtWriter:    bufio.NewWriter(txtFile),
		descWriter:   bufio.NewWriter(descFile),
	}
}

func (w *GenWriter) Put(val any) {
	err := binary.Write(w.rawWriter, binary.BigEndian, val)
	if err != nil {
		log.Fatalf("failed write file %s, %s", w.rawFilePath, err)
	}
	_, err = fmt.Fprintf(w.txtWriter, "%v\n", val)
	if err != nil {
		log.Fatalf("failed write file %s, %s", w.txtFilePath, err)
	}
}

func (w *GenWriter) WriteDesc(desc string) {
	_, err := w.descWriter.WriteString(desc)
	if err != nil {
		log.Fatalf("failed write file %s, %s", w.descFilePath, err)
	}
}

func (w *GenWriter) Close() {
	err := w.rawWriter.Flush()
	if err != nil {
		log.Fatalf("failed flush file %s, %s", w.rawFilePath, err)
	}
	err = w.txtWriter.Flush()
	if err != nil {
		log.Fatalf("failed flush file %s, %s", w.txtFilePath, err)
	}
	err = w.descWriter.Flush()
	if err != nil {
		log.Fatalf("failed flush file %s, %s", w.descFilePath, err)
	}
	err = w.rawFile.Close()
	if err != nil {
		log.Fatalf("failed close file %s, %s", w.rawFilePath, err)
	}
	err = w.txtFile.Close()
	if err != nil {
		log.Fatalf("failed close file %s, %s", w.txtFilePath, err)
	}
	err = w.descFile.Close()
	if err != nil {
		log.Fatalf("failed close file %s, %s", w.descFilePath, err)
	}
}

func shrink(min, max, val int64) any {
	if max <= math.MaxInt8 && min >= math.MinInt8 {
		return int8(val)
	}
	if max <= math.MaxInt16 && min >= math.MinInt16 {
		return int16(val)
	}
	if max <= math.MaxInt32 && min >= math.MinInt32 {
		return int32(val)
	}
	return val
}

func sinx(val int64) int64 {
	return int64(10000 * math.Sin(float64(val)/1000))
}

func powx(val int64, base int64) int64 {
	return int64(math.Pow(float64(base), float64(val)))
}

func squarex(val int64) int64 {
	return val * val
}

func random(c *cli.Context) error {
	outFile := c.GlobalString(outPath.Name)
	num := c.Int(numFlag.Name)
	min := c.Int64(minFlag.Name)
	max := c.Int64(maxFlag.Name)

	if len(outFile) <= 0 {
		return fmt.Errorf("output file required")
	}
	if min >= max {
		return fmt.Errorf("min value %d expect smaller than max value %d", min, max)
	}

	writer := GenWriterCreate(outFile)
	defer writer.Close()

	for range num {
		val := rand.Int63n(int64(max - min))
		val += min
		writer.Put(shrink(min, max, val))
	}
	writer.WriteDesc(fmt.Sprintf("random(num=%d, min=%d, max=%d)\n", num, min, max))
	return nil
}

func repeat(c *cli.Context) error {
	outFile := c.GlobalString(outPath.Name)
	num := c.Int(numFlag.Name)
	min := c.Int64(minFlag.Name)
	max := c.Int64(maxFlag.Name)
	repeat := c.Int(repeatFlag.Name)
	repeatVal := c.Int64(repeatValFlag.Name)

	if len(outFile) <= 0 {
		return fmt.Errorf("output file required")
	}
	if min >= max {
		return fmt.Errorf("min value %d expect smaller than max value %d", min, max)
	}
	if repeat > num {
		return fmt.Errorf("repeat %d should not greater than num %d", repeat, num)
	}

	writer := GenWriterCreate(outFile)
	defer writer.Close()

	for range num {
		var val int64
		coin := rand.Int31n(int32(num))
		if coin < int32(repeat) {
			val = repeatVal
		} else {
			val = rand.Int63n(int64(max - min))
			val += min
		}
		writer.Put(shrink(min, max, val))
	}

	writer.WriteDesc(fmt.Sprintf("repeat(num=%d, min=%d, max=%d, repeat=%d)\n", num, min, max, repeat))
	return nil
}

func series(c *cli.Context) error {
	outFile := c.GlobalString(outPath.Name)
	num := c.Int(numFlag.Name)
	start := c.Int64(startFlag.Name)
	funcArg := c.Int64(funcArgFlag.Name)
	funcStr := c.String(funcFlag.Name)

	if len(outFile) <= 0 {
		return fmt.Errorf("output file required")
	}

	writer := GenWriterCreate(outFile)
	defer writer.Close()

	for range num {
		var val int64
		switch funcStr {
		case "linear":
			val = start
			start += funcArg
		case "square":
			val = squarex(start)
			start++
		case "pow":
			val = powx(start, funcArg)
			start++
		case "sin":
			val = sinx(start)
			start++
		default:
			return fmt.Errorf("unsupported func name %s", funcStr)
		}
		writer.Put(val)
	}
	writer.WriteDesc(fmt.Sprintf("random(num=%d, func=%s, start=%d, arg=%d)\n", num, funcStr, start, funcArg))
	return nil
}

func cluster(c *cli.Context) error {
	outFile := c.GlobalString(outPath.Name)
	num := c.Int(numFlag.Name)
	min := c.Int64(minFlag.Name)
	max := c.Int64(maxFlag.Name)
	subnum := c.Int(subnumFlag.Name)
	submin := c.Int64(subminFlag.Name)
	submax := c.Int64(submaxFlag.Name)

	if len(outFile) <= 0 {
		return fmt.Errorf("output file required")
	}
	if min >= max {
		return fmt.Errorf("min value %d expect smaller than max value %d", min, max)
	}
	if submin >= submax {
		return fmt.Errorf("submin value %d expect smaller than submax value %d", submin, submax)
	}
	if subnum > num {
		return fmt.Errorf("subnum %d expect smaller than num %d", subnum, num)
	}
	if submin < min {
		return fmt.Errorf("submin %d expect not less than min %d", submin, min)
	}
	if submax > max {
		return fmt.Errorf("submax %d expect not greater than max %d", submax, max)
	}

	writer := GenWriterCreate(outFile)
	defer writer.Close()

	for range num {
		var val int64
		coin := rand.Int31n(int32(num))
		if coin < int32(subnum) {
			val = rand.Int63n(int64(submax - submin))
			val += submin
		} else {
			val = rand.Int63n(int64(max - submax + submin - min))
			if val >= (submin - min) {
				val += (min + submax - submin)
			} else {
				val += min
			}
		}
		writer.Put(shrink(min, max, val))
	}

	writer.WriteDesc(fmt.Sprintf("repeat(num=%d, min=%d, max=%d, subnum=%d, submin=%d, submax=%d)\n",
		num, min, max, subnum, submin, submax))
	return nil
}
