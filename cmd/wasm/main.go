//go:build js && wasm
package main

import (
	"log"
	"syscall/js"

	"github.com/openeggbert/cna-go/Microsoft/Xna/Framework"
	"github.com/openeggbert/cna-go-template/game"
)

func main() {
	smokeTest := false
	href := js.Global().Get("location").Get("href").String()
	if slice := "smoke-test"; href != "" && (href == slice || (len(href) > len(slice) && href[len(href)-len(slice):] == slice)) {
		smokeTest = true
	}

	g := game.NewHelloGame(smokeTest)
	if err := framework.Run(g); err != nil {
		log.Fatalf("Game exited with error: %v", err)
	}
}
