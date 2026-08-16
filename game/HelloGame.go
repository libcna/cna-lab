package game

import (
	"fmt"
	"math"
	"strings"

	"github.com/openeggbert/cna-go/Microsoft/Xna/Framework"
	"github.com/openeggbert/cna-go/Microsoft/Xna/Framework/Content"
	"github.com/openeggbert/cna-go/Microsoft/Xna/Framework/Graphics"
	"github.com/openeggbert/cna-go/Microsoft/Xna/Framework/Input"
)

type HelloGame struct {
	Graphics      *framework.GraphicsDeviceManager
	content       *content.ContentManager
	spriteBatch   *graphics.SpriteBatch
	logo          *graphics.Texture2D
	solid         *graphics.Texture2D
	cubeEffect    *graphics.BasicEffect
	
	rendererName  string
	supports3D    bool
	supportsDepth bool
	
	animationSeconds float32
	drawnFrames      int
	smokeTest        bool
	
	position framework.Vector2
	velocity framework.Vector2
}

const (
	AnimationSpeed        = 2.0
	RendererBannerSeconds = 5.0
)

func NewHelloGame(smokeTest bool) *HelloGame {
	g := &HelloGame{
		smokeTest: smokeTest,
		velocity:  framework.Vector2{X: 104, Y: 74},
	}
	return g
}

func (g *HelloGame) Initialize() error {
	g.Graphics = framework.NewGraphicsDeviceManager(g)
	return nil
}

func (g *HelloGame) LoadContent() error {
	g.content = content.NewContentManager(g, "Content")
	gd := g.Graphics.GraphicsDevice()
	g.spriteBatch = graphics.NewSpriteBatch(gd)
	
	var err error
	g.logo, err = g.content.LoadTexture2D("logo")
	if err != nil {
		return err
	}
	
	g.solid = graphics.NewTexture2D(gd, 1, 1)
	g.solid.SetData([]graphics.Color{graphics.ColorWhite})
	
	g.rendererName = g.getRendererName()
	g.supports3D = g.supportsCapability("ThreeD")
	g.supportsDepth = g.supportsCapability("DepthStencilBuffer")
	
	if g.supports3D {
		g.cubeEffect = graphics.NewBasicEffect(gd)
		g.cubeEffect.SetTextureEnabled(true)
		g.cubeEffect.SetTexture(g.logo)
	}
	
	vp := gd.Viewport()
	g.position = framework.Vector2{
		X: float32(vp.Width) * 0.5,
		Y: float32(vp.Height) * 0.5,
	}
	
	g.reportRendererCapabilities()
	return nil
}

func (g *HelloGame) getRendererName() string {
	return "CNA (Go)"
}

func (g *HelloGame) supportsCapability(cap string) bool {
	switch cap {
	case "ThreeD", "DepthStencilBuffer":
		return true
	}
	return false
}

func (g *HelloGame) reportRendererCapabilities() {
	fmt.Printf("cna-go-template: renderer %s\n", g.rendererName)
	if g.supports3D {
		fmt.Println("  3D pipeline     : yes")
	} else {
		fmt.Println("  3D pipeline     : no (2D only)")
	}
	if g.supportsDepth {
		fmt.Println("  depth/stencil   : yes")
	} else {
		fmt.Println("  depth/stencil   : no")
	}
}

func (g *HelloGame) Update(gameTime framework.GameTime) error {
	dt := float32(gameTime.ElapsedSeconds)
	g.animationSeconds += dt
	
	if input.GetKeyboardState().IsKeyDown(input.KeyEscape) {
		g.Exit()
	}
	
	if !g.supports3D {
		movementDelta := dt * 2.0
		g.position.X += g.velocity.X * movementDelta
		g.position.Y += g.velocity.Y * movementDelta
		
		vp := g.Graphics.GraphicsDevice().Viewport()
		logoWidth := float32(g.logo.Width())
		logoHeight := float32(g.logo.Height())
		logoSize := logoWidth
		if logoHeight > logoSize {
			logoSize = logoHeight
		}
		
		minX, minY := logoSize*0.5, logoSize*0.5
		maxX := float32(vp.Width) - minX
		maxY := float32(vp.Height) - minY
		
		if g.position.X < minX {
			g.position.X = minX
			g.velocity.X = float32(math.Abs(float64(g.velocity.X)))
		} else if g.position.X > maxX {
			g.position.X = maxX
			g.velocity.X = -float32(math.Abs(float64(g.velocity.X)))
		}
		
		if g.position.Y < minY {
			g.position.Y = minY
			g.velocity.Y = float32(math.Abs(float64(g.velocity.Y)))
		} else if g.position.Y > maxY {
			g.position.Y = maxY
			g.velocity.Y = -float32(math.Abs(float64(g.velocity.Y)))
		}
	}
	
	return nil
}

func (g *HelloGame) Draw(gameTime framework.GameTime) error {
	gd := g.Graphics.GraphicsDevice()
	
	if g.supports3D && g.supportsDepth {
		gd.Clear(graphics.ClearOptionsTarget|graphics.ClearOptionsDepthBuffer, graphics.ColorCornflowerBlue, 1.0, 0)
	} else {
		gd.Clear(graphics.ClearOptionsTarget, graphics.ColorCornflowerBlue, 1.0, 0)
	}
	
	if g.supports3D {
		g.draw3DLogoCube()
	} else {
		g.draw2DLogo()
	}
	
	if g.animationSeconds < RendererBannerSeconds {
		g.drawRendererBanner()
	}
	
	if g.smokeTest {
		g.drawnFrames++
		if g.drawnFrames >= 3 {
			fmt.Printf("cna-go-template: smoke test drew %d frames; exiting\n", g.drawnFrames)
			g.Exit()
		}
	}
	
	return nil
}

func (g *HelloGame) UnloadContent() error {
	return nil
}

func (g *HelloGame) Exit() {
	// Native exit placeholder
}

func (g *HelloGame) draw2DLogo() {
	motionSeconds := g.animationSeconds * AnimationSpeed
	scale := 0.96 + 0.12*float32(math.Sin(float64(motionSeconds*0.65)))
	rotation := 0.11 * float32(math.Sin(float64(motionSeconds*0.55)))
	origin := framework.Vector2{X: float32(g.logo.Width()) * 0.5, Y: float32(g.logo.Height()) * 0.5}
	
	g.spriteBatch.Begin()
	g.spriteBatch.Draw(g.logo, g.position, nil, graphics.ColorWhite, rotation, origin, scale, graphics.SpriteEffectsNone, 0)
	g.spriteBatch.End()
}

func (g *HelloGame) draw3DLogoCube() {
	gd := g.Graphics.GraphicsDevice()
	vp := gd.Viewport()
	aspectRatio := float32(vp.Width) / float32(vp.Height)
	
	motionSeconds := g.animationSeconds * AnimationSpeed
	scale := 0.88 + 0.10*float32(math.Sin(float64(motionSeconds*0.48)))
	moveX := 1.15 * float32(math.Sin(float64(motionSeconds*0.24)))
	moveY := 0.65 * float32(math.Sin(float64(motionSeconds*0.36)))
	
	world := framework.MatrixCreateScale(scale).
		Multiply(framework.MatrixCreateRotationY(motionSeconds * 0.55)).
		Multiply(framework.MatrixCreateRotationX(motionSeconds * 0.35)).
		Multiply(framework.MatrixCreateTranslation(moveX, moveY, 0))
	
	g.cubeEffect.SetWorld(world)
	g.cubeEffect.SetView(framework.MatrixCreateLookAt(framework.Vector3{Z: 6}, framework.Vector3Zero, framework.Vector3Up))
	g.cubeEffect.SetProjection(framework.MatrixCreatePerspectiveFieldOfView(0.7853982, aspectRatio, 0.1, 100))
	
	gd.SetBlendState(graphics.BlendStateOpaque)
	if g.supportsDepth {
		gd.SetDepthStencilState(graphics.DepthStencilStateDefault)
	} else {
		gd.SetDepthStencilState(graphics.DepthStencilStateNone)
	}
	gd.SetRasterizerState(graphics.RasterizerStateCullNone)
	
	vertices := getCubeVertices()
	for _, pass := range g.cubeEffect.CurrentTechnique().Passes() {
		pass.Apply()
		gd.DrawUserPrimitives(graphics.PrimitiveTypeTriangleList, vertices, 0, len(vertices)/3)
	}
}

func (g *HelloGame) drawRendererBanner() {
	vp := g.Graphics.GraphicsDevice().Viewport()
	
	glyphColumns := len(g.rendererName)*6 - 1
	if glyphColumns < 1 { glyphColumns = 1 }
	pixelSize := (vp.Width - 48) / glyphColumns
	if pixelSize < 1 { pixelSize = 1 }
	if pixelSize > 8 { pixelSize = 8 }
	
	textWidth := glyphColumns * pixelSize
	textHeight := 7 * pixelSize
	textX := (vp.Width - textWidth) / 2
	textY := vp.Height - textHeight - 24
	
	translucentWhite := graphics.NewColor(255, 255, 255, 180)
	padding := 8
	
	g.spriteBatch.Begin()
	g.spriteBatch.Draw(g.solid, framework.Rectangle{X: textX - padding, Y: textY - padding, Width: textWidth + padding*2, Height: textHeight + padding*2}, translucentWhite)
	
	for i, char := range strings.ToUpper(g.rendererName) {
		rows := getGlyphRows(char)
		charX := textX + i*6*pixelSize
		
		for row := 0; row < 7; row++ {
			rowData := rows[row]
			for col := 0; col < 5; col++ {
				if (rowData>>(4-col))&1 == 1 {
					g.spriteBatch.Draw(g.solid, framework.Rectangle{X: charX + col*pixelSize, Y: textY + row*pixelSize, Width: pixelSize, Height: pixelSize}, graphics.ColorBlack)
				}
			}
		}
	}
	g.spriteBatch.End()
}

func getCubeVertices() []graphics.VertexPositionTexture {
	vertices := []graphics.VertexPositionTexture{}
	
	addFace := func(tl, tr, br, bl framework.Vector3) {
		vertices = append(vertices, graphics.VertexPositionTexture{Position: tl, TextureCoordinate: framework.Vector2{X: 0, Y: 0}})
		vertices = append(vertices, graphics.VertexPositionTexture{Position: tr, TextureCoordinate: framework.Vector2{X: 1, Y: 0}})
		vertices = append(vertices, graphics.VertexPositionTexture{Position: br, TextureCoordinate: framework.Vector2{X: 1, Y: 1}})
		vertices = append(vertices, graphics.VertexPositionTexture{Position: tl, TextureCoordinate: framework.Vector2{X: 0, Y: 0}})
		vertices = append(vertices, graphics.VertexPositionTexture{Position: br, TextureCoordinate: framework.Vector2{X: 1, Y: 1}})
		vertices = append(vertices, graphics.VertexPositionTexture{Position: bl, TextureCoordinate: framework.Vector2{X: 0, Y: 1}})
	}
	
	// Front
	addFace(framework.Vector3{X: -1, Y: 1, Z: 1}, framework.Vector3{X: 1, Y: 1, Z: 1}, framework.Vector3{X: 1, Y: -1, Z: 1}, framework.Vector3{X: -1, Y: -1, Z: 1})
	// Back
	addFace(framework.Vector3{X: 1, Y: 1, Z: -1}, framework.Vector3{X: -1, Y: 1, Z: -1}, framework.Vector3{X: -1, Y: -1, Z: -1}, framework.Vector3{X: 1, Y: -1, Z: -1})
	// Right
	addFace(framework.Vector3{X: 1, Y: 1, Z: 1}, framework.Vector3{X: 1, Y: 1, Z: -1}, framework.Vector3{X: 1, Y: -1, Z: -1}, framework.Vector3{X: 1, Y: -1, Z: 1})
	// Left
	addFace(framework.Vector3{X: -1, Y: 1, Z: -1}, framework.Vector3{X: -1, Y: 1, Z: 1}, framework.Vector3{X: -1, Y: -1, Z: 1}, framework.Vector3{X: -1, Y: -1, Z: -1})
	// Top
	addFace(framework.Vector3{X: -1, Y: 1, Z: -1}, framework.Vector3{X: 1, Y: 1, Z: -1}, framework.Vector3{X: 1, Y: 1, Z: 1}, framework.Vector3{X: -1, Y: 1, Z: 1})
	// Bottom
	addFace(framework.Vector3{X: -1, Y: -1, Z: 1}, framework.Vector3{X: 1, Y: -1, Z: 1}, framework.Vector3{X: 1, Y: -1, Z: -1}, framework.Vector3{X: -1, Y: -1, Z: -1})
	
	return vertices
}

func getGlyphRows(c rune) []byte {
	switch c {
	case 'A': return []byte{0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11}
	case 'B': return []byte{0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e}
	case 'C': return []byte{0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e}
	case 'D': return []byte{0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e}
	case 'E': return []byte{0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f}
	case 'F': return []byte{0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10}
	case 'G': return []byte{0x0e, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0f}
	case 'H': return []byte{0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11}
	case 'I': return []byte{0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e}
	case 'J': return []byte{0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0c}
	case 'K': return []byte{0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}
	case 'L': return []byte{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f}
	case 'M': return []byte{0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11}
	case 'N': return []byte{0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}
	case 'O': return []byte{0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e}
	case 'P': return []byte{0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10}
	case 'Q': return []byte{0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d}
	case 'R': return []byte{0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11}
	case 'S': return []byte{0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e}
	case 'T': return []byte{0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}
	case 'U': return []byte{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e}
	case 'V': return []byte{0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04}
	case 'W': return []byte{0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11}
	case 'X': return []byte{0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11}
	case 'Y': return []byte{0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04}
	case 'Z': return []byte{0x1f, 0x02, 0x04, 0x08, 0x10, 0x10, 0x1f}
	case '(': return []byte{0x04, 0x08, 0x08, 0x08, 0x08, 0x08, 0x04}
	case ')': return []byte{0x08, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08}
	case ' ': return []byte{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	default: return []byte{0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f}
	}
}
