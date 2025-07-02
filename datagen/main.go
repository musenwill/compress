package main

import (
	"fmt"
	"os"
)

func main() {
	err := New().Run(os.Args)
	if err != nil {
		fmt.Println(err)
	}
}
