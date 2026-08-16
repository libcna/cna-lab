require_relative 'game/HelloGame'
require 'optparse'

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: main.rb [options]"
  opts.on("--smoke-test", "Run a short smoke test") do |s|
    options[:smoke_test] = s
  end
end.parse!

begin
  game = HelloGame.new(options[:smoke_test])
  game.Run
ensure
  game.Exit if game
end
