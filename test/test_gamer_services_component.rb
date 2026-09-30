# frozen_string_literal: true

require "minitest/autorun"
require "json"
require "open3"
require "pathname"
require_relative "reviewed_measurements"
require_relative "../lib/cna"
require_relative "../tools/api_compat/verifier"

# `GamerServices.GamerServicesComponent` — the first type of a new namespace, and the first XNA type
# in this binding that **inherits** its events rather than declaring them.
class GamerServicesComponentTest < Minitest::Test
  F = Microsoft::Xna::Framework
  GS = Microsoft::Xna::Framework::GamerServices
  ROOT = Pathname(__dir__).join("..").expand_path
  REFERENCE_RAW = JSON.parse(
    ROOT.join("tools", "api_compat", "reference", "xna40-windows-runtime-contract.json").read
  ).freeze
  REFERENCE = REFERENCE_RAW.fetch("types").to_h { |type| [type.fetch("name"), type] }.freeze
  STRICT = JSON.parse(ROOT.join("docs", "generated", "api-compat-report.json").read).freeze
  NAME = "Microsoft.Xna.Framework.GamerServices.GamerServicesComponent"

  # ------------------------------------------------------------------- the contract, from metadata

  def test_it_is_a_game_component_with_three_identities_and_no_state
    contract = REFERENCE.fetch(NAME)
    assert_equal "Microsoft.Xna.Framework.GameComponent", contract.fetch("baseType")
    assert_equal 3, contract.fetch("members").length
    assert_equal [%w[constructor .ctor], %w[method Initialize], %w[method Update]],
                 contract.fetch("members").map { |m| [m.fetch("kind"), m.fetch("name")] }
    # The private InstallingTitleUpdate handler is not an identity and is not projected as one.
    assert_equal %i[Initialize Update], GS::GamerServicesComponent.public_instance_methods(false).sort
    assert GS::GamerServicesComponent < F::GameComponent
  end

  def test_the_scoreboard_records_it_complete
    assert_equal ReviewedScoreboard::COMPLETE_TYPES, STRICT.fetch("COMPLETE_TYPES")
    assert_equal ReviewedScoreboard::PARTIAL_TYPES, STRICT.fetch("PARTIAL_TYPES")
    assert_includes STRICT.fetch("completeTypeNames"), NAME
    assert_equal 0, STRICT.fetch("EVENT_MAPPING_MISMATCH")
  end

  # ---------------------------------------------------------------- the verifier change it forced

  # `xna_event_identities` walks the whole ancestor chain, because a subscriber does not care where
  # a reader was declared. So the first type to inherit events would have been reported as
  # registering three identities its own contract does not select. The verifier now subtracts what
  # the **CLR base chain** declares, read from the reference rather than from Ruby's ancestors.
  def test_inherited_events_are_not_this_types_identities
    contract = REFERENCE.fetch(NAME)
    assert_empty contract.fetch("members").select { |m| m.fetch("kind") == "event" },
                 "it declares no event of its own"
    assert_equal %i[EnabledChanged UpdateOrderChanged Disposed].sort,
                 GS::GamerServicesComponent.xna_event_identities.sort,
                 "and inherits exactly GameComponent's three"

    verifier = CNAApiCompat::Verifier.new(reference: REFERENCE_RAW, target: { "types" => [] })
    assert_equal %i[EnabledChanged UpdateOrderChanged Disposed].sort,
                 verifier.send(:inherited_event_identities, NAME).sort
    assert_empty verifier.send(:inherited_event_identities, "Microsoft.Xna.Framework.GameComponent"),
                 "GameComponent declares its three rather than inheriting them"
  end

  # The control for that relaxation: an event the base chain does **not** declare is still caught,
  # so the change admits inheritance and nothing else.
  def test_an_invented_event_on_a_subclass_is_still_caught
    reference = Marshal.load(Marshal.dump(REFERENCE_RAW))
    reference["types"] = reference["types"].select do |type|
      [NAME, "Microsoft.Xna.Framework.GameComponent"].include?(type.fetch("name"))
    end
    verifier = CNAApiCompat::Verifier.new(reference: reference, target: { "types" => [] })
    inherited = verifier.send(:inherited_event_identities, NAME)
    refute_includes inherited, :Invented
    assert_includes inherited, :Disposed
  end

  # ---------------------------------------------------------------------------- live behaviour

  # `GamerServicesDispatcher` is a process-global static that XNA initializes once per process:
  # its `Initialize` opens with `if (IsInitialized) throw new
  # InvalidOperationException(GamerServicesAlreadyInitialized)`, and CNA (ABI 0.35.0) refuses a
  # second initialization the same way. So every live scenario runs in a fresh child process, which
  # is exactly one process lifetime as XNA knows it.
  CHILD_PREAMBLE = <<~'CHILD'
    require "cna"
    require "json"
    F = Microsoft::Xna::Framework
    GS = Microsoft::Xna::Framework::GamerServices

    class HostGame < F::Game
      attr_reader :component, :updates

      def initialize
        @updates = 0
        super
        @component = GS::GamerServicesComponent.new(self)
        self.Components.Add(@component)
      end

      def Update(gameTime)
        @updates += 1
        self.Exit if @updates >= 2
        super
      end
    end

    def with_game
      game = HostGame.new
      game.Run
      yield game
    ensure
      game&.Dispose
    end
  CHILD

  def measured(body)
    skip "CNA_NATIVE_LIBRARY not supplied" unless ENV["CNA_NATIVE_LIBRARY"]

    output, status = Open3.capture2e("ruby", "-I#{ROOT.join("lib")}", "-e", CHILD_PREAMBLE + body,
                                     chdir: ROOT.to_s)
    assert status.success?, "child failed:\n#{output}"
    JSON.parse(output[/^\{.*\}$/] || "{}")
  end

  # The dispatcher routes are process-global statics with no handle, which is what XNA's
  # `GamerServicesDispatcher` is too — the game-scoped asymmetry the audio and window families
  # record does not apply here.
  def test_the_dispatcher_routes_are_statics_and_only_initialize_takes_a_game
    handles = CNA::Native::Manifest::FUNCTIONS
              .select { |entry| entry.symbol.start_with?("cna_gamer_services_dispatcher_") }
              .to_h { |entry| [entry.symbol, entry.c_arguments] }
    assert_equal [], handles.fetch("cna_gamer_services_dispatcher_update")
    assert_equal ["CNA_Bool"], handles.fetch("cna_gamer_services_dispatcher_get_is_initialized")
    assert_equal ["uint64_t"], handles.fetch("cna_gamer_services_dispatcher_set_window_handle")
    assert_equal ["CNA_Handle"], handles.fetch("cna_gamer_services_dispatcher_initialize")
  end

  # `Initialize` runs through the component engine, and it really reaches the dispatcher: the
  # subscription is a real owned registration and the dispatcher reports itself initialized.
  def test_initialize_pushes_the_window_handle_and_registers_a_subscription
    result = measured(<<~'BODY')
      out = nil
      with_game do |game|
        flag = CNA::Native.library.pointer_for("C", 0)
        CNA::Native.library.call("cna_gamer_services_dispatcher_get_is_initialized", flag)
        out = { registration: game.component.instance_variable_get(:@registration),
                enabled: game.component.Enabled, same_game: game.component.Game.equal?(game),
                initialized: flag[0, 1].unpack1("C") == 1 }
      end
      puts JSON.generate(out)
    BODY
    refute_equal 0, result.fetch("registration"), "the title-update subscription is a real owned registration"
    assert result.fetch("enabled"), "and the component is an ordinary enabled GameComponent"
    assert result.fetch("same_game")
    assert result.fetch("initialized")
  end

  # `GamerServicesDispatcher.Update()` then `base.Update(gameTime)`. The component engine drives it,
  # so a running Game pumps the dispatcher without the consumer doing anything.
  def test_the_component_updates_with_the_game
    result = measured(<<~'BODY')
      out = nil
      with_game { |game| out = { updates: game.updates, listed: game.Components.include?(game.component) } }
      puts JSON.generate(out)
    BODY
    assert_operator result.fetch("updates"), :>=, 1
    assert result.fetch("listed")
  end

  # XNA subscribes to a static event and never unsubscribes. Here the subscription is a native
  # registration with an owner, so it is released on disposal -- through the type's own `Disposed`
  # event rather than by declaring a `Dispose` the contract does not have.
  def test_the_registration_is_released_on_disposal_without_declaring_dispose
    refute GS::GamerServicesComponent.public_instance_methods(false).include?(:Dispose)
    result = measured(<<~'BODY')
      out = nil
      with_game do |game|
        component = game.component
        before = component.instance_variable_get(:@registration)
        component.Dispose
        after = component.instance_variable_get(:@registration)
        component.Dispose # idempotent
        out = { before: before, after: after, again: component.instance_variable_get(:@registration) }
      end
      puts JSON.generate(out)
    BODY
    refute_equal 0, result.fetch("before")
    assert_equal 0, result.fetch("after")
    assert_equal 0, result.fetch("again")
  end

  # A second component in the same process is XNA's second `GamerServicesDispatcher.Initialize`,
  # which the IL refuses with `InvalidOperationException(GamerServicesAlreadyInitialized)` -- after
  # the window handle and the subscription, which come first in `GamerServicesComponent.Initialize`.
  def test_a_second_initialization_in_one_process_is_xnas_invalid_operation
    result = measured(<<~'BODY')
      with_game { |_game| nil }
      second = begin
        with_game { |_game| nil }
        "ok"
      rescue StandardError => error
        [error.class.name, error.message]
      end
      puts JSON.generate({ second: second })
    BODY
    assert_equal ["RuntimeError", "GamerServicesAlreadyInitialized"], result.fetch("second")
  end

  # ------------------------------------------------------------------- and exactly what it does not

  def test_it_opens_the_namespace_and_nothing_else_in_it
    assert_equal %i[GamerServicesComponent], GS.constants(false)
    %i[Guide Gamer SignedInGamer GamerProfile GamerServicesDispatcher
       NetworkSession StorageDevice].each do |absent|
      refute GS.const_defined?(absent, false), absent.to_s
    end
    # The canonical CNA component is deliberately unused: a second component pass is what the
    # GraphicsDeviceManager producer audit refused.
    refute_includes CNA::Native::Manifest::FUNCTIONS.map(&:symbol), "cna_gamer_services_component_create"
  end
end
