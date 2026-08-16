require 'microsoft/xna/framework'
require 'microsoft/xna/framework/graphics'
require 'microsoft/xna/framework/input'

include Microsoft::Xna::Framework
include Microsoft::Xna::Framework::Graphics
include Microsoft::Xna::Framework::Input

class HelloGame < Game
  def initialize(smoke_test = false)
    super()
    @graphics = GraphicsDeviceManager.new(self)
    @smoke_test = smoke_test
    @drawn_frames = 0
    @animation_seconds = 0.0
    @renderer_banner_seconds = 5.0
    @velocity = Vector2.new(104.0, 74.0)
    @position = Vector2.Zero
  end

  def Initialize
    # Set preferred buffer size if needed
    super
  end

  def LoadContent
    @sprite_batch = SpriteBatch.new(GraphicsDevice)
    
    # In a real app: @logo = Content.Load("logo")
    @logo = nil # Placeholder
    @solid = nil # Placeholder
    
    @renderer_name = "CNA (Ruby)"
    @supports_3d = true # Assume 3D support
    
    if @supports_3d
      @cube_effect = BasicEffect.new(GraphicsDevice)
      @cube_effect.TextureEnabled = true
      @cube_effect.Texture = @logo
    end
    
    vp = GraphicsDevice.Viewport
    @position = Vector2.new(vp.Width / 2.0, vp.Height / 2.0)
    
    puts "cna-ruby-template: renderer #{@renderer_name}"
  end

  def Update(gameTime)
    dt = gameTime.ElapsedGameTime
    @animation_seconds += dt
    
    if Keyboard.GetState.IsKeyDown(Keys::Escape)
      Exit()
    end
    
    unless @supports_3d
      movement_delta = dt * 2.0
      @position.X += @velocity.X * movement_delta
      @position.Y += @velocity.Y * movement_delta
      
      vp = GraphicsDevice.Viewport
      logo_size = 256.0
      
      min_x, min_y = logo_size / 2.0, logo_size / 2.0
      max_x, max_y = vp.Width - min_x, vp.Height - min_y
      
      if @position.X < min_x
        @position.X = min_x
        @velocity.X = @velocity.X.abs
      elsif @position.X > max_x
        @position.X = max_x
        @velocity.X = -@velocity.X.abs
      end
      
      if @position.Y < min_y
        @position.Y = min_y
        @velocity.Y = @velocity.Y.abs
      elsif @position.Y > max_y
        @position.Y = max_y
        @velocity.Y = -@velocity.Y.abs
      end
    end
    
    super
  end

  def Draw(gameTime)
    GraphicsDevice.Clear(Color.CornflowerBlue)
    
    if @supports_3d
      draw_3d_cube
    else
      draw_2d_logo
    end
    
    if @animation_seconds < @renderer_banner_seconds
      draw_renderer_banner
    end
    
    if @smoke_test
      @drawn_frames += 1
      if @drawn_frames >= 3
        puts "cna-ruby-template: smoke test drew #{@drawn_frames} frames; exiting"
        Exit()
      end
    end
    
    super
  end

  private

  def draw_2d_logo
    motion = @animation_seconds * 2.0
    scale = 0.96 + 0.12 * Math.sin(motion * 0.65)
    rotation = 0.11 * Math.sin(motion * 0.55)
    origin = Vector2.new(128.0, 128.0)
    
    @sprite_batch.Begin
    @sprite_batch.Draw(@logo, @position, nil, Color.White, rotation, origin, scale)
    @sprite_batch.End
  end

  def draw_3d_cube
    vp = GraphicsDevice.Viewport
    aspect = vp.Width.to_f / vp.Height
    
    motion = @animation_seconds * 2.0
    scale = 0.88 + 0.10 * Math.sin(motion * 0.48)
    move_x = 1.15 * Math.sin(motion * 0.24)
    move_y = 0.65 * Math.sin(motion * 0.36)
    
    world = Matrix.CreateScale(scale)
    world = world * Matrix.CreateRotationY(motion * 0.55)
    world = world * Matrix.CreateRotationX(motion * 0.35)
    world = world * Matrix.CreateTranslation(move_x, move_y, 0.0)
    
    @cube_effect.World = world
    @cube_effect.View = Matrix.CreateLookAt(Vector3.new(0, 0, 6), Vector3.Zero, Vector3.Up)
    @cube_effect.Projection = Matrix.CreatePerspectiveFieldOfView(0.7853982, aspect, 0.1, 100.0)
    
    @cube_effect.Apply
    # Draw primitives...
  end

  def draw_renderer_banner
    vp = GraphicsDevice.Viewport
    name = @renderer_name.upcase
    
    glyph_cols = name.length * 6 - 1
    pixel_size = [1, [8, (vp.Width - 48) / [1, glyph_cols].max].min].max
    
    text_w = glyph_cols * pixel_size
    text_h = 7 * pixel_size
    text_x = (vp.Width - text_w) / 2
    text_y = vp.Height - text_h - 24
    
    @sprite_batch.Begin
    # Background
    bg_rect = [text_x - 8, text_y - 8, text_w + 16, text_h + 16]
    @sprite_batch.DrawRect(@solid, bg_rect, Color.new(255, 255, 255, 180))
    
    name.each_char.with_index do |char, i|
      rows = get_glyph_rows(char)
      char_x = text_x + i * 6 * pixel_size
      7.times do |row|
        row_data = rows[row]
        5.times do |col|
          if (row_data >> (4 - col)) & 1 == 1
            rect = [char_x + col * pixel_size, text_y + row * pixel_size, pixel_size, pixel_size]
            @sprite_batch.DrawRect(@solid, rect, Color.Black)
          end
        end
      end
    end
    @sprite_batch.End
  end

  def get_glyph_rows(char)
    font = {
      'A' => [0x04, 0x0A, 0x11, 0x11, 0x1F, 0x11, 0x11],
      'B' => [0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E],
      'C' => [0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E],
      'D' => [0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C],
      'E' => [0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F],
      'F' => [0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10],
      'G' => [0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F],
      'H' => [0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11],
      'I' => [0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E],
      'J' => [0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C],
      'K' => [0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11],
      'L' => [0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F],
      'M' => [0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11],
      'N' => [0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11],
      'O' => [0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E],
      'P' => [0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10],
      'Q' => [0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D],
      'R' => [0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11],
      'S' => [0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E],
      'T' => [0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04],
      'U' => [0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E],
      'V' => [0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04],
      'W' => [0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11],
      'X' => [0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11],
      'Y' => [0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04],
      'Z' => [0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F],
      ' ' => [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
      '(' => [0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02],
      ')' => [0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08],
      '-' => [0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00],
      '.' => [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04],
    }
    font[char] || Array.new(7, 0x1F)
  end
end
