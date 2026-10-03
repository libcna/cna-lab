# frozen_string_literal: true

module CNA
  module Runtime
    module Ownership
      OWNED = :owned
      BORROWED = :borrowed
      PARENT_OWNED = :parent_owned
      PROCESS_GLOBAL = :process_global
      MANAGED_VALUE = :managed_value
      BORROWED_EXTERNAL_SCALAR = :borrowed_external_scalar
      ALL = [OWNED, BORROWED, PARENT_OWNED, PROCESS_GLOBAL, MANAGED_VALUE,
             BORROWED_EXTERNAL_SCALAR].freeze
    end

    class NativeHandle
      attr_reader :ownership, :generation

      def initialize(handle:, ownership:, generation:, release: nil, parent: nil)
        raise ArgumentError, "invalid ownership category" unless Ownership::ALL.include?(ownership)
        raise ArgumentError, "native handle zero is invalid" if handle.zero?

        @handle = handle
        @ownership = ownership
        @generation = generation
        @release = release
        @parent = parent
        @disposed = false
        generation.claim_owned(handle, self) if ownership == Ownership::OWNED
      end

      def disposed? = @disposed

      def value
        raise CNA::DisposedObjectError, "native resource is disposed" if disposed?
        generation.assert_valid!
        raise CNA::DisposedObjectError, "native parent is disposed" if @parent&.__send__(:disposed?)

        @handle
      end

      def dispose
        return false if disposed?
        generation.assert_owner_thread!
        if ownership == Ownership::OWNED && @release
          # The handle stays intact if CNA refuses the destruction. A later
          # owner-thread retry therefore remains possible.
          @release.call(@handle)
        end
        generation.release_owned(@handle, self) if ownership == Ownership::OWNED
        @handle = 0
        @disposed = true
        true
      end
    end

    # The owner of a device made by `GraphicsDevice`'s public constructor. A game's device and its
    # resources belong to the game; a caller-made device (`cna_graphics_device_create`) belongs to
    # itself -- CNA documents that its resources "belong to this device, not to a game, and are
    # released with it" -- so this plays the game's part for them: one generation, one owner
    # thread, and the children released before the device.
    class StandaloneDeviceOwner
      attr_reader :generation, :owner_thread

      def initialize
        @generation = Generation.new
        @owner_thread = Thread.current
        @children = []
        @disposed = false
      end

      def disposed? = @disposed

      def assert_owner_thread! = generation.assert_owner_thread!

      def register_native_child(child)
        @children << child unless @children.any? { |value| value.equal?(child) }
      end

      def unregister_native_child(child)
        @children.reject! { |value| value.equal?(child) }
      end

      # A caller-made device raises its events from inside the Ruby call that caused them, so there
      # is no later lifecycle boundary to defer an exception to.
      def record_callback_exception(exception) = raise(exception)

      def release_children
        @children.reverse_each do |child|
          child.Dispose
        rescue CNA::Error
          nil
        end
        @children.clear
      end

      def invalidate!
        @disposed = true
        generation.invalidate!
      end
    end

    module NativeResource
      def initialize_native_resource(game, handle, release)
        @native_game = game
        @native_handle = NativeHandle.new(
          handle: handle,
          ownership: Ownership::OWNED,
          generation: game.__send__(:generation),
          release: release,
          parent: game
        )
        game.__send__(:register_native_child, self)
      end

      # A handle another native object owns and this wrapper only reads.
      #
      # `Model` is why it exists: a content-loaded model owns its parts' `Effect`, `VertexBuffer`
      # and `IndexBuffer`, and CNA refuses `cna_*_destroy` on each of the three with
      # `CNA_RESULT_INVALID_STATE` — measured, and documented by the model header. So the wrapper
      # takes `PARENT_OWNED`: it never claims the handle in the generation, never releases it, and
      # is not registered as an owned child, which is exactly what the ownership already means.
      def initialize_parent_owned_resource(game, handle)
        @native_game = game
        @native_handle = NativeHandle.new(
          handle: handle,
          ownership: Ownership::PARENT_OWNED,
          generation: game.__send__(:generation),
          release: nil,
          parent: game
        )
      end

      def IsDisposed = @native_handle.nil? || @native_handle.disposed?

      def Dispose
        return if self.IsDisposed

        prepare_native_dispose
        parent_owned = @native_handle.ownership == Ownership::PARENT_OWNED
        @native_handle.dispose
        @native_game.__send__(:unregister_native_child, self) unless parent_owned
        nil
      end

      private

      def prepare_native_dispose = nil
      def native_handle = @native_handle.value
      def native_game = @native_game
    end
  end
end
