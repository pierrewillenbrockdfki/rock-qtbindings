#!/usr/bin/env ruby

require 'Qt5'
require './analogclock.rb'

app = Qt5::Application.new(ARGV)
clock = AnalogClock.new
clock.show
app.exec
