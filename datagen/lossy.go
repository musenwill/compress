package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"github.com/urfave/cli"
)

type tsval struct {
	timestamp int64
	value     string
}

type tsvalGroup struct {
	vals []tsval
}

func (g *tsvalGroup) Len() int {
	return len(g.vals)
}

func (g *tsvalGroup) Less(i, j int) bool {
	ri := g.vals[i]
	rj := g.vals[j]
	return ri.timestamp < rj.timestamp
}

func (g *tsvalGroup) Swap(i, j int) {
	g.vals[i], g.vals[j] = g.vals[j], g.vals[i]
}

func (g *tsvalGroup) Dump(filePath string) {
	file, err := os.Create(filePath)
	if err != nil {
		log.Fatalf("failed create file %s, %s", filePath, err)
	}
	defer file.Close()

	writer := bufio.NewWriter(file)
	defer writer.Flush()

	for _, v := range g.vals {
		writer.WriteString(fmt.Sprintf("%v    %v\n", v.timestamp, v.value))
	}
}

func lossyExtract(c *cli.Context) error {
	filePath := c.String(csvPath.Name)
	tsIdx := c.Int(tsIdx.Name)
	tgIdx := c.Int(tgIdx.Name)
	fdIdx := c.Int(fdIdx.Name)

	groups := make(map[string]*tsvalGroup)

	tgCol := csvCol(filePath, tgIdx)
	tsCol := csvCol(filePath, tsIdx)
	fdCol := csvCol(filePath, fdIdx)

	for i := 0; i < len(tgCol); i++ {
		ele := tsval{timestamp: parseTimestamp(tsCol[i]), value: fdCol[i]}
		group := groups[tgCol[i]]
		if group == nil {
			group = &tsvalGroup{vals: make([]tsval, 0)}
			groups[tgCol[i]] = group
		}
		group.vals = append(group.vals, ele)
	}

	for _, group := range groups {
		sort.Sort(group)
	}

	dirPath := filepath.Dir(filePath)
	fileName := filepath.Base(filePath)
	fileName = strings.TrimSuffix(fileName, filepath.Ext(fileName))

	outPath := filepath.Join(dirPath, fileName+"_lossy")
	err := os.MkdirAll(outPath, 0755)
	if err != nil {
		log.Fatalf("failed create dir %s, %s", outPath, err)
	}

	for k, group := range groups {
		outColFileName := fmt.Sprintf("%d_%s.txt", fdIdx, k)
		outColFile := filepath.Join(outPath, outColFileName)
		group.Dump(outColFile)
	}

	return nil
}
