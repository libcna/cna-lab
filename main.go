package main

import (
	"github.com/openeggbert/cna-go/framework"
)

func main() {
	game := NewHelloGame()
	framework.Run(game)
}
