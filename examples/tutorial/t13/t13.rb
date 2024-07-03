#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt5'
require_relative 'gamebrd.rb'

app = Qt5::Application.new(ARGV)
gb = GameBoard.new
gb.setGeometry(100, 100, 500, 355)
gb.show
app.exec
