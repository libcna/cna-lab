package main

import (
	"math"
	"github.com/openeggbert/cna-go/framework"
	"github.com/openeggbert/cna-go/framework/graphics"
	"github.com/openeggbert/cna-go/framework/math32"
)

type HelloGame struct {
	*framework.Game
	graphics *framework.GraphicsDeviceManager
	batch    *graphics.SpriteBatch
	logo     *graphics.Texture2D
	position math32.Vector2
}

func NewHelloGame() *HelloGame {
	g := &HelloGame{
		Game: framework.NewGame(),
	}
	g.graphics = framework.NewGraphicsDeviceManager(g.Game)
	g.Game.Content().SetRootDirectory("Content")
	return g
}

func (g *HelloGame) Initialize() {
	g.graphics.SetPreferredBackBufferWidth(1280)
	g.graphics.SetPreferredBackBufferHeight(720)
	g.graphics.ApplyChanges()
	g.Game.Initialize()
}

func (g *HelloGame) LoadContent() {
	g.batch = graphics.NewSpriteBatch(g.Game.GraphicsDevice())
	g.logo = g.Game.Content().LoadTexture2D("logo")
}

func (g *HelloGame) Update(gameTime framework.GameTime) {
	time := float32(gameTime.TotalGameTime().Seconds())
	g.position.X = 640.0 + 200.0*float32(math.Sin(float64(time)))
	g.position.Y = 360.0 + 200.0*float32(math.Cos(float64(time)))
	g.Game.Update(gameTime)
}

func (g *HelloGame) Draw(gameTime framework.GameTime) {
	g.Game.GraphicsDevice().Clear(graphics.ColorCornflowerBlue)
	g.batch.Begin()
	g.batch.Draw(g.logo, g.position, graphics.ColorWhite)
	g.batch.End()
	g.Game.Draw(gameTime)
}
