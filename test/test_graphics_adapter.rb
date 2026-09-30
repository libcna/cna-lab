# frozen_string_literal: true

require "minitest/autorun"
require "json"
require "pathname"
require_relative "reviewed_measurements"
require_relative "renderer_environment"
require_relative "../lib/cna"

# `Graphics.GraphicsAdapter`, `GraphicsDeviceInformation`, `PreparingDeviceSettingsEventArgs`, and the
# eight `GraphicsDevice`/`GraphicsDeviceManager` members they unblocked. All of them waited on one
# upstream defect -- the adapter list was built before the display existed
# (docs/graphics-adapter-ordering-upstream-defect.md) -- which the ABI 0.35.0 requalification found
# fixed. The live tests assert what the loaded artifact's own window says, so they hold on HEADLESS
# (no display: the fallback) and on a windowed renderer (the real display) alike.
class GraphicsAdapterTest < Minitest::Test
  F = Microsoft::Xna::Framework
  G = Microsoft::Xna::Framework::Graphics
  ROOT = Pathname(__dir__).join("..").expand_path
  STRICT = JSON.parse(ROOT.join("docs", "generated", "api-compat-report.json").read).freeze

  class ProbeGame < F::Game
    attr_reader :result, :prepared, :manager

    def initialize(profile = G::GraphicsProfile::Reach, &body)
      @body = body
      @prepared = []
      super()
      @manager = F::GraphicsDeviceManager.new(self)
      @manager.GraphicsProfile = profile
    end

    def Draw(_time)
      @result = @body.call(self)
    ensure
      self.Exit
    end
  end

  def with_game(profile = G::GraphicsProfile::Reach, setup: nil, &body)
    skip "CNA_NATIVE_LIBRARY not supplied" unless ENV["CNA_NATIVE_LIBRARY"]

    game = ProbeGame.new(profile, &body)
    setup&.call(game)
    begin
      game.Run
      game
    ensure
      game.Dispose
    end
  end

  def error_of
    yield
    :ok
  rescue StandardError => error
    [error.class, error.message]
  end

  # ------------------------------------------------------------------------- the contract

  def test_all_three_types_are_complete_and_nothing_is_missing
    %w[Microsoft.Xna.Framework.Graphics.GraphicsAdapter Microsoft.Xna.Framework.GraphicsDeviceInformation
       Microsoft.Xna.Framework.PreparingDeviceSettingsEventArgs Microsoft.Xna.Framework.Graphics.GraphicsDevice
       Microsoft.Xna.Framework.GraphicsDeviceManager].each do |name|
      assert_includes STRICT.fetch("completeTypeNames"), name
    end
    assert_equal ReviewedScoreboard::COMPLETE_TYPES, STRICT.fetch("COMPLETE_TYPES")
    assert_equal 0, STRICT.fetch("TOTAL_DIAGNOSTICS")
  end

  def test_the_adapter_has_no_public_constructor
    assert_raises(NoMethodError) { G::GraphicsAdapter.new(0) }
  end

  # ------------------------------------------------------------------------- live answers

  def test_the_adapter_describes_the_display_the_window_is_on
    values = with_game do |game|
      adapters = G::GraphicsAdapter.Adapters
      adapter = adapters[0]
      mode = adapter.CurrentDisplayMode
      modes = adapter.SupportedDisplayModes.GetEnumerator.to_a
      { count: adapters.Count, same_collection: adapters.equal?(G::GraphicsAdapter.Adapters),
        default: adapter.equal?(G::GraphicsAdapter.DefaultAdapter), device: game.GraphicsDevice.Adapter.equal?(adapter),
        is_default: adapter.IsDefaultAdapter, description: adapter.Description, name: adapter.DeviceName,
        window: game.Window.ScreenDeviceName, mode: [mode.Width, mode.Height, mode.Format.to_s],
        cached: mode.equal?(adapter.CurrentDisplayMode), modes: modes.length,
        includes_current: modes.any? { |each| each.Width == mode.Width && each.Height == mode.Height },
        wide: adapter.IsWideScreen, aspect: mode.AspectRatio,
        reach: adapter.IsProfileSupported(G::GraphicsProfile::Reach),
        monitor: error_of { adapter.MonitorHandle } }
    end.result
    assert_operator values.fetch(:count), :>=, 1
    assert values.fetch(:same_collection), "one collection for the process"
    assert values.fetch(:default)
    assert values.fetch(:device), "the game's device is on the default adapter"
    assert values.fetch(:is_default)
    refute_empty values.fetch(:description)
    assert values.fetch(:cached), "CurrentDisplayMode is built once and cached"
    assert values.fetch(:includes_current)
    assert_equal values.fetch(:aspect) > 1.600000023841858, values.fetch(:wide), "IsWideScreen is XNA's 1.6f test"
    assert values.fetch(:reach)
    assert_equal CNA::CapabilityError, values.fetch(:monitor).first, "no native monitor handle at the C boundary"
    if RendererEnvironment.measured? && RendererEnvironment.windowed?
      # The fixed defect: the adapter is the window's display, not the invented 800x480 fallback.
      assert_equal values.fetch(:window), values.fetch(:description)
      refute_equal [800, 480], values.fetch(:mode).first(2)
      assert_operator values.fetch(:modes), :>, 1
    else
      # A windowless renderer answers whatever the platform sees: the real display when one is
      # reachable, CNA's "Default Display" fallback when none is. Either way the mode is real-sized.
      assert_operator values.fetch(:mode)[0], :>, 0
      assert_operator values.fetch(:mode)[1], :>, 0
    end
  end

  def test_format_queries_answer_the_multiple_out_tuple
    values = with_game do |_game|
      adapter = G::GraphicsAdapter.DefaultAdapter
      [adapter.QueryBackBufferFormat(G::GraphicsProfile::Reach, G::SurfaceFormat::Color, G::DepthFormat::Depth24, 0),
       adapter.QueryRenderTargetFormat(G::GraphicsProfile::Reach, G::SurfaceFormat::Color, G::DepthFormat::None, 0)]
    end.result
    values.each do |supported, format, depth, samples|
      assert_includes [true, false], supported
      assert_instance_of G::SurfaceFormat, format
      assert_instance_of G::DepthFormat, depth
      assert_kind_of Integer, samples
    end
    assert_equal G::SurfaceFormat::Color, values[1][1]
  end

  def test_the_device_display_mode_is_one_object_updated_in_place
    values = with_game do |game|
      device = game.GraphicsDevice
      first = device.DisplayMode
      [first.equal?(device.DisplayMode), [first.Width, first.Height],
       [device.Adapter.CurrentDisplayMode.Width, device.Adapter.CurrentDisplayMode.Height]]
    end.result
    assert values[0]
    assert_equal values[2], values[1], "the device's mode is its adapter's current mode"
  end

  # CNA's adapter routes are device-scoped, so a read outside a lifecycle callback refuses rather
  # than inventing an answer. `DefaultAdapter` needs no read and answers anywhere.
  def test_reads_outside_a_callback_refuse_and_the_default_adapter_answers_anywhere
    skip "CNA_NATIVE_LIBRARY not supplied" unless ENV["CNA_NATIVE_LIBRARY"]

    assert_instance_of G::GraphicsAdapter, G::GraphicsAdapter.DefaultAdapter
    assert G::GraphicsAdapter.DefaultAdapter.IsDefaultAdapter
    # `IsProfileSupported` is a live question on every call (the identity and the modes are cached,
    # as XNA's are, so they answer once read).
    assert_raises(CNA::InvalidBindingStateError) do
      G::GraphicsAdapter.DefaultAdapter.IsProfileSupported(G::GraphicsProfile::Reach)
    end
  end

  def test_the_two_device_flags_are_stored_statics
    before = [G::GraphicsAdapter.UseNullDevice, G::GraphicsAdapter.UseReferenceDevice]
    G::GraphicsAdapter.UseNullDevice = true
    assert G::GraphicsAdapter.UseNullDevice
    assert_raises(TypeError) { G::GraphicsAdapter.UseReferenceDevice = 1 }
  ensure
    G::GraphicsAdapter.UseNullDevice = before[0] if before
  end

  # ------------------------------------------------------------------ GraphicsDeviceInformation

  def test_device_information_follows_the_il
    skip "CNA_NATIVE_LIBRARY not supplied" unless ENV["CNA_NATIVE_LIBRARY"]

    info = F::GraphicsDeviceInformation.new
    assert info.Adapter.equal?(G::GraphicsAdapter.DefaultAdapter), "the field initializer is DefaultAdapter"
    assert_equal G::GraphicsProfile::Reach, info.GraphicsProfile
    assert_instance_of G::PresentationParameters, info.PresentationParameters
    assert info.PresentationParameters.IsFullScreen, "a fresh PresentationParameters is full screen"

    clone = info.Clone
    refute clone.equal?(info)
    refute clone.PresentationParameters.equal?(info.PresentationParameters), "the parameters are copied"
    assert clone.Adapter.equal?(info.Adapter), "and the adapter shared"
    assert info.Equals(clone)
    assert_equal info.GetHashCode, clone.GetHashCode
    clone.PresentationParameters.BackBufferWidth = 1234
    refute info.Equals(clone)
    clone.GraphicsProfile = G::GraphicsProfile::HiDef
    refute_equal info.GraphicsProfile, clone.GraphicsProfile
    refute info.Equals(Object.new)

    # The setter tests the field, not the value: a null is stored while an adapter is set, and
    # then nothing can be.
    info.Adapter = nil
    assert_nil info.Adapter
    assert_raises(ArgumentError) { info.Adapter = G::GraphicsAdapter.DefaultAdapter }
    assert_raises(TypeError) { F::GraphicsDeviceInformation.new.Adapter = Object.new }

    args = F::PreparingDeviceSettingsEventArgs.new(clone)
    assert args.GraphicsDeviceInformation.equal?(clone)
    assert_kind_of CNA::Runtime::EventArgs, args
  end

  # ------------------------------------------------------------------ PreparingDeviceSettings

  # CNA raises it while preparing the device, before anything is created, and what the handler
  # writes is what the device gets.
  def test_preparing_device_settings_is_raised_and_its_changes_are_applied
    game = with_game(setup: lambda do |g|
      g.manager.PreparingDeviceSettings.add(lambda do |sender, args|
        info = args.GraphicsDeviceInformation
        g.prepared << [sender.equal?(g.manager), info.GraphicsProfile.to_s,
                       info.PresentationParameters.BackBufferWidth]
        info.PresentationParameters.RenderTargetUsage = G::RenderTargetUsage::PreserveContents
      end)
    end) { |g| g.GraphicsDevice.PresentationParameters.RenderTargetUsage }
    refute_empty game.prepared
    assert(game.prepared.all? { |sender, profile, _width| sender && profile == "Reach" })
    assert_equal G::RenderTargetUsage::PreserveContents, game.result
  end

  def test_a_handler_exception_surfaces_from_run
    skip "CNA_NATIVE_LIBRARY not supplied" unless ENV["CNA_NATIVE_LIBRARY"]

    game = ProbeGame.new { |_g| nil }
    game.manager.PreparingDeviceSettings.add(->(_s, _a) { raise ArgumentError, "planted" })
    begin
      error = assert_raises(ArgumentError) { game.Run }
      assert_equal "planted", error.message
    ensure
      game.Dispose
    end
  end

  # ------------------------------------------------------------------ choosing a device

  def test_find_best_device_ranks_the_real_adapters_as_xna_does
    values = with_game do |game|
      manager = game.manager
      best = manager.__send__(:FindBestDevice, true)
      found = [best.Clone, best.Clone]
      found[1].PresentationParameters.BackBufferFormat = G::SurfaceFormat::Bgr565
      manager.__send__(:RankDevices, found)
      [best.Adapter.equal?(G::GraphicsAdapter.DefaultAdapter), best.GraphicsProfile.to_s,
       [best.PresentationParameters.BackBufferWidth, best.PresentationParameters.BackBufferHeight],
       found.map { |info| info.PresentationParameters.BackBufferFormat.to_s },
       manager.__send__(:CanResetDevice, best),
       manager.__send__(:CanResetDevice, best.Clone.tap { |i| i.GraphicsProfile = G::GraphicsProfile::HiDef })]
    end.result
    assert values[0]
    assert_equal "Reach", values[1]
    assert_equal [800, 480], values[2], "windowed: the preferred back-buffer size"
    assert_equal %w[Color Bgr565], values[3], "the preferred format ranks first"
    assert_equal [true, false], values[4..5], "CanResetDevice compares the profile and nothing else"
  end

  def test_the_four_choosers_are_protected
    %i[FindBestDevice CanResetDevice RankDevices OnPreparingDeviceSettings].each do |name|
      assert F::GraphicsDeviceManager.protected_method_defined?(name), name.to_s
    end
    assert F::GraphicsDeviceManager.public_method_defined?(:PreparingDeviceSettings)
  end

  # ------------------------------------------------------------------ the public constructor

  # `cna_graphics_device_create`: an independent device the caller owns, with resources of its own.
  def test_the_public_constructor_makes_an_independent_device
    skip "CNA_NATIVE_LIBRARY not supplied" unless ENV["CNA_NATIVE_LIBRARY"]

    parameters = G::PresentationParameters.new
    parameters.IsFullScreen = false
    parameters.BackBufferWidth = 64
    parameters.BackBufferHeight = 32
    assert_equal [ArgumentError, "presentationParameters"],
                 error_of { G::GraphicsDevice.new(G::GraphicsAdapter.DefaultAdapter, G::GraphicsProfile::Reach, nil) }
    assert_equal [ArgumentError, "adapter"],
                 error_of { G::GraphicsDevice.new(nil, G::GraphicsProfile::Reach, parameters) }

    device = G::GraphicsDevice.new(G::GraphicsAdapter.DefaultAdapter, G::GraphicsProfile::HiDef, parameters)
    begin
      assert_equal G::GraphicsProfile::HiDef, device.GraphicsProfile
      assert_equal [64, 32], [device.PresentationParameters.BackBufferWidth, device.PresentationParameters.BackBufferHeight]
      assert device.Adapter.equal?(G::GraphicsAdapter.DefaultAdapter)
      texture = G::Texture2D.new(device, 2, 2)
      texture.SetData(F::Color, [F::Color.new(1, 2, 3, 4)] * 4)
      refute texture.IsDisposed
      device.Dispose
      assert device.IsDisposed
      assert texture.IsDisposed, "the device's resources are released with it"
    ensure
      device&.Dispose
    end
  end
end
