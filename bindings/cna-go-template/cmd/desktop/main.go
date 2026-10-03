package main

import (
	"flag"
	"log"

	"github.com/openeggbert/cna-go-template/game"
	framework "github.com/openeggbert/cna-go/Microsoft/Xna/Framework"
)

func main() {
	frames := flag.Int("frames", 60, "exact native Draw callback target (60 or 600)")
	asset := flag.String("asset", "Content/logo.png", "PNG asset path")
	flag.Parse()

	callbacks, err := game.NewHelloGame(*frames, *asset)
	if err != nil {
		log.Fatal(err)
	}
	host, err := framework.NewGame(callbacks)
	if err != nil {
		log.Fatal(err)
	}
	if err := host.Run(); err != nil {
		log.Fatalf("Game exited with error: %v", err)
	}
}
