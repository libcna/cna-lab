// Package game contains the Linux desktop CNA-Go Foundation 1 canary.
package game

import (
	"errors"
	"fmt"
	"math"
	"os"

	framework "github.com/openeggbert/cna-go/Microsoft/Xna/Framework"
	graphics "github.com/openeggbert/cna-go/Microsoft/Xna/Framework/Graphics"
	input "github.com/openeggbert/cna-go/Microsoft/Xna/Framework/Input"
)

const animationSpeed float32 = 2

// HelloGame exercises only the Foundation 1 surface: native Game callbacks,
// device/viewport/clear, PNG decoding, SpriteBatch, and keyboard polling.
type HelloGame struct {
	frameTarget int
	assetPath   string

	manager     *framework.GraphicsDeviceManager
	device      *graphics.GraphicsDevice
	spriteBatch *graphics.SpriteBatch
	logo        *graphics.Texture2D

	animationSeconds float32
	drawnFrames      int
	position         framework.Vector2
	velocity         framework.Vector2
	logoWidth        int32
	logoHeight       int32
}

func NewHelloGame(frameTarget int, assetPath string) (*HelloGame, error) {
	if frameTarget != 60 && frameTarget != 600 {
		return nil, fmt.Errorf("--frames must be exactly 60 or 600, got %d", frameTarget)
	}
	if assetPath == "" {
		return nil, errors.New("asset path must not be empty")
	}
	return &HelloGame{
		frameTarget: frameTarget,
		assetPath:   assetPath,
		velocity:    framework.Vector2{X: 104, Y: 74},
	}, nil
}

func (g *HelloGame) Initialize(host *framework.Game) error {
	manager, err := framework.NewGraphicsDeviceManager(host)
	if err != nil {
		return err
	}
	g.manager = manager
	return nil
}

func (g *HelloGame) LoadContent(_ *framework.Game) error {
	device, err := graphics.GraphicsDeviceManagerGraphicsDevice(g.manager)
	if err != nil {
		return err
	}
	g.device = device
	g.spriteBatch, err = graphics.NewSpriteBatch(device)
	if err != nil {
		return err
	}
	asset, err := os.Open(g.assetPath)
	if err != nil {
		return err
	}
	g.logo, err = graphics.Texture2DFromStreamByGraphicsDeviceAndStream(device, asset)
	closeErr := asset.Close()
	if err != nil {
		return errors.Join(err, closeErr)
	}
	if closeErr != nil {
		return closeErr
	}
	// Width and Height are plain field reads in XNA and in CNA-Go, so neither
	// carries an error: the texture's description is cached at construction.
	g.logoWidth, g.logoHeight = g.logo.Width(), g.logo.Height()
	viewport, err := device.Viewport()
	if err != nil {
		return err
	}
	g.position = framework.Vector2{X: float32(viewport.Width()) * 0.5, Y: float32(viewport.Height()) * 0.5}
	fmt.Printf("cna-go-template: loaded PNG %dx%d; native viewport %dx%d\n", g.logoWidth, g.logoHeight, viewport.Width(), viewport.Height())
	return nil
}

func (g *HelloGame) Update(host *framework.Game, gameTime framework.GameTime) error {
	seconds := float32(gameTime.ElapsedGameTime().TotalSeconds())
	g.animationSeconds += seconds
	keyboard, err := input.KeyboardGetStateByNone()
	if err != nil {
		return err
	}
	if keyboard.IsKeyDown(input.KeysEscape) {
		return host.Exit()
	}
	g.position.X += g.velocity.X * seconds
	g.position.Y += g.velocity.Y * seconds
	return g.keepLogoInViewport()
}

func (g *HelloGame) Draw(host *framework.Game, _ framework.GameTime) error {
	if err := g.device.ClearByColor(framework.ColorCornflowerBlue()); err != nil {
		return err
	}
	motion := g.animationSeconds * animationSpeed
	scale := float32(0.96) + float32(0.12)*float32(math.Sin(float64(motion*0.65)))
	rotation := float32(0.11) * float32(math.Sin(float64(motion*0.55)))
	origin := framework.Vector2{X: float32(g.logoWidth) * 0.5, Y: float32(g.logoHeight) * 0.5}
	if err := g.spriteBatch.BeginByNone(); err != nil {
		return err
	}
	if err := g.spriteBatch.DrawByTexture2DAndVector2AndNullableOfRectangleAndColorAndSingleAndVector2AndSingleAndSpriteEffectsAndSingle(
		g.logo, g.position, nil, framework.ColorWhite(), rotation, origin, scale, graphics.SpriteEffectsNone, 0,
	); err != nil {
		return err
	}
	if err := g.spriteBatch.End(); err != nil {
		return err
	}
	g.drawnFrames++
	if g.drawnFrames == g.frameTarget {
		fmt.Printf("CNA_GO_CANARY_DRAW_FRAMES=%d\n", g.drawnFrames)
		return host.Exit()
	}
	if g.drawnFrames > g.frameTarget {
		return fmt.Errorf("native loop exceeded exact frame target: target=%d observed=%d", g.frameTarget, g.drawnFrames)
	}
	return nil
}

func (g *HelloGame) UnloadContent(_ *framework.Game) error {
	var batchErr, textureErr, managerErr error
	if g.spriteBatch != nil {
		batchErr = g.spriteBatch.DisposeByBoolean(true)
	}
	if g.logo != nil {
		textureErr = g.logo.DisposeByBoolean(true)
	}
	if g.manager != nil {
		managerErr = g.manager.Dispose(true)
	}
	return errors.Join(batchErr, textureErr, managerErr)
}

func (g *HelloGame) keepLogoInViewport() error {
	viewport, err := g.device.Viewport()
	if err != nil {
		return err
	}
	halfWidth := float32(g.logoWidth) * 0.5
	halfHeight := float32(g.logoHeight) * 0.5
	maxX := float32(viewport.Width()) - halfWidth
	maxY := float32(viewport.Height()) - halfHeight
	if g.position.X < halfWidth {
		g.position.X = halfWidth
		g.velocity.X = abs32(g.velocity.X)
	} else if g.position.X > maxX {
		g.position.X = maxX
		g.velocity.X = -abs32(g.velocity.X)
	}
	if g.position.Y < halfHeight {
		g.position.Y = halfHeight
		g.velocity.Y = abs32(g.velocity.Y)
	} else if g.position.Y > maxY {
		g.position.Y = maxY
		g.velocity.Y = -abs32(g.velocity.Y)
	}
	return nil
}

func abs32(value float32) float32 {
	if value < 0 {
		return -value
	}
	return value
}
