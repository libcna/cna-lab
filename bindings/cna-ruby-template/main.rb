# frozen_string_literal: true

require "optparse"
require_relative "game/HelloGame"

options = {frames: nil}
OptionParser.new do |parser|
  parser.banner = "Usage: ruby main.rb [--frames 60|600]"
  parser.on("--frames COUNT", Integer, "Run exactly 60 or 600 native Draw frames") { |count| options[:frames] = count }
end.parse!

abort "--frames must be exactly 60 or 600" if options[:frames] && ![60, 600].include?(options[:frames])

game = HelloGame.new(frame_limit: options[:frames])
begin
  game.Run
ensure
  game.Dispose
end

puts "CANARY_UPDATES=#{game.successful_updates}"
puts "CANARY_DRAWS=#{game.successful_draws}"
# `--frames` counts Draw calls. XNA's fixed time step runs as many Updates as it needs to catch up
# before each Draw, so on a real renderer whose Present waits for the display there are more
# Updates than Draws -- measured 84 for 60 on OPENGLES3 -- and never fewer.
if options[:frames] && (game.successful_draws != options[:frames] || game.successful_updates < game.successful_draws)
  abort "native canary frame count mismatch"
end
