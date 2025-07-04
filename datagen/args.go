package main

import (
	"fmt"
	"os"
	"time"

	"github.com/urfave/cli"
)

var outPath = cli.StringFlag{
	Name:     "out",
	Usage:    "output file path",
	Required: false,
}

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

var numFlag = cli.IntFlag{
	Name:     "num",
	Usage:    "number of values",
	Required: true,
}

var minFlag = cli.Int64Flag{
	Name:     "min",
	Usage:    "min value",
	Required: true,
}

var maxFlag = cli.Int64Flag{
	Name:     "max",
	Usage:    "max value",
	Required: true,
}

var repeatFlag = cli.IntFlag{
	Name:     "repeat",
	Usage:    "how many repeat values, <= num",
	Required: true,
}

var repeatValFlag = cli.Int64Flag{
	Name:     "repeatVal",
	Usage:    "repeat value",
	Required: true,
}

var startFlag = cli.Int64Flag{
	Name:     "start",
	Usage:    "series start",
	Required: true,
}

var funcFlag = cli.StringFlag{
	Name:     "func",
	Usage:    "series function, support {linear, pow, sin, square}",
	Required: true,
}

var funcArgFlag = cli.Int64Flag{
	Name:     "arg",
	Usage:    "func argument if required",
	Required: false,
}

var subnumFlag = cli.Int64Flag{
	Name:     "subnum",
	Usage:    "number of values",
	Required: true,
}

var subminFlag = cli.Int64Flag{
	Name:     "submin",
	Usage:    "min value",
	Required: true,
}

var submaxFlag = cli.Int64Flag{
	Name:     "submax",
	Usage:    "max value",
	Required: true,
}

func New() *cli.App {

	app := cli.NewApp()
	app.ErrWriter = os.Stdout
	app.EnableBashCompletion = true
	app.Name = "datagen"
	app.Usage = "column data generator, support extractor from csv file or generate by self"
	app.Author = "musenwill"
	app.Email = "musenwill@qq.com"
	app.Copyright = fmt.Sprintf("Copyright © 2025 - %v musenwill. All Rights Reserved.", time.Now().Year())
	app.Flags = []cli.Flag{outPath}
	app.Commands = []cli.Command{
		{
			Name:   "extract",
			Usage:  "extract column data from csv file",
			Flags:  []cli.Flag{csvPath, colIdx, colType, isRaw},
			Action: extract,
		},
		{
			Name:   "random",
			Usage:  "generate values randomly",
			Flags:  []cli.Flag{numFlag, minFlag, maxFlag},
			Action: random,
		},
		{
			Name:   "repeat",
			Usage:  "generate values with many repeats",
			Flags:  []cli.Flag{numFlag, minFlag, maxFlag, repeatFlag, repeatValFlag},
			Action: repeat,
		},
		{
			Name:   "series",
			Usage:  "generate series by math function",
			Flags:  []cli.Flag{numFlag, startFlag, funcFlag, funcArgFlag},
			Action: series,
		},
		{
			Name:   "cluster",
			Usage:  "generate values moustly clustered in a small section",
			Flags:  []cli.Flag{numFlag, minFlag, maxFlag, subnumFlag, subminFlag, submaxFlag},
			Action: cluster,
		},
	}

	return app
}
