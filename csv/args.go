package main

import (
	"fmt"
	"os"
	"time"

	"github.com/urfave/cli"
)

var csvPath = cli.StringFlag{
	Name:     "file",
	Usage:    "csv file path",
	Required: true,
}
var colIdx = cli.IntFlag{
	Name:     "idx",
	Usage:    "column index, start from 0",
	Required: true,
}
var colType = cli.StringFlag{
	Name:  "type",
	Usage: "column data type, including [int8, int16, int32, int64, float32, float64, bool, string]",
}
var isRaw = cli.BoolFlag{
	Name:  "raw",
	Usage: "raw output or plain output",
}

func New() *cli.App {

	app := cli.NewApp()
	app.ErrWriter = os.Stdout
	app.EnableBashCompletion = true
	app.Name = "extractor"
	app.Usage = "Extractor column data from csv file and write into new file"
	app.Author = "musenwill"
	app.Email = "musenwill@qq.com"
	app.Copyright = fmt.Sprintf("Copyright © 2025 - %v musenwill. All Rights Reserved.", time.Now().Year())
	app.Flags = []cli.Flag{csvPath, colIdx, colType, isRaw}
	app.Action = action

	return app
}
