//go:build android
package main

import (
	"github.com/openeggbert/cna-go/Microsoft/Xna/Framework"
	"github.com/openeggbert/cna-go-template/game"
)

func main() {
	// Smoke test is typically not used on mobile via command line flags
	g := game.NewHelloGame(false)
	if err := framework.Run(g); err != nil {
		panic(err)
	}
}
