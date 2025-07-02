package main

import (
	"bufio"
	"encoding/binary"
	"encoding/csv"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/urfave/cli"
)

type Desc struct {
	min        int64
	max        int64
	count      int64
	sum        int64
	average    int64
	attLen     int
	avgldeltal int64
	continuity int64
	repeats    int64
	smallNums  int64
	hasDesc    bool
}

type Column struct {
	values  []any
	colType string
	desc    Desc
}

func (c *Column) calDesc() {
	if len(c.values) < 2 {
		return
	}

	var min, max, pre int64
	var sum, delta int64
	var sumldeltal, continuity, repeats, smallNums int64
	var val int64
	preDeltaSign := 0 // 0 undefined, -1 negative, 1 positive

	for i, v := range c.values {
		switch any(v).(type) {
		case int8:
			val = int64(v.(int8))
		case int16:
			val = int64(v.(int16))
		case int32:
			val = int64(v.(int32))
		case int64:
			val = int64(v.(int64))
		default:
			return
		}

		if i == 0 {
			min = val
			max = val
		} else {
			if val == pre {
				repeats++
			}

			if val < min {
				min = val
			}
			if val > max {
				max = val
			}
			delta = val - pre

			if delta == 0 {
				continuity++
				preDeltaSign = 0
			} else if delta > 0 && preDeltaSign >= 0 {
				continuity++
			} else if delta < 0 && preDeltaSign <= 0 {
				continuity++
			}
			if delta > 0 {
				preDeltaSign = 1
				sumldeltal += delta
			} else if delta < 0 {
				preDeltaSign = -1
				sumldeltal += (delta * -1)
			}
		}
		if val < 256 && val*-1 < 256 {
			smallNums++
		}
		sum += val
		pre = val
	}

	c.desc.hasDesc = true
	c.desc.count = int64(len(c.values))
	c.desc.min = min
	c.desc.max = max
	c.desc.sum = sum
	c.desc.average = sum / c.desc.count
	c.desc.attLen = colTypeLen(c.colType)
	c.desc.avgldeltal = sumldeltal / (c.desc.count - 1)
	c.desc.continuity = continuity
	c.desc.repeats = repeats
	c.desc.smallNums = smallNums
}

func (c *Column) print() {
	for _, v := range c.values {
		println(v)
	}
}

func (c *Column) printDesc() {
	fmt.Printf("%#v\n", c.desc)
}

func (c *Column) dumpCol(filePath string, isRaw bool) {
	file, err := os.Create(filePath)
	if err != nil {
		log.Fatalf("failed create file %s, %s", filePath, err)
	}
	defer file.Close()

	writer := bufio.NewWriter(file)
	defer writer.Flush()

	if isRaw {
		for _, v := range c.values {
			err := binary.Write(writer, binary.BigEndian, v)
			if err != nil {
				log.Fatalf("failed write file %s, %s", filePath, err)
			}
		}
	} else {
		for _, v := range c.values {
			writer.WriteString(fmt.Sprintf("%v\n", v))
		}
	}
}

func (c *Column) dumpDesc(filePath string) {
	file, err := os.Create(filePath)
	if err != nil {
		log.Fatalf("failed create file %s, %s", filePath, err)
	}
	defer file.Close()

	writer := bufio.NewWriter(file)
	defer writer.Flush()

	writer.WriteString(fmt.Sprintf("%#v", c.desc))
}

func colTypeSupports(colType string) {
	supports := []string{"int8", "int16", "int32", "int64", "float32", "float64", "bool", "string"}
	supported := false
	for _, s := range supports {
		if s == colType {
			supported = true
		}
	}
	if !supported {
		log.Fatalf("column type %s unsupported", colType)
	}
}

func colTypeLen(colType string) int {
	switch colType {
	case "int8":
		return 1
	case "int16":
		return 2
	case "int32":
		return 4
	case "int64":
		return 8
	case "float32":
		return 4
	case "float64":
		return 8
	case "bool":
		return 1
	default:
		log.Fatalf("column type %s has no fix len", colType)
	}
	return 0
}

func csvCol(filePath string, colIdx int) []string {
	var colVals []string

	file, err := os.Open(filePath)
	if err != nil {
		log.Fatal("Failed open csv file: ", err)
	}
	defer file.Close()

	reader := csv.NewReader(file)
	_, _ = reader.Read() // skip header

	for {
		record, err := reader.Read()
		if err == io.EOF {
			break // 文件结束
		}
		if err != nil {
			log.Fatal("Failed read from csv file: ", err)
		}
		if int(colIdx) >= len(record) {
			log.Fatal("column index out of bound: ", err)
		}
		if len(record[colIdx]) == 0 {
			continue
		}
		colVals = append(colVals, record[colIdx])
	}

	return colVals
}

func csvCol2Column(colVals []string, colType string) Column {
	var column Column
	column.colType = colType

	switch colType {
	case "int8":
		for _, val := range colVals {
			parsed, err := strconv.ParseInt(val, 10, 8)
			if err != nil {
				log.Fatalf("column value %s can not convert to %s, %s", val, colType, err)
			}
			v := int8(parsed)
			column.values = append(column.values, v)
		}
	case "int16":
		for _, val := range colVals {
			parsed, err := strconv.ParseInt(val, 10, 16)
			if err != nil {
				log.Fatalf("column value %s can not convert to %s, %s", val, colType, err)
			}
			v := int16(parsed)
			column.values = append(column.values, v)
		}
	case "int32":
		for _, val := range colVals {
			parsed, err := strconv.ParseInt(val, 10, 32)
			if err != nil {
				log.Fatalf("column value %s can not convert to %s, %s", val, colType, err)
			}
			v := int32(parsed)
			column.values = append(column.values, v)
		}
	case "int64":
		for _, val := range colVals {
			parsed, err := strconv.ParseInt(val, 10, 64)
			if err != nil {
				log.Fatalf("column value %s can not convert to %s, %s", val, colType, err)
			}
			v := int64(parsed)
			column.values = append(column.values, v)
		}
	case "float32":
		for _, val := range colVals {
			parsed, err := strconv.ParseFloat(val, 32)
			if err != nil {
				log.Fatalf("column value %s can not convert to %s, %s", val, colType, err)
			}
			v := float32(parsed)
			column.values = append(column.values, v)
		}
	case "float64":
		for _, val := range colVals {
			parsed, err := strconv.ParseFloat(val, 64)
			if err != nil {
				log.Fatalf("column value %s can not convert to %s, %s", val, colType, err)
			}
			v := float64(parsed)
			column.values = append(column.values, v)
		}
	case "bool":
		for _, val := range colVals {
			val = strings.ToLower(val)
			val = strings.TrimSpace(val)
			if val == "true" || val == "false" || val == "0" || val == "1" || val == "t" || val == "f" {
				if val == "true " || val == "1" || val == "t" {
					column.values = append(column.values, true)
				} else {
					column.values = append(column.values, false)
				}
			} else {
				log.Fatalf("column value %s can not convert to %s", val, colType)
			}
		}
	case "string":
		for _, val := range colVals {
			column.values = append(column.values, val)
		}
	default:
		log.Fatalf("unsupported column type %s", colType)
	}

	return column
}

func extract(c *cli.Context) {
	filePath := c.GlobalString(csvPath.Name)
	colIdx := c.GlobalInt(colIdx.Name)
	colType := c.GlobalString(colType.Name)
	isRaw := c.GlobalBoolT(isRaw.Name)
	colTypeSupports(colType)

	colVals := csvCol(filePath, int(colIdx))
	col := csvCol2Column(colVals, colType)
	col.calDesc()

	dirPath := filepath.Dir(filePath)
	fileName := filepath.Base(filePath)
	fileName = strings.TrimSuffix(fileName, filepath.Ext(fileName))

	outPath := filepath.Join(dirPath, fileName)
	outColFileName := ""
	if isRaw {
		outColFileName = fmt.Sprintf("%d.raw", colIdx)
	} else {
		outColFileName = fmt.Sprintf("%d.txt", colIdx)
	}
	outColFile := filepath.Join(outPath, outColFileName)
	outColDesc := filepath.Join(outPath, fmt.Sprintf("%d.desc", colIdx))

	err := os.MkdirAll(outPath, 0755)
	if err != nil {
		log.Fatalf("failed create dir %s, %s", outPath, err)
	}

	col.dumpDesc(outColDesc)
	col.dumpCol(outColFile, isRaw)
}
