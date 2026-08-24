# frozen_string_literal: true

require "cna"

class HelloGame < Microsoft::Xna::Framework::Game
  F = Microsoft::Xna::Framework
  G = Microsoft::Xna::Framework::Graphics
  I = Microsoft::Xna::Framework::Input

  attr_reader :successful_updates, :successful_draws

  def initialize(frame_limit: nil)
    super()
    @graphics = F::GraphicsDeviceManager.new(self)
    @frame_limit = frame_limit
    @successful_updates = 0
    @successful_draws = 0
    @animation_seconds = 0.0
    @position = F::Vector2.Zero
    @velocity = F::Vector2.new(104.0, 74.0)
  end

  protected

  def LoadContent
    asset = File.expand_path("../Content/logo.png", __dir__)
    File.open(asset, "rb") { |stream| @logo = G::Texture2D.FromStream(self.GraphicsDevice, stream) }
    @sprite_batch = G::SpriteBatch.new(self.GraphicsDevice)
    viewport = self.GraphicsDevice.Viewport
    @position = F::Vector2.new(viewport.Width / 2.0, viewport.Height / 2.0)
  end

  def Update(game_time)
    elapsed = game_time.ElapsedGameTime
    @animation_seconds += elapsed
    keyboard = I::Keyboard.GetState
    self.Exit if keyboard.IsKeyDown(I::Keys::Escape)

    @position.X += @velocity.X * elapsed
    @position.Y += @velocity.Y * elapsed
    viewport = self.GraphicsDevice.Viewport
    half_width = @logo.Width / 2.0
    half_height = @logo.Height / 2.0

    if @position.X < half_width
      @position.X = half_width
      @velocity.X = @velocity.X.abs
    elsif @position.X > viewport.Width - half_width
      @position.X = viewport.Width - half_width
      @velocity.X = -@velocity.X.abs
    end
    if @position.Y < half_height
      @position.Y = half_height
      @velocity.Y = @velocity.Y.abs
    elsif @position.Y > viewport.Height - half_height
      @position.Y = viewport.Height - half_height
      @velocity.Y = -@velocity.Y.abs
    end
    @successful_updates += 1
  end

  def Draw(_game_time)
    self.GraphicsDevice.Clear(F::Color.CornflowerBlue)
    rotation = 0.25 * Math.sin(@animation_seconds * 1.1)
    uniform_scale = 0.85 + 0.15 * Math.sin(@animation_seconds * 0.8)
    origin = F::Vector2.new(@logo.Width / 2.0, @logo.Height / 2.0)

    @sprite_batch.Begin
    begin
      @sprite_batch.Draw(
        @logo, @position, nil, F::Color.White, rotation, origin,
        F::Vector2.new(uniform_scale), G::SpriteEffects::None, 0.0
      )
    ensure
      @sprite_batch.End
    end
    @successful_draws += 1
    self.Exit if @frame_limit && @successful_draws >= @frame_limit
  end

  def UnloadContent
    @sprite_batch&.Dispose
    @logo&.Dispose
    @sprite_batch = nil
    @logo = nil
  end
end
