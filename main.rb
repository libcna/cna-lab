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
if options[:frames] && (game.successful_updates != options[:frames] || game.successful_draws != options[:frames])
  abort "native canary frame count mismatch"
end
