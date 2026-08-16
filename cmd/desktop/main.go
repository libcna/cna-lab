package main

import (
	"flag"
	"log"

	"github.com/openeggbert/cna-go/Microsoft/Xna/Framework"
	"github.com/openeggbert/cna-go-template/game"
)

func main() {
	smokeTest := flag.Bool("smoke-test", false, "Enable smoke test mode")
	flag.Parse()

	g := game.NewHelloGame(*smokeTest)
	if err := framework.Run(g); err != nil {
		log.Fatalf("Game exited with error: %v", err)
	}
}
